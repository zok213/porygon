module top (
    input  wire clk_24mhz,
    input  wire btn,
    output wire led,
    output wire uart_tx
);

    wire clk_50mhz;
    wire pll_locked;
    wire btn_debounced;
    wire [7:0] uart_data;
    wire uart_start;
    wire uart_done;

    pll_50mhz u_pll (
        .clk_in  (clk_24mhz),
        .clk_out (clk_50mhz),
        .locked  (pll_locked)
    );

    debouncer u_btn (
        .clk     (clk_50mhz),
        .btn_in  (btn),
        .btn_out (btn_debounced)
    );

    breathing_pwm u_pwm (
        .clk      (clk_50mhz),
        .mode     (3'd2),
        .pwm_out  (led)
    );

    uart_tx u_uart (
        .clk      (clk_50mhz),
        .rst      (!pll_locked),
        .tx_start (uart_start),
        .tx_data  (uart_data),
        .tx_done  (uart_done),
        .tx_pin   (uart_tx)
    );

    reg [1:0] state;
    localparam S_IDLE = 2'd0,
               S_TX   = 2'd1;

    always @(posedge clk_50mhz or posedge !pll_locked) begin
        if (!pll_locked) begin
            state <= S_IDLE;
        end else begin
            case (state)
                S_IDLE: begin
                    uart_start <= 1'b0;
                    if (btn_debounced) begin
                        uart_data <= 8'h41;
                        uart_start <= 1'b1;
                        state <= S_TX;
                    end
                end
                S_TX: begin
                    uart_start <= 1'b0;
                    if (uart_done)
                        state <= S_IDLE;
                end
                default: state <= S_IDLE;
            endcase
        end
    end

endmodule