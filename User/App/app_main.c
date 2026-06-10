/*
 * ===========================================================================
 * MODULE: app_main.c
 * DESCRIPTION: Định nghĩa các hàm Core Service cho hệ thống cảnh báo cháy.
 * Các hàm này sẽ được gọi trực tiếp bên trong các Task tương ứng
 * của FreeRTOS (nằm tại Core/Src/app_freertos.c).
 * ===========================================================================
 */

#include "app_main.h"
#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os2.h"
#include "main.h"
#include <stdio.h>

/* App-level includes */
#include "display/display_panel.h"
#include "../Services/display/display_service.h"
#include "../Services/sensor_service.h"
#include "../Drivers/mq2.h"
#include "alarm_logic.h"

/* Biến toàn cục link từ các Unit khác */
extern void LCD_init(void);
extern SensorData_t myData;
extern volatile uint8_t is_buzzer_muted;

/* Định nghĩa renderer cho màn hình đồ họa LCD */
static const DisplayRenderer renderer = {
    .Clear = Display_Clear,
    .DrawHeader = Display_DrawHeader,
    .DrawLine = Display_DrawLine,
    .DrawFooter = Display_DrawFooter,
    .DrawString = Display_DrawString,
};

/**
 * @brief Khởi tạo toàn bộ các phân hệ ngoại vi và dịch vụ ứng dụng
 */
void App_Subsystems_Init(void)
{
    LCD_init();
    SensorService_Init();
    AlarmLogic_Init();
}

/**
 * @brief Hàm xử lý cập nhật cảm biến và tính toán tầng cảnh báo
 * @note  Tần suất khuyến nghị gọi trong FreeRTOS: 100ms - 200ms (Sensor_Task)
 */
void App_Sensor_Process(void)
{
    // 1. Cập nhật dữ liệu từ tầng HAL phần cứng (Non-blocking)
    SensorService_Update(&myData);

    // 2. Đọc dữ liệu nút nhấn bảo vệ tắt tiếng còi
    Alarm_SetUserMute();

    // 3. Tính toán phân tầng mức độ cảnh báo cháy theo thuật toán lõi
    AlarmLevel_t current_level = Alarm_ProcessLogic(&myData);
    Alarm_ExecuteAction(current_level);
}

/**
 * @brief Hàm cập nhật LCD Đồ họa và In Log Debug qua cổng UART Máy tính
 * @note  Tần suất khuyến nghị gọi trong FreeRTOS: 500ms - 1000ms (Alarm_Logic_Task)
 */
void App_Alarm_And_Display_Execute(void)
{
    float smoke_v = myData.smoke_conc;
    float smoke_percentage = (smoke_v / 5.0f) * 100.0f;
    int supervisor = (int)smoke_percentage;
    float base_v = MQ2_GetBaseVoltage();
    bool fire_danger = (myData.mh_sensor_do == 0);
    uint8_t disable = is_buzzer_muted ? 1 : 0;
    uint8_t fire = fire_danger ? 1 : 0;

    // 1. Cập nhật giao diện màn hình LCD tầng đồ họa
    Display_FirePanelStatus(&renderer, alarm_status_text, alarm_status_color,
                            fire, (uint8_t)myData.temperature, supervisor, disable);

    // 2. Quản lý in Log tập trung qua cổng debug UART về máy tính
    const char* smoke_status;
    if (smoke_v < base_v + 0.2f) {
        smoke_status = "SACH";
    } else if (smoke_v < base_v + 0.6f) {
        smoke_status = "CO KHOI NHE";
    } else {
        smoke_status = "NGUY HIEM!";
    }
    const char* mh_status = fire_danger ? "CO LUA/VAT CAN!!" : "BINH THUONG";

    // ÉP DÒNG STATUS PANEL CHỈ IN TRẠNG THÁI CHÍNH (Xóa bỏ logic check lỗi hiện chữ MAT CAM BIEN cũ)
    printf("[STM32_DEBUG] STATUS PANEL: %s\r\n", alarm_status_text);
    
    if (myData.temperature == -999.0f) {
        printf("[STM32_DEBUG] Nhiet do:  0.00 C (Loi ket noi DS18B20)\r\n");
    } else {
        printf("[STM32_DEBUG] Nhiet do: %5.2f C\r\n", myData.temperature);
    }
    
    printf("[STM32_DEBUG]    Khoi MQ-2:   %4.1f %% - %s (Base: %.2fV | Now: %.2fV)\r\n",
           smoke_percentage, smoke_status, base_v, smoke_v);
    printf("[STM32_DEBUG]    Lua MH-SENS: %s (AO: %.2fV)\r\n", mh_status, myData.mh_sensor_ao_volt);
    printf("[STM32_DEBUG] ----------------------------------------\r\n");
}

/**
 * @brief Hàm chuẩn hóa gói tin và bắn qua UART giao tiếp sang ESP32
 * @note  Tần suất khuyến nghị gọi trong FreeRTOS: 300ms - 500ms (Comm_4G_Task)
 */
void App_Communication_Transmit(void)
{
    float smoke_v = myData.smoke_conc;
    float smoke_percentage = (smoke_v / 5.0f) * 100.0f;
    int supervisor = (int)smoke_percentage;
    uint8_t fire = (myData.mh_sensor_do == 0) ? 1 : 0;

    // Truyền dữ liệu trạng thái sạch sang ESP32
    Process_Alarm_System(myData.temperature, supervisor, fire);
}