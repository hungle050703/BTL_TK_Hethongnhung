#include "esp32_service.h"
#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef huart3; // Đảm bảo cấu hình USART3 trong CubeMX

void ESP32Service_Init(void) {
    char *msg = "=== STM32 READY ===\n";
    HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), 100);
}

void ESP32Service_Process(void) {
    static SystemState_t previous_state = SYS_STATE_AN_TOAN;
    SystemState_t current_state;

    // Đọc trạng thái cảm biến (PA1)
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_RESET) {
        current_state = SYS_STATE_DE_PHONG;
    } else {
        current_state = SYS_STATE_AN_TOAN;
    }

    // Gửi lệnh khi có sự thay đổi
    if (current_state != previous_state) {
        char tx_buffer[64];
        if (current_state == SYS_STATE_DE_PHONG) {
            sprintf(tx_buffer, "TRANG THAI: DE PHONG\n");
        } else {
            sprintf(tx_buffer, "TRANG THAI: AN TOAN\n");
        }

        HAL_UART_Transmit(&huart3, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
        previous_state = current_state;
    }
}
