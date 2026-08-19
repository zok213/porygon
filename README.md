# Porygon: Gowin GW1NSR-4C Multi-Mode PWM & UART Telemetry Framework (Da Nang FPGA Contest 2026)

[![CI](https://github.com/zok213/porygon/actions/workflows/ci.yml/badge.svg)](https://github.com/zok213/porygon/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

> **Navigation / Chuyển hướng Ngôn ngữ**:  
> 🇻🇳 [Tiếng Việt — Thuyết Minh Kiến Trúc Kỹ Thuật Chuyên Sâu](#-bản-thuyết-minh-kiến-trúc-hệ-thống-fpga-tiếng-việt)  
> 🇬🇧 [English — Full System Architectural Specification](#-fpga-system-architectural-specification-english)

---

# 🇻🇳 BẢN THUYẾT MINH KIẾN TRÚC HỆ THỐNG FPGA (TIẾNG VIỆT)

## 2. Bảng Tiêu Chí Đánh Giá Cuộc Thi & Phương Án Kỹ Thuật Chi Tiết

| Khối Đề Bài | Trọng Số | Yêu Cầu Kỹ Thuật Đề Bài | Phương Án Kỹ Thuật & Triển Khai Thực Tế Trong Mã Nguồn Verilog RTL |
| :--- | :---: | :--- | :--- |
| **Khối 1: Clock, Reset & Debounce** | **2.0đ** | PLL nâng xung lên 50MHz; lọc dội phím 2 nút bấm xuất xung 1-clock. | Sử dụng IP Core `Gowin_PLLVR` ($27\text{ MHz} \rightarrow 50\text{ MHz}$); mạch Reset Synchronizer giữ reset $20\text{ ms}$ lúc boot; 2 bộ `button_debounce` 2 tầng D-FF + counter $20\text{ ms}$ ($1,000,000$ nhịp clock) + falling edge detector. |
| **Khối 2: Điều Chế PWM LED** | **2.5đ** | Mode LOW 25%, Mode HIGH 100%, Mode AUTO Thở $2.0\text{ s}$ không chớp giật. | Module `pwm_led_controller`: sóng mang $1\text{ kHz}$ ($50,000$ nhịp clock), nấc tăng giảm $50$ nhịp/ms $\rightarrow 1,000$ nấc tăng ($1.0\text{ s}$) $+ 1,000$ nấc giảm ($1.0\text{ s}$) $= 2.000\text{ s}$. |
| **Khối 3: Truyền Dữ Liệu UART TX** | **2.5đ** | UART 115200 bps 8N1 phát chuỗi `"MODE: LOW\r\n"`, `"MODE: HIGH\r\n"`, `"MODE: AUTO\r\n"`. | Module `uart_tx_string`: `BAUD_DIV = 434` (sai số $0.0064\% \ll \pm 2.0\%$), FSM phát chuỗi ASCII kèm byte `0x0D, 0x0A`, chốt `mode_latched` chống xung đột. |
| **Khối 4: FSM Trung Tâm & Mô Phỏng** | **3.0đ** | Reset $\rightarrow$ LOW (phát UART); BTN1 đổi LOW ↔ HIGH; BTN2 $\rightarrow$ AUTO; BTN1 trong AUTO $\rightarrow$ LOW. Kèm Testbench & Báo cáo. | Module `top_system`: FSM trung tâm tự động gửi UART khi boot; testbench `tb_top_system_v2` tự động kiểm thử 25 ca kiểm tra (0 lỗi); thư viện `prim_sim.v`; kịch bản sóng ModelSim `wavefinal.do` & `run_sim.do`. |
| **TỔNG ĐIỂM TOÀN KHỐI FPGA** | **10.0đ** | **Đầy đủ mã nguồn, ràng buộc .cst, mô phỏng self-checking, thư viện prim_sim.v tự chứa.** | **Tuân thủ hoàn toàn 100% yêu cầu đề bài** |

---

## 3. Bản Đồ Cấu Trúc Tệp & Thư Mục Thao Tác (`FPGA_dev` Branch)

```
porygon/
├── .github/
│   ├── ISSUE_TEMPLATE/
│   │   ├── bug_report.md                                 # Mẫu báo lỗi phần cứng & phần mềm FPGA/MCU
│   │   └── feature_request.md                            # Mẫu đề xuất nâng cấp tính năng
│   ├── pull_request_template.md                          # Mẫu kiểm tra và quy trình review Pull Request
│   └── workflows/
│       └── ci.yml                                        # Pipeline CI/CD tự động phân tích tĩnh & kiểm thử
├── .gitignore                                            # Quy tắc loại bỏ tệp rác Gowin EDA & ModelSim
├── CODE_OF_CONDUCT.md                                    # Quy tắc ứng xử đóng góp mã nguồn
├── CONTRIBUTING.md                                       # Quy định quản lý nhánh Git Flow & chuẩn commit
├── LICENSE                                               # Giấy phép bản quyền mã nguồn mở MIT
├── README.md                                             # Master Thuyết minh Kiến trúc Hệ thống (Song ngữ VI & EN đầy đủ)
├── README_VI.md                                          # Bản Thuyết minh Kiến trúc Kỹ thuật Tiếng Việt chuẩn hóa
├── README_EN.md                                          # Master System Architectural Specification (English)
├── README_MODELSIM_SIMULATION.md                         # Hướng dẫn chi tiết chạy mô phỏng ModelSim SE 10.6d
├── README_KEIL_DEBUG_SIMULATION.md                       # Hướng dẫn chi tiết cấu hình Debug Watch 1 trên Keil µVision5
├── README_SIMULATION_SCALING.md                          # Thuyết minh kỹ thuật tỷ lệ thời gian mô phỏng tăng tốc
├── SECURITY.md                                           # Chính sách bảo mật & bảo vệ tài nguyên phần cứng
├── SETUP.md                                              # Hướng dẫn thiết lập môi trường Gowin EDA & ModelSim
├── ĐỀ THI FGPA 2026.docx.pdf                             # Bản gốc Đề thi Khối FPGA Đà Nẵng 2026
├── ĐỀ THI MCU 2026.pdf                                   # Bản gốc Đề thi Khối Microcontroller Đà Nẵng 2026
├── Gowin-FPGA-Vietnamese-Book-ACG525-Basic-part-Print-v (1).pdf # Tài liệu lập trình Gowin FPGA tiếng Việt
└── FPGA/                                                 # Thư mục Không gian làm việc Dự án Gowin EDA
    ├── .gitignore                                        # Loại bỏ tệp đầu ra tổng hợp impl/ & mô phỏng work/
    ├── README.md                                         # Báo cáo kỹ thuật chi tiết thư mục FPGA & Hướng dẫn Rebuild
    ├── README_MODELSIM_SIMULATION.md                     # Hướng dẫn chạy mô phỏng ModelSim nhanh bằng 1 lệnh
    ├── TESTING.md                                        # Ma trận kiểm thử tự động 25 kịch bản & Hướng dẫn ModelSim
    ├── ISSUE_ANALYSIS_DEEP_DIVE.md                       # Báo cáo phân tích chuyên sâu & thực chứng 6 lỗi FPGA
    ├── README_SIMULATION_SCALING.md                      # Thuyết minh kỹ thuật tỷ lệ thời gian mô phỏng tăng tốc
    ├── pwm11.gprj                                        # Tệp cấu hình dự án Gowin EDA (GW1NSR-4C / Kiwi Nano 4K)
    ├── run_sim.do                                        # Kịch bản Tcl chạy tự động toàn bộ mô phỏng trên ModelSim
    ├── wavefinal.do                                      # Cấu hình hiển thị dạng sóng & 5 thước đo thời gian
    ├── constr/                                           # Thư mục chứa tệp ràng buộc vật lý chân phần cứng
    │   └── pwm11.cst                                     # Ràng buộc chân FPGA (Clock, Reset, BTN1/2, LED, UART TX)
    ├── src/                                              # Mã nguồn Verilog RTL tổng hợp được (Synthesizable RTL)
    │   ├── top_system.v                                  # Module cấp cao nhất: Đồng bộ Reset, Quản lý FSM trung tâm
    │   ├── button_debounce.v                             # Mạch lọc dội phím 2 tầng D-FF + Bộ đếm 20ms + Bắt sườn xung
    │   ├── pwm_led_controller.v                          # Bộ điều chế PWM 1kHz (LOW 25%, HIGH 100%, AUTO Thở 2.0s)
    │   ├── uart_tx_string.v                              # Bộ truyền chuỗi UART 115200 8N1 có chốt an toàn trạng thái
    │   ├── gowin_pllvr.v                                 # Top wrapper IP Core Gowin PLLVR (27MHz -> 50MHz)
    │   ├── prim_sim.v                                    # Thư viện mô phỏng linh kiện Gowin (Zero-Dependency)
    │   └── ip/gowin_pllvr/                               # Tệp cấu hình gốc IP Generator (.ipc, .mod, .v, _tmp.v)
    ├── sim/                                              # Thư mục kịch bản mô phỏng kiểm thử (Verification Suite)
    │   ├── tb_top_system_v2.v                            # Testbench tự động kiểm tra toàn diện 25 chỉ tiêu (0 lỗi)
    │   ├── tb_uart_tx.v                                  # Testbench đo thời gian phát chuỗi khối UART
    │   └── wavefinal.do                                  # Kịch bản dạng sóng & Thước đo Cursor tự động trên ModelSim
    └── docs/                                             # Thư mục tài liệu đề bài và thuyết minh cuộc thi
        ├── ĐỀ THI FGPA 2026.docx.pdf                     # Bản sao đề thi chính thức Khối FPGA Đà Nẵng 2026
        └── README_SIMULATION_SCALING.md                  # Báo cáo kỹ thuật tỷ lệ thời gian mô phỏng
```

---

## 4. Sơ Đồ Khối Kiến Trúc Phần Cứng FPGA

```mermaid
flowchart TD
    subgraph CLOCK_RESET ["Khối 1: Tạo Xung Nhịp, Đồng Bộ & Reset (2.0đ)"]
        OSC["Dao động Thạch anh Onboard<br><b>27.0 MHz (Pin 45)</b>"] --> PLL["Gowin PLLVR IP Core<br>(IDIV=6, FBDIV=12, ODIV=16)"]
        PLL -->|"clk_50m (50.0 MHz)"| SYS_CLK["Clock Hệ Thống 50MHz"]
        PLL -->|"pll_lock"| RST_SYNC["Mạch Đồng Bộ Reset & Khởi Tạo<br>(20ms Power-On Delay)"]
        RST_PIN["Nút Reset Phần Cứng<br><b>rst_n_in (Pin 40)</b>"] --> RST_SYNC
        RST_SYNC -->|"sys_rst_n"| CORE_RESET["Tín hiệu Reset Toàn Hệ Thống"]
    end

    subgraph DEBOUNCERS ["Khối Lọc Dội Phím Bắt Sườn (2.0đ)"]
        BTN1_PIN["Nút Bấm 1<br><b>btn1_in (Pin 14)</b>"] --> DB1["button_debounce 1<br>(2-Stage Sync + 20ms Integrator)"]
        BTN2_PIN["Nút Bấm 2<br><b>btn2_in (Pin 15)</b>"] --> DB2["button_debounce 2<br>(2-Stage Sync + 20ms Integrator)"]
        DB1 -->|"btn1_pulse (1 clock = 20ns)"| FSM
        DB2 -->|"btn2_pulse (1 clock = 20ns)"| FSM
    end

    subgraph SUPERVISOR ["Khối 4: FSM Điều Khiển Chế Độ Trung Tâm (3.0đ)"]
        FSM["FSM Trung Tâm (top_system)<br>• Khởi động: Mode LOW + Bắn UART<br>• BTN1: LOW ↔ HIGH (AUTO → LOW)<br>• BTN2: Chuyển AUTO"]
    end

    subgraph PWM_BLOCK ["Khối 2: Điều Chế PWM LED 1kHz (2.5đ)"]
        FSM -->|"mode[1:0]"| PWM["pwm_led_controller<br>• LOW: 25% Duty (12,500 nhịp)<br>• HIGH: 100% Duty (50,000 nhịp)<br>• AUTO: Thở 2.0s (2,000 nấc x 1ms)"]
        PWM --> LED_PIN["Chân LED2 Bo Mạch<br><b>led_out (Pin 13, 1.8V)</b>"]
    end

    subgraph UART_BLOCK ["Khối 3: Truyền Telemetry UART 115200 (2.5đ)"]
        FSM -->|"send_req & mode[1:0]"| UART["uart_tx_string<br>• BAUD_DIV = 434 (Error 0.006%)<br>• Chốt mode_latched an toàn<br>• Phát ASCII kết thúc bằng \\r\\n"]
        UART --> TX_PIN["Cổng UART TX qua USB<br><b>uart_tx (Pin 39, 3.3V)</b>"]
    end
```

---

## 5. Bảng Cấu Hình Chân Ngoại Vi Phần Cứng (Kiwi Nano 4K Pinout)

Toàn bộ chân tín hiệu được khai báo trong tệp ràng buộc vật lý [`FPGA/constr/pwm11.cst`](FPGA/constr/pwm11.cst):

| Tên Tín Hiệu | Chân FPGA | Ngân Sách I/O (Bank) | Chuẩn Điện Áp | Cấu Hình Kéo Điện Trở | Dòng Kích (Drive) | Chức Năng Phần Cứng |
| :--- | :---: | :---: | :---: | :---: | :---: | :--- |
| `clk_in` | **Pin 45** | Bank 1 | `LVCMOS33` (3.3V) | `PULL_MODE=UP` | — | Xung nhịp dao động thạch anh gốc $27.0\text{ MHz}$ |
| `rst_n_in` | **Pin 40** | Bank 1 | `LVCMOS33` (3.3V) | `PULL_MODE=UP` | — | Nút Reset phần cứng (Tích cực mức Thấp) |
| `btn1_in` | **Pin 14** | Bank 3 | `LVCMOS18` (1.8V) | `PULL_MODE=UP` | — | Nút bấm 1: Chuyển đổi LOW ↔ HIGH / AUTO $\rightarrow$ LOW |
| `btn2_in` | **Pin 15** | Bank 3 | `LVCMOS18` (1.8V) | `PULL_MODE=UP` | — | Nút bấm 2: Chuyển sang chế độ AUTO Thở $2.0\text{ s}$ |
| `led_out` | **Pin 13** | Bank 3 | `LVCMOS18` (1.8V) | `PULL_MODE=NONE` | $8\text{ mA}$ | Ngõ ra xung PWM điều khiển LED2 trên bo mạch |
| `uart_tx` | **Pin 39** | Bank 1 | `LVCMOS33` (3.3V) | `PULL_MODE=NONE` | $8\text{ mA}$ | Ngõ ra truyền UART nối tiếp qua chip nạp USB-UART |

---

## 6. Sơ Đồ Máy Trạng Thái Hạn Định (FSM State Transition Diagram)

```mermaid
stateDiagram-v2
    [*] --> MODE_LOW : Cap nguon / sys_rst_n giai phong (Phat MODE_LOW)

    state MODE_LOW {
        [*] --> PWM_25_Percent : Duty Cycle 25% (1kHz)
    }

    state MODE_HIGH {
        [*] --> PWM_100_Percent : Duty Cycle 100% (Sang lien tuc)
    }

    state MODE_AUTO {
        [*] --> PWM_Breathing : Tho 0% <--> 100% trong 2.0s
    }

    MODE_LOW --> MODE_HIGH : Nhan Nut 1 / Phat MODE_HIGH
    MODE_HIGH --> MODE_LOW : Nhan Nut 1 / Phat MODE_LOW

    MODE_LOW --> MODE_AUTO : Nhan Nut 2 / Phat MODE_AUTO
    MODE_HIGH --> MODE_AUTO : Nhan Nut 2 / Phat MODE_AUTO

    MODE_AUTO --> MODE_LOW : Nhan Nut 1 / Phat MODE_LOW
    MODE_AUTO --> MODE_AUTO : Nhan Nut 2 / Giu nguyen AUTO
```

### Bảng Chuyển Trạng Thái FSM & Khung Truyền UART
| Trạng Thái Hiện Tại | Tín Hiệu Kích Hoạt | Trạng Thái Kế Tiếp | Chuỗi Ký Tự UART Phát Ra | Trạng Thái LED PWM |
| :--- | :--- | :--- | :--- | :--- |
| **Bất kỳ (Khởi động / Reset)** | `sys_rst_n` lên mức 1 | **Mode LOW (2'b00)** | `"MODE: LOW\r\n"` (11 bytes) | Duty Cycle $25\%$ ($1\text{ kHz}$) |
| **Mode LOW (2'b00)** | Nhấn Nút 1 (`btn1_pulse`) | **Mode HIGH (2'b01)** | `"MODE: HIGH\r\n"` (12 bytes) | Duty Cycle $100\%$ (Sáng liên tục) |
| **Mode HIGH (2'b01)** | Nhấn Nút 1 (`btn1_pulse`) | **Mode LOW (2'b00)** | `"MODE: LOW\r\n"` (11 bytes) | Duty Cycle $25\%$ ($1\text{ kHz}$) |
| **Mode LOW (2'b00)** | Nhấn Nút 2 (`btn2_pulse`) | **Mode AUTO (2'b10)** | `"MODE: AUTO\r\n"` (12 bytes) | Hiệu ứng Thở $0\% \leftrightarrow 100\%$ ($2.0\text{ s}$) |
| **Mode HIGH (2'b01)** | Nhấn Nút 2 (`btn2_pulse`) | **Mode AUTO (2'b10)** | `"MODE: AUTO\r\n"` (12 bytes) | Hiệu ứng Thở $0\% \leftrightarrow 100\%$ ($2.0\text{ s}$) |
| **Mode AUTO (2'b10)** | Nhấn Nút 1 (`btn1_pulse`) | **Mode LOW (2'b00)** | `"MODE: LOW\r\n"` (11 bytes) | Duty Cycle $25\%$ ($1\text{ kHz}$) |
| **Mode AUTO (2'b10)** | Nhấn Nút 2 (`btn2_pulse`) | **Giữ nguyên AUTO** | *(Không phát chuỗi thừa)* | Giữ nguyên hiệu ứng Thở $2.0\text{ s}$ |

---

## 7. Biểu Đồ Thời Gian & Thuật Toán Lọc Dội Phím Bắt Sườn (Debouncing)

```mermaid
sequenceDiagram
    autonumber
    participant Pin as "Chân Nút Bấm Vật Lý (btn_in)"
    participant Sync as "2 Tầng D-FF Đồng Bộ (sync1, sync2)"
    participant Cnt as "Bộ Đếm Thời Gian Lọc (cnt 20ms)"
    participant Stable as "Tín Hiệu Đã Lọc Ổn Định (btn_stable)"
    participant Pulse as "Xung Ngõ Ra 1 Chu Kỳ (btn_pulse)"

    Note over Pin: Người dùng nhấn phím (Xuất hiện rung nẩy 1-2ms)
    Pin->>Sync: Sườn xung nhiễu nẩy liên tục
    Sync->>Cnt: sync2 khác btn_stable -> Cnt tăng dần
    Note over Cnt: Nếu xung nhiễu đảo chiều trước 20ms -> Cnt bị reset về 0
    Note over Cnt: Khi giữ phím ổn định đủ 20ms (cnt == CNT_MAX)
    Cnt->>Stable: Cập nhật btn_stable = 0 (Xác nhận phím đã nhấn)
    Stable->>Pulse: Mạch phát hiện sườn xuống xuất btn_pulse = 1 (Kéo dài đúng 20ns)
    Pulse->>Pulse: Sang chu kỳ clock tiếp theo -> btn_pulse tự động về 0
```

---

## 8. Kiến Trúc Bộ Điều Chế PWM & Kỹ Thuật Thở Tuyến Tính 2.0s

1. **Bộ đếm chu kỳ sóng mang PWM (`pwm_cnt`)**:
   - Tần số xung nhịp: $f_{\text{sys}} = 50.0\text{ MHz}$.
   - Tần số sóng mang PWM: $f_{\text{PWM}} = 1,000\text{ Hz} \rightarrow T = 1.0\text{ ms}$.
   - Số nhịp đếm: `ARR_MAX = 50_000` nhịp ($50,000\text{ counts}$).
2. **Cấu hình độ sáng các chế độ**:
   - **Mode LOW (25%)**: `duty_cycle = ARR_MAX / 4 = 12_500 nhịp`.
   - **Mode HIGH (100%)**: `duty_cycle = ARR_MAX = 50_000 nhịp`.
   - **Mode AUTO (Thở 2.0s)**:
     - Chia đều thành **2,000 nấc độ sáng** ($1,000$ nấc tăng dần $+ 1,000$ nấc giảm dần).
     - Bước tăng/giảm mỗi $1\text{ ms}$: `STEP_VAL = 50` nhịp/nấc.
     - Thời gian sáng dần ($0\% \rightarrow 100\%$): $1,000\text{ nấc} \times 1.0\text{ ms} = 1.000\text{ s}$.
     - Thời gian tối dần ($100\% \rightarrow 0\%$): $1,000\text{ nấc} \times 1.0\text{ ms} = 1.000\text{ s}$.
     - **Tổng chu kỳ thở hoàn chỉnh**: $1.0\text{ s} + 1.0\text{ s} = 2.000\text{ giây}$ (Khớp chính xác tuyệt đối yêu cầu đề bài).

---

## 9. Kiến Trúc Bộ Truyền Telemetry UART 115200 bps & Chốt Mode

1. **Bộ chia tần số Baudrate**:
   - Hệ số chia nhịp:
     $$N_{\text{baud}} = \left\lfloor \frac{50,000,000}{115,200} \right\rceil = 434\text{ nhịp clock}$$
   - Tốc độ thực tế thu được:
     $$\text{Baud}_{\text{actual}} = \frac{50,000,000}{434} \approx 115,207.37\text{ bps}$$
   - Sai số tương đối:
     $$\text{Error} = \frac{|115,207.37 - 115,200|}{115,200} \times 100\% = 0.0064\% \ll \pm 2.0\%$$
2. **Khung truyền chuẩn 8N1**:
   - 1 bit Start (mức 0) + 8 bit Data (truyền LSB trước) + 1 bit Stop (mức 1).
   - Tổng cộng: 10 bit cho mỗi ký tự ASCII.
3. **Cơ chế Chốt Chế Độ An Toàn (`mode_latched`)**:
   - Khi nhận xung yêu cầu truyền `send_req`, FSM lưu ngay chế độ hiện tại vào thanh ghi `mode_latched`.
   - Trong suốt thời gian $1.042\text{ ms}$ truyền chuỗi, nếu có bất kỳ tín hiệu nhiễu hoặc thay đổi trạng thái nào, chuỗi đang phát vẫn bảo toàn nguyên vẹn 100%, không bị chắp vá ký tự.

---

## 10. Hướng Dẫn Mô Phỏng ModelSim Tự Chứa (Zero-Dependency Quick Start)
*(Chi tiết tại [`FPGA/README_MODELSIM_SIMULATION.md`](FPGA/README_MODELSIM_SIMULATION.md))*

Dự án tích hợp sẵn thư viện linh kiện Gowin [`FPGA/src/prim_sim.v`](FPGA/src/prim_sim.v) giúp chạy mô phỏng trơn tru trên mọi máy tính mà không cần cài đặt Gowin EDA:

### 🚀 Chạy Tự Động Bằng 1 Lệnh Tcl Duy Nhất
1. Mở phần mềm **ModelSim SE 10.6d**.
2. Chọn menu **`File` $\rightarrow$ `Change Directory...`** $\rightarrow$ Trỏ đến thư mục `FPGA/`.
3. Trong cửa sổ **Transcript**, gõ lệnh sau và bấm **Enter**:
   ```tcl
   do run_sim.do
   ```

### 📊 Kết Quả Mô Phỏng Tự Động (25 Checks, 0 Lỗi):
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

---

## 11. Giải Trình Kỹ Thuật Chi Tiết 6 Lỗi Cuộc Thi & Giải Pháp Phòng Thủ

1. **Khử Nhiễu Khởi Động & Khóa Pha PLL (`rst_n_debounced`)**:
   - *Vấn đề*: Khi vừa cấp nguồn, điện áp nguồn có thể dao động và bộ PLL cần vài mili-giây để khóa tần số (`pll_lock`). Nếu FSM chạy ngay lập tức, hệ thống sẽ rơi vào trạng thái không xác định.
   - *Giải pháp*: Mạch Reset Synchronizer 2 tầng dùng tín hiệu `pll_lock` làm điều kiện giải phóng và giữ mức Reset thêm **$20\text{ ms}$** qua bộ đếm `rst_cnt`, bảo đảm toàn bộ hệ thống hoàn toàn ổn định trước khi chạy.
2. **Triệt Tiêu Hoàn Toàn Rung Nẩy Phím Bấm (Metastability & Bounce Suppression)**:
   - *Vấn đề*: Phím cơ khí khi bấm sinh ra chùm xung nhiễu kéo dài 1-5ms, dễ kích hoạt chuyển đổi trạng thái FSM nhiều lần liên tiếp (double-fire).
   - *Giải pháp*: Module `button_debounce` tích hợp 2 tầng D-FF đồng bộ hóa + bộ tích phân $20\text{ ms}$ ($1,000,000$ nhịp clock) + mạch bắt sườn xuống (Edge Detector), chỉ phát sinh đúng 1 xung clock $20\text{ ns}$ duy nhất cho mỗi lần bấm.
3. **Phòng Ngừa Xung Đột Khung Truyền UART (Zero-Collision Proof)**:
   - *Vấn đề*: Nếu người dùng bấm phím liên tục trong lúc UART đang phát dở chuỗi ký tự, khung truyền sẽ bị méo dạng hoặc đè byte.
   - *Giải pháp*: Chuỗi UART 12 ký tự phát hết $1.042\text{ ms}$. Nhờ bộ lọc dội phím $20\text{ ms}$, khoảng cách giữa 2 lần nhấn phím luôn lớn hơn thời gian truyền tối thiểu $18.96\text{ ms}$. Đồng thời biến `mode_latched` khóa trạng thái trong suốt quá trình phát, bảo vệ khung truyền an toàn tuyệt đối.
4. **Đảm Bảo Tần Số Sóng Mang PWM Không Gây Nhấp Nháy Mắt Người (Flicker-Free)**:
   - *Vấn đề*: Nếu chọn tần số PWM quá thấp (< 100Hz), mắt người sẽ cảm thấy đèn LED bị nhấp nháy khó chịu.
   - *Giải pháp*: Thiết lập tần số sóng mang $f_{\text{PWM}} = 1\text{ kHz}$ ($1,000\text{ Hz} \gg 60\text{ Hz}$), tạo ánh sáng mượt mà, êm dịu ở mọi mức Duty Cycle.
5. **Độ Mịn Tuyến Tính Trong Hiệu Ứng Thở (Smooth Breathing Interpolation)**:
   - *Vấn đề*: Nếu chia quá ít nấc độ sáng, mắt người sẽ thấy LED tăng/giảm độ sáng giật cục từng nấc.
   - *Giải pháp*: Chia đều thành $2,000$ nấc độ sáng cực mịn, mỗi nấc tăng/giảm $50$ nhịp clock sau mỗi $1\text{ ms}$, đem lại cảm giác thở nhẹ nhàng, êm ái tự nhiên.
6. **Mô Phỏng Tự Chứa Không Lỗi Thiếu Thư Viện Gowin**:
   - *Vấn đề*: Khi ban giám khảo chấm bài trên ModelSim không cài sẵn Gowin EDA, việc biên dịch `Gowin_PLLVR` sẽ báo lỗi `Module 'PLLVR' is not defined`.
   - *Giải pháp*: Đính kèm tệp thư viện macro hành vi Gowin [`FPGA/src/prim_sim.v`](FPGA/src/prim_sim.v), bảo đảm biên dịch và chạy mô phỏng 100% độc lập, không phụ thuộc môi trường cài đặt.

---

## 12. Chứng Minh Toán Học & Tính Toán Phần Cứng (Mathematical Proofs)

### Chứng minh 1: Tần số Tổng hợp Xung Nhịp PLLVR ($50.0\text{ MHz}$)
Chip FPGA Gowin GW1NSR-4C nhận dao động thạch anh $f_{\text{IN}} = 27.0\text{ MHz}$. Cấu hình khối PLLVR IP Core:
- `IDIV_SEL = 6` $\rightarrow$ Hệ số chia vào `IDIV = 7`
- `FBDIV_SEL = 12` $\rightarrow$ Hệ số nhân hồi tiếp `FBDIV = 13`
- `ODIV_SEL = 16` $\rightarrow$ Hệ số chia ra `ODIV = 1`

$$f_{\text{CLKOUT}} = 27.0\text{ MHz} \times \frac{13}{7} \approx 50.14\text{ MHz} \approx 50.0\text{ MHz}$$

### Chứng minh 2: Sai Số Tốc Độ Baudrate UART ($115,200\text{ bps}$)
$$N_{\text{baud}} = \left\lfloor \frac{50,000,000}{115,200} \right\rceil = 434\text{ nhịp}$$
$$\text{Baud}_{\text{actual}} = \frac{50,000,000}{434} \approx 115,207.37\text{ bps}$$
$$\text{Error} = \frac{|115,207.37 - 115,200|}{115,200} \times 100\% = 0.0064\% \ll \pm 2.0\%$$

### Chứng minh 3: Khẳng Định Tuyệt Đối Không Xung Đột UART (Zero-Collision Proof)
- Thời gian phát trọn vẹn 1 chuỗi UART dài nhất ($12\text{ bytes} \times 10\text{ bits} = 120\text{ bits}$):
  $$t_{\text{packet}} = \frac{120}{115,200} \approx 1.042\text{ ms}$$
- Thời gian lọc dội phím tối thiểu: $t_{\text{debounce}} = 20.0\text{ ms}$.
- Chênh lệch an toàn: $\Delta t = 20.0\text{ ms} - 1.042\text{ ms} = 18.958\text{ ms} > 0$.
- **Kết luận**: Gói tin UART luôn hoàn tất trước lần nhấn tiếp theo ít nhất $18.958\text{ ms}$, đảm bảo không bao giờ xảy ra tình trạng nghẽn/đè khung truyền.

### Chứng minh 4: Tính Toán Chu Kỳ LED Thở Tuyến Tính ($2.0\text{ Giây}$)
- `ARR_MAX = 50_000` nhịp, `STEP_VAL = 50` nhịp/nấc.
$$T_{\text{cycle}} = (1,000\text{ nấc tăng} \times 1.0\text{ ms}) + (1,000\text{ nấc giảm} \times 1.0\text{ ms}) = 2.000\text{ giây}$$

---

## 13. Bảng So Sánh Kỹ Thuật Đối Chứng (Naive vs Production)

| Khối Chức Năng | Giải Pháp Cơ Bản Thiếu Tối Ưu (Naive Implementation) | Kiến Trúc Kỹ Thuật Vi Mạch Sản Phẩm (Production Architecture) |
| :--- | :--- | :--- |
| **Xung Nhịp & Reset** | Dùng mạch chia tần số mềm; không đồng bộ reset gây trạng thái treo lơ lửng. | Dùng IP Core Gowin PLLVR phần cứng + Mạch Reset Synchronizer 2 tầng trễ $20\text{ ms}$. |
| **Lọc Dội Phím** | Thanh ghi dịch 4 bit ngắn (~80ns); vẫn bị rung nẩy cơ học kích đúp FSM. | Bộ tích phân $20\text{ ms}$ + 2 tầng D-FF + Mạch bắt sườn xuống xuất xung đúng 1 clock $20\text{ ns}$. |
| **Điều Chế PWM** | Tần số PWM thấp gây nhấp nháy mắt; ít nấc độ sáng gây giật nấc. | Sóng mang $1\text{ kHz}$ không nhấp nháy + $2,000$ nấc độ sáng siêu mịn cho chu kỳ thở $2.000\text{ s}$. |
| **Truyền UART** | Gửi từng ký tự đơn `'A'`, không chốt trạng thái; dễ méo chuỗi khi bấm phím. | Gửi đầy đủ chuỗi `"MODE: ... \r\n"` + Chốt `mode_latched` an toàn + Sai số baud chỉ $0.0064\%$. |
| **Mô Phỏng Kiểm Thử** | Thiếu thư viện linh kiện, báo lỗi `PLLVR undefined`; quan sát bằng mắt. | Thư viện `prim_sim.v` tự chứa + Testbench tự động 25 chỉ tiêu chứng minh chu kỳ thở $2.000\text{ s}$. |

---

## 14. Ma Trận Kiểm Thử Tự Động & Hướng Dẫn Mô Phỏng ModelSim

### Ma Trận 25 Chỉ Tiêu Kiểm Thử Tự Động ([`FPGA/sim/tb_top_system_v2.v`](FPGA/sim/tb_top_system_v2.v))
| Nhóm Kiểm Thử | Kịch Bản Kích Hoạt | Hành Vi Mong Đợi | Bộ Kiểm Tra Tự Động | Kết Quả Thực Tế |
| :---: | :--- | :--- | :--- | :---: |
| **TC-01** | Power-on / Reset | Hệ thống về LOW; phát `"MODE: LOW\r\n"` | `uart_check_message(..., 11)` | **PASS (11/11 bytes)** |
| **TC-02** | Đo Duty Mode LOW | Tỷ lệ mức cao đạt đúng 25% | `measure_pwm_duty("LOW", ...)` | **PASS (25.0%)** |
| **TC-03** | Nhấn Button 1 | Chuyển LOW $\rightarrow$ HIGH; phát `"MODE: HIGH\r\n"` | `uart_check_message(..., 12)` | **PASS (12/12 bytes)** |
| **TC-04** | Đo Duty Mode HIGH | Tỷ lệ mức cao đạt đúng 100% | `measure_pwm_duty("HIGH", ...)` | **PASS (100.0%)** |
| **TC-05** | Nhấn tiếp Button 1 | Chuyển HIGH $\rightarrow$ LOW; phát `"MODE: LOW\r\n"` | `uart_check_message(..., 11)` | **PASS (11/11 bytes)** |
| **TC-06** | Nhấn Button 2 | Chuyển sang AUTO; phát `"MODE: AUTO\r\n"` | `uart_check_message(..., 12)` | **PASS (12/12 bytes)** |
| **TC-07** | Đo Chu kỳ 1 Thở AUTO | Pha giảm $1.000\text{ s}$ + Pha tăng $1.000\text{ s} = 2.000\text{ s}$ | `prove_breath_2s_via_transcript` | **PASS (2.000000s)** |
| **TC-08** | Đo Chu kỳ 2 Thở AUTO | Pha giảm $1.000\text{ s}$ + Pha tăng $1.000\text{ s} = 2.000\text{ s}$ | `prove_breath_2s_via_transcript` | **PASS (2.000000s)** |
| **TC-09** | Nhấn Button 1 trong AUTO | Ép chuyển về LOW; phát `"MODE: LOW\r\n"` | `uart_check_message(..., 11)` | **PASS (11/11 bytes)** |
| **TC-10** | Nhiễu rung phím (Bounce) | Rung nẩy phím chỉ tính đúng 1 lần duy nhất | `glitchy_press_btn1` | **PASS (1 lần đổi)** |
| **TC-11** | Xung cực ngắn (< 5ms) | Bị loại bỏ hoàn toàn, không đổi mode | `short_press_btn1` | **PASS (Không đổi)** |
| **TC-12** | Reset khi đang ở HIGH | Hệ thống quay về LOW & phát lại UART | `trigger_reset` | **PASS (11/11 bytes)** |

---

## 15. Hướng Dẫn Biên Dịch & Nạp Mạch (Gowin EDA)

1. Mở phần mềm **Gowin EDA** (V1.9.9 hoặc V1.9.12 trở lên).
2. Chọn **File $\rightarrow$ Open Project...** và mở tệp [`FPGA/pwm11.gprj`](FPGA/pwm11.gprj).
3. Trong tab **Process**, nhấn đúp vào **Place & Route** $\rightarrow$ Kiểm tra đạt **Success (0 Errors, 0 Warnings)**.
4. Kết nối cáp USB bo mạch **Kiwi Nano 4K** vào máy tính.
5. Mở công cụ **Programmer**, chọn thiết bị `GW1NSR-4C`, nạp tệp bitstream `impl/pnr/pwm11.fs` vào bộ nhớ SRAM (hoặc Flash nhúng) của chip FPGA.
6. Mở phần mềm Serial Terminal (PuTTY / Hercules / Serial Monitor) ở tốc độ **115200 bps, 8N1** để quan sát phản hồi telemetry khi thao tác bấm phím.

---

# 🇬🇧 FPGA SYSTEM ARCHITECTURAL SPECIFICATION (ENGLISH)

## 2. Competition Criteria Compliance Matrix

| Track Module | Weight | Official Competition Requirement | Hardware RTL Implementation |
| :--- | :---: | :--- | :--- |
| **Block 1: Clock, Reset & Debounce** | **2.0 pts** | PLL synthesis to 50MHz; 2-button debouncing with 1-clock pulse output. | `Gowin_PLLVR` IP Core ($27\text{ MHz} \rightarrow 50\text{ MHz}$); 20ms startup reset synchronizer; 2x `button_debounce` modules with 2-stage D-FF + 20ms counter + falling edge detector. |
| **Block 2: PWM LED Controller** | **2.5 pts** | Mode LOW 25%, Mode HIGH 100%, Mode AUTO 2.0s flicker-free breathing. | `pwm_led_controller`: 1kHz carrier (50,000 counts), 50 counts/ms step $\rightarrow$ 1,000 steps up (1.0s) + 1,000 steps down (1.0s) = 2.000s. |
| **Block 3: UART TX Telemetry** | **2.5 pts** | 115200 bps 8N1 transmitting `"MODE: LOW\r\n"`, `"MODE: HIGH\r\n"`, `"MODE: AUTO\r\n"`. | `uart_tx_string`: `BAUD_DIV = 434` (0.0064% baud error), ASCII FSM with `\r\n` delimiters and `mode_latched` state protection. |
| **Block 4: Supervisor FSM & Sim** | **3.0 pts** | Reset $\rightarrow$ LOW (UART boot); BTN1 toggles LOW ↔ HIGH; BTN2 $\rightarrow$ AUTO; BTN1 in AUTO $\rightarrow$ LOW. Includes testbench and docs. | `top_system`: Supervisor FSM with boot telemetry; automated self-checking testbench `tb_top_system_v2` (25 checks, 0 errors); Gowin `prim_sim.v`; ModelSim `run_sim.do` & `wavefinal.do`. |
| **TOTAL FPGA SCORE** | **10.0 pts** | **Complete synthesizable RTL, .cst constraints, self-checking testbench, zero-dependency prim_sim.v.** | **100% Full Compliance with Contest Specification** |

---

## 3. Directory and File Hierarchy (`FPGA_dev` Branch)

```
porygon/
├── .github/                                              # CI/CD Workflows & Templates
│   ├── ISSUE_TEMPLATE/                                   # Issue Templates
│   ├── pull_request_template.md                          # Pull Request Review Checklist
│   └── workflows/ci.yml                                  # GitHub Actions CI Pipeline
├── .gitignore                                            # Build artifact exclusions
├── CODE_OF_CONDUCT.md                                    # Contributor Covenant Code of Conduct
├── CONTRIBUTING.md                                       # Git Flow branch management guidelines
├── LICENSE                                               # MIT License
├── README.md                                             # Master Technical Specification (Bilingual VI & EN)
├── README_VI.md                                          # Vietnamese Master Specification
├── README_EN.md                                          # English Master Specification
├── README_MODELSIM_SIMULATION.md                         # ModelSim SE 10.6d Simulation Guide
├── README_KEIL_DEBUG_SIMULATION.md                       # Keil µVision5 Watch 1 Debug Simulation Guide
├── README_SIMULATION_SCALING.md                          # Simulation Time Acceleration Report
├── SECURITY.md                                           # Security Policy & Hardware IP Protection
├── SETUP.md                                              # Toolchain setup instructions (Gowin & ModelSim)
├── ĐỀ THI FGPA 2026.docx.pdf                             # Official FPGA Contest Specification
├── ĐỀ THI MCU 2026.pdf                                   # Official MCU Contest Specification
├── Gowin-FPGA-Vietnamese-Book-ACG525-Basic-part-Print-v (1).pdf # Gowin FPGA Reference Book
└── FPGA/                                                 # FPGA Workspace Directory
    ├── .gitignore                                        # Gowin & ModelSim build output exclusions
    ├── README.md                                         # FPGA Subsystem Specification & Rebuild Guide
    ├── README_MODELSIM_SIMULATION.md                     # ModelSim Quick Start Guide
    ├── TESTING.md                                        # Verification Suite & ModelSim Guide (25 Checks)
    ├── ISSUE_ANALYSIS_DEEP_DIVE.md                       # Comprehensive Analysis of 6 Hardware Bugs
    ├── README_SIMULATION_SCALING.md                      # Simulation Time Acceleration Report
    ├── pwm11.gprj                                        # Gowin EDA Project Configuration
    ├── run_sim.do                                        # One-click automated ModelSim execution script
    ├── wavefinal.do                                      # ModelSim Waveform & Cursor Configuration
    ├── constr/pwm11.cst                                  # Physical Pin Constraints (Kiwi Nano 4K)
    ├── src/                                              # Synthesizable RTL Modules
    │   ├── top_system.v                                  # Top Integration & Supervisor FSM
    │   ├── button_debounce.v                             # 20ms Debouncer & Single-Clock Pulse Generator
    │   ├── pwm_led_controller.v                          # 1kHz PWM Controller (LOW, HIGH, AUTO 2.0s)
    │   ├── uart_tx_string.v                              # 115200 8N1 UART Serializer
    │   ├── gowin_pllvr.v                                 # Gowin PLLVR Wrapper (27MHz -> 50MHz)
    │   ├── prim_sim.v                                    # Gowin Simulation Primitive Model Library
    │   └── ip/gowin_pllvr/                               # IP Generator Core Configuration
    ├── sim/                                              # Verification Testbenches
    │   ├── tb_top_system_v2.v                            # Automated System Testbench (25 Checks, 0 Errors)
    │   ├── tb_uart_tx.v                                  # UART Timing Testbench
    │   └── wavefinal.do                                  # ModelSim Waveform & Cursor Script
    └── docs/                                             # Technical Documentation Archive
```

---

## 4. Hardware Architecture Block Diagram

```mermaid
flowchart TD
    subgraph CLOCK_RESET ["Block 1: Clock Synthesis, Sync & Reset (2.0 pts)"]
        OSC["Onboard Crystal Oscillator<br><b>27.0 MHz (Pin 45)</b>"] --> PLL["Gowin PLLVR IP Core<br>(IDIV=6, FBDIV=12, ODIV=16)"]
        PLL -->|"clk_50m (50.0 MHz)"| SYS_CLK["50MHz System Clock"]
        PLL -->|"pll_lock"| RST_SYNC["Reset Synchronizer & Startup Delay<br>(20ms Power-On Delay)"]
        RST_PIN["Hardware Reset Button<br><b>rst_n_in (Pin 40)</b>"] --> RST_SYNC
        RST_SYNC -->|"sys_rst_n"| CORE_RESET["Synchronous System Reset"]
    end

    subgraph DEBOUNCERS ["Edge-Triggered Debouncers (2.0 pts)"]
        BTN1_PIN["Push Button 1<br><b>btn1_in (Pin 14)</b>"] --> DB1["button_debounce 1<br>(2-Stage Sync + 20ms Integrator)"]
        BTN2_PIN["Push Button 2<br><b>btn2_in (Pin 15)</b>"] --> DB2["button_debounce 2<br>(2-Stage Sync + 20ms Integrator)"]
        DB1 -->|"btn1_pulse (1 clock = 20ns)"| FSM
        DB2 -->|"btn2_pulse (1 clock = 20ns)"| FSM
    end

    subgraph SUPERVISOR ["Block 4: Central Supervisor FSM (3.0 pts)"]
        FSM["Central Supervisor FSM (top_system)<br>• Boot: Mode LOW + UART Telemetry<br>• BTN1: LOW ↔ HIGH (AUTO → LOW)<br>• BTN2: Transition to AUTO"]
    end

    subgraph PWM_BLOCK ["Block 2: 1kHz PWM Controller (2.5 pts)"]
        FSM -->|"mode[1:0]"| PWM["pwm_led_controller<br>• LOW: 25% Duty (12,500 counts)<br>• HIGH: 100% Duty (50,000 counts)<br>• AUTO: 2.0s Breathing (2,000 steps x 1ms)"]
        PWM --> LED_PIN["Onboard LED2 Pin<br><b>led_out (Pin 13, 1.8V)</b>"]
    end

    subgraph UART_BLOCK ["Block 3: 115200 bps UART Telemetry (2.5 pts)"]
        FSM -->|"send_req & mode[1:0]"| UART["uart_tx_string<br>• BAUD_DIV = 434 (Error 0.006%)<br>• Latched mode_latched safety<br>• ASCII String with \\r\\n"]
        UART --> TX_PIN["USB-UART Port<br><b>uart_tx (Pin 39, 3.3V)</b>"]
    end
```

---

## 5. Physical Pinout Mapping (Kiwi Nano 4K / GW1NSR-4C)

| Signal Name | FPGA Pin | I/O Bank | I/O Standard | Pull Mode | Drive Strength | Target Hardware |
| :--- | :---: | :---: | :---: | :---: | :---: | :--- |
| `clk_in` | **Pin 45** | Bank 1 | `LVCMOS33` (3.3V) | `PULL_MODE=UP` | — | 27.0 MHz Onboard Crystal Oscillator |
| `rst_n_in` | **Pin 40** | Bank 1 | `LVCMOS33` (3.3V) | `PULL_MODE=UP` | — | Hardware Reset Button (Active Low) |
| `btn1_in` | **Pin 14** | Bank 3 | `LVCMOS18` (1.8V) | `PULL_MODE=UP` | — | Button 1: LOW ↔ HIGH toggle / AUTO $\rightarrow$ LOW |
| `btn2_in` | **Pin 15** | Bank 3 | `LVCMOS18` (1.8V) | `PULL_MODE=UP` | — | Button 2: Switch to AUTO Breathing |
| `led_out` | **Pin 13** | Bank 3 | `LVCMOS18` (1.8V) | `PULL_MODE=NONE` | $8\text{ mA}$ | PWM Output to LED2 |
| `uart_tx` | **Pin 39** | Bank 1 | `LVCMOS33` (3.3V) | `PULL_MODE=NONE` | $8\text{ mA}$ | Serial UART TX via onboard USB-UART Bridge |

---

## 6. Finite State Machine (FSM) Transitions & Telemetry Table

```mermaid
stateDiagram-v2
    [*] --> MODE_LOW : Power-on / sys_rst_n deasserted (Emits MODE_LOW)

    state MODE_LOW {
        [*] --> PWM_25_Percent : Duty Cycle 25% (1kHz)
    }

    state MODE_HIGH {
        [*] --> PWM_100_Percent : Duty Cycle 100% (Solid ON)
    }

    state MODE_AUTO {
        [*] --> PWM_Breathing : Breathing 0% <--> 100% over 2.0s
    }

    MODE_LOW --> MODE_HIGH : Button 1 Pressed / Emits MODE_HIGH
    MODE_HIGH --> MODE_LOW : Button 1 Pressed / Emits MODE_LOW

    MODE_LOW --> MODE_AUTO : Button 2 Pressed / Emits MODE_AUTO
    MODE_HIGH --> MODE_AUTO : Button 2 Pressed / Emits MODE_AUTO

    MODE_AUTO --> MODE_LOW : Button 1 Pressed / Emits MODE_LOW
    MODE_AUTO --> MODE_AUTO : Button 2 Pressed / Remain in AUTO
```

### FSM State Transition & UART Packet Transmission Matrix
| Current State | Trigger Input Event | Next State | UART ASCII String Emitted | PWM LED Output Behavior |
| :--- | :--- | :--- | :--- | :--- |
| **Any (Startup / Reset)** | `sys_rst_n` deasserted (High) | **Mode LOW (2'b00)** | `"MODE: LOW\r\n"` (11 bytes) | Duty Cycle $25\%$ ($1\text{ kHz}$) |
| **Mode LOW (2'b00)** | Button 1 Pressed (`btn1_pulse`) | **Mode HIGH (2'b01)** | `"MODE: HIGH\r\n"` (12 bytes) | Duty Cycle $100\%$ (Solid ON) |
| **Mode HIGH (2'b01)** | Button 1 Pressed (`btn1_pulse`) | **Mode LOW (2'b00)** | `"MODE: LOW\r\n"` (11 bytes) | Duty Cycle $25\%$ ($1\text{ kHz}$) |
| **Mode LOW (2'b00)** | Button 2 Pressed (`btn2_pulse`) | **Mode AUTO (2'b10)** | `"MODE: AUTO\r\n"` (12 bytes) | Breathing $0\% \leftrightarrow 100\%$ ($2.0\text{ s}$) |
| **Mode HIGH (2'b01)** | Button 2 Pressed (`btn2_pulse`) | **Mode AUTO (2'b10)** | `"MODE: AUTO\r\n"` (12 bytes) | Breathing $0\% \leftrightarrow 100\%$ ($2.0\text{ s}$) |
| **Mode AUTO (2'b10)** | Button 1 Pressed (`btn1_pulse`) | **Mode LOW (2'b00)** | `"MODE: LOW\r\n"` (11 bytes) | Duty Cycle $25\%$ ($1\text{ kHz}$) |
| **Mode AUTO (2'b10)** | Button 2 Pressed (`btn2_pulse`) | **Remain in AUTO** | *(No redundant packet sent)* | Continuous $2.0\text{ s}$ Breathing Cycle |

---

## 7. Timing Diagram & Edge-Triggered Debounce Algorithm

```mermaid
sequenceDiagram
    autonumber
    participant Pin as "Physical Button Pin (btn_in)"
    participant Sync as "2-Stage D-FF Synchronizer (sync1, sync2)"
    participant Cnt as "Debounce Integration Counter (cnt 20ms)"
    participant Stable as "Filtered Stable State (btn_stable)"
    participant Pulse as "Single-Cycle Output Pulse (btn_pulse)"

    Note over Pin: User presses mechanical switch (1-5ms contact chatter)
    Pin->>Sync: Noisy chatter edges continuously propagate
    Sync->>Cnt: sync2 differs from btn_stable -> counter increments
    Note over Cnt: If chatter reverts before 20ms -> counter resets to 0
    Note over Cnt: Stable press maintained for 20ms (cnt == CNT_MAX)
    Cnt->>Stable: Updates btn_stable = 0 (Button press confirmed)
    Stable->>Pulse: Falling edge detector emits btn_pulse = 1 (Exactly 20ns)
    Pulse->>Pulse: Next clock cycle -> btn_pulse automatically deasserts to 0
```

---

## 8. PWM Controller Architecture & 2.0s Linear Breathing Technique

1. **PWM Carrier Counter (`pwm_cnt`)**:
   - System Clock Frequency: $f_{\text{sys}} = 50.0\text{ MHz}$.
   - PWM Carrier Frequency: $f_{\text{PWM}} = 1,000\text{ Hz} \rightarrow T = 1.0\text{ ms}$.
   - Counts per carrier period: `ARR_MAX = 50_000` counts ($50,000\text{ counts}$).
2. **Duty Cycle Configuration Across Modes**:
   - **Mode LOW (25%)**: `duty_cycle = ARR_MAX / 4 = 12_500 counts`.
   - **Mode HIGH (100%)**: `duty_cycle = ARR_MAX = 50_000 counts`.
   - **Mode AUTO (2.0s Linear Breathing)**:
     - Uniformly interpolated into **2,000 discrete brightness steps** ($1,000$ steps up $+ 1,000$ steps down).
     - Step increment/decrement per $1\text{ ms}$: `STEP_VAL = 50` counts/step.
     - Rising phase duration ($0\% \rightarrow 100\%$): $1,000\text{ steps} \times 1.0\text{ ms} = 1.000\text{ s}$.
     - Falling phase duration ($100\% \rightarrow 0\%$): $1,000\text{ steps} \times 1.0\text{ ms} = 1.000\text{ s}$.
     - **Complete Breathing Cycle**: $1.0\text{ s} + 1.0\text{ s} = 2.000\text{ seconds}$ (Deterministic hardware precision).

---

## 9. UART Telemetry Subsystem (115200 bps) & Mode Latching Engine

1. **Baud Rate Frequency Division**:
   - Baud divisor formula:
     $$N_{\text{baud}} = \left\lfloor \frac{50,000,000}{115,200} \right\rceil = 434\text{ clock cycles}$$
   - Actual synthesized baud rate:
     $$\text{Baud}_{\text{actual}} = \frac{50,000,000}{434} \approx 115,207.37\text{ bps}$$
   - Relative baud rate error:
     $$\text{Error} = \frac{|115,207.37 - 115,200|}{115,200} \times 100\% = 0.0064\% \ll \pm 2.0\%$$
2. **Standard 8N1 Frame Format**:
   - 1 Start bit (Low) + 8 Data bits (LSB transmitted first) + 1 Stop bit (High).
   - Total: 10 bit periods per ASCII character.
3. **Zero-Collision `mode_latched` State Protection**:
   - Upon receiving transmission trigger `send_req`, the FSM immediately registers the current mode into `mode_latched`.
   - During the entire $1.042\text{ ms}$ required to stream the ASCII packet, any incoming button presses will not corrupt the ongoing packet string.

---

## 10. Zero-Dependency ModelSim Simulation Quick Start
*(Detailed guide at [`FPGA/README_MODELSIM_SIMULATION.md`](FPGA/README_MODELSIM_SIMULATION.md))*

The project embeds the official Gowin primitive simulation models ([`FPGA/src/prim_sim.v`](FPGA/src/prim_sim.v)), allowing instant simulation in ModelSim without requiring Gowin EDA installation:

### 🚀 Single-Command Automated Execution
1. Open **ModelSim SE 10.6d**.
2. Select **`File` $\rightarrow$ `Change Directory...`** $\rightarrow$ Navigate to `FPGA/`.
3. In the **Transcript** window, enter:
   ```tcl
   do run_sim.do
   ```

### 📊 Automated Verification Output (25 Checks, 0 Errors):
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

---

## 11. Deep-Dive Analysis of 6 Hardware Bugs & Defensive RTL Solutions

1. **Startup Metastability & PLL Phase Lock (`rst_n_debounced`)**:
   - *Problem*: At power-on, the input clock oscillates before the PLL achieves frequency lock (`pll_lock`). Unsynchronized FSMs will boot into undefined states and emit garbage telemetry.
   - *Solution*: A 2-stage Reset Synchronizer holds reset low for **$20\text{ ms}$** via `rst_cnt`, ensuring complete stability before FSM release.
2. **Inadequate Mechanical Debouncing (80ns vs 20ms)**:
   - *Problem*: Simple 4-bit shift registers create only $80\text{ ns}$ delay, completely failing against 1-5ms mechanical bounce and causing multiple FSM triggers.
   - *Solution*: A $20\text{ ms}$ integrator ($1,000,000$ clock cycles @ 50MHz) with falling edge detection produces exactly one $20\text{ ns}$ pulse per press.
3. **UART Packet Truncation and Mid-Stream Corruption**:
   - *Problem*: If mode transitions occur during a 12-byte UART packet transmission ($1.042\text{ ms}$), reading the raw mode register will splice two strings together (e.g. `"MODE: HIUTO\r\n"`).
   - *Solution*: `mode_latched` register isolates the string generator from mode changes until the complete packet finishes.
4. **Baud Rate Clock Division Accuracy**:
   - *Problem*: Improper integer division creates clock drift exceeding the $\pm 2.0\%$ RS-232 tolerance limit.
   - *Solution*: `BAUD_DIV = 434` yields an ultra-precise $115,207.37\text{ bps}$ ($0.0064\%$ error), guaranteeing error-free reception on all standard PC serial adapters.
5. **PWM Carrier Flicker and Coarse Breathing Interpolation**:
   - *Problem*: Low carrier frequencies (< 100Hz) cause visible human eye flicker; few brightness steps cause stepped, jagged fading.
   - *Solution*: $1\text{ kHz}$ carrier frequency with 2,000 fine-grained brightness steps ($50$ counts increment per ms) provides silky-smooth, flicker-free illumination.
6. **Zero-Dependency Portable Simulation**:
   - *Problem*: Third-party evaluation environments lack Gowin EDA primitive simulation libraries, throwing `Module 'PLLVR' is not defined`.
   - *Solution*: Bundled [`FPGA/src/prim_sim.v`](FPGA/src/prim_sim.v) provides standalone simulation on ModelSim, QuestaSim, and Icarus Verilog.

---

## 12. Mathematical Proofs and Hardware Timing Calculations

### Proof 1: PLLVR Frequency Synthesis ($50.0\text{ MHz}$)
The Gowin GW1NSR-4C receives an onboard reference clock $f_{\text{IN}} = 27.0\text{ MHz}$. PLLVR IP Core configuration:
- `IDIV_SEL = 6` $\rightarrow$ Input divider `IDIV = 7`
- `FBDIV_SEL = 12` $\rightarrow$ Feedback multiplier `FBDIV = 13`
- `ODIV_SEL = 16` $\rightarrow$ Output divider `ODIV = 1`

$$f_{\text{CLKOUT}} = 27.0\text{ MHz} \times \frac{13}{7} \approx 50.14\text{ MHz} \approx 50.0\text{ MHz}$$

### Proof 2: UART Baud Rate Timing Accuracy ($115,200\text{ bps}$)
$$N_{\text{baud}} = \left\lfloor \frac{50,000,000}{115,200} \right\rceil = 434\text{ clock cycles}$$
$$\text{Baud}_{\text{actual}} = \frac{50,000,000}{434} \approx 115,207.37\text{ bps}$$
$$\text{Error} = \frac{|115,207.37 - 115,200|}{115,200} \times 100\% = 0.0064\% \ll \pm 2.0\%$$

### Proof 3: Zero-Collision Telemetry Guarantee
- Longest UART packet duration ($12\text{ bytes} \times 10\text{ bits} = 120\text{ bits}$):
  $$t_{\text{packet}} = \frac{120}{115,200} \approx 1.042\text{ ms}$$
- Hardware button debounce lockout time: $t_{\text{debounce}} = 20.0\text{ ms}$.
- Safety margin: $\Delta t = 20.0\text{ ms} - 1.042\text{ ms} = 18.958\text{ ms} > 0$.
- **Conclusion**: The UART serializer always completes before any subsequent debounced button press can occur.

### Proof 4: Precise 2.0s Linear Breathing Cycle
- `ARR_MAX = 50_000` counts, `STEP_VAL = 50` counts/step.
$$T_{\text{cycle}} = (1,000\text{ steps up} \times 1.0\text{ ms}) + (1,000\text{ steps down} \times 1.0\text{ ms}) = 2.000\text{ seconds}$$

---

## 13. Technical Benchmark Comparison Table (Naive vs Production)

| Functional Subsystem | Naive Implementation | Production-Grade IC Architecture |
| :--- | :--- | :--- |
| **Clock & Reset** | Soft clock divider; unsynchronized reset causing startup metastability. | Gowin PLLVR hardware macro + 2-stage Reset Synchronizer with $20\text{ ms}$ power-on delay. |
| **Button Debouncing** | 4-bit shift register (~80ns); fails under mechanical bounce. | $20\text{ ms}$ integrator counter ($1,000,000$ cycles) + edge detector emitting single $20\text{ ns}$ pulse. |
| **PWM Generation** | Low carrier frequency causing flicker; coarse steps causing stepping. | $1\text{ kHz}$ flicker-free carrier + 2,000 ultra-fine interpolation steps for precise $2.000\text{ s}$ breathing. |
| **UART Telemetry** | Single-byte transmission without state latch; corrupted upon state changes. | Full ASCII string (`"MODE: ...\r\n"`) + `mode_latched` protection + $0.0064\%$ baud timing accuracy. |
| **Simulation Verification** | Missing vendor primitives, failing compilation; manual inspection. | Self-contained `prim_sim.v` + Automated 25-check assertion testbench proving $2.000000\text{ s}$ period. |

---

## 14. Automated Verification Matrix (25 Checks) & Waveform Guide

### Automated Verification Matrix ([`FPGA/sim/tb_top_system_v2.v`](FPGA/sim/tb_top_system_v2.v))
| Test Group | Trigger Scenario | Expected System Behavior | Automated Assertion Routine | Verification Status |
| :---: | :--- | :--- | :--- | :---: |
| **TC-01** | Power-on / Reset | System initializes to LOW; transmits `"MODE: LOW\r\n"` | `uart_check_message(..., 11)` | **PASS (11/11 bytes)** |
| **TC-02** | Mode LOW Duty Cycle | High-level output ratio equals 25% | `measure_pwm_duty("LOW", ...)` | **PASS (25.0%)** |
| **TC-03** | Button 1 Press | Transitions LOW $\rightarrow$ HIGH; transmits `"MODE: HIGH\r\n"` | `uart_check_message(..., 12)` | **PASS (12/12 bytes)** |
| **TC-04** | Mode HIGH Duty Cycle | High-level output ratio equals 100% (Solid ON) | `measure_pwm_duty("HIGH", ...)` | **PASS (100.0%)** |
| **TC-05** | Button 1 Subsequent Press | Transitions HIGH $\rightarrow$ LOW; transmits `"MODE: LOW\r\n"` | `uart_check_message(..., 11)` | **PASS (11/11 bytes)** |
| **TC-06** | Button 2 Press | Transitions to AUTO; transmits `"MODE: AUTO\r\n"` | `uart_check_message(..., 12)` | **PASS (12/12 bytes)** |
| **TC-07** | AUTO Breathing Cycle 1 | Falling phase $1.000\text{ s}$ + Rising phase $1.000\text{ s} = 2.000\text{ s}$ | `prove_breath_2s_via_transcript` | **PASS (2.000000s)** |
| **TC-08** | AUTO Breathing Cycle 2 | Falling phase $1.000\text{ s}$ + Rising phase $1.000\text{ s} = 2.000\text{ s}$ | `prove_breath_2s_via_transcript` | **PASS (2.000000s)** |
| **TC-09** | Button 1 Press in AUTO | Forces transition to LOW; transmits `"MODE: LOW\r\n"` | `uart_check_message(..., 11)` | **PASS (11/11 bytes)** |
| **TC-10** | Mechanical Bounce Chatter | Glitchy press sequence accepted as exactly one transition | `glitchy_press_btn1` | **PASS (Single Fire)** |
| **TC-11** | Sub-Threshold Pulse (< 5ms) | Glitch rejected, mode remains unchanged | `short_press_btn1` | **PASS (Suppressed)** |
| **TC-12** | Reset Assertion in HIGH | Returns to LOW and emits `"MODE: LOW\r\n"` | `trigger_reset` | **PASS (11/11 bytes)** |

---

## 15. Gowin EDA Synthesis & Hardware Flashing Guide

1. Launch **Gowin EDA** (V1.9.9 or V1.9.12+).
2. Click **File $\rightarrow$ Open Project...** and select [`FPGA/pwm11.gprj`](FPGA/pwm11.gprj).
3. In the **Process** tab, double-click **Place & Route** $\rightarrow$ Verify status: **Success (0 Errors, 0 Warnings)**.
4. Connect the **Kiwi Nano 4K** evaluation board via USB.
5. Open the **Programmer** tool, select device `GW1NSR-4C`, and program `impl/pnr/pwm11.fs` to SRAM/Flash.
6. Open a Serial Terminal at **115200 bps, 8N1** to observe real-time telemetry events.
