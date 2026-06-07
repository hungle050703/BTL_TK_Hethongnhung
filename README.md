# BTL_TK_Hethongnhung

## Tài liệu Phân tích & Chuyển đổi Kiến trúc sang RTOS Toàn diện

### 1. Phương pháp hoạt động của cấu trúc cũ (Hiện tại)
Hệ thống hiện tại tuy đã nhúng FreeRTOS nhưng vẫn sử dụng tư duy "Super Loop" (vòng lặp vô hạn dạng bare-metal) đặt bên trong một Task khổng lồ duy nhất:

*   **Tập trung logic (Monolithic Task):** Luồng `defaultTask` gánh vác mọi nhiệm vụ: từ ra lệnh đọc cảm biến, tính toán ngưỡng báo cháy, điều khiển phần cứng (Còi, Đèn) cho đến cả việc đọc trạng thái nút nhấn cơ học thông qua phương pháp "hỏi vòng" (Polling).
*   **Trễ do Polling và State-Machine phức tạp:** Việc đọc cảm biến nhiệt chuẩn 1-Wire (DS18B20) cần chờ 750ms để chip nội bộ chuyển đổi. Để không khóa CPU, cơ chế cũ thiết kế hàm đọc thành 2 state (trạng thái), kết hợp với `osDelay(1000)` ở cuối hàm. Điều này khiến chu kỳ cập nhật một giá trị nhiệt độ mất tối thiểu 2 giây. Các nút nhấn và cảm biến khác cũng có độ trễ lớn vì phải chờ `defaultTask` quay vòng tới.
*   **Quản lý bộ nhớ thiếu đồng bộ (Data Race):** Hệ thống dùng biến toàn cục `myData` (`extern SensorData_t myData`). Task cảm biến ghi đè liên tục vào biến này, trong khi Task 4G có thể xen ngang đọc cấu trúc lúc dữ liệu đang ghi dở, dẫn đến rủi ro rác dữ liệu.
*   **Bỏ sót module báo động (Orphaned Code):** Module `alarm_logic.c` trước đây được viết dành riêng cho việc phân loại ngưỡng cảnh báo (Warning/Critical) nhưng lại đang bị bỏ quên, thay vào đó logic còi/đèn bị viết gộp "lổn nhổn" trực tiếp trong `app_freertos.c`.

### 2. Đề xuất Kiến trúc Mới (Chuyển đổi RTOS Toàn diện)
Mục tiêu là biến hệ thống thành dạng Hướng Sự Kiện (Event-Driven) và chạy đồng thời (Concurrent), chia nhỏ công việc (Separation of Concerns). 

Kiến trúc RTOS mới sẽ thiết kế theo mô hình **3 Task + Cơ chế Đồng bộ hóa & Ngắt**:

#### a. Quy hoạch lại các Luồng (Tasks)
1.  **`Sensor_Task` (Luồng Thu Thập Dữ Liệu):** 
    *   **Nhiệm vụ:** Chuyên gọi driver giao tiếp với phần cứng lớp dưới (DS18B20, MQ2, MH-Sensor).
    *   **Ưu điểm Delay:** Sửa lại logic DS18B20 cho phép dùng hàm `osDelay(750)` cắm trực tiếp giữa bước Ra lệnh đo và Đọc kết quả. Hành động này trả CPU lại cho các task khác làm việc mà không cần viết biến state-machine phức tạp. Đảm bảo dữ liệu trơn tru.
2.  **`Alarm_Logic_Task` (Luồng Nhận thức & Điều khiển Hành vi):** 
    *   **Nhiệm vụ:** Là bộ não trung tâm. Lấy dữ liệu cảm biến sạch về, nhét vào hàm của `alarm_logic.c` để bộ lọc phân loại mức độ nguy hiểm, từ đó ra lệnh điều khiển Còi/Đèn LED, xuất Log theo chu kỳ và bắn Cờ cấp cứu cho Viễn thông nếu có cháy.
3.  **`Comm_4G_Task` (Luồng Viễn thông Khẩn cấp):** 
    *   **Nhiệm vụ:** (Giữ nguyên cấu trúc chờ ưu việt) Luôn ở trạng thái Bị Block chờ Cờ (Event Flag) `EVENT_FIRE_DETECTED`. Khi có cờ lập tức thi hành gọi điện báo cháy nhanh nhất.

#### b. Cải tiến Quản lý Tài nguyên & Sự kiện
*   **Data Mutex (Khóa dữ liệu):** Bao bọc biến cấu trúc `myData` bằng một Mutex (Resource Lock). `Sensor_Task` phải xin được thìa khóa Mutex mới được chép số liệu vào. Khi `Alarm_Task` hoặc `4G_Task` cần đọc, phải có Mutex thì mới lấy. Điều này đảm bảo tính vẹn toàn cho các packet mạng.
*   **Ngắt phần cứng cho Nút nhấn (EXTI):** Loại bỏ việc dùng IF/THEN dò nút ấn trên `defaultTask`. Thay vào đó, chân nút ấn MUTE còi sẽ dùng Ngắt ngoài (EXTI). Khi user ấn, ngắt CPU xảy ra lập tức (< 10ms) và gọi hàm Callback, hàm này sẽ cập nhật biến cờ tắt báo động. Tăng tối đa độ nhạy của hệ thống.

