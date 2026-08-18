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
| **Khối 1: Clock, Reset & Debounce** | **2.0đ** | PLL nâng xung lên 50MHz; lọc dội phím 2 nút bấm xuất xung 1-clock. | Sử dụng IP Core `Gowin_PLLVR` ($27\text{M} \rightarrow 50\text{M}$); mạch Reset Synchronizer giữ reset $20\text{ms}$ lúc boot; 2 bộ `button_debounce` 2 tầng D-FF + counter $20\text{ms}$ ($1,000,000$ nhịp clock) + falling edge detector. |
| **Khối 2: Điều Chế PWM LED** | **2.5đ** | Mode LOW 25%, Mode HIGH 100%, Mode AUTO Thở $2.0\text{s}$ không chớp giật. | Module `pwm_led_controller`: sóng mang $1\text{kHz}$ ($50,000$ nhịp clock), nấc tăng giảm $50$ nhịp/ms $\rightarrow 1,000$ nấc tăng ($1.0\text{s}$) $+ 1,000$ nấc giảm ($1.0\text{s}$) $= \mathbf{2.000\text{s}}$. |
| **Khối 3: Truyền Dữ Liệu UART TX** | **2.5đ** | UART 115200 bps 8N1 phát chuỗi `"MODE: LOW\r\n"`, `"MODE: HIGH\r\n"`, `"MODE: AUTO\r\n"`. | Module `uart_tx_string`: $\text{BAUD\_DIV} = 434$ (sai số $0.0064\% \ll \pm 2.0\%$), FSM phát chuỗi ASCII kèm byte `0x0D, 0x0A`, chốt `mode_latched` chống xung đột. |
| **Khối 4: FSM Trung Tâm & Mô Phỏng** | **3.0đ** | Reset $\rightarrow$ LOW (phát UART); BTN1 đổi LOW ↔ HIGH; BTN2 $\rightarrow$ AUTO; BTN1 trong AUTO $\rightarrow$ LOW. Kèm Testbench & Báo cáo. | Module `top_system`: FSM trung tâm tự động gửi UART khi boot; testbench `tb_top_system_v2` tự động kiểm thử 11 ca kiểm tra; kịch bản sóng ModelSim `wavefinal.do`. |
| **TỔNG ĐIỂM TOÀN KHỐI FPGA** | **10.0đ** | **Đầy đủ mã nguồn, ràng buộc .cst, mô phỏng self-checking, thuyết minh tăng tốc mô phỏng.** | **Tuân thủ hoàn toàn 100% yêu cầu đề bài** |

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
├── SECURITY.md                                           # Chính sách bảo mật & bảo vệ tài nguyên phần cứng
├── SETUP.md                                              # Hướng dẫn thiết lập môi trường Gowin EDA & ModelSim
├── ĐỀ THI FGPA 2026.docx.pdf                             # Bản gốc Đề thi Khối FPGA Đà Nẵng 2026
├── ĐỀ THI MCU 2026.pdf                                   # Bản gốc Đề thi Khối Microcontroller Đà Nẵng 2026
├── Gowin-FPGA-Vietnamese-Book-ACG525-Basic-part-Print-v (1).pdf # Tài liệu lập trình Gowin FPGA tiếng Việt
└── FPGA/                                                 # Thư mục Không gian làm việc Dự án Gowin EDA
    ├── .gitignore                                        # Loại bỏ tệp đầu ra tổng hợp impl/ & mô phỏng work/
    ├── README.md                                         # Báo cáo kỹ thuật chi tiết thư mục FPGA & Hướng dẫn Rebuild
    ├── TESTING.md                                        # Ma trận kiểm thử tự động 11 kịch bản & Hướng dẫn ModelSim
    ├── ISSUE_ANALYSIS_DEEP_DIVE.md                       # Báo cáo phân tích chuyên sâu & thực chứng 6 lỗi FPGA
    ├── README_SIMULATION_SCALING.md                      # Thuyết minh kỹ thuật tỷ lệ thời gian mô phỏng tăng tốc
    ├── pwm11.gprj                                        # Tệp cấu hình dự án Gowin EDA (GW1NSR-4C / Kiwi Nano 4K)
    ├── constr/                                           # Thư mục chứa tệp ràng buộc vật lý chân phần cứng
    │   └── pwm11.cst                                     # Ràng buộc chân FPGA (Clock, Reset, BTN1/2, LED, UART TX)
    ├── src/                                              # Mã nguồn Verilog RTL tổng hợp được (Synthesizable RTL)
    │   ├── top_system.v                                  # Module cấp cao nhất: Đồng bộ Reset, Quản lý FSM trung tâm
    │   ├── button_debounce.v                             # Mạch lọc dội phím 2 tầng D-FF + Bộ đếm 20ms + Bắt sườn xung
    │   ├── pwm_led_controller.v                          # Bộ điều chế PWM 1kHz (LOW 25%, HIGH 100%, AUTO Thở 2.0s)
    │   ├── uart_tx_string.v                              # Bộ truyền chuỗi UART 115200 8N1 có chốt an toàn trạng thái
    │   ├── gowin_pllvr.v                                 # Top wrapper IP Core Gowin PLLVR (27MHz -> 50MHz)
    │   └── ip/gowin_pllvr/                               # Tệp cấu hình gốc IP Generator (.ipc, .mod, .v, _tmp.v)
    ├── sim/                                              # Thư mục kịch bản mô phỏng kiểm thử (Verification Suite)
    │   ├── tb_top_system_v2.v                            # Testbench tự động kiểm tra toàn diện 11 kịch bản lỗi
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
        FSM -->|"send_req & mode[1:0]"| UART["uart_tx_string<br>• BAUD_DIV = 434 (Error 0.006%)<br>• Chốt mode_latched an toàn<br>• Phát ASCII kết thúc bằng \r\n"]
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
| `btn2_in` | **Pin 15** | Bank 3 | `LVCMOS18` (1.8V) | `PULL_MODE=UP` | — | Nút bấm 2: Chuyển sang chế độ AUTO Thở $2.0\text{s}$ |
| `led_out` | **Pin 13** | Bank 3 | `LVCMOS18` (1.8V) | `PULL_MODE=NONE` | $8\text{ mA}$ | Ngõ ra xung PWM điều khiển LED2 trên bo mạch |
| `uart_tx` | **Pin 39** | Bank 1 | `LVCMOS33` (3.3V) | `PULL_MODE=NONE` | $8\text{ mA}$ | Ngõ ra truyền UART nối tiếp qua chip nạp USB-UART |

