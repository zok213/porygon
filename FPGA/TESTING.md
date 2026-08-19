# KỊCH BẢN KIỂM THỬ TỰ ĐỘNG & HƯỚNG DẪN MÔ PHỎNG DẠNG SÓNG (TESTING SUITE)
## Dự Án FPGA GW1NSR-4C — Đội Thi Porygon (Da Nang Contest 2026)

---

## 1. 🎯 TỔNG QUAN HỆ THỐNG KIỂM THỬ TỰ ĐỘNG (SELF-CHECKING TESTBENCH)

Tệp Testbench [`sim/tb_top_system_v2.v`](sim/tb_top_system_v2.v) được thiết kế theo tiêu chuẩn kiểm thử vi mạch số công nghiệp, tích hợp **25 chỉ tiêu kiểm tra tự động (Self-Checking Assertions)**:
- **Tự động đối soát từng byte UART**: Bắt sườn xuống của bit Start, lấy mẫu tại trung tâm chu kỳ bit ($8,680.56\text{ ns}$), so sánh chuỗi ASCII và kiểm tra 2 byte kết thúc `0x0D (\r)`, `0x0A (\n)`.
- **Tự động đo thời gian chu kỳ Thở LED 2.0s**: Sử dụng cờ đảo chiều `breath_dir` để bắt đỉnh ($100\%$) và đáy ($0\%$), tính toán thời gian pha tăng, pha giảm và tổng chu kỳ với độ chính xác đến 6 chữ số thập phân ($1.000000\text{ s} + 1.000000\text{ s} = 2.000000\text{ s}$).
- **Tự động kiểm tra chống nẩy phím (Bounce)**: Tạo chuỗi xung rung nẩy kéo dài $5\text{ ms}$ để chứng minh FSM chỉ kích hoạt đúng 1 lần duy nhất.
- **Tự chứa hoàn toàn (Zero-Dependency)**: Sử dụng thư viện macro [`src/prim_sim.v`](src/prim_sim.v) để mô phỏng Gowin PLLVR trên mọi phiên bản ModelSim.

---

## 2. 📊 MA TRẬN 25 CHỈ TIÊU KIỂM THỬ TỰ ĐỘNG

| Mã Ca Kiểm Thử | Tên Kịch Bản | Hành Vi Kích Hoạt | Cơ Chế Kiểm Tra Tự Động | Tiêu Chuẩn Đạt (Pass Criteria) | Kết Quả Thực Tế |
| :---: | :--- | :--- | :--- | :--- | :---: |
| **TC-01** | Khởi động / Reset | Kích hoạt `trigger_reset` | `uart_check_message` | Nhận đúng 11 byte `"MODE: LOW\r\n"` | **PASS (11/11 bytes)** |
| **TC-02** | Đo Duty Mode LOW | Lấy mẫu tín hiệu `led_out` | `measure_pwm_duty` | Tỷ lệ mức cao đạt đúng $25.0\%$ | **PASS (25.0%)** |
| **TC-03** | Chuyển LOW $\rightarrow$ HIGH | Nhấn Button 1 (`press_btn1`) | `uart_check_message` | Nhận đúng 12 byte `"MODE: HIGH\r\n"` | **PASS (12/12 bytes)** |
| **TC-04** | Đo Duty Mode HIGH | Lấy mẫu tín hiệu `led_out` | `measure_pwm_duty` | Tỷ lệ mức cao đạt đúng $100.0\%$ (Sáng liên tục) | **PASS (100.0%)** |
| **TC-05** | Chuyển HIGH $\rightarrow$ LOW | Nhấn Button 1 (`press_btn1`) | `uart_check_message` | Nhận đúng 11 byte `"MODE: LOW\r\n"` | **PASS (11/11 bytes)** |
| **TC-06** | Chuyển LOW $\rightarrow$ AUTO | Nhấn Button 2 (`press_btn2`) | `uart_check_message` | Nhận đúng 12 byte `"MODE: AUTO\r\n"` | **PASS (12/12 bytes)** |
| **TC-07** | Đo Chu kỳ 1 Thở AUTO | Theo dõi sườn `breath_dir` | `prove_breath_2s_via_transcript` | Pha giảm $1.000\text{ s}$ + Pha tăng $1.000\text{ s} = 2.000000\text{ s}$ | **PASS (2.000000s)** |
| **TC-08** | Đo Chu kỳ 2 Thở AUTO | Theo dõi sườn `breath_dir` | `prove_breath_2s_via_transcript` | Pha giảm $1.000\text{ s}$ + Pha tăng $1.000\text{ s} = 2.000000\text{ s}$ | **PASS (2.000000s)** |
| **TC-09** | Thoát AUTO $\rightarrow$ LOW | Nhấn Button 1 trong AUTO | `uart_check_message` | Nhận đúng 11 byte `"MODE: LOW\r\n"` | **PASS (11/11 bytes)** |
| **TC-10** | Lọc Nhiễu Rung Phím (Bounce) | Kích hoạt `glitchy_press_btn1` | `uart_check_message` + `check_no_uart` | Chỉ phát đúng 1 chuỗi `"MODE: HIGH\r\n"` duy nhất | **PASS (1 lần đổi)** |
| **TC-11** | Loại Bỏ Xung Phím Ngắn | Nhấn cực ngắn ($5\text{ ms} < 20\text{ ms}$) | `short_press_btn1` | Không phát UART, giữ nguyên trạng thái | **PASS (Không đổi)** |
| **TC-12** | Reset Khi Đang Ở HIGH | Kích hoạt `trigger_reset` | `uart_check_message` | Quay về LOW và phát `"MODE: LOW\r\n"` | **PASS (11/11 bytes)** |

