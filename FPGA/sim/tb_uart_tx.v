`timescale 1ns/1ps

module tb_uart_tx();

    reg clk;
    reg rst_n;
    reg send_req;
    reg [1:0] mode;

    wire tx_pin;
    wire tx_busy;

    // 1. Gọi Module UART TX ở tần số thực 50MHz và Baudrate thực 115200 bps
    uart_tx_string #(
        .CLK_FREQ(50_000_000),
        .BAUD_RATE(115_200)
    ) uut_uart (
        .clk(clk),
        .rst_n(rst_n),
        .send_req(send_req),
        .mode(mode),
        .tx_pin(tx_pin),
        .tx_busy(tx_busy)
    );

    // 2. Tạo xung nhịp hệ thống 50MHz (Chu kỳ = 20ns)
    initial clk = 0;
    always #10 clk = ~clk;

    // 3. Kịch bản kiểm thử đo đạc thời gian UART 115200 bps
    initial begin
        $display("[%0t ns] Bắt đầu đo đạc kiểm thử khối UART TX (115200 bps 8N1)...", $time);
        rst_n = 0; send_req = 0; mode = 2'b00;
        #100; rst_n = 1;
        #200;

        // --- Gửi chuỗi MODE: LOW\r\n ---
        $display("[%0t ns] Kích truyền chuỗi Mode LOW ('MODE: LOW\\r\\n')...", $time);
        mode = 2'b00;
        send_req = 1; #20; send_req = 0;
        
        wait(!tx_busy);
        $display("[%0t ns] Đã phát xong chuỗi Mode LOW!", $time);
        #50_000;

        // --- Gửi chuỗi MODE: HIGH\r\n ---
        $display("[%0t ns] Kích truyền chuỗi Mode HIGH ('MODE: HIGH\\r\\n')...", $time);
        mode = 2'b01;
        send_req = 1; #20; send_req = 0;
        
        wait(!tx_busy);
        $display("[%0t ns] Đã phát xong chuỗi Mode HIGH!", $time);
        #50_000;

        // --- Gửi chuỗi MODE: AUTO\r\n ---
        $display("[%0t ns] Kích truyền chuỗi Mode AUTO ('MODE: AUTO\\r\\n')...", $time);
        mode = 2'b10;
        send_req = 1; #20; send_req = 0;
        
        wait(!tx_busy);
        $display("[%0t ns] Đã phát xong chuỗi Mode AUTO!", $time);
        #100_000;

        $display("[%0t ns] KIỂM THỬ THỜI GIAN UART THÀNH CÔNG!", $time);
        $finish;
    end

endmodule
