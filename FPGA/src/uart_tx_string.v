module uart_tx_string #(
    parameter CLK_FREQ  = 50_000_000, // Tần số xung nhịp hệ thống (Hz)
    parameter BAUD_RATE = 115_200      // Tốc độ Baud (bps)
)(
    input wire clk,          // Clock 50MHz
    input wire rst_n,
    input wire send_req,     // Xung yêu cầu phát chuỗi
    input wire [1:0] mode,   // Chế độ hiện tại
    output reg tx_pin,       // Chân UART_TX
    output reg tx_busy       // Bận truyền
);
    localparam BAUD_DIV = CLK_FREQ / BAUD_RATE; // 50MHz / 115200 = 434
    
    reg [7:0] tx_data;
    reg tx_start;
    reg [8:0] baud_cnt;      // Tối ưu 9-bit đủ đếm tới 434 (tiết kiệm tài nguyên thanh ghi)
    reg [3:0] bit_cnt;
    reg [9:0] shift_reg;
    reg transmitting;

    // Khối phát UART 1 Byte
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            transmitting <= 1'b0;
            tx_pin <= 1'b1;
            baud_cnt <= 9'd0;
            bit_cnt <= 4'd0;
        end else begin
            if (tx_start && !transmitting) begin
                transmitting <= 1'b1;
                shift_reg <= {1'b1, tx_data, 1'b0}; // Stop(1) + Data(8) + Start(0)
                bit_cnt <= 4'd0;
                baud_cnt <= 9'd0;
            end else if (transmitting) begin
                if (baud_cnt == BAUD_DIV - 1) begin
                    baud_cnt <= 9'd0;
                    tx_pin <= shift_reg[0];
                    shift_reg <= {1'b1, shift_reg[9:1]};
                    if (bit_cnt == 4'd9) transmitting <= 1'b0;
                    else bit_cnt <= bit_cnt + 1'b1;
                end else begin
                    baud_cnt <= baud_cnt + 1'b1;
                end
            end else begin
                tx_pin <= 1'b1;
            end
        end
    end

    wire tx_done = (bit_cnt == 4'd9 && baud_cnt == BAUD_DIV - 1);

    // FSM Gửi toàn bộ chuỗi ASCII
    reg [3:0] char_idx;
    reg [1:0] str_state;
    reg [1:0] mode_latched; // Latch mode khi nhận send_req để chống nhiễu trạng thái mid-transmission

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            str_state <= 2'd0;
            char_idx <= 4'd0;
            tx_start <= 1'b0;
            tx_busy <= 1'b0;
            tx_data <= 8'd0;
            mode_latched <= 2'd0;
        end else begin
            case (str_state)
                2'd0: begin // IDLE
                    if (send_req) begin
                        tx_busy <= 1'b1;
                        char_idx <= 4'd0;
                        mode_latched <= mode; // Latch mode an toàn
                        str_state <= 2'd1;
                    end
                end
                2'd1: begin // LOAD_CHAR
                    tx_start <= 1'b1;
                    str_state <= 2'd2;
                    case (mode_latched)
                        2'b00: // "MODE: LOW\r\n"
                            case (char_idx)
                                4'd0: tx_data <= "M"; 4'd1: tx_data <= "O"; 4'd2: tx_data <= "D"; 4'd3: tx_data <= "E";
                                4'd4: tx_data <= ":"; 4'd5: tx_data <= " "; 4'd6: tx_data <= "L"; 4'd7: tx_data <= "O";
                                4'd8: tx_data <= "W"; 4'd9: tx_data <= 8'h0D; 4'd10: tx_data <= 8'h0A; default: tx_data <= 0;
                            endcase
                        2'b01: // "MODE: HIGH\r\n"
                            case (char_idx)
                                4'd0: tx_data <= "M"; 4'd1: tx_data <= "O"; 4'd2: tx_data <= "D"; 4'd3: tx_data <= "E";
                                4'd4: tx_data <= ":"; 4'd5: tx_data <= " "; 4'd6: tx_data <= "H"; 4'd7: tx_data <= "I";
                                4'd8: tx_data <= "G"; 4'd9: tx_data <= "H"; 4'd10: tx_data <= 8'h0D; 4'd11: tx_data <= 8'h0A; default: tx_data <= 0;
                            endcase
                        2'b10: // "MODE: AUTO\r\n"
                            case (char_idx)
                                4'd0: tx_data <= "M"; 4'd1: tx_data <= "O"; 4'd2: tx_data <= "D"; 4'd3: tx_data <= "E";
                                4'd4: tx_data <= ":"; 4'd5: tx_data <= " "; 4'd6: tx_data <= "A"; 4'd7: tx_data <= "U";
                                4'd8: tx_data <= "T"; 4'd9: tx_data <= "O"; 4'd10: tx_data <= 8'h0D; 4'd11: tx_data <= 8'h0A; default: tx_data <= 0;
                            endcase
                        default: tx_data <= 8'd0;
                    endcase
                end
                2'd2: begin // WAIT_TRANSMIT
                    tx_start <= 1'b0;
                    if (tx_done) begin
                        char_idx <= char_idx + 1'b1;
                        str_state <= 2'd3;
                    end
                end
                2'd3: begin // CHECK_FINISHED
                    if ((mode_latched == 2'b00 && char_idx == 4'd11) || 
                        (mode_latched != 2'b00 && char_idx == 4'd12)) begin
                        str_state <= 2'd0;
                        tx_busy <= 1'b0;
                    end else begin
                        str_state <= 2'd1;
                    end
                end
                default: str_state <= 2'd0;
            endcase
        end
    end
endmodule