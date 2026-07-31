module pll_50mhz (
    input  wire clk_in,
    output wire clk_out,
    output wire locked
);

    reg [24:0] counter;
    reg clk_int;
    integer i;

    initial begin
        counter = 0;
        clk_int = 0;
        locked = 0;
    end

    always @(posedge clk_in or negedge locked) begin
        if (!locked) begin
            counter <= 0;
            clk_int <= 0;
        end else begin
            if (counter < 24)
                counter <= counter + 1;
            else begin
                counter <= 0;
                clk_int <= ~clk_int;
            end
        end
    end

    assign clk_out = clk_int;

    always @(posedge clk_in) begin
        if (counter == 24)
            locked <= 1;
    end

endmodule