#include "alarm_logic.h"
#include "main.h"
#include <stdio.h> 

// SỬA LỖI LINKER: Khai báo extern để tham chiếu biến từ file gpio.c, không định nghĩa lại ô nhớ
extern volatile uint8_t is_buzzer_muted;

/**
  * @brief  Khởi tạo trạng thái ban đầu cho hệ thống cảnh báo ngoại vi
  */
void AlarmLogic_Init(void) {
    HAL_GPIO_WritePin(BUZZER_ALARM_GPIO_Port, BUZZER_ALARM_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_RED_ALARM_GPIO_Port, LED_RED_ALARM_Pin, GPIO_PIN_RESET);
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
