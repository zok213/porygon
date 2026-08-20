# Porygon: Gowin GW1NSR-4C Multi-Mode PWM & UART Telemetry Framework (Da Nang FPGA Contest 2026)

## 1. Tóm Tắt Hệ Thống & Phân Tích Mục Tiêu Kỹ Thuật

Tài liệu này trình bày giải pháp kiến trúc vi mạch số (Digital IC Architecture) và mã nguồn Verilog RTL thương mại cho **Hệ Thống Điều Khiển LED Đa Chế Độ & Truyền Thông Telemetry UART 115200 bps** chạy trên chip FPGA **Gowin GW1NSR-LV4CQN48PC7/I6 (Bo mạch Kiwi Nano 4K)**.

Mã nguồn được thiết kế tuân thủ nghiêm ngặt các nguyên lý **Thiết Kế Vi Mạch Số Phòng Thủ (Defensive Digital IC Design)**:
- **Đồng Bộ Hóa Xung Nhịp & Khử Metastability**: Sử dụng mạch Reset Synchronizer 2 tầng giữ reset $20\text{ ms}$ cho đến khi bộ `Gowin_PLLVR` đạt trạng thái khóa pha ổn định (`pll_lock`).
- **Lọc Dội Phím Bắt Sườn (Edge-Triggered Debouncing)**: Triệt tiêu hoàn toàn rung nẩy cơ học qua bộ tích phân $20\text{ ms}$ ($1,000,000$ nhịp clock @ 50MHz), xuất xung kích hoạt **đúng 1 chu kỳ clock ($20\text{ ns}$)**.
- **Điều Chế PWM $1\text{ kHz}$ Không Nhấp Nháy Mắt (Flicker-Free)**: Sóng mang $1,000\text{ Hz}$ (`ARR_MAX = 50_000`), chia mịn thành 2,000 nấc độ sáng ($1,000$ nấc tăng $+ 1,000$ nấc giảm) cho chu kỳ thở chính xác tuyệt đối **$2.000\text{ giây}$**.
- **Bộ Truyền Telemetry UART $115,200\text{ bps}$ Có Chốt An Toàn**: Hệ số chia `BAUD_DIV = 434` (sai số $0.0064\% \ll \pm 2.0\%$) với cơ chế chốt `mode_latched` chống méo chuỗi ký tự khi chuyển trạng thái giữa chừng.
- **Mô Phỏng Tự Chứa (Zero-Dependency ModelSim Simulation)**: Tích hợp sẵn thư viện linh kiện Gowin ([`FPGA/src/prim_sim.v`](FPGA/src/prim_sim.v)), kịch bản chạy 1 lệnh [`FPGA/sim/run_sim.do`](FPGA/sim/run_sim.do), và Testbench tự động kiểm thử 25 chỉ tiêu (`25 checks, 0 errors`) chứng minh chu kỳ thở $2.000000\text{ s}$ trên ModelSim.
- **Minh Chứng Thực Nghiệm Phần Cứng 100%**: Đã xác thực thành công trên phần cứng thật qua cổng COM3 với phần mềm Hercules Terminal ở tốc độ 115200 8N1.

> 📖 **Tài liệu Hướng dẫn Mô phỏng Chuyên sâu**:
> - 🌊 [Hướng dẫn Mô phỏng ModelSim FPGA Toàn diện](FPGA/sim/README_MODELSIM_SIMULATION.md)
> - 🐞 [Hướng dẫn Debug Mô phỏng MCU SN32F407 trên Keil µVision5](MCU_Contest_2026/README_KEIL_DEBUG_SIMULATION.md)
> - 🔍 [Báo cáo Phân tích Chuyên sâu 6 Lỗi Phần Cứng FPGA](FPGA/ISSUE_ANALYSIS_DEEP_DIVE.md)
> - 🧪 [Ma trận Kiểm thử Tự động & Hướng dẫn Dạng sóng Waveform](FPGA/TESTING.md)

