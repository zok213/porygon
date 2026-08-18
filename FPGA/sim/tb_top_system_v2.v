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

    // Scale PWM_FREQ len 50kHz chi trong mo phong -> chu ky tho con 40ms
    // thay vi 2.0s that, de test nhanh. STEP_VAL van la so nguyen sach
    // (ARR_MAX=1000, STEP_VAL=1) nen khong lam sai logic breathing.
    defparam uut.u_pwm.PWM_FREQ = 50_000;

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

    // Theo doi xu huong tang/giam cua duty trong lan quet breathing
    real breath_prev_duty = -1;
    integer breath_rising = 0;
    integer breath_falling = 0;

    task automatic measure_breath_sample(input integer window_ns, input integer step_ns);
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
            $display("[%0t ns] breath duty = %.2f%%", $time, duty_pct);
            if (breath_prev_duty >= 0) begin
                if (duty_pct > breath_prev_duty) breath_rising = breath_rising + 1;
                else if (duty_pct < breath_prev_duty) breath_falling = breath_falling + 1;
            end
            breath_prev_duty = duty_pct;
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
        measure_pwm_duty("LOW", 20_000_000, 10_000);

        // LOW -> HIGH
        fork
            press_btn1();
            uart_check_message({"MODE: HIGH", 8'h0D, 8'h0A}, 12);
        join
        measure_pwm_duty("HIGH", 20_000_000, 10_000);

        // HIGH -> LOW
        fork
            press_btn1();
            uart_check_message({"MODE: LOW", 8'h0D, 8'h0A}, 11);
        join
        measure_pwm_duty("LOW", 20_000_000, 10_000);

        // LOW -> AUTO
        fork
            press_btn2();
            uart_check_message({"MODE: AUTO", 8'h0D, 8'h0A}, 12);
        join

        // Quet 1 chu ky tho (~40ms sau khi da scale PWM_FREQ), 40 mau 1ms/mau
        repeat (40) measure_breath_sample(1_000_000, 10_000);
        $display("breath: %0d mau tang, %0d mau giam", breath_rising, breath_falling);
        if (breath_rising < 5 || breath_falling < 5) begin
            total_errors = total_errors + 1;
            $display("loi: duty khong the hien ro xu huong tang/giam cua hieu ung tho");
        end
        total_checks = total_checks + 1;

        // AUTO -> LOW
        fork
            press_btn1();
            uart_check_message({"MODE: LOW", 8'h0D, 8'h0A}, 11);
        join
        measure_pwm_duty("LOW", 20_000_000, 10_000);

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