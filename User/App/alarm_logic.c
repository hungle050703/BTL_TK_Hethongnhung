#include "alarm_logic.h"
#include "main.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// SỬA LỖI LINKER: Khai báo extern để tham chiếu biến từ file gpio.c, không định nghĩa lại ô nhớ
extern volatile uint8_t is_buzzer_muted;
extern UART_HandleTypeDef huart3;

char alarm_status_text[24] = "AN TOAN!";
UG_COLOR alarm_status_color = C_WHITE;

/**
  * @brief  Khởi tạo trạng thái ban đầu cho hệ thống cảnh báo ngoại vi
  */
void AlarmLogic_Init(void) {
    HAL_GPIO_WritePin(BUZZER_ALARM_GPIO_Port, BUZZER_ALARM_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_RED_ALARM_GPIO_Port, LED_RED_ALARM_Pin, GPIO_PIN_RESET);
}

void Process_Alarm_System(float temperature, int smoke, int fire_detected) {
    int current_status = ST_AN_TOAN;
    int danger_count = 0;
    char tx_buffer[64];
    bool sensor_missing = (temperature == 0.0f || temperature == -999.0f || temperature < -50.0f || smoke > 100);
    bool temp_danger = (temperature > 55.0f);
    bool smoke_danger = (smoke > 50);
    bool fire_danger = (fire_detected == 1);

    if (sensor_missing) {
        current_status = ST_MAT_CAM_BIEN;
    } else {
        if (temp_danger) danger_count++;
        if (smoke_danger) danger_count++;
        if (fire_danger) danger_count++;

        if (danger_count >= 2) {
            current_status = ST_HOA_HOAN;
        } else if (danger_count == 1) {
            current_status = ST_DE_PHONG;
        } else {
            current_status = ST_AN_TOAN;
        }
    }

    switch (current_status) {
        case ST_MAT_CAM_BIEN:
            strcpy(alarm_status_text, "MAT CAM BIEN");
            alarm_status_color = C_LIGHT_GRAY;
            break;
        case ST_DE_PHONG:
            strcpy(alarm_status_text, "DE PHONG!");
            alarm_status_color = C_YELLOW;
            break;
        case ST_HOA_HOAN:
            strcpy(alarm_status_text, "HOA HOAN!!!");
            alarm_status_color = C_RED;
            break;
        default:
            strcpy(alarm_status_text, "AN TOAN!");
            alarm_status_color = C_WHITE;
            break;
    }

    snprintf(tx_buffer, sizeof(tx_buffer), "*%d,%.1f,%d,%d#\r\n",
             current_status, temperature, smoke, fire_detected);

    if (current_status != ST_AN_TOAN) {
        HAL_UART_Transmit(&huart3, (uint8_t*)tx_buffer, strlen(tx_buffer), 50);
    }
}

/**
  * @brief  Hàm xử lý tắt tiếng còi (Mute) khi nhận tín hiệu từ Nút nhấn (EXTI hoặc Polling)
  */
void Alarm_SetUserMute(void) {
    static uint32_t last_exti_tick = 0;
    uint32_t current_tick = HAL_GetTick();
    
    // Đọc nút Mute từ GPIO (Kiểm tra chân PC13 hoặc chân định nghĩa trong main.h)
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET) {
        // Debounce chống rung nút nhấn bằng phần mềm (200ms)
        if (current_tick - last_exti_tick > 200) {
            is_buzzer_muted = 1; // Kích hoạt trạng thái ngắt còi tạm thời
            last_exti_tick = current_tick;
            printf("[HANH DONG NGUOI DUNG] Da tat Buzzer bang nut bam tai %lu ms.\r\n", current_tick);
        }
    }
}

/**
  * @brief  Xử lý logic cảnh báo phân tầng dựa trên thuật toán Sensor Fusion 3 cấp
  * @param  data: Con trỏ cấu trúc chứa toàn bộ dữ liệu cảm biến (Nhiệt độ, Khói, Lửa)
  * @retval AlarmLevel_t: Cấp độ báo động (ALARM_NONE, ALARM_WARNING, ALARM_CRITICAL)
  */
