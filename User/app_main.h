#ifndef APP_MAIN_H_
#define APP_MAIN_H_

#include "sensor_service.h"

// Khởi tạo toàn bộ phân hệ phần cứng và dịch vụ nền
void App_Subsystems_Init(void);

// Logic xử lý đọc và tính toán cảm biến (Dành cho Sensor_Task)
void App_Sensor_Process(void);

// Logic điều khiển còi và cập nhật màn hình LCD đồ họa (Dành cho Alarm_Logic_Task)
void App_Alarm_And_Display_Execute(void);

// Logic đóng gói chuỗi dữ liệu chuẩn gửi sang ESP32 qua UART (Dành cho Comm_4G_Task)
void App_Communication_Transmit(void);

#endif /* APP_MAIN_H_ */