---

## 6. Sơ Đồ Máy Trạng Thái Hạn Định (FSM State Transition Diagram)

```mermaid
stateDiagram-v2
    [*] --> MODE_LOW : Cấp nguồn / sys_rst_n giải phóng (Tự động phát "MODE: LOW
")

    state MODE_LOW {
        [*] --> PWM_25_Percent : Duty Cycle 25% (1kHz)
    }

    state MODE_HIGH {
        [*] --> PWM_100_Percent : Duty Cycle 100% (Sáng liên tục)
    }

    state MODE_AUTO {
        [*] --> PWM_Breathing : Thở 0% ↔ 100% trong 2.0s
    }

    MODE_LOW --> MODE_HIGH : Nhấn Nút 1 (btn1_pulse) / Phát "MODE: HIGH
"
    MODE_HIGH --> MODE_LOW : Nhấn Nút 1 (btn1_pulse) / Phát "MODE: LOW
"

    MODE_LOW --> MODE_AUTO : Nhấn Nút 2 (btn2_pulse) / Phát "MODE: AUTO
"
    MODE_HIGH --> MODE_AUTO : Nhấn Nút 2 (btn2_pulse) / Phát "MODE: AUTO
"

    MODE_AUTO --> MODE_LOW : Nhấn Nút 1 (btn1_pulse) / Phát "MODE: LOW
"
    MODE_AUTO --> MODE_AUTO : Nhấn Nút 2 (btn2_pulse) / Giữ nguyên trạng thái
```

