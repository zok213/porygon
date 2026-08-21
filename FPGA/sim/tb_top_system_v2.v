`timescale 1ns/1ps

module tb_top_system_v2();

    reg  clk_in;
    reg  rst_n_in;
    reg  btn1_in;
    reg  btn2_in;
    wire led_out;
    wire uart_tx;

    integer total_checks = 0;
    integer total_errors = 0;

    real BIT_NS = 1.0e9 / 115200.0; // ~8680.56 ns/bit theo baud 115200

    top_system uut (
        .clk_in   (clk_in),
        .rst_n_in (rst_n_in),
        .btn1_in  (btn1_in),
        .btn2_in  (btn2_in),
        .led_out  (led_out),
        .uart_tx  (uart_tx)
    );

    // KHONG override PWM_FREQ nua. Dung dung gia tri THAT (1000 Hz)
    // duoc top_system.v truyen xuong pwm_led_controller, de chu ky
    // tho AUTO chay dung 2.0s that, lam bang chung chinh xac cho bao cao.

    initial clk_in = 0;
    always #18.518 clk_in = ~clk_in; // 27MHz

    task press_btn1;
        begin
            $display("[%0t ns] Nhan Button 1", $time);
            btn1_in = 0;
            #30_000_000;
            btn1_in = 1;
            #5_000_000;
        end
    endtask

    task press_btn2;
        begin
            $display("[%0t ns] Nhan Button 2", $time);
            btn2_in = 0;
            #30_000_000;
            btn2_in = 1;
            #5_000_000;
        end
    endtask

    task trigger_reset;
        begin
            $display("[%0t ns] Reset", $time);
            rst_n_in = 0;
            #5_000_000;
            rst_n_in = 1;
            #15_000_000;
        end
    endtask

    // Mo phong nut nhieu tiep xuc truoc khi on dinh
    task glitchy_press_btn1;
        begin
            $display("[%0t ns] Nhan B1 (bounce)", $time);
            btn1_in = 0; #1_000_000;
            btn1_in = 1; #700_000;
            btn1_in = 0; #900_000;
            btn1_in = 1; #500_000;
            btn1_in = 0; #1_200_000;
            btn1_in = 1; #600_000;
            btn1_in = 0;
            #25_000_000;
            btn1_in = 1;
            #5_000_000;
        end
    endtask

    task short_press_btn1;
        begin
            $display("[%0t ns] Nhan B1 ngan (5ms)", $time);
            btn1_in = 0;
            #5_000_000;
            btn1_in = 1;
            #2_000_000;
        end
    endtask

    // Doc 1 byte UART qua uart_tx, mid-bit sampling theo baud danh nghia
    task automatic uart_recv_byte(output [7:0] data, output real t_edge);
        integer i;
        begin
            @(negedge uart_tx);
            t_edge = $realtime;
            #(BIT_NS/2.0);
            for (i = 0; i < 8; i = i + 1) begin
                #(BIT_NS);
                data[i] = uart_tx;
            end
            #(BIT_NS);
        end
    endtask

    task automatic uart_check_message(input [8*16-1:0] expected_str, input integer len);
        integer i, errors_local;
        reg  [7:0] rx_byte, exp_byte;
        real t_first, t_last, t_tmp, avg_bit_ns, baud_actual, baud_err_pct;
        begin
            errors_local = 0;
            for (i = 0; i < len; i = i + 1) begin
                uart_recv_byte(rx_byte, t_tmp);
                if (i == 0) t_first = t_tmp;
                t_last = t_tmp;
                exp_byte = expected_str[8*(len-1-i) +: 8];
                total_checks = total_checks + 1;
                if (rx_byte !== exp_byte) begin
                    errors_local = errors_local + 1;
                    total_errors = total_errors + 1;
                    $display("  byte %0d: ky vong 0x%02h thuc te 0x%02h -> SAI", i, exp_byte, rx_byte);
                end
            end
            $display("  %0d byte, %0d loi", len, errors_local);
            if (len > 1) begin
                avg_bit_ns  = (t_last - t_first) / (10.0 * (len - 1));
                baud_actual = 1.0e9 / avg_bit_ns;
                baud_err_pct = (baud_actual - 115200.0) / 115200.0 * 100.0;
                $display("  bit period ~%.3f ns, baud ~%.1f bps, sai so %.3f%%", avg_bit_ns, baud_actual, baud_err_pct);
            end
        end
    endtask

    task automatic measure_pwm_duty(input [8*8-1:0] mode_label, input integer window_ns, input integer step_ns);
        integer n_samples, n_high, i;
        real duty_pct;
        begin
            n_samples = window_ns / step_ns;
            n_high = 0;
            for (i = 0; i < n_samples; i = i + 1) begin
                #(step_ns);
                if (led_out) n_high = n_high + 1;
            end
            duty_pct = (n_high * 100.0) / n_samples;
            $display("[%0t ns] duty %0s = %.2f%%", $time, mode_label, duty_pct);
        end
    endtask

    task automatic check_no_uart_within(input integer window_ns);
        reg detected;
        begin
            detected = 0;
            fork
                begin : WAIT_EDGE
                    @(negedge uart_tx);
                    detected = 1;
                    disable TIMEOUT_BLK;
                end
                begin : TIMEOUT_BLK
                    #(window_ns);
                    disable WAIT_EDGE;
                end
            join
            total_checks = total_checks + 1;
            if (detected) begin
                total_errors = total_errors + 1;
                $display("[%0t ns] loi: van co UART phat du nhan ngan", $time);
            end
        end
    endtask

    // Chung minh chu ky tho AUTO dung 2.0s that bang canh breath_dir:
    // breath_dir dao trang thai dung 2 lan moi chu ky (0->1 tai dinh,
    // 1->0 tai day). Khoang cach giua 2 lan posedge lien tiep = dung
    // 1 chu ky tho day du (1.0s pha giam + 1.0s pha tang).
    task automatic prove_breath_2s_via_transcript(input integer n_cycles);
        real t_top, t_bottom, t_top_next;
        real rise_s, fall_s, period_s;
        integer i;
        begin
            $display("========================================");
            $display("CHUNG MINH LED THO DUNG 2.0s (qua transcript)");
            $display("========================================");

            @(posedge uut.u_pwm.breath_dir); // moc dinh lan dau
            t_top = $realtime;

            for (i = 0; i < n_cycles; i = i + 1) begin
                @(negedge uut.u_pwm.breath_dir); // xuong day
                t_bottom = $realtime;
                @(posedge uut.u_pwm.breath_dir); // len dinh ke tiep
                t_top_next = $realtime;

                fall_s   = (t_bottom   - t_top)     / 1.0e9;
                rise_s   = (t_top_next - t_bottom)  / 1.0e9;
                period_s = (t_top_next - t_top)     / 1.0e9;

                $display("Chu ky %0d: pha giam=%.6fs, pha tang=%.6fs, TONG=%.6fs (ky vong 2.000000s)",
                          i + 1, fall_s, rise_s, period_s);

                total_checks = total_checks + 1;
                if (period_s < 1.9995 || period_s > 2.0005) begin
                    total_errors = total_errors + 1;
                    $display("  -> LOI: chu ky %0d lech qua nguong cho phep +-0.5ms", i + 1);
                end else begin
                    $display("  -> OK: chu ky %0d dat yeu cau 2.0s", i + 1);
                end

                t_top = t_top_next;
            end
            $display("========================================");
        end
    endtask

    initial begin
        rst_n_in = 1; btn1_in = 1; btn2_in = 1;
        #1_000_000;

        // Reset -> LOW
        fork
            trigger_reset();
            uart_check_message({"MODE: LOW", 8'h0D, 8'h0A}, 11);
        join
        measure_pwm_duty("LOW", 20_000_000, 200);

        // LOW -> HIGH
        fork
            press_btn1();
            uart_check_message({"MODE: HIGH", 8'h0D, 8'h0A}, 12);
        join
        measure_pwm_duty("HIGH", 20_000_000, 200);

        // HIGH -> LOW
        fork
            press_btn1();
            uart_check_message({"MODE: LOW", 8'h0D, 8'h0A}, 11);
        join
        measure_pwm_duty("LOW", 20_000_000, 200);

        // LOW -> AUTO
        fork
            press_btn2();
            uart_check_message({"MODE: AUTO", 8'h0D, 8'h0A}, 12);
        join

        // Do 2 chu ky tho lien tiep, dung PWM_FREQ that -> moi chu ky ~2.0s
        // (tong ~4s mo phong cho doan nay, khong dang ke voi thoi gian chay may that)
        prove_breath_2s_via_transcript(2);

        // AUTO -> LOW
        fork
            press_btn1();
            uart_check_message({"MODE: LOW", 8'h0D, 8'h0A}, 11);
        join
        measure_pwm_duty("LOW", 20_000_000, 200);

        // Bounce test: rung phim chi duoc tinh 1 lan
        fork
            glitchy_press_btn1();
            uart_check_message({"MODE: HIGH", 8'h0D, 8'h0A}, 12);
        join
        check_no_uart_within(10_000_000);

        // Negative test: nhan ngan hon nguong debounce
        fork
            short_press_btn1();
            check_no_uart_within(20_000_000);
        join

        // Reset khi dang o HIGH
        fork
            trigger_reset();
            uart_check_message({"MODE: LOW", 8'h0D, 8'h0A}, 11);
        join

        #10_000_000;
        $display("Tong: %0d check, %0d loi", total_checks, total_errors);
        $stop;
    end

endmodule