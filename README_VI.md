# Porygon: FPGA & MCU Multi-Track System Design Framework (Da Nang Contest 2026)

> **Navigation / Chuyển hướng Ngôn ngữ**:  
> 🇻🇳 [Tiếng Việt — Thuyết Minh Định Hướng & Tổng Quan Hệ Thống](#-bản-thuyết-minh-định-hướng--tổng-quan-hệ-thống-tiếng-việt)  
> 🇬🇧 [English — Master Routing & System Specification](#-master-routing--system-specification-english)

---

# 🇻🇳 BẢN THUYẾT MINH ĐỊNH HƯỚNG & TỔNG QUAN HỆ THỐNG (TIẾNG VIỆT)

## 📍 BẢNG ĐIỀU HƯỚNG CÁC NHÁNH DỰ ÁN TRÊN GITHUB

Nhánh `main` đóng vai trò là **Trung tâm Định hướng Kiến trúc Hệ thống Master**. Hệ thống phân tách thành hai khối bài thi độc lập theo sơ đồ quản lý nhánh bên dưới:

| Phân Khối Kỹ Thuật | Nhánh Sản Phẩm Baseline (Production) | Nhánh Phát Triển (Active Dev) | Thư Mục Mã Nguồn |
| :--- | :--- | :--- | :--- |
| **MCU Track (ARM Cortex-M0 SN32F407)** | 🟢 [`MCU_main`](https://github.com/zok213/porygon/tree/MCU_main) | 🛠️ [`MCU_dev`](https://github.com/zok213/porygon/tree/MCU_dev) | [`MCU_Contest_2026/`](MCU_Contest_2026/) |
| **FPGA Track (Gowin GW1N Verilog RTL)**| 🟢 [`FPGA_main`](https://github.com/zok213/porygon/tree/FPGA_main) | 🛠️ [`FPGA_dev`](https://github.com/zok213/porygon/tree/FPGA_dev) | [`FPGA/`](FPGA/) |
| **Bản Phát Hành Chính Thức** | 🏆 [`release`](https://github.com/zok213/porygon/tree/release) | — | Toàn bộ Repository |

---

## 1. Tóm Tắt Hệ Thống & Phân Phối Kiến Trúc 2 Khối Cuộc Thi

Tài liệu này trình bày tổng quan giải pháp kiến trúc cho **Hội thi Thiết kế Hệ thống Nhúng & Microcontroller Đà Nẵng 2026**.

Hệ thống bao gồm hai khối phần cứng độc lập được tích hợp đồng bộ:

### 1.1 Khối Vi điều khiển (MCU Track - ARM Cortex-M0 SN32F407)
- **Kiến trúc Mã nguồn**: Giải pháp phần mềm nhúng C99 thương mại chạy trên vi điều khiển **SN32F407_EVK**. Thực thi đồng hồ số 24 giờ, quét đa kênh LED 7 đoạn chống lem màu bóng ma, quản lý bộ nhớ EEPROM AT24C02 qua bus I2C0 ngắt phần cứng, thuật toán ACK Polling, SysTick 1ms RTC, quét phím ma trận chống nẩy, và tự phục hồi khi gặp sự cố điện từ (HardFault Self-Healing Reset).
- **Trạng thái Thực chứng**: Đã kiểm thử trực quan 100% trên bo mạch thực tế.
- **Thư mục Mã nguồn**: [`MCU_Contest_2026/`](MCU_Contest_2026/)
- **Liên kết Nhánh**: Nhánh chính thức [`MCU_main`](https://github.com/zok213/porygon/tree/MCU_main) và Nhánh phát triển [`MCU_dev`](https://github.com/zok213/porygon/tree/MCU_dev).

### 1.2 Khối Vi mạch Khả trình (FPGA Track - Gowin GW1N)
- **Kiến trúc Mã nguồn**: Mã nguồn RTL Verilog chuẩn hóa mô phỏng và tổng hợp trên FPGA **Gowin GW1N** (Kiwi 1P5 / Kiwi Nano 4K). Tích hợp IP Core rPLL $50\text{MHz}$, bộ lọc nẩy phím cơ 4 công đoạn, bộ phát xung PWM 3 chế độ thở/chớp/tĩnh, và khối truyền thông RS-232 UART TX ($115200\text{ bps}$).
- **Thư mục Mã nguồn**: [`FPGA/`](FPGA/)
- **Liên kết Nhánh**: Nhánh chính thức [`FPGA_main`](https://github.com/zok213/porygon/tree/FPGA_main) và Nhánh phát triển [`FPGA_dev`](https://github.com/zok213/porygon/tree/FPGA_dev).

### 1.3 Bản Phát Hành Chính Thức (`release`)
- Nhánh [`release`](https://github.com/zok213/porygon/tree/release) đóng vai trò là **Bản Giao Nộp Cuối Cùng (Final Deliverable Release)** chứa toàn bộ mã nguồn verified sẵn sàng phục vụ công tác chấm thi.

---

## 2. Bảng Tiêu Chí Đánh Giá Cuộc Thi & Tổng Quan Kỹ Thuật

| Thành phần Đánh giá | Trọng số | Phương án Kỹ thuật & Thực thi trong Mã nguồn (C / Verilog) |
| :--- | :--- | :--- |
| **Chức năng Đồng hồ Số & RTL Core** | 35% | Ngắt SysTick $1\text{ms}$, quét 7 đoạn $HH.MM$; Khối FSM Supervisor FPGA điều khiển ngắt UART & PWM. |
| **Báo thức, EEPROM & UART Telemetry**| 15% | Driver I2C0 ngắt phần cứng đọc/ghi AT24C02; Khối UART TX $115200\text{ bps}$ truyền telemetry RS-232. |
| **Tính năng Thưởng (Bonus Features)** | 10% | Tự động hủy chỉnh sau 30s (MCU); 3 chế độ PWM thở/chớp/tĩnh điều khiển độ sáng LED (FPGA). |
| **Thuyết minh & Demo Video** | 20% | Kịch bản kiểm thử trực quan trên bo thật MCU SN32F407_EVK và bo FPGA Gowin GW1N. |
| **Kiến trúc Mã nguồn & Tài liệu** | 10% | Chuẩn C99 nhúng phòng thủ & Verilog IEEE 1364-2001, 0 lỗi cảnh báo khi tổng hợp và biên dịch. |
| **Vòng Phỏng vấn Kỹ thuật Q&A** | 10% | Phân tích cấp thanh ghi AHB/APB MCU, chứng minh toán học tần số rPLL FPGA & bộ chia Baudrate UART. |

---

## 3. Bản Đồ Cấu Trúc Thư Mục Hệ Thống Master (`main` Branch)

```
porygon/
├── .github/
│   ├── ISSUE_TEMPLATE/
│   │   ├── bug_report.md                                 # Mẫu báo lỗi phần cứng/phần mềm MCU & FPGA
│   │   └── feature_request.md                            # Mẫu đề xuất nâng cấp tính năng
│   ├── pull_request_template.md                          # Mẫu kiểm tra review Pull Request hợp nhất
│   └── workflows/
│       └── ci.yml                                        # Pipeline CI/CD tự động kiểm tra tĩnh MCU & FPGA RTL
├── .gitignore                                            # Quy tắc loại bỏ tệp rác biên dịch Keil & Gowin EDA
├── CODE_OF_CONDUCT.md                                    # Quy tắc ứng xử đóng góp mã nguồn
├── CONTRIBUTING.md                                       # Quy định quản lý nhánh Git Flow & định dạng commit
├── LICENSE                                               # Giấy phép bản quyền mã nguồn mở MIT
├── README.md                                             # Master Thuyết minh Định hướng Kiến trúc (Gồm cả VI và EN)
├── README_VI.md                                          # Bản Thuyết minh Định hướng & Tổng quan Tiếng Việt
├── README_EN.md                                          # Master Routing & System Specification (English)
├── SECURITY.md                                           # Chính sách bảo mật & bảo vệ bộ nhớ Flash RDP
├── SETUP.md                                              # Hướng dẫn thiết lập Keil MDK-ARM & Gowin EDA Diamond
├── ĐỀ THI MCU 2026.pdf                                   # Bản gốc Đề thi Khối Microcontroller 2026
├── ĐỀ THI FGPA 2026.docx.pdf                             # Bản gốc Đề thi Khối FPGA 2026
├── Gowin-FPGA-Vietnamese-Book-ACG525-Basic-part-Print-v (1).pdf # Sách hướng dẫn lập trình Gowin FPGA
├── MCU_Contest_2026/                                     # Sub-Repository Khối MCU (Tương ứng MCU_main)
│   ├── README.md                                         # Báo cáo kỹ thuật chi tiết khối MCU
│   ├── main_clock_skeleton.c                             # Mã nguồn C chính điều khiển FSM & SysTick 1ms
│   ├── I2C0.c                                            # Driver I2C0 ngắt phần cứng chuẩn hãng SONiX DFP
│   ├── I2C.h                                             # Định nghĩa địa chỉ thanh ghi ngoại vi & hàm I2C0
│   ├── ISSUE_ANALYSIS_DEEP_DIVE.md                       # Báo cáo phân tích thực chứng 6 lỗi MCU
│   └── Clock_Simulation.uvprojx                          # File project Keil MDK-ARM uVision
└── FPGA/                                                 # Sub-Repository Khối FPGA (Tương ứng FPGA_main)
    ├── README.md                                         # Báo cáo kỹ thuật chi tiết khối FPGA
    ├── top.v                                             # Module điều khiển FSM cấp cao nhất Supervisor
    ├── pll_50mhz.v                                       # IP Core rPLL tổng hợp xung clock 24MHz -> 50MHz
    ├── debouncer.v                                       # Module chống nẩy phím bấm ma trận 4-stage
    ├── breathing_pwm.v                                   # Module phát xung PWM 3 chế độ (Thở/Chớp/Tĩnh)
    └── uart_tx.v                                         # Module truyền dữ liệu UART TX 115200 bps 8N1
```