---

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
├── .github/                                              # CI/CD Workflows & Issue Templates
│   ├── ISSUE_TEMPLATE/                                   # Mẫu báo lỗi & đề xuất tính năng
│   ├── pull_request_template.md                          # Mẫu kiểm tra review Pull Request
│   └── workflows/ci.yml                                  # Pipeline CI/CD tự động kiểm thử
├── .gitignore                                            # Quy tắc loại bỏ tệp rác Gowin EDA & ModelSim
├── CODE_OF_CONDUCT.md                                    # Quy tắc ứng xử cộng đồng mã nguồn
├── CONTRIBUTING.md                                       # Quy định quản lý nhánh Git Flow & chuẩn commit
├── LICENSE                                               # Giấy phép bản quyền mã nguồn mở MIT
├── README.md                                             # Master Thuyết minh Hệ thống (Song ngữ VI & EN đầy đủ)
├── README_VI.md                                          # Bản Thuyết minh Kiến trúc Kỹ thuật Tiếng Việt chuẩn hóa
├── README_EN.md                                          # Master System Architectural Specification (English)
├── SECURITY.md                                           # Chính sách bảo mật & bảo vệ tài nguyên phần cứng
├── SETUP.md                                              # Hướng dẫn thiết lập môi trường Gowin EDA & ModelSim
├── docs/                                                 # Thư mục tài liệu chung toàn dự án
│   ├── assets/                                           # Hình ảnh sơ đồ và kết quả minh chứng phần cứng
│   │   └── uart_telemetry_hercules_com3.png              # Hình ảnh minh chứng truyền nhận UART thực tế trên cổng COM3
│   ├── ĐỀ THI FGPA 2026.docx.pdf                         # Bản gốc Đề thi Khối FPGA Đà Nẵng 2026
│   ├── ĐỀ THI MCU 2026.pdf                               # Bản gốc Đề thi Khối Microcontroller Đà Nẵng 2026
│   ├── Gowin-FPGA-Vietnamese-Book-ACG525-Basic-part-Print-v (1).pdf # Tài liệu lập trình Gowin FPGA tiếng Việt
│   └── Thuyết Minh PBL3 Chốt 1705.pdf                   # Báo cáo thuyết minh đề tài PBL3
├── FPGA/                                                 # Thư mục Không gian làm việc Dự án Gowin EDA
│   ├── .gitignore                                        # Loại bỏ tệp đầu ra tổng hợp impl/ & mô phỏng work/
│   ├── README.md                                         # Báo cáo kỹ thuật chi tiết thư mục FPGA & Hướng dẫn Rebuild
│   ├── TESTING.md                                        # Ma trận kiểm thử tự động 25 kịch bản & Hướng dẫn ModelSim
│   ├── ISSUE_ANALYSIS_DEEP_DIVE.md                       # Báo cáo phân tích chuyên sâu & thực chứng 6 lỗi FPGA
│   ├── pwm11.gprj                                        # Tệp cấu hình dự án Gowin EDA (GW1NSR-4C / Kiwi Nano 4K)
│   ├── constr/                                           # Thư mục chứa tệp ràng buộc vật lý chân phần cứng
│   │   └── pwm11.cst                                     # Ràng buộc chân FPGA (Clock, Reset, BTN1/2, LED, UART TX)
│   ├── src/                                              # Mã nguồn Verilog RTL tổng hợp được (Synthesizable RTL)
│   │   ├── top_system.v                                  # Module cấp cao nhất: Đồng bộ Reset, Quản lý FSM trung tâm
│   │   ├── button_debounce.v                             # Mạch lọc dội phím 2 tầng D-FF + Bộ đếm 20ms + Bắt sườn xung
│   │   ├── pwm_led_controller.v                          # Bộ điều chế PWM 1kHz (LOW 25%, HIGH 100%, AUTO Thở 2.0s)
│   │   ├── uart_tx_string.v                              # Bộ truyền chuỗi UART 115200 8N1 có chốt an toàn trạng thái
│   │   ├── gowin_pllvr.v                                 # Top wrapper IP Core Gowin PLLVR (27MHz -> 50MHz)
│   │   ├── prim_sim.v                                    # Thư viện mô phỏng linh kiện Gowin (Zero-Dependency)
│   │   └── ip/gowin_pllvr/                               # Tệp cấu hình gốc IP Generator (.ipc, .mod, .v, _tmp.v)
│   ├── sim/                                              # Thư mục kịch bản mô phỏng kiểm thử (Verification Suite)
│   │   ├── README_MODELSIM_SIMULATION.md                 # Hướng dẫn chi tiết chạy mô phỏng ModelSim SE 10.6d
│   │   ├── README_SIMULATION_SCALING.md                  # Thuyết minh kỹ thuật tỷ lệ thời gian mô phỏng tăng tốc
│   │   ├── run_sim.do                                    # Kịch bản Tcl chạy tự động toàn bộ mô phỏng trên ModelSim
│   │   ├── wavefinal.do                                  # Cấu hình hiển thị dạng sóng & 5 thước đo thời gian
│   │   ├── tb_top_system_v2.v                            # Testbench tự động kiểm tra toàn diện 25 chỉ tiêu (0 lỗi)
│   │   └── tb_uart_tx.v                                  # Testbench đo thời gian phát chuỗi khối UART
│   └── docs/                                             # Thư mục tài liệu đề bài khối FPGA
│       ├── assets/                                       # Ảnh minh chứng phần cứng nội bộ khối FPGA
│       │   └── uart_telemetry_hercules_com3.png          # Ảnh minh chứng UART cổng COM3
│       └── ĐỀ THI FGPA 2026.docx.pdf                     # Bản sao đề thi chính thức Khối FPGA Đà Nẵng 2026
└── MCU_Contest_2026/                                     # Thư mục Không gian làm việc Dự án Vi điều khiển SN32F407
    ├── README.md                                         # Báo cáo kiến trúc hệ thống Firmware MCU
    ├── README_KEIL_DEBUG_SIMULATION.md                   # Hướng dẫn chi tiết cấu hình Debug Watch 1 trên Keil µVision5
    ├── ISSUE_ANALYSIS_DEEP_DIVE.md                       # Báo cáo phân tích chuyên sâu 6 lỗi vi điều khiển
    ├── docs/                                             # Thư mục tài liệu đề bài khối MCU
    │   └── ĐỀ THI MCU 2026.pdf                           # Bản sao đề thi chính thức Khối MCU Đà Nẵng 2026
    └── src/                                              # Mã nguồn C/Assembly cho MCU SN32F407
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

