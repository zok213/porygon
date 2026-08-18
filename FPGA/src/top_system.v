module top_system (
    input wire clk_in,      // Clock gốc (27MHz cho 4K hoặc 24MHz cho 1P5)
    input wire rst_n_in,    // Nút Reset
    input wire btn1_in,     // Button 1
    input wire btn2_in,     // Button 2
    output wire led_out,    // Tín hiệu PWM ra LED
    output wire uart_tx     // Chân UART_TX
);

    // 1. Khai báo Clock 50MHz từ PLL IP Core
    wire clk_50m;
    wire pll_lock;
    
    Gowin_PLLVR u_pll (
        .clkin(clk_in),
        .clkout(clk_50m),
        .lock(pll_lock)
    );

    // ===================================================================
    // 1.5. MẠCH LỌC NHIỄU & ĐỒNG BỘ HÓA RESET (Reset Synchronizer & Debouncer)
    // ===================================================================
    reg rst_sync1, rst_sync2;
    always @(posedge clk_50m or negedge pll_lock) begin
        if (!pll_lock) {rst_sync2, rst_sync1} <= 2'b00;
        else {rst_sync2, rst_sync1} <= {rst_sync1, rst_n_in};
    end

    reg [19:0] rst_cnt;
    reg rst_n_debounced;

    always @(posedge clk_50m or negedge pll_lock) begin
        if (!pll_lock) begin
            rst_cnt <= 20'd0;
            rst_n_debounced <= 1'b0; // Giữ hệ thống ở trạng thái Reset
        end else if (!rst_sync2) begin
            rst_cnt <= 20'd0;
            rst_n_debounced <= 1'b0; // Reset tức thì khi nhấn nút
        end else begin
            if (rst_n_debounced == 1'b0) begin
                if (rst_cnt == 20'd999_999) begin // Đợi ổn định đúng 20ms khi nhả nút/khởi động
                    rst_n_debounced <= 1'b1;  // Giải phóng Reset
                    rst_cnt <= 20'd0;
                end else begin
                    rst_cnt <= rst_cnt + 1'b1;
                end
            end else begin
                rst_cnt <= 20'd0;
            end
        end
    end

    // Gán tín hiệu Reset toàn hệ thống bằng tín hiệu đã được lọc nhiễu sạch sẽ
    wire sys_rst_n = rst_n_debounced;
    // ===================================================================

    // 2. Chống dội phím (Generic RTL Instantiation)
    wire btn1_pulse, btn2_pulse;
    button_debounce #(
        .CLK_FREQ(50_000_000),
        .DEBOUNCE_TIME_MS(20)
    ) u_btn1 (
        .clk(clk_50m),
        .rst_n(sys_rst_n),
        .btn_in(btn1_in),
        .btn_pulse(btn1_pulse)
    );

    button_debounce #(
        .CLK_FREQ(50_000_000),
        .DEBOUNCE_TIME_MS(20)
    ) u_btn2 (
        .clk(clk_50m),
        .rst_n(sys_rst_n),
        .btn_in(btn2_in),
        .btn_pulse(btn2_pulse)
    );

    // 3. FSM Trung tâm quản lý trạng thái Chế độ
    reg [1:0] current_mode;
    reg uart_send_req;
    reg first_boot;

    always @(posedge clk_50m or negedge sys_rst_n) begin
        if (!sys_rst_n) begin
            current_mode <= 2'b00; // Mặc định Mode LOW
            uart_send_req <= 1'b0;
            first_boot <= 1'b1;
        end else begin
            uart_send_req <= 1'b0;
            
            if (first_boot) begin
                uart_send_req <= 1'b1; // Tự động phát chuỗi "MODE: LOW\r\n" khi khởi động
                first_boot <= 1'b0;
            end else if (btn1_pulse) begin
                if (current_mode == 2'b00) current_mode <= 2'b01;      // LOW -> HIGH
                else if (current_mode == 2'b01) current_mode <= 2'b00; // HIGH -> LOW
                else if (current_mode == 2'b10) current_mode <= 2'b00; // AUTO -> LOW
                uart_send_req <= 1'b1;
            end else if (btn2_pulse) begin
                if (current_mode != 2'b10) begin
                    current_mode <= 2'b10; // Switch to AUTO
                    uart_send_req <= 1'b1;
                end
            end
        end
    end

    // 4. Kết nối Điều khiển PWM & UART TX (Generic Parameter Overriding)
    pwm_led_controller #(
        .CLK_FREQ(50_000_000),
        .PWM_FREQ(1_000)
    ) u_pwm (
        .clk(clk_50m),
        .rst_n(sys_rst_n),
        .mode(current_mode),
        .led_out(led_out)
    );

    wire tx_busy;
    uart_tx_string #(
        .CLK_FREQ(50_000_000),
        .BAUD_RATE(115_200)
    ) u_uart (
        .clk(clk_50m),
        .rst_n(sys_rst_n),
        .send_req(uart_send_req),
        .mode(current_mode),
        .tx_pin(uart_tx),
        .tx_busy(tx_busy)
    );

endmodule