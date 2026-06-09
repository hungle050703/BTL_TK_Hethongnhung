#include "alarm_logic.h"
#include "mq2.h"
#include <string.h>

// Khai báo các biến toàn cục mà app_freertos.c đang external để đồng bộ hiển thị
char alarm_status_text[32] = "AN TOAN";
UG_COLOR alarm_status_color = C_GREEN;
static uint8_t is_muted = 0;

void AlarmLogic_Init(void) {
    is_muted = 0;
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

    // --- PHÂN CHIA TRẠNG THÁI THEO YÊU CẦU CỦA THẦY ---
    AlarmLevel_t current_level = ALARM_NONE;

    if (active_sensors >= 2) {
        strcpy(alarm_status_text, "HOA HOAN !");
        alarm_status_color = C_RED;
        current_level = ALARM_CRITICAL; // Khớp Enum cũ trong project của bạn
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

    // Áp dụng cơ chế nhấn nút tắt còi MUTE
    if (is_muted && (current_level != ALARM_NONE)) {
        return ALARM_NONE;
    }

    return current_level;
}

void Alarm_ExecuteAction(AlarmLevel_t level) {
    // Kiểm tra nút nhấn B1 trên kit (PC13 - Active High trên dòng H5)
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET) {
        is_muted = 1;
    }

    // Điều khiển LED Đỏ PG4 làm còi/đèn báo
    if (level != ALARM_NONE) {
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, GPIO_PIN_RESET);
    }
}

// Hàm Wrapper bọc luồng 4G cũ để giữ an toàn hệ thống
void Process_Alarm_System(float temperature, int smoke, int fire_detected) {
    // Để trống vì logic đếm 3 trạng thái mới đã xử lý triệt để bên trên
}

void Alarm_SetUserMute(void) {
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET) {
        is_muted = 1;
    }
}