AlarmLevel_t Alarm_ProcessLogic(SensorData_t *data) {
    if (data == NULL) return ALARM_NONE;
    
    static AlarmLevel_t previous_level = ALARM_NONE;

    // -------------------------------------------------------------------------
    // TẦNG 3: HOẢ HOẠN THỰC TẾ (Sensor Fusion - Đồng pha các cảm biến)
    // -------------------------------------------------------------------------
    if (data->fire_detected == 1 && data->temperature > 50.0f && data->smoke_conc > 2.0f) {
        previous_level = ALARM_CRITICAL;
        return ALARM_CRITICAL; 
    }
    
    // XỬ LÝ QUÁN TÍNH NHIỆT (Hysteresis - Giữ trạng thái nếu chưa thực sự an toàn)
    if (previous_level == ALARM_CRITICAL) {
        if (data->temperature > 42.0f && data->fire_detected == 1) {
            return ALARM_CRITICAL; // Khóa chặt mức nguy hiểm cho đến khi hạ nhiệt hẳn
        }
    }
    
    // -------------------------------------------------------------------------
    // TẦNG 2: CẢNH BÁO NGUY CƠ (Chớm vượt ngưỡng đơn lẻ của bất kỳ sensor nào)
    // -------------------------------------------------------------------------
    if (data->temperature > 40.0f || data->smoke_conc > 1.2f || data->fire_detected == 1) {
        previous_level = ALARM_WARNING;
        return ALARM_WARNING;
    }
    
    // -------------------------------------------------------------------------
    // TẦNG 1: AN TOÀN HOÀN TOÀN (Tự động giải phóng hệ thống)
    // -------------------------------------------------------------------------
    is_buzzer_muted = 0; // Tự động reset lại cờ im lặng khi hệ thống đã hết cháy hẳn
    previous_level = ALARM_NONE;
    return ALARM_NONE;
}

/**
  * @brief  Thực thi các hành vi phần cứng dựa trên cấp độ báo động của hệ thống
  * @param  level: Cấp độ báo động hiện hành
  */
void Alarm_ExecuteAction(AlarmLevel_t level) {
    switch(level) {
        case ALARM_CRITICAL:
            // Tác vụ nguy hiểm: Cho nháy LED đỏ liên tục để báo động thị giác
            HAL_GPIO_TogglePin(LED_RED_ALARM_GPIO_Port, LED_RED_ALARM_Pin);
            
            // Nếu người dùng chưa ấn nút Mute -> Hú còi liên tục
            if (is_buzzer_muted == 0) {
                HAL_GPIO_WritePin(BUZZER_ALARM_GPIO_Port, BUZZER_ALARM_Pin, GPIO_PIN_SET);
            } else {
                // Nếu đã ấn Mute -> Ép còi tắt im lặng
                HAL_GPIO_WritePin(BUZZER_ALARM_GPIO_Port, BUZZER_ALARM_Pin, GPIO_PIN_RESET);
            }
            break;
            
        case ALARM_WARNING:
            // Tác vụ cảnh cáo: Sáng im LED đỏ cố định, giữ còi im lặng
            HAL_GPIO_WritePin(LED_RED_ALARM_GPIO_Port, LED_RED_ALARM_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(BUZZER_ALARM_GPIO_Port, BUZZER_ALARM_Pin, GPIO_PIN_RESET);
            break;
            
        default:
            // Tác vụ an toàn: Tắt sạch toàn bộ thiết bị ngoại vi báo động
            HAL_GPIO_WritePin(BUZZER_ALARM_GPIO_Port, BUZZER_ALARM_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED_RED_ALARM_GPIO_Port, LED_RED_ALARM_Pin, GPIO_PIN_RESET);
            break;
    }
}
