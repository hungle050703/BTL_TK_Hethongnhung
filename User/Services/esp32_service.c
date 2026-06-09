/*
 * esp32_service.c
 *
 *  Created on: Jun 10, 2026
 *      Author: ADMIN
 */
#include "esp32_service.h"
#include "cmsis_os2.h" // Sử dụng osDelay của FreeRTOS CMSIS-V2
#include <stdio.h>
#include <string.h>

// Khai báo handle UART kết nối với ESP32 (Kiểm tra lại xem bạn dùng UART4 hay UART khác)
extern UART_HandleTypeDef huart4;

// Các biến quản lý trạng thái được đặt là static để bảo mật nội bộ file này
static SystemState_t current_state = SYS_STATE_AN_TOAN;
static SystemState_t previous_state = SYS_STATE_AN_TOAN;
static char tx_buffer[64];

void ESP32Service_Init(void) {
    current_state = SYS_STATE_AN_TOAN;
    previous_state = SYS_STATE_AN_TOAN;

    // Gửi một bản tin chào mừng sang ESP32 khi STM32 vừa khởi động xong
    sprintf(tx_buffer, "=== STM32 READY ===\r\n");
    HAL_UART_Transmit(&huart4, (uint8_t*)tx_buffer, strlen(tx_buffer), 50);
    printf("[ESP32_SERVICE] Init Done. Sent Hello to ESP32.\r\n");
}

void ESP32Service_Process(void) {
    // 1. Đọc tín hiệu từ cảm biến Hồng Ngoại (Giả sử nối vào GPIOA, chân PIN 1)
    // Cảm biến hồng ngoại trên lớp phát hiện lửa/vật cản sẽ kéo chân OUT xuống LOW (0)
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_RESET)
    {
        current_state = SYS_STATE_DE_PHONG;
    }
    else
    {
        current_state = SYS_STATE_AN_TOAN;
    }

    // 2. Thuật toán sườn xung (Edge-Triggered) - Chỉ truyền khi ĐỔI trạng thái
    if (current_state != previous_state)
    {
        if (current_state == SYS_STATE_DE_PHONG)
        {
            sprintf(tx_buffer, "=== STATE_CHANGED: DE PHONG ===\r\n");
            HAL_UART_Transmit(&huart4, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
            printf("[UART TO ESP32] >>> Gửi lệnh: DE PHONG\r\n");
        }
        else if (current_state == SYS_STATE_AN_TOAN)
        {
            sprintf(tx_buffer, "=== STATE_CHANGED: AN TOAN ===\r\n");
            HAL_UART_Transmit(&huart4, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
            printf("[UART TO ESP32] >>> Gửi lệnh: AN TOAN\r\n");
        }

        // Cập nhật mốc trạng thái cũ
        previous_state = current_state;
    }
}

