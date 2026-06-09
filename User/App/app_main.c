/*
 * ============================================================
 * WARNING: This file (app_main.c) is DEPRECATED and NOT USED
 * ============================================================
 * The FreeRTOS task system now uses Core/Src/app_freertos.c
 * to manage Sensor_Task, Alarm_Logic_Task, and Comm_4G_Task.
 * 
 * Keep this file for reference only. Do NOT call App_Main().
 * ============================================================
 */

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

/* Forward declares từ các unit khác */
extern void LCD_init(void);
extern SensorData_t myData;
extern volatile uint8_t is_buzzer_muted;

void App_Main(void)
{
    /* Khởi tạo phân hệ hiển thị và phân hệ xử lý dịch vụ cảm biến */
    LCD_init();
    SensorService_Init();
    AlarmLogic_Init();

    const DisplayRenderer renderer = {
        .Clear = Display_Clear,
        .DrawHeader = Display_DrawHeader,
        .DrawLine = Display_DrawLine,
        .DrawFooter = Display_DrawFooter,
        .DrawString = Display_DrawString,
    };

    while (1)
    {
        // 1. Cập nhật dữ liệu từ tầng phần cứng (Non-blocking)
        SensorService_Update(&myData);
        Alarm_SetUserMute();  // Đọc dữ liệu nút nhấn bảo vệ còi
        
        // 2. Tính toán phân tầng mức độ cảnh báo cháy
        AlarmLevel_t current_level = Alarm_ProcessLogic(&myData);
        Alarm_ExecuteAction(current_level);

        float smoke_v = myData.smoke_conc;
        float smoke_percentage = (smoke_v / 5.0f) * 100.0f;
        int supervisor = (int)smoke_percentage;
        float base_v = MQ2_GetBaseVoltage();
        bool fire_danger = (myData.mh_sensor_do == 0);
        uint8_t disable = is_buzzer_muted ? 1 : 0;
        uint8_t fire = myData.fire_detected ? 1 : 0;

        // 3. Bắn dữ liệu UART truyền tin báo cháy sang Module ESP32 4G
        Process_Alarm_System(myData.temperature, supervisor, fire);

        // 4. Cập nhật giao diện màn hình LCD tầng đồ họa
        Display_FirePanelStatus(&renderer, alarm_status_text, alarm_status_color,
                                 fire, (uint8_t)myData.temperature, supervisor, disable);

        // 5. Quản lý in Log tập trung qua cổng debug UART định kỳ 1000ms
        static uint32_t last_app_log = 0;
        uint32_t current_tick = HAL_GetTick();
        if (current_tick - last_app_log >= 1000) {
            const char* smoke_status;
            if (smoke_v < base_v + 0.2f) {
                smoke_status = "SACH";
            } else if (smoke_v < base_v + 0.6f) {
                smoke_status = "CO KHOI NHE";
            } else {
                smoke_status = "NGUY HIEM!";
            }
            const char* mh_status = fire_danger ? "CO LUA/VAT CAN!!" : "BINH THUONG";

            // Hiển thị trạng thái DS18B20 trực quan: Nếu lỗi hiện "MAT CAM BIEN" thay vì 0.0C
            if (myData.temperature == -999.0f) {
                printf("[UNG DUNG CHINH] STATUS: MAT CAM BIEN\r\n");
                printf("[UNG DUNG CHINH] Nhiet do:  0.00 C\r\n");
            } else {
                printf("[UNG DUNG CHINH] STATUS: %s\r\n", alarm_status_text);
                printf("[UNG DUNG CHINH] Nhiet do: %5.2f C\r\n", myData.temperature);
            }

            printf("[UNG DUNG CHINH]   Khoi MQ-2:   %4.1f %% - %s (Base: %.2fV | Now: %.2fV)\r\n",
                   smoke_percentage, smoke_status, base_v, smoke_v);
            printf("[UNG DUNG CHINH]   Lua MH-SENS: %s (AO: %.2fV)\r\n",
                   mh_status, myData.mh_sensor_ao_volt);
            printf("[UNG DUNG CHINH] ----------------------------------------\r\n");
            last_app_log = current_tick;
        }

        // 🟢 Nhường quyền điều phối luồng đa nhiệm chu kỳ 300ms của FreeRTOS
        osDelay(300);
    }
}
