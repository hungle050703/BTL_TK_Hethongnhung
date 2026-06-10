#include "alarm_logic.h"
#include "mq2.h"
#include <string.h>
#include <stdio.h>
#include "main.h"

extern UART_HandleTypeDef huart3;

// Khai báo các biến toàn cục mà app_freertos.c đang external để đồng bộ hiển thị
char alarm_status_text[32] = "AN TOAN";
UG_COLOR alarm_status_color = C_GREEN;
static uint8_t is_muted = 0;
static AlarmLevel_t last_alarm_state = ALARM_NONE;

void AlarmLogic_Init(void) {
    is_muted = 0;
    last_alarm_state = ALARM_NONE;
    // Khởi tạo tất cả LED/buzzer ở trạng thái tắt
    HAL_GPIO_WritePin(LED_GREEN_ON_GPIO_Port, LED_GREEN_ON_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_YELLOW_ALARM_GPIO_Port, LED_YELLOW_ALARM_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_RED_ALARM_GPIO_Port, LED_RED_ALARM_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BUZZER_ALARM_GPIO_Port, BUZZER_ALARM_Pin, GPIO_PIN_RESET);
}

AlarmLevel_t Alarm_ProcessLogic(SensorData_t *data) {
    if (data == NULL) return ALARM_NONE;

    int active_sensors = 0;

    // 1. Kiểm tra Nhiệt độ (> 45.0 độ C)
    if (data->temperature > 45.0f) {
        active_sensors++;
    }

    // 2. Kiểm tra Khói MQ-2 (Vượt điện áp nền + 0.5V)
    float base_v = MQ2_GetBaseVoltage();
    if (data->smoke_conc > (base_v + 0.5f)) {
        active_sensors++;
    }

    // 3. Kiểm tra Cảm biến lửa Hồng ngoại
    if (data->fire_detected == 1) {
        active_sensors++;
    }

    // --- PHÂN CHIA TRẠNG THÁI THEO YÊU CẦU ---
    AlarmLevel_t current_level = ALARM_NONE;

    if (active_sensors >= 2) {
        strcpy(alarm_status_text, "HOA HOAN !");
        alarm_status_color = C_RED;
        current_level = ALARM_CRITICAL;
    }
    else if (active_sensors == 1) {
        strcpy(alarm_status_text, "DE PHONG");
        alarm_status_color = C_YELLOW;
        current_level = ALARM_WARNING;
    }
    else {
        strcpy(alarm_status_text, "AN TOAN");
        alarm_status_color = C_GREEN;
        is_muted = 0; // Tự động reset trạng thái im lặng khi hệ thống an toàn
        current_level = ALARM_NONE;
    }

    // Áp dụng cơ chế nhấn nút tắt còi MUTE (nhưng vẫn hiển thị trạng thái)
    if (is_muted && (current_level != ALARM_NONE)) {
        return ALARM_NONE;  // Trả về AN_TOAN nhưng không reset is_muted
    }

    return current_level;
}

void Alarm_ExecuteAction(AlarmLevel_t level) {
    switch(level) {
        case ALARM_CRITICAL:  // HOA_HOAN
            HAL_GPIO_WritePin(LED_GREEN_ON_GPIO_Port, LED_GREEN_ON_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED_YELLOW_ALARM_GPIO_Port, LED_YELLOW_ALARM_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED_RED_ALARM_GPIO_Port, LED_RED_ALARM_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(BUZZER_ALARM_GPIO_Port, BUZZER_ALARM_Pin, GPIO_PIN_SET);
            break;
            
        case ALARM_WARNING:  // DE_PHONG
            HAL_GPIO_WritePin(LED_GREEN_ON_GPIO_Port, LED_GREEN_ON_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED_YELLOW_ALARM_GPIO_Port, LED_YELLOW_ALARM_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED_RED_ALARM_GPIO_Port, LED_RED_ALARM_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(BUZZER_ALARM_GPIO_Port, BUZZER_ALARM_Pin, GPIO_PIN_SET);
            break;
            
        default:  // ALARM_NONE (AN_TOAN)
            HAL_GPIO_WritePin(LED_GREEN_ON_GPIO_Port, LED_GREEN_ON_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED_YELLOW_ALARM_GPIO_Port, LED_YELLOW_ALARM_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED_RED_ALARM_GPIO_Port, LED_RED_ALARM_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(BUZZER_ALARM_GPIO_Port, BUZZER_ALARM_Pin, GPIO_PIN_RESET);
            break;
    }
}

void Process_Alarm_System(float temperature, int smoke, int fire_detected) {
    // Xác định trạng thái hiện tại
    int active_sensors = 0;
    if (temperature > 45.0f) active_sensors++;
    if (smoke > 50) active_sensors++;  // Để ý: smoke ở đây là percentage (0-100)
    if (fire_detected == 1) active_sensors++;
    
    AlarmLevel_t current_state = ALARM_NONE;
    int state_code = 1;  // 1 = AN_TOAN, 2 = DE_PHONG, 3 = HOA_HOAN
    
    if (active_sensors >= 2) {
        current_state = ALARM_CRITICAL;
        state_code = 3;
    } else if (active_sensors == 1) {
        current_state = ALARM_WARNING;
        state_code = 2;
    } else {
        current_state = ALARM_NONE;
        state_code = 1;
    }
    
    // Chỉ gửi UART khi trạng thái thay đổi (tránh spam)
    if (current_state != last_alarm_state) {
        char tx_buffer[64];
        sprintf(tx_buffer, "*%d,%.1f,%d,%d#\r\n", state_code, temperature, smoke, fire_detected);
        HAL_UART_Transmit(&huart3, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
        last_alarm_state = current_state;
    }
}

void Alarm_SetUserMute(void) {
    // Đọc nút nhấn (PC13 - Active High trên STM32H5)
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET) {
        // Debounce: chỉ xử lý một lần
        static uint32_t last_press_time = 0;
        uint32_t current_time = HAL_GetTick();
        
        if (current_time - last_press_time > 200) {  // Debounce 200ms
            is_muted = 1;  // Kích hoạt mute
            last_press_time = current_time;
            
            // Force trở về AN_TOAN: tắt tất cả LED cảnh báo, chỉ bật LED xanh, tắt buzzer
            HAL_GPIO_WritePin(LED_GREEN_ON_GPIO_Port, LED_GREEN_ON_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED_YELLOW_ALARM_GPIO_Port, LED_YELLOW_ALARM_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED_RED_ALARM_GPIO_Port, LED_RED_ALARM_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(BUZZER_ALARM_GPIO_Port, BUZZER_ALARM_Pin, GPIO_PIN_RESET);
        }
    }
}
