module breathing_pwm (
    input  wire clk,
    input  wire [2:0] mode,
    output reg  pwm_out
);

    reg [15:0] duty_cycle;
    reg [15:0] period;
    reg [15:0] counter;

    parameter MODE_OFF   = 3'd0,
              MODE_STATIC = 3'd1,
              MODE_BREATH = 3'd2,
              MODE_BLINK  = 3'd3;

    always @(posedge clk) begin
        case (mode)
            MODE_OFF: begin
                duty_cycle <= 16'd0;
                period     <= 16'd1000;
            end
            MODE_STATIC: begin
                duty_cycle <= 16'd500;
                period     <= 16'd1000;
            end
            MODE_BREATH: begin
                duty_cycle <= counter[15:8];
                period     <= 16'd1000;
            end
            MODE_BLINK: begin
                duty_cycle <= (counter[15:8] < 8'd128) ? 16'd500 : 16'd0;
                period     <= 16'd1000;
            end
            default: begin
                duty_cycle <= 16'd0;
                period     <= 16'd1000;
            end
        endcase
    end

    always @(posedge clk) begin
        if (counter >= period)
            counter <= 16'd0;
        else
            counter <= counter + 16'd1;
    end

    always @(posedge clk) begin
        if (counter < duty_cycle)
            pwm_out <= 1'b1;
        else
            pwm_out <= 1'b0;
    end

endmodule