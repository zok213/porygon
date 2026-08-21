module button_debounce #(
    parameter CLK_FREQ = 50_000_000,   // Tần số xung nhịp hệ thống (Hz)
    parameter DEBOUNCE_TIME_MS = 20    // Thời gian lọc dội phím (ms)
)(
    input wire clk,         // Clock 50MHz
    input wire rst_n,       // Reset mức thấp
    input wire btn_in,      // Tín hiệu nút nhấn thô từ chân I/O
    output wire btn_pulse   // Xung ngõ ra kéo dài ĐÚNG 1 chu kỳ clock
);
    localparam CNT_MAX = (CLK_FREQ / 1000) * DEBOUNCE_TIME_MS - 1;

    // 1. Đồng bộ hóa 2 tầng (Chống trạng thái lơ lửng Metastability)
    reg sync1, sync2;
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) {sync2, sync1} <= 2'b11; // Mặc định nút nhả = 1
        else {sync2, sync1} <= {sync1, btn_in};
    end

    // 2. Mạch lọc nhiễu 20ms
    reg [23:0] cnt;
    reg btn_stable;
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            cnt <= 24'd0;
            btn_stable <= 1'b1;
        end else if (sync2 != btn_stable) begin
            cnt <= cnt + 1'b1;
            if (cnt >= CNT_MAX) begin 
                btn_stable <= sync2;
                cnt <= 24'd0;
            end
        end else begin
            cnt <= 24'd0;
        end
    end

    // 3. Bộ bắt sườn xuống (Edge Detector)
    reg btn_stable_dly;
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) btn_stable_dly <= 1'b1;
        else btn_stable_dly <= btn_stable;
    end

    assign btn_pulse = (~btn_stable) & btn_stable_dly;
endmodule