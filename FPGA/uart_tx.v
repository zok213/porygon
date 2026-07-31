module uart_tx (
    input  wire clk,
    input  wire rst,
    input  wire tx_start,
    input  wire [7:0] tx_data,
    output reg  tx_done,
    output reg  tx_pin
);

    parameter CLK_FREQ = 50000000;
    parameter BAUD_RATE = 115200;
    parameter CLKS_PER_BIT = CLK_FREQ / BAUD_RATE;

    parameter IDLE     = 3'd0,
              START_BIT = 3'd1,
              DATA_BITS = 3'd2,
              STOP_BIT  = 3'd3;

    reg [2:0] state;
    reg [31:0] bit_count;
    reg [31:0] clk_count;
    reg [7:0] shift_reg;

    always @(posedge clk or posedge rst) begin
        if (rst) begin
            state     <= IDLE;
            tx_pin    <= 1'b1;
            tx_done   <= 1'b0;
            bit_count <= 0;
            clk_count <= 0;
            shift_reg <= 8'd0;
        end else begin
            case (state)
                IDLE: begin
                    tx_pin  <= 1'b1;
                    tx_done <= 1'b0;
                    if (tx_start) begin
                        shift_reg <= tx_data;
                        state     <= START_BIT;
                        clk_count <= 0;
                        bit_count <= 0;
                    end
                end
                START_BIT: begin
                    tx_pin <= 1'b0;
                    if (clk_count == CLKS_PER_BIT - 1) begin
                        clk_count <= 0;
                        state     <= DATA_BITS;
                    end else begin
                        clk_count <= clk_count + 1;
                    end
                end
                DATA_BITS: begin
                    tx_pin <= shift_reg[0];
                    if (clk_count == CLKS_PER_BIT - 1) begin
                        clk_count <= 0;
                        shift_reg <= {1'b0, shift_reg[7:1]};
                        bit_count <= bit_count + 1;
                        if (bit_count == 7)
                            state <= STOP_BIT;
                    end else begin
                        clk_count <= clk_count + 1;
                    end
                end
                STOP_BIT: begin
                    tx_pin <= 1'b1;
                    if (clk_count == CLKS_PER_BIT - 1) begin
                        clk_count <= 0;
                        tx_done   <= 1'b1;
                        state     <= IDLE;
                    end else begin
                        clk_count <= clk_count + 1;
                    end
                end
                default: state <= IDLE;
            endcase
        end
    end

endmodule