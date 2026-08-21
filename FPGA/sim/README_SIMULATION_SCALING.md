# THUYẾT MINH KỸ THUẬT: LƯU Ý VỀ TỶ LỆ THỜI GIAN VÀ TẦN SỐ TRONG MÔ PHỎNG FPGA
*(SIMULATION TIME ACCELERATION & PARAMETER SCALING METHODOLOGY)*

Báo cáo này thuyết minh chi tiết về kỹ thuật co ngắn thời gian mô phỏng (**Simulation Time Acceleration**) được áp dụng trong kịch bản kiểm thử Testbench (`tb_top_system_v2.v`), giúp Ban Giám Khảo và người đánh giá dễ dàng đối chiếu dạng sóng trên ModelSim với hoạt động thực tế trên bo mạch phần cứng **Kiwi Nano 4K**.

---

## 1. TỔNG QUAN VỀ KỸ THUẬT TĂNG TỐC MÔ PHỎNG (SIMULATION ACCELERATION)

Trong thiết kế vi mạch ASIC/FPGA chuyên nghiệp, các chu kỳ hoạt động thực tế kéo dài hàng giây (như hiệu ứng LED Thở $2.0\text{s}$) nếu chạy mô phỏng nguyên bản trên phần mềm ModelSim sẽ mất hàng triệu nhịp đếm xung clock, gây đơ giật phần mềm và tốn thời gian tính toán của máy tính.

Do đó, dự án áp dụng kỹ thuật **Truyền đè tham số (Parameter Overriding)** để thu hẹp khoảng thời gian quan sát trên phần mềm mô phỏng nhưng **bảo toàn 100% tính đúng đắn của logic thiết kế**.

---

## 2. BẢNG SO SÁNH THÔNG SỐ: MÔ PHỎNG VS MẠCH THẬT PHẦN CỨNG

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

## 3. CHỨNG MINH TÍNH TƯƠNG ĐƯƠNG VỀ LOGIC (FUNCTIONAL EQUIVALENCE)

Mặc dù tần số xung PWM được nâng lên $50\text{kHz}$ trong mô phỏng, toàn bộ thuật toán điều khiển vẫn hoàn toàn tương đương với mạch thật $1\text{kHz}$:

1. **Tỷ lệ Duty Cycle không đổi**:
   - Mode LOW: $\text{Duty} = \frac{5.0\mu\text{s}}{20.0\mu\text{s}} = \mathbf{25.0\%}$.
   - Mode HIGH: $\text{Duty} = \frac{20.0\mu\text{s}}{20.0\mu\text{s}} = \mathbf{100.0\%}$.
2. **Quy trình Thở (Breathing) tuyến tính mịn màng**:
   - Một vòng thở được chia đều thành **2,000 nấc điều chỉnh độ sáng** (1,000 nấc tăng từ $0\% \rightarrow 100\%$ và 1,000 nấc giảm từ $100\% \rightarrow 0\%$).
   - Trong mô phỏng: $2,000 \text{ nấc} \times 20\mu\text{s} = \mathbf{40\text{ ms}}$.
   - Trên mạch thật: $2,000 \text{ nấc} \times 1.0\text{ms} = \mathbf{2.0\text{ giây}}$.
3. **Độ chính xác Baudrate UART**:
   - Khối UART vẫn truyền phát ở tốc độ thực $115,200\text{ bps}$, mỗi bit kéo dài đúng $8.680\mu\text{s}$ ($434$ nhịp clock $50\text{MHz}$), sai số Baudrate đạt $\mathbf{0.006\% \ll 2.0\%}$.

---

## 4. HƯỚNG DẪN DÀNH CHO BAN GIÁM KHẢO KHI ĐỌC DẠNG SÓNG MODELSIM

Khi mở tệp kịch bản mô phỏng `wavefinal.do` trên ModelSim:
- **Đo chu kỳ xung PWM (`led_out`)**: Đặt 2 thước đo Cursor ôm trọn 1 xung PWM màu đỏ $\rightarrow$ Kết quả đo $\Delta T \approx \mathbf{20.0\mu\text{s}}$ (Tương ứng tần số $50\text{kHz}$ đã tăng tốc).
- **Đo thời gian 1 vòng Thở (Mode AUTO)**: Quát từ thời điểm bắt đầu chớm thở $40\text{ns}$ đến khi kết thúc 1 chu trình $\rightarrow$ Kết quả đo $\Delta T = \mathbf{40.0\text{ ms}}$ (Tương ứng trọn vẹn 1 chu kỳ $2.0\text{s}$ trên mạch thật).
- **Đo khung truyền UART (`uart_tx`)**: Đặt thước đo 1 bit màu vàng $\rightarrow$ Kết quả đo $\Delta T = \mathbf{8,680\text{ ns}}$ ($8.68\mu\text{s}$ chuẩn Baudrate $115200\text{ bps}$).

---

### 📌 KẾT LUẬN

Kỹ thuật **Simulation Time Acceleration** giúp bài báo cáo mô phỏng vừa đạt tốc độ hiển thị tối ưu trên phần mềm ModelSim, vừa chứng minh được tính đúng đắn tuyệt đối của thiết kế RTL trên bo mạch thực tế **Kiwi Nano 4K**.