## 9. Kiến Trúc Bộ Truyền Telemetry UART 115200 bps & Minh Chứng Thực Nghiệm

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

### 📸 Hình Ảnh Minh Chứng Kết Quả Truyền Nhận Dữ Liệu UART Thực Tế Trên Phần Cứng

Dưới đây là hình ảnh chụp thực tế màn hình giám sát Terminal qua cổng nạp USB-UART (**Hercules SETUP utility**) kết nối cổng **COM3** ở cấu hình **115200 bps, 8 Data bits, No Parity, Handshake OFF**:

![Minh chứng truyền nhận dữ liệu UART Telemetry thực tế trên phần cứng COM3 115200 bps](docs/assets/uart_telemetry_hercules_com3.png)

*Hình 1: Kết quả nhận chuỗi Telemetry thời gian thực trên bo mạch Kiwi Nano 4K (COM3 @ 115200 bps).*

**Giải trình trình tự chuỗi telemetry thực nghiệm trên phần cứng**:
1. `Serial port COM3 opened` $\rightarrow$ Mở cổng COM3 thành công.
2. `MODE: LOW` $\rightarrow$ Trạng thái khởi động mặc định (Power-on Reset) khi mạch vừa được cấp nguồn.
3. `MODE: HIGH` $\rightarrow$ Nhấn Nút 1 (BTN1) chuyển sang chế độ Sáng 100%.
4. `MODE: AUTO` $\rightarrow$ Nhấn Nút 2 (BTN2) chuyển sang chế độ Thở 2.0s.
5. `MODE: LOW` $\rightarrow$ Nhấn Nút 1 (BTN1) khi đang ở AUTO để ép thoát về chế độ LOW 25%.
6. `MODE: HIGH` $\rightarrow$ Nhấn Nút 1 (BTN1) đổi sang HIGH.
7. `MODE: LOW` $\rightarrow$ Nhấn Nút 1 (BTN1) đổi lại về LOW.

👉 **Kết luận thực nghiệm**: Toàn bộ chuỗi dữ liệu telemetry ASCII nhận được nguyên vẹn 100%, kết thúc bằng cặp ký tự `\r\n` (xuống dòng chuẩn xác), hoàn toàn không bị mất ký tự, không bị rác dữ liệu hay nghẽn khung truyền.

---

## 10. Hướng Dẫn Mô Phỏng ModelSim Tự Chứa (Zero-Dependency Quick Start)
*(Chi tiết tại [`FPGA/sim/README_MODELSIM_SIMULATION.md`](FPGA/sim/README_MODELSIM_SIMULATION.md))*

Dự án tích hợp sẵn thư viện linh kiện Gowin [`FPGA/src/prim_sim.v`](FPGA/src/prim_sim.v) giúp chạy mô phỏng trơn tru trên mọi máy tính mà không cần cài đặt Gowin EDA:

### 🚀 Chạy Tự Động Bằng 1 Lệnh Tcl Duy Nhất
1. Mở phần mềm **ModelSim SE 10.6d**.
2. Chọn menu **`File` $\rightarrow$ `Change Directory...`** $\rightarrow$ Trỏ đến thư mục `FPGA/sim/`.
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