---

## 3. 🚀 HƯỚNG DẪN THỰC THI MÔ PHỎNG TRÊN MODELSIM SE 10.6d

### Cách 1: Chạy tự động bằng 1 lệnh duy nhất (Khuyên dùng)
```tcl
cd d:/FPGA&MCU/FPGA
do run_sim.do
```

### Cách 2: Thực thi từng lệnh Tcl
```tcl
cd d:/FPGA&MCU/FPGA
vlib work
vmap work work
vlog src/prim_sim.v
vlog src/gowin_pllvr.v
vlog src/button_debounce.v
vlog src/pwm_led_controller.v
vlog src/uart_tx_string.v
vlog src/top_system.v
vlog sim/tb_top_system_v2.v
vsim -voptargs="+acc" work.tb_top_system_v2
do wavefinal.do
run -all
```

---

## 4. 📈 BẢNG TÍN HIỆU DẠNG SÓNG WAVEFORM (`wavefinal.do`)

| Tín Hiệu Trên Wave | Màu Sắc | Định Dạng (Radix) | Vị Trí Cursor Đo Thời Gian | Ý Nghĩa Kỹ Thuật |
| :--- | :---: | :---: | :---: | :--- |
| `btn1_in` | Xanh Lá | Binary | — | Tín hiệu nút bấm 1 từ chân vật lý (Active-LOW). |
| `btn1_pulse` | **Hồng (Pink)** | Binary | — | Xung 1-clock $20\text{ ns}$ sau khi lọc dội $20\text{ ms}$. |
| `btn2_in` | Xanh Lá | Binary | — | Tín hiệu nút bấm 2 chuyển AUTO (Active-LOW). |
| `btn2_pulse` | **Xanh Oliu** | Binary | — | Xung 1-clock $20\text{ ns}$ chuyển sang chế độ Thở. |
| `led_out` | **Đỏ (Red)** | Binary | — | Ngõ ra xung PWM điều khiển LED2 (Active-HIGH). |
| `uart_tx` | **Vàng (Gold)** | Binary | — | Chuỗi bit UART TX $115,200\text{ bps}$ 8N1. |
| `u_uart/bit_cnt` | **Tím** | Unsigned | — | Đếm vị trí bit UART ($0$: Start, $1..8$: Data, $9$: Stop). |
| `current_mode` | Mặc định | Unsigned | — | Chế độ FSM: `0` (LOW 25%), `1` (HIGH 100%), `2` (AUTO). |
| `u_pwm/breath_duty` | Mặc định | Unsigned | — | Độ rộng nấc thở biến thiên $0 \leftrightarrow 50,000$. |
| `u_pwm/breath_dir` | Mặc định | Binary | **Cursor 5, 6, 7, 8** | Cờ đảo chiều thở (Đo chính xác chu kỳ $2.000\text{ s}$). |

---

## 5. 📋 LOG TRANSCRIPT BÁO CÁO THỰC NGHIỆM MODELSIM

```text
# [35000000 ns] Nhan Button 1
#   12 byte, 0 loi
#   bit period ~8680.556 ns, baud ~115200.0 bps, sai so 0.000%
# [75000000 ns] duty HIGH = 100.00%
# ========================================
# CHUNG MINH LED THO DUNG 2.0s (qua transcript)
# ========================================
# Chu ky 1: pha giam=1.000000s, pha tang=1.000000s, TONG=2.000000s (ky vong 2.000000s)
#   -> OK: chu ky 1 dat yeu cau 2.0s
# Chu ky 2: pha giam=1.000000s, pha tang=1.000000s, TONG=2.000000s (ky vong 2.000000s)
#   -> OK: chu ky 2 dat yeu cau 2.0s
# ========================================
# Tong: 25 check, 0 loi
```
