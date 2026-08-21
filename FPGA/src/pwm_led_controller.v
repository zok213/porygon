module pwm_led_controller #(
    parameter CLK_FREQ = 50_000_000, // Tần số xung nhịp hệ thống (Hz)
    parameter PWM_FREQ = 1_000       // Tần số xung PWM (Hz)
)(
    input wire clk,         // Clock 50MHz
    input wire rst_n,
    input wire [1:0] mode,  // 00: LOW (25%), 01: HIGH (100%), 10: AUTO (Thở 2s)
    output reg led_out
);
    localparam ARR_MAX = CLK_FREQ / PWM_FREQ; // 50,000 nhịp cho 1kHz @ 50MHz
    localparam STEP_VAL = ARR_MAX / 1000;      // 50,000 / 1000 = 50

    // Counter đếm chu kỳ PWM (0 - 49,999)
    reg [15:0] pwm_cnt;
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) pwm_cnt <= 16'd0;
        else if (pwm_cnt >= ARR_MAX - 1) pwm_cnt <= 16'd0;
        else pwm_cnt <= pwm_cnt + 1'b1;
    end

    // Hiệu ứng "Thở" (Mỗi 1ms đổi 1 lần, 1000 bước = 1.0s tăng + 1.0s giảm = 2.0s chu kỳ)
    reg [15:0] breath_duty;
    reg breath_dir; // 0: Sáng dần, 1: Tối dần

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            breath_duty <= 16'd0;
            breath_dir  <= 1'b0;
        end else if (mode == 2'b10) begin
            if (pwm_cnt == ARR_MAX - 1) begin // Cứ sau 1ms
                if (breath_dir == 1'b0) begin
                    if (breath_duty >= ARR_MAX - STEP_VAL) breath_dir <= 1'b1;
                    else breath_duty <= breath_duty + STEP_VAL;
                end else begin
                    if (breath_duty <= STEP_VAL) breath_dir <= 1'b0;
                    else breath_duty <= breath_duty - STEP_VAL;
                end
            end
        end else begin
            breath_duty <= 16'd0;
            breath_dir  <= 1'b0;
        end
    end

    // Chọn giá trị Duty Cycle theo Mode (Đồng bộ hóa thanh ghi để tối ưu timing)
    reg [15:0] duty_cycle;
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            duty_cycle <= ARR_MAX / 4;
        end else begin
            case (mode)
                2'b00: duty_cycle <= ARR_MAX / 4;   // LOW: 25%
                2'b01: duty_cycle <= ARR_MAX;       // HIGH: 100%
                2'b10: duty_cycle <= breath_duty;   // AUTO: Breathing
                default: duty_cycle <= ARR_MAX / 4;
            endcase
        end
    end

    // Xuất xung PWM
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) led_out <= 1'b0;
        else led_out <= (pwm_cnt < duty_cycle) ? 1'b1 : 1'b0;
    end
endmodule