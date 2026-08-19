# 🌊 HƯỚNG DẪN MÔ PHỎNG DỰ ÁN FPGA TRÊN MODELSIM SE 10.6d

Tài liệu này hướng dẫn chi tiết các bước cài đặt, nạp License, cấu hình dự án tự chứa (**Self-contained Project**) với thư viện linh kiện Gowin (`src/prim_sim.v`) và thực thi kịch bản mô phỏng kiểm thử tự động (**Self-Checking Testbench**) chứng minh chu kỳ LED Thở đúng $2.000\text{ s}$ trên phần mềm **ModelSim SE 10.6d**.

---

## 1. ⚙️ THÔNG TIN PHẦN MỀM & THƯ VIỆN LINH KIỆN GOWIN

- **Tên phần mềm**: ModelSim SE (Special Edition - 64-bit)
- **Phiên bản khuyến nghị**: `ModelSim SE-64 10.6d` (hoặc ModelSim-Altera / QuestaSim).
- **Bộ cài đặt & Thư viện tự chứa (Zero-Dependency)**:
  - Tệp cài đặt phần mềm: `modelsim-win64-10.6d-se.exe`.
  - Thư viện mô phỏng linh kiện Gowin EDA: [`FPGA/src/prim_sim.v`](FPGA/src/prim_sim.v) (Chứa macro mô phỏng `PLLVR`, `GSR`, `OSC`, `LUT`... không cần cài đặt thêm thư viện ngoài).
- **Loại License**: Mentor Graphics License (`MGLS License Server` / `FlexLM`).

---

## 2. 🚀 QUY TRÌNH CHẠY MÔ PHỎNG BẰNG 1 LỆNH DUY NHẤT (QUICK START)

### **Cách 1: Chạy tự động qua tệp kịch bản `run_sim.do` (Khuyên Dùng)**
1. Mở phần mềm **ModelSim SE 10.6d**.
2. Trên thanh menu, chọn: **`File` $\rightarrow$ `Change Directory...`** $\rightarrow$ Trỏ đến thư mục `FPGA/`.
3. Trong cửa sổ **Transcript**, gõ lệnh sau và bấm **Enter**:
   ```tcl
   do run_sim.do
   ```

---

### **Cách 2: Thực thi từng bước bằng lệnh Tcl tương đối**
Dán toàn bộ đoạn lệnh Tcl dưới đây vào cửa sổ **Transcript** của ModelSim và bấm **Enter**:

```tcl
# Step 1: Khởi tạo và liên kết thư viện mô phỏng work
vlib work
vmap work work

# Step 2: Biên dịch thư viện linh kiện Gowin và các tệp nguồn Verilog RTL
vlog src/prim_sim.v
vlog src/gowin_pllvr.v
vlog src/button_debounce.v
vlog src/pwm_led_controller.v
vlog src/uart_tx_string.v
vlog src/top_system.v

# Step 3: Biên dịch Testbench tự động kiểm thử
vlog sim/tb_top_system_v2.v

# Step 4: Khởi chạy mô phỏng vsim và nạp giao diện sóng wavefinal.do
vsim -voptargs="+acc" work.tb_top_system_v2
do wavefinal.do
run -all
```

---

## 3. 🔍 GIẢI THÍCH CÁC TÍN HIỆU TRÊN CỬA SỔ WAVEFORM (`wavefinal.do`)

Tệp `wavefinal.do` được thiết lập sẵn phân màu và định dạng chuyên nghiệp:

| Tên Tín hiệu trên Wave | Loại Tín hiệu | Định dạng (Radix) | Màu sắc (Color) | Ý nghĩa Kỹ thuật |
| :--- | :--- | :---: | :---: | :--- |
| `btn1_in` | Input | Binary | Mặc định (Xanh lá) | Nút bấm BTN1 (Chuyển chế độ LOW $\leftrightarrow$ HIGH). Active-LOW ($0$ là nhấn, $1$ là nhả). |
| `btn1_pulse` | Internal Wire | Binary | **Hồng (Pink)** | Xung phát hiện sườn xuống nút BTN1 kéo dài **đúng 1 chu kỳ clock ($20\text{ ns}$)** sau bộ lọc $20\text{ ms}$. |
| `btn2_in` | Input | Binary | Mặc định (Xanh lá) | Nút bấm BTN2 (Chuyển sang chế độ AUTO Breathing). Active-LOW. |
| `btn2_pulse` | Internal Wire | Binary | **Xanh Oliu (Olive Drab)** | Xung phát hiện sườn xuống nút BTN2 kéo dài **đúng 1 chu kỳ clock ($20\text{ ns}$)**. |
| `led_out` | Output | Binary | **Đỏ (Red)** | Ngõ ra xung PWM điều khiển độ sáng LED D5. Active-HIGH ($1$ là LED SÁNG). |
| `uart_tx` | Output | Binary | **Vàng (Gold)** | Chân truyền phát telemetry UART TX tốc độ $115,200\text{ bps}$ ($8\text{N}1$). |
| `u_uart/bit_cnt` | Reg Counter | **Unsigned** | **Tím (Magenta)** | Bộ đếm vị trí bit UART ($0$: Start Bit, $1..8$: Data Bits, $9$: Stop Bit). |
| `current_mode` | Reg State | **Unsigned** | Mặc định | Trạng thái FSM chính: `0` (Mode LOW 25%), `1` (Mode HIGH 100%), `2` (Mode AUTO Thở). |
| `u_pwm/breath_duty` | Reg Counter | **Unsigned** | Mặc định | Độ rộng xung duty điều chỉnh hiệu ứng Thở ($0 \rightarrow 50,000$). |
| `u_pwm/breath_dir` | Reg Flag | **Binary** | Mặc định | Hướng thở: `0` (Sáng dần $0\% \rightarrow 100\%$), `1` (Tối dần $100\% \rightarrow 0\%$). |

---

## 4. 📋 ĐOẠN LOG MẪU TỰ ĐỘNG BÁO KẾT QUẢ TRÊN TRANSCRIPT (25 CHECKS, 0 LỖI)

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
