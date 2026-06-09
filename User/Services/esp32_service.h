/*
 * esp32_service.h
 *
 *  Created on: Jun 10, 2026
 *      Author: ADMIN
 */

#ifndef SERVICES_ESP32_SERVICE_H_
#define SERVICES_ESP32_SERVICE_H_

#include "main.h"

// Định nghĩa các trạng thái hệ thống rút gọn để test trên lớp
typedef enum {
    SYS_STATE_AN_TOAN = 0,
    SYS_STATE_DE_PHONG
} SystemState_t;

/* Khởi tạo luồng/dịch vụ giao tiếp ESP32 */
void ESP32Service_Init(void);

/* Hàm thực thi quét cảm biến và truyền UART (Sẽ gọi trong Task FreeRTOS) */
void ESP32Service_Process(void);

#endif /* SERVICES_ESP32_SERVICE_H_ */
