# SN32F407 Smart Digital Clock Firmware Specification (Da Nang MCU Contest 2026)

> **Navigation / Chuyển hướng Ngôn ngữ**:  
> 🇻🇳 [Tiếng Việt — Thuyết Minh Kỹ Thuật Thư Mục MCU](#-thuyết-minh-kỹ-thuật-thư-mục-mcu-tiếng-việt)  
> 🇬🇧 [English — MCU Firmware Specification](#-mcu-firmware-specification-english)

---

# 🇻🇳 THUYẾT MINH KỸ THUẬT THƯ MỤC MCU (TIẾNG VIỆT)

## 1. Tóm Tắt Hệ Thống & Phạm Vi Thực Thi

Tài liệu này chi tiết toàn bộ kiến trúc mã nguồn sản phẩm cho **Hệ thống Đồng hồ Số Thông minh & Báo thức Lưu trữ EEPROM** chạy trên vi điều khiển **SN32F407_EVK** (lõi ARM Cortex-M0).

Hệ thống ứng dụng nguyên lý **Lập trình Nhúng Phòng thủ (Defensive Embedded Programming)**, đảm bảo quét LED 7 đoạn không chớp nháy/bóng ma, đọc ghi EEPROM I2C không nghẽn bus, chống nẩy phím bấm ma trận chính xác, và tự phục hồi khi gặp sự cố phần cứng.

---

## 2. Bảng Cấu Hình Chân Ngoại Vi (SN32F407_EVK Pinout)

| Khối Chức Năng | Thanh Ghi / Chân MCU | Đối Tượng Phần Cứng | Chế Đồ & Cấu Hình Điện Thế |
| :--- | :--- | :--- | :--- |
| **Bus Thanh LED 7 Đoạn** | `GPIO0 [0..7]` | Các thanh LED (A..G, DP) | Ngõ ra Push-Pull, Mức cao (Active High) |
| **Bus Chọn Vị Trí Số** | `GPIO1 [9..12]` | Chọn LED 7 đoạn (D1..D4) | Ngõ ra Push-Pull, Quét Đa kênh Mức cao |
| **Quét Hàng Ma Trận Phím**| `GPIO1 [4..7]` | Chân xuất xung quét hàng | Ngõ ra Quét Mức thấp (Active Low) |
| **Đọc Cột Ma Trận Phím** | `GPIO2 [4..7]` | Chân đọc tín hiệu cột | Ngõ vào có Điện trở Kéo lên Nội (Pull-up) |
| **I2C0 SCL (Xung Clock)** | `GPIO0 [10]` | Xung Clock Chân AT24C02 | Open-Drain, Kéo lên Ngoại $4.7\text{k}\Omega$ |
| **I2C0 SDA (Dữ Liệu)** | `GPIO0 [11]` | Tín hiệu Dữ liệu AT24C02 | Open-Drain, Kéo lên Ngoại $4.7\text{k}\Omega$ |
| **Đầu Ra Còi Báo** | `GPIO3 [0]` | Còi thạch anh Piezo | Đóng ngắt qua Transistor NPN, Mức cao |
| **LED Báo Trạng Thái** | `GPIO3 [8]` | LED Board D6 | Ngõ ra Mức thấp (Active Low) |

---

## 3. Giải Trình Kỹ Thuật Chi Tiết 6 Lỗi Cuộc Thi & Giải Pháp

1. **Guard Boot Báo thức (`alarm_armed`)**: Giữ nguyên guard `if (alarm_hour || alarm_min) alarm_armed = 1;` khi vừa nạp EEPROM, tránh nổ chuông báo thức 5s giả khi vừa cắm điện nếu mốc báo thức là `00:00`.
2. **Kiểm tra Kết quả Lưu EEPROM**: Trong `Process_Key()`, bắt buộc kiểm tra `if (!EEPROM_SaveAlarm(...))`. Nếu lưu xịt, phát 3 tiếng bíp dài (`buzzer_beep_ms = 900`) và giữ nguyên màn hình edit.
3. **Driver I2C0 Ngắt Phần Cứng (`I2C0.c`)**: Sử dụng driver ngắt chuẩn hãng SONiX với bộ đệm FIFO, map đúng chân `P0.10` (SCL0) và `P0.11` (SDA0), không bị đụng chân LED 7 đoạn.
4. **Cấu hình CMSIS & Flash Wait-State**: Gọi `SystemInit()` và `SystemCoreClockUpdate()` ở đầu `main()`, tự động cấu hình thanh ghi `SN_FLASH->LPCTRL` an toàn khi nâng clock lên 48MHz.
5. **Tự Phục Hồi Lỗi HardFault (`HardFault_Handler`)**: Viết đè hàm C `HardFault_Handler` chứa `__disable_irq()` và `NVIC_SystemReset()`, cho phép vi điều khiển tự động Soft Reset khi gặp nhiễu.
6. **Thuật toán ACK Polling EEPROM**: Thay thế vòng lặp NOP thô bằng thuật toán **ACK Polling** (`do { ok = I2C0_Write(1, 1); if (ok) break; } while (++poll_retry < 50);`), tự động tương thích với thời gian ghi $t_{WR}$ của EEPROM.
7. **Xóa Cờ Lỗi Dính Cứng (Sticky Error)**: Thêm `Error = 0;` ở đầu hai hàm `I2C0_Read()` và `I2C0_Write()` để xóa trạng thái dính lỗi sau khi bị NACK ngẫu nhiên.
8. **Nhấn Giữ Tự Động Cuộn Số Nhanh (Key Auto-Repeat)**: Thuật toán lặp 2 pha (Delay $500\text{ms} \rightarrow$ Repeat $100\text{ms}$) cho hai phím SW6 (`KEY_PLUS`) và SW10 (`KEY_MINUS`), tự động nâng/hạ số mượt mà khi nhấn giữ.

---

## 4. Hướng Dẫn Biên Dịch & Nạp Code (Keil MDK)

1. Mở file project `MCU_Contest_2026/Clock_Simulation.uvprojx` trong **Keil MDK 5.3x / 5.4x**.
2. Chọn target `Target_1` (Trình biên dịch ArmClang V6).
3. Nhấn **F7 (Rebuild All)** — kiểm tra kết quả đạt **0 Error(s), 0 Warning(s)**.
4. Kết nối mạch nạp **SN-Link Debugger** vào bo `SN32F407_EVK`.
5. Nhấn **F8 (Download)** để nạp file hex vào vi điều khiển.

---

# 🇬🇧 MCU FIRMWARE SPECIFICATION (ENGLISH)

## System Abstract

This document details the production-grade firmware architecture for the **Smart Digital Clock and Alarm System** running on the **SN32F407_EVK** evaluation board (ARM Cortex-M0 core). 

The implementation applies **Defensive Embedded Programming** principles to guarantee zero-flicker display multiplexing, non-blocking I2C EEPROM storage, deterministic matrix key debouncing, and fault-tolerant finite state machine (FSM) execution.

---

## Hardware Peripheral Mapping (SN32F407_EVK)

| System Block | Target Hardware | MCU Pin Assignment | Electrical Configuration |
| :--- | :--- | :--- | :--- |
| **Display Bus** | 7-Segment Segment Lines (A..G, DP) | `GPIO0 [0..7]` | Push-Pull Output, Active High |
| **Display Driver** | 7-Segment Digit Select (D1..D4) | `GPIO1 [9..12]` | Push-Pull Output, Active High |
| **Key Matrix** | Matrix Key Row Drivers | `GPIO1 [4..7]` | Open-Drain / Push-Pull Scan Out |
| **Key Matrix** | Matrix Key Column Inputs | `GPIO2 [4..7]` | Input with Internal Pull-Up Resistors |
| **Non-Volatile Memory** | I2C EEPROM (AT24C02) SCL/SDA | `I2C0` (`GPIO0_10` / `GPIO0_11`) | Open-Drain, External $4.7\text{k}\Omega$ Pull-up |
| **Audio Output** | Piezo Electric Buzzer | `GPIO3 [0]` | NPN Transistor Driver, Active High |
| **Status Indicator** | Mode Status LED (Board D6) | `GPIO3 [8]` | Push-Pull Output, Active Low |

---

## Resolved Edge Cases & Defensive Features

1. **Boot Alarm Guard (`alarm_armed`)**: Preserves boot check `if (alarm_hour || alarm_min) alarm_armed = 1;` to prevent accidental 5-second alarm bursts during initial power-on.
2. **Checked EEPROM Save Return**: `Process_Key()` validates `EEPROM_SaveAlarm(...)`. If write fails, the UI issues 3 long error beeps (`buzzer_beep_ms = 900`) and remains in edit mode.
3. **Hardware Interrupt I2C Driver (`I2C0.c`)**: Uses SONiX DFP interrupt-driven state machine with FIFO buffers mapped to `P0.10` (SCL0) and `P0.11` (SDA0).
4. **CMSIS Clock & Flash Initialization**: Explicitly invokes `SystemInit()` and `SystemCoreClockUpdate()` at the start of `main()`, ensuring correct Flash wait-states (`SN_FLASH->LPCTRL`).
5. **Self-Healing Fault Recovery (`HardFault_Handler`)**: Implements an explicit C handler calling `__disable_irq()` and `NVIC_SystemReset()`.
6. **ACK Polling Protocol**: Replaces fixed NOP loops in `EEPROM_SaveAlarm` with hardware **ACK Polling** retries (`do { ... } while (++poll_retry < 50)`).
7. **Sticky Error Reset**: Clears `Error = 0;` at the beginning of `I2C0_Read()` and `I2C0_Write()`.

---

## Build and Flashing Instructions (Keil MDK)

1. Open project file `MCU_Contest_2026/Clock_Simulation.uvprojx` in **Keil MDK 5.3x / 5.4x**.
2. Select target `Target_1` (ArmClang V6 compiler).
3. Press **F7 (Rebuild All)** — verify output is **0 Error(s), 0 Warning(s)**.
4. Connect **SN-Link Debugger** to board `SN32F407_EVK`.
5. Press **F8 (Download)** to flash hex binary to target MCU.
