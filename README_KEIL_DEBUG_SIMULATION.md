# 🐞 HƯỚNG DẪN CHI TIẾT MÔ PHỎNG DEBUG MCU SN32F407 TRÊN KEIL µVISION5

Tài liệu này hướng dẫn chi tiết các bước thiết lập môi trường mô phỏng **Debug Simulator**, mở cửa sổ **Watch 1**, thêm và giải thích ý nghĩa toàn bộ 13 biến hệ thống thời gian thực cho dự án MCU Cortex-M0 trên phần mềm **Keil µVision5**.

---

## 1. ⚙️ THÔNG TIN PHẦN MỀM & LICENSE

- **Tên phần mềm**: Keil MDK-ARM (Microcontroller Development Kit)
- **Phiên bản khuyến nghị**: `Keil µVision5 v5.38.0.0` (hoặc phiên bản v5.3x).
- **Gói hỗ trợ vi điều khiển (Pack)**: `SONiX.SN32F400_DFP.2.6.0.pack`.
- **Loại License**: Keil MDK-ARM Community / Professional License (`FlexLM` / `CID Code`).
- **Dòng vi điều khiển mục tiêu**: `SN32F407` (ARM Cortex-M0 Core, $48\text{ MHz}$).

---

## 2. 🚀 CÁCH CẤU HÌNH VÀ BẮT ĐẦU PHIÊN MÔ PHỎNG DEBUG

### **Bước 1: Cấu hình chế độ mô phỏng Simulator**
1. Mở project Keil: `Clock_Simulation.uvprojx` (hoặc `SN32F407_Clock.uvprojx`).
2. Bấm phím **Alt + F7** (hoặc chuột phải vào Target 1 chọn **`Options for Target...`**).
3. Chuyển sang tab **`Debug`**:
   - Tích chọn **`Use Simulator`** (Nếu muốn mô phỏng ảo trên máy tính).
   - Hoặc chọn **`CMSIS-DAP Debugger` / `ULINK2/ME`** (Nếu cắm mạch thật qua cổng SWD).
4. Bấm **OK** để lưu cấu hình.

### **Bước 2: Khởi chạy phiên Debug**
1. Nhấn tổ hợp phím **Ctrl + F5** (hoặc bấm biểu tượng **`Start/Stop Debug Session`** hình kính lúp đỏ trên thanh công cụ).
2. Giao diện Keil sẽ chuyển sang giao diện Debugger với các cửa sổ mã assembly, thanh ghi CPU và bộ đếm thời gian.
3. Bấm **F5** (hoặc nút **`Run`**) để chương trình bắt đầu thực thi thời gian thực.

---

## 3. 🖥️ HƯỚNG DẪN MỞ CỬA SỔ WATCH 1 VÀ NHẬP CÁC BIẾN THEO DÕI

### **Các bước mở cửa sổ Watch 1**:
1. Trên thanh menu trên cùng của Keil, chọn:  
   **`View` $\rightarrow$ `Watch Windows` $\rightarrow$ `Watch 1`** (hoặc bấm phím tắt **Alt + 1**).
2. Cửa sổ **Watch 1** sẽ xuất hiện ở góc dưới màn hình.
3. Nhấp đôi chuột vào ô **`<Enter expression>`**, gõ tên từng biến dưới đây rồi bấm **Enter**.

---

## 📊 BẢNG GIẢI THÍCH 13 BIẾN HỆ THỐNG TRONG WINDOW WATCH 1

| STT | Tên biến trong Watch 1 | Kiểu dữ liệu (Type) | Giải thích Chức năng & Ý nghĩa Kỹ thuật Chi tiết |
| :---: | :--- | :---: | :--- |
| **1** | `system_mode` | `enum (uchar)` | **Trạng thái chế độ hệ thống**: <br>• `0`: `MODE_NORMAL` (Đồng hồ chạy thời gian thực bình thường)<br>• `1`: `MODE_EDIT_HOUR` (Chế độ chỉnh Giờ)<br>• `2`: `MODE_EDIT_MIN` (Chế độ chỉnh Phút)<br>• `3`: `MODE_EDIT_AL_HOUR` (Chế độ chỉnh Giờ Báo thức)<br>• `4`: `MODE_EDIT_AL_MIN` (Chế độ chỉnh Phút Báo thức). |
| **2** | `time_sec` | `uchar` (`uint8_t`) | **Bộ đếm Giây thời gian thực** ($0..59$). Tự động tăng 1 khi ngắt SysTick đếm đủ 1,000ms. Tự động về 0 khi vượt quá 59. |
| **3** | `time_min` | `uchar` (`uint8_t`) | **Bộ đếm Phút thời gian thực** ($0..59$). Tự động tăng 1 khi `time_sec` tràn từ 59 về 0. |
| **4** | `time_hour` | `uchar` (`uint8_t`) | **Bộ đếm Giờ thời gian thực** ($0..23$). Tự động tăng 1 khi `time_min` tràn từ 59 về 0. |
| **5** | `alarm_hour` | `uchar` (`uint8_t`) | **Giờ hẹn Báo thức** ($0..23$). Được lưu cố định vào chip EEPROM AT24C02 qua I2C0 và tự nạp lại khi khởi động mạch. |
| **6** | `alarm_min` | `uchar` (`uint8_t`) | **Phút hẹn Báo thức** ($0..59$). Được lưu cố định vào chip EEPROM AT24C02 qua I2C0. |
| **7** | `inactivity_ms` | `uint` (`uint32_t`) | **Bộ đếm thời gian rảnh rỗi** (tính bằng ms). Tự động reset về 0 mỗi khi người dùng ấn nút. Nếu quá $30,000\text{ ms}$ ($30\text{ s}$) không thao tác khi đang cài đặt, MCU tự thoát về `MODE_NORMAL`. |
| **8** | `blink_ms` | `uint` (`uint32_t`) | **Bộ đếm nhấp nháy LED 7 đoạn** (tính bằng ms). Tự đảo cờ `blink_on` sau mỗi $500\text{ ms}$ ($0.5\text{ s}$) để tạo hiệu ứng nhấp nháy số đang chỉnh. |
| **9** | `blink_on` | `uchar` (`uint8_t`) | **Cờ trạng thái nhấp nháy**: `1` (Sáng chữ số), `0` (Tắt chữ số để tạo hiệu ứng chớp tắt $0.5\text{ s}/0.5\text{ s}$). |
| **10** | `buzzer_beep_ms` | `ushort` (`uint16_t`) | **Bộ đếm thời gian phát tiếng bíp còi** ($300\text{ ms}$ mỗi khi nhấn phím). Tự động đếm lùi về 0 trong ngắt SysTick 1ms. |
| **11** | `alarm_ringing` | `uchar` (`uint8_t`) | **Cờ báo chuông Báo thức đang reng**: `1` (Đang reng chuông khi đồng hồ trùng khớp `alarm_hour:alarm_min`), `0` (Tắt chuông). |
| **12** | `alarm_ring_ms` | `uint` (`uint32_t`) | **Bộ đếm thời gian Báo thức đang reng** (tính bằng ms). Đếm đủ $5,000\text{ ms}$ ($5\text{ s}$) sẽ tự động tắt còi Báo thức. |
| **13** | `buzzer_active` | `uchar` (`uint8_t`) | **Cờ đóng/ngắt còi Buzzer phần cứng** (Chân `P3.0`): `1` (Đóng mạch phát chuỗi xung 4kHz kêu tít tít), `0` (Ngắt còi im lặng). |