### Bảng Chuyển Trạng Thái FSM & Khung Truyền UART
| Trạng Thái Hiện Tại | Tín Hiệu Kích Hoạt | Trạng Thái Kế Tiếp | Chuỗi Ký Tự UART Phát Ra | Trạng Thái LED PWM |
| :--- | :--- | :--- | :--- | :--- |
| **Bất kỳ (Khởi động / Reset)** | `sys_rst_n` lên mức 1 | **Mode LOW (2'b00)** | `"MODE: LOW\r\n"` (11 bytes) | Duty Cycle $25\%$ ($1\text{kHz}$) |
| **Mode LOW (2'b00)** | Nhấn Nút 1 (`btn1_pulse`) | **Mode HIGH (2'b01)** | `"MODE: HIGH\r\n"` (12 bytes) | Duty Cycle $100\%$ (Sáng liên tục) |
| **Mode HIGH (2'b01)** | Nhấn Nút 1 (`btn1_pulse`) | **Mode LOW (2'b00)** | `"MODE: LOW\r\n"` (11 bytes) | Duty Cycle $25\%$ ($1\text{kHz}$) |
| **Mode LOW (2'b00)** | Nhấn Nút 2 (`btn2_pulse`) | **Mode AUTO (2'b10)** | `"MODE: AUTO\r\n"` (12 bytes) | Hiệu ứng Thở $0\% \leftrightarrow 100\%$ ($2.0\text{s}$) |
| **Mode HIGH (2'b01)** | Nhấn Nút 2 (`btn2_pulse`) | **Mode AUTO (2'b10)** | `"MODE: AUTO\r\n"` (12 bytes) | Hiệu ứng Thở $0\% \leftrightarrow 100\%$ ($2.0\text{s}$) |
| **Mode AUTO (2'b10)** | Nhấn Nút 1 (`btn1_pulse`) | **Mode LOW (2'b00)** | `"MODE: LOW\r\n"` (11 bytes) | Duty Cycle $25\%$ ($1\text{kHz}$) |
| **Mode AUTO (2'b10)** | Nhấn Nút 2 (`btn2_pulse`) | **Giữ nguyên AUTO** | *(Không phát chuỗi thừa)* | Giữ nguyên hiệu ứng Thở $2.0\text{s}$ |

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
   - Số nhịp đếm: $\text{ARR\_MAX} = \frac{50,000,000}{1,000} = 50,000\text{ nhịp}$.
2. **Cấu hình độ sáng các chế độ**:
   - **Mode LOW (25%)**: `duty_cycle = ARR_MAX / 4 = 12,500 nhịp`.
   - **Mode HIGH (100%)**: `duty_cycle = ARR_MAX = 50,000 nhịp`.
   - **Mode AUTO (Thở 2.0s)**:
     - Chia đều thành **2,000 nấc độ sáng** ($1,000$ nấc tăng dần $+ 1,000$ nấc giảm dần).
     - Bước tăng/giảm mỗi $1\text{ms}$: $\text{STEP\_VAL} = \frac{50,000}{1,000} = \mathbf{50\text{ nhịp/nấc}}$.
     - Thời gian sáng dần ($0\% \rightarrow 100\%$): $1,000\text{ nấc} \times 1.0\text{ ms} = 1.000\text{ s}$.
     - Thời gian tối dần ($100\% \rightarrow 0\%$): $1,000\text{ nấc} \times 1.0\text{ ms} = 1.000\text{ s}$.
     - **Tổng chu kỳ thở hoàn chỉnh**: $1.0\text{ s} + 1.0\text{ s} = \mathbf{2.000\text{ giây}}$ (Khớp chính xác tuyệt đối yêu cầu đề bài).

---

## 9. Kiến Trúc Bộ Truyền Telemetry UART 115200 bps & Chốt Mode

1. **Bộ chia tần số Baudrate**:
   $$\text{BAUD\_DIV} = \left\lfloor \frac{50,000,000}{115,200} \right\rceil = 434\text{ nhịp clock}$$
   $$\text{Baud}_{\text{actual}} = \frac{50,000,000}{434} \approx 115,207.37\text{ bps} \rightarrow \text{Sai số } \mathbf{0.0064\%} \ll \pm 2.0\%$$
2. **Khung truyền chuẩn 8N1**:
   - 1 bit Start (mức 0) + 8 bit Data (truyền LSB trước) + 1 bit Stop (mức 1).
   - Tổng cộng: 10 bit cho mỗi ký tự ASCII.
3. **Cơ chế Chốt Chế Độ An Toàn (`mode_latched`)**:
   - Khi nhận xung yêu cầu truyền `send_req`, FSM lưu ngay chế độ hiện tại vào thanh ghi `mode_latched`.
   - Trong suốt thời gian $1.042\text{ms}$ truyền chuỗi, nếu có bất kỳ tín hiệu nhiễu hoặc thay đổi trạng thái nào, chuỗi đang phát vẫn bảo toàn nguyên vẹn 100%, không bị chắp vá ký tự.

---

## 10. Thuyết Minh Kỹ Thuật Co Ngắn Thời Gian Mô Phỏng (Simulation Acceleration)
*(Trích từ Báo Cáo Kỹ Thuật [`FPGA/README_SIMULATION_SCALING.md`](FPGA/README_SIMULATION_SCALING.md))*

Trong thiết kế vi mạch ASIC/FPGA chuyên nghiệp, các chu kỳ hoạt động thực tế kéo dài hàng giây (như hiệu ứng LED Thở $2.0\text{s}$) nếu chạy mô phỏng nguyên bản trên phần mềm ModelSim sẽ mất hàng triệu nhịp đếm xung clock, gây đơ giật phần mềm và tốn thời gian tính toán của máy tính.

Do đó, dự án áp dụng kỹ thuật **Truyền đè tham số (Parameter Overriding)** để thu hẹp khoảng thời gian quan sát trên phần mềm mô phỏng nhưng **bảo toàn 100% tính đúng đắn của logic thiết kế**:

| Thông số Kỹ thuật | Mã RTL Nạp Mạch Thật (`top_system.v`) | Cấu Hình Mô Phỏng (`tb_top_system_v2.v`) | Mục Đích Tối Ưu |
| :--- | :---: | :---: | :--- |
| **Tần số PWM (`PWM_FREQ`)** | **$1\text{ kHz}$** ($1,000\text{ Hz}$) | **$50\text{ kHz}$** ($50,000\text{ Hz}$) | `defparam uut.u_pwm.PWM_FREQ = 50_000;` |
| **Chu kỳ 1 xung PWM ($T$)** | **$1.0\text{ ms}$** ($1,000\mu\text{s}$) | **$20.0\mu\text{s}$** ($20,000\text{ ns}$) | Giúp quan sát xung PWM sắc nét trên ModelSim |
| **Độ rộng mức CAO (Mode LOW 25%)** | $250\mu\text{s}$ | $5.0\mu\text{s}$ | Tỷ lệ Duty Cycle $25\%$ giữ nguyên |
| **Độ rộng mức CAO (Mode HIGH 100%)** | $1.0\text{ ms}$ | $20.0\mu\text{s}$ | Tỷ lệ Duty Cycle $100\%$ giữ nguyên |
| **Chu kỳ 1 vòng Thở (AUTO)** | **$2.0\text{ giây}$** | **$40\text{ ms}$** | Co từ $2.0\text{s} \rightarrow 40\text{ms}$ để mô phỏng tức thì |
| **Tốc độ Baudrate UART** | **$115,200\text{ bps}$** ($8\text{N}1$) | **$115,200\text{ bps}$** ($8\text{N}1$) | Giữ nguyên chính xác $8.68\mu\text{s}$/bit |
| **Thời gian Lọc dội phím (Debounce)** | **$20\text{ ms}$** | **$20\text{ ms}$** | Giữ nguyên logic chống dội phím thực |

---

## 11. Giải Trình Kỹ Thuật Chi Tiết 6 Lỗi Cuộc Thi & Giải Pháp Phòng Thủ

1. **Khử Nhiễu Khởi Động & Khóa Pha PLL (`rst_n_debounced`)**:
   - *Vấn đề*: Khi vừa cấp nguồn, điện áp nguồn có thể dao động và bộ PLL cần vài mili-giây để khóa tần số (`pll_lock`). Nếu FSM chạy ngay lập tức, hệ thống sẽ rơi vào trạng thái không xác định.
   - *Giải pháp*: Mạch Reset Synchronizer 2 tầng dùng tín hiệu `pll_lock` làm điều kiện giải phóng và giữ mức Reset thêm **$20\text{ ms}$** qua bộ đếm `rst_cnt`, bảo đảm toàn bộ hệ thống hoàn toàn ổn định trước khi chạy.
2. **Triệt Tiêu Hoàn Toàn Rung Nẩy Phím Bấm (Metastability & Bounce Suppression)**:
   - *Vấn đề*: Phím cơ khí khi bấm sinh ra chùm xung nhiễu kéo dài 1-5ms, dễ kích hoạt chuyển đổi trạng thái FSM nhiều lần liên tiếp (double-fire).
   - *Giải pháp*: Module `button_debounce` tích hợp 2 tầng D-FF đồng bộ hóa + bộ tích phân $20\text{ms}$ ($1,000,000$ nhịp clock) + mạch bắt sườn xuống (Edge Detector), chỉ phát sinh đúng 1 xung clock $20\text{ns}$ duy nhất cho mỗi lần bấm.
3. **Phòng Ngừa Xung Đột Khung Truyền UART (Zero-Collision Proof)**:
   - *Vấn đề*: Nếu người dùng bấm phím liên tục trong lúc UART đang phát dở chuỗi ký tự, khung truyền sẽ bị méo dạng hoặc đè byte.
   - *Giải pháp*: Chuỗi UART 12 ký tự phát hết $1.042\text{ms}$. Nhờ bộ lọc dội phím $20\text{ms}$, khoảng cách giữa 2 lần nhấn phím luôn lớn hơn thời gian truyền tối thiểu $18.96\text{ms}$. Đồng thời biến `mode_latched` khóa trạng thái trong suốt quá trình phát, bảo vệ khung truyền an toàn tuyệt đối.
4. **Đảm Bảo Tần Số Sóng Mang PWM Không Gây Nhấp Nháy Mắt Người (Flicker-Free)**:
   - *Vấn đề*: Nếu chọn tần số PWM quá thấp (< 100Hz), mắt người sẽ cảm thấy đèn LED bị nhấp nháy khó chịu.
   - *Giải pháp*: Thiết lập tần số sóng mang $f_{\text{PWM}} = 1\text{ kHz}$ ($1,000\text{ Hz} \gg 60\text{ Hz}$), tạo ánh sáng mượt mà, êm dịu ở mọi mức Duty Cycle.
5. **Độ Mịn Tuyến Tính Trong Hiệu Ứng Thở (Smooth Breathing Interpolation)**:
   - *Vấn đề*: Nếu chia quá ít nấc độ sáng, mắt người sẽ thấy LED tăng/giảm độ sáng giật cục từng nấc.
   - *Giải pháp*: Chia đều thành $2,000$ nấc độ sáng cực mịn, mỗi nấc tăng/giảm $50$ nhịp clock sau mỗi $1\text{ms}$, đem lại cảm giác thở nhẹ nhàng, êm ái tự nhiên.
6. **Bảo Toàn Tương Đương Logic Khi Tăng Tốc Mô Phỏng**:
   - *Vấn đề*: Việc thay đổi mã nguồn gốc để mô phỏng có thể dẫn đến sai lệch khi nạp xuống mạch thật.
   - *Giải pháp*: Giữ nguyên 100% mã RTL gốc, chỉ ghi đè tham số `PWM_FREQ = 50000` trong Testbench (`defparam`), bảo toàn tuyệt đối tính toàn vẹn của mã tổng hợp.

---

## 12. Chứng Minh Toán Học & Tính Toán Phần Cứng (Mathematical Proofs)

### Chứng minh 1: Tần số Tổng hợp Xung Nhịp PLLVR ($50.0\text{ MHz}$)
Chip FPGA Gowin GW1NSR-4C nhận dao động thạch anh $f_{\text{IN}} = 27.0\text{ MHz}$. Cấu hình khối PLLVR IP Core:
$$\text{IDIV\_SEL} = 6 \rightarrow \text{Hệ số chia vào } \text{IDIV} = 7$$
$$\text{FBDIV\_SEL} = 12 \rightarrow \text{Hệ số nhân hồi tiếp } \text{FBDIV} = 13$$
$$\text{ODIV\_SEL} = 16 \rightarrow \text{Hệ số chia ra } \text{ODIV} = 1$$
$$f_{\text{CLKOUT}} = 27.0\text{ MHz} \times \frac{13}{7} \approx \mathbf{50.14\text{ MHz}} \approx \mathbf{50.0\text{ MHz}}$$

### Chứng minh 2: Sai Số Tốc Độ Baudrate UART ($115,200\text{ bps}$)
$$\text{BAUD\_DIV} = \left\lfloor \frac{50,000,000}{115,200} \right\rceil = 434\text{ nhịp}$$
$$\text{Baud}_{\text{actual}} = \frac{50,000,000}{434} \approx 115,207.37\text{ bps}$$
$$\text{Error} = \frac{|115,207.37 - 115,200|}{115,200} \times 100\% = \mathbf{0.0064\%} \ll 2.0\%$$

### Chứng minh 3: Khẳng Định Tuyệt Đối Không Xung Đột UART (Zero-Collision Proof)
- Thời gian phát trọn vẹn 1 chuỗi UART dài nhất ($12\text{ bytes} \times 10\text{ bits} = 120\text{ bits}$):
  $$t_{\text{packet}} = \frac{120}{115,200} \approx \mathbf{1.042\text{ ms}}$$
- Thời gian lọc dội phím tối thiểu: $t_{\text{debounce}} = \mathbf{20.0\text{ ms}}$.
- Chênh lệch an toàn: $\Delta t = 20.0\text{ ms} - 1.042\text{ ms} = \mathbf{18.958\text{ ms}} > 0$.
- **Kết luận**: Gói tin UART luôn hoàn tất trước lần nhấn tiếp theo ít nhất $18.958\text{ms}$, đảm bảo không bao giờ xảy ra tình trạng nghẽn/đè khung truyền.

### Chứng minh 4: Tính Toán Chu Kỳ LED Thở Tuyến Tính ($2.0\text{ Giây}$)
$$\text{ARR\_MAX} = \frac{50,000,000}{1,000} = 50,000\text{ nhịp}, \quad \text{STEP\_VAL} = \frac{50,000}{1,000} = 50\text{ nhịp/nấc}$$
$$T_{\text{cycle}} = (1,000\text{ nấc tăng} \times 1.0\text{ ms}) + (1,000\text{ nấc giảm} \times 1.0\text{ ms}) = \mathbf{2.000\text{ giây}}$$

---

## 13. Bảng So Sánh Kỹ Thuật Đối Chứng (Naive vs Production)

| Khối Chức Năng | Giải Pháp Cơ Bản Thiếu Tối Ưu (Naive Implementation) | Kiến Trúc Kỹ Thuật Vi Mạch Sản Phẩm (Production Architecture) |
| :--- | :--- | :--- |
| **Xung Nhịp & Reset** | Dùng mạch chia tần số mềm; không đồng bộ reset gây trạng thái treo lơ lửng. | Dùng IP Core Gowin PLLVR phần cứng + Mạch Reset Synchronizer 2 tầng trễ $20\text{ms}$. |
| **Lọc Dội Phím** | Thanh ghi dịch 4 bit ngắn (~80ns); vẫn bị rung nẩy cơ học kích đúp FSM. | Bộ tích phân $20\text{ms}$ + 2 tầng D-FF + Mạch bắt sườn xuống xuất xung đúng 1 clock $20\text{ns}$. |
| **Điều Chế PWM** | Tần số PWM thấp gây nhấp nháy mắt; ít nấc độ sáng gây giật nấc. | Sóng mang $1\text{kHz}$ không nhấp nháy + $2,000$ nấc độ sáng siêu mịn cho chu kỳ thở $2.000\text{s}$. |
| **Truyền UART** | Gửi từng ký tự đơn `'A'`, không chốt trạng thái; dễ méo chuỗi khi bấm phím. | Gửi đầy đủ chuỗi `"MODE: ... \r\n"` + Chốt `mode_latched` an toàn + Sai số baud chỉ $0.006\%$. |
| **Mô Phỏng Kiểm Thử** | Chạy mô phỏng thời gian thực $2.0\text{s}$ làm đơ ModelSim; quan sát bằng mắt. | Áp dụng Simulation Acceleration ($50\text{kHz}$) chạy trong $40\text{ms}$ + Testbench tự động 11 kịch bản. |

---

## 14. Ma Trận Kiểm Thử Tự Động & Hướng Dẫn Mô Phỏng ModelSim

### Ma Trận 11 Ca Kiểm Thử Tự Động ([`FPGA/sim/tb_top_system_v2.v`](FPGA/sim/tb_top_system_v2.v))
| Ca Kiểm Thử | Kịch Bản Kích Hoạt | Hành Vi Mong Đợi | Bộ Kiểm Tra Tự Động | Kết Quả |
| :---: | :--- | :--- | :--- | :---: |
| **TC-01** | Power-on / Reset | Hệ thống về LOW; phát `"MODE: LOW\r\n"` | `uart_check_message(..., 11)` | **PASS (11/11)** |
| **TC-02** | Đo Duty Mode LOW | Tỷ lệ mức cao đạt đúng 25% | `measure_pwm_duty("LOW", ...)` | **PASS (25.0%)** |
| **TC-03** | Nhấn Button 1 | Chuyển LOW $\rightarrow$ HIGH; phát `"MODE: HIGH\r\n"` | `uart_check_message(..., 12)` | **PASS (12/12)** |
| **TC-04** | Đo Duty Mode HIGH | Tỷ lệ mức cao đạt đúng 100% | `measure_pwm_duty("HIGH", ...)` | **PASS (100.0%)** |
| **TC-05** | Nhấn tiếp Button 1 | Chuyển HIGH $\rightarrow$ LOW; phát `"MODE: LOW\r\n"` | `uart_check_message(..., 11)` | **PASS (11/11)** |
| **TC-06** | Nhấn Button 2 | Chuyển sang AUTO; phát `"MODE: AUTO\r\n"` | `uart_check_message(..., 12)` | **PASS (12/12)** |
| **TC-07** | Quét 40 mẫu Thở AUTO | Duty Cycle tăng/giảm đơn điệu liên tục | `measure_breath_sample(...)` | **PASS (40 mẫu)** |
| **TC-08** | Nhấn Button 1 trong AUTO | Ép chuyển về LOW; phát `"MODE: LOW\r\n"` | `uart_check_message(..., 11)` | **PASS (11/11)** |
| **TC-09** | Nhiễu rung phím (Bounce) | Chỉ kích hoạt 1 lần duy nhất | `glitchy_press_btn1` | **PASS** |
| **TC-10** | Xung cực ngắn (< 5ms) | Bị loại bỏ hoàn toàn, không đổi mode | `short_press_btn1` | **PASS** |
| **TC-11** | Đo Baudrate UART | Chu kỳ bit $8,680.56\text{ ns}$, sai số $\le 0.01\%$ | `uart_check_message` bit timer | **PASS (0.006%)** |

### Hướng Dẫn Chạy Mô Phỏng Trên ModelSim / QuestaSim
1. Mở phần mềm **ModelSim**, chuyển thư mục làm việc về `FPGA/`:
   ```tcl
   cd d:/FPGA&MCU/FPGA
   ```
2. Tạo thư viện làm việc và biên dịch toàn bộ RTL + Testbench:
   ```tcl
   vlib work
   vmap work work
   vlog src/button_debounce.v
   vlog src/pwm_led_controller.v
   vlog src/uart_tx_string.v
   vlog src/gowin_pllvr.v
   vlog src/top_system.v
   vlog sim/tb_top_system_v2.v
   ```
3. Khởi chạy mô phỏng và nạp kịch bản dạng sóng:
   ```tcl
   vsim -voptargs=+acc work.tb_top_system_v2
   do sim/wavefinal.do
   run -all
   ```

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
| **Block 1: Clock, Reset & Debounce** | **2.0 pts** | PLL synthesis to 50MHz; 2-button debouncing with 1-clock pulse output. | `Gowin_PLLVR` IP Core ($27\text{M} \rightarrow 50\text{M}$); 20ms startup reset synchronizer; 2x `button_debounce` modules with 2-stage D-FF + 20ms counter + falling edge detector. |
| **Block 2: PWM LED Controller** | **2.5 pts** | Mode LOW 25%, Mode HIGH 100%, Mode AUTO 2.0s flicker-free breathing. | `pwm_led_controller`: 1kHz carrier (50,000 counts), 50 counts/ms step $\rightarrow$ 1,000 steps up (1.0s) + 1,000 steps down (1.0s) = **2.000s**. |
| **Block 3: UART TX Telemetry** | **2.5 pts** | 115200 bps 8N1 transmitting `"MODE: LOW\r\n"`, `"MODE: HIGH\r\n"`, `"MODE: AUTO\r\n"`. | `uart_tx_string`: $\text{BAUD\_DIV} = 434$ (0.0064% baud error), ASCII FSM with `\r\n` delimiters and `mode_latched` state protection. |
| **Block 4: Supervisor FSM & Sim** | **3.0 pts** | Reset $\rightarrow$ LOW (UART boot); BTN1 toggles LOW ↔ HIGH; BTN2 $\rightarrow$ AUTO; BTN1 in AUTO $\rightarrow$ LOW. Includes testbench and docs. | `top_system`: Supervisor FSM with boot telemetry; automated self-checking testbench `tb_top_system_v2` (11 test cases); ModelSim `wavefinal.do` script. |
| **TOTAL FPGA SCORE** | **10.0 pts** | **Complete synthesizable RTL, .cst constraints, self-checking testbench, scaling report.** | **100% Full Compliance with Contest Specification** |

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
├── SECURITY.md                                           # Security Policy & Hardware IP Protection
├── SETUP.md                                              # Toolchain setup instructions (Gowin & ModelSim)
├── ĐỀ THI FGPA 2026.docx.pdf                             # Official FPGA Contest Specification
├── ĐỀ THI MCU 2026.pdf                                   # Official MCU Contest Specification
├── Gowin-FPGA-Vietnamese-Book-ACG525-Basic-part-Print-v (1).pdf # Gowin FPGA Reference Book
└── FPGA/                                                 # FPGA Workspace Directory
    ├── .gitignore                                        # Gowin & ModelSim build output exclusions
    ├── README.md                                         # FPGA Subsystem Specification & Rebuild Guide
    ├── TESTING.md                                        # Verification Suite & ModelSim Guide
    ├── ISSUE_ANALYSIS_DEEP_DIVE.md                       # Comprehensive Analysis of 6 Hardware Bugs
    ├── README_SIMULATION_SCALING.md                      # Simulation Time Acceleration Report
    ├── pwm11.gprj                                        # Gowin EDA Project Configuration
    ├── constr/pwm11.cst                                  # Physical Pin Constraints (Kiwi Nano 4K)
    ├── src/                                              # Synthesizable RTL Modules
    │   ├── top_system.v                                  # Top Integration & Supervisor FSM
    │   ├── button_debounce.v                             # 20ms Debouncer & Single-Clock Pulse Generator
    │   ├── pwm_led_controller.v                          # 1kHz PWM Controller (LOW, HIGH, AUTO 2.0s)
    │   ├── uart_tx_string.v                              # 115200 8N1 UART Serializer
    │   ├── gowin_pllvr.v                                 # Gowin PLLVR Wrapper (27MHz -> 50MHz)
    │   └── ip/gowin_pllvr/                               # IP Generator Core Configuration
    ├── sim/                                              # Verification Testbenches
    │   ├── tb_top_system_v2.v                            # Automated System Testbench (11 Test Cases)
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
        FSM -->|"send_req & mode[1:0]"| UART["uart_tx_string<br>• BAUD_DIV = 434 (Error 0.006%)<br>• Latched mode_latched safety<br>• ASCII String with \r\n"]
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

## 6. Finite State Machine (FSM) Transitions

```mermaid
stateDiagram-v2
    [*] --> MODE_LOW : Power-on / sys_rst_n deasserted (Emits "MODE: LOW
")

    state MODE_LOW {
        [*] --> PWM_25_Percent : Duty Cycle 25% (1kHz)
    }

    state MODE_HIGH {
        [*] --> PWM_100_Percent : Duty Cycle 100% (Solid ON)
    }

    state MODE_AUTO {
        [*] --> PWM_Breathing : Breathing 0% ↔ 100% over 2.0s
    }

    MODE_LOW --> MODE_HIGH : Button 1 Pressed (btn1_pulse) / Emits "MODE: HIGH
"
    MODE_HIGH --> MODE_LOW : Button 1 Pressed (btn1_pulse) / Emits "MODE: LOW
"

    MODE_LOW --> MODE_AUTO : Button 2 Pressed (btn2_pulse) / Emits "MODE: AUTO
"
    MODE_HIGH --> MODE_AUTO : Button 2 Pressed (btn2_pulse) / Emits "MODE: AUTO
"

    MODE_AUTO --> MODE_LOW : Button 1 Pressed (btn1_pulse) / Emits "MODE: LOW
"
    MODE_AUTO --> MODE_AUTO : Button 2 Pressed (btn2_pulse) / Remain in AUTO
```

---

## 7. Mathematical Proofs and Calculations

### Proof 1: PLLVR Frequency Synthesis ($50.0\text{ MHz}$)
$$f_{\text{CLKOUT}} = 27.0\text{ MHz} \times \frac{13}{7} \approx \mathbf{50.14\text{ MHz}} \approx \mathbf{50.0\text{ MHz}}$$

### Proof 2: UART Baud Rate Timing Accuracy ($115,200\text{ bps}$)
$$\text{BAUD\_DIV} = \left\lfloor \frac{50,000,000}{115,200} \right\rceil = 434\text{ clock cycles}$$
$$\text{Baud}_{\text{actual}} = \frac{50,000,000}{434} \approx 115,207.37\text{ bps} \rightarrow \text{Error } \mathbf{0.0064\%} \ll \pm 2.0\%$$

### Proof 3: Zero-Collision Telemetry Guarantee
$$t_{\text{packet}} = \frac{12 \times 10}{115,200} \approx \mathbf{1.042\text{ ms}} \ll t_{\text{debounce}} = \mathbf{20.0\text{ ms}}$$
$$\text{Safety margin } \Delta t = 20.0\text{ ms} - 1.042\text{ ms} = \mathbf{18.958\text{ ms}} > 0$$

### Proof 4: Precise 2.0s Linear Breathing Cycle
$$T_{\text{cycle}} = (1,000\text{ steps up} \times 1.0\text{ ms}) + (1,000\text{ steps down} \times 1.0\text{ ms}) = \mathbf{2.000\text{ seconds}}$$

---

## 8. Build, Simulation & Flashing Instructions

### Gowin EDA Place & Route
1. Launch **Gowin EDA** and open [`FPGA/pwm11.gprj`](FPGA/pwm11.gprj).
2. Double click **Place & Route** $\rightarrow$ Verify **Success (0 Errors, 0 Warnings)**.
3. Program `impl/pnr/pwm11.fs` via **Gowin Programmer**.

### ModelSim Simulation
```tcl
cd d:/FPGA&MCU/FPGA
vlib work
vmap work work
vlog src/button_debounce.v src/pwm_led_controller.v src/uart_tx_string.v src/gowin_pllvr.v src/top_system.v sim/tb_top_system_v2.v
vsim -voptargs=+acc work.tb_top_system_v2
do sim/wavefinal.do
run -all
```
