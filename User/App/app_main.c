#include <stdint.h>
#include "cmsis_os2.h"
#include "main.h"
#include <stdio.h>
/* App-level includes */
#include "display/display_panel.h"
#include "../Services/display/display_service.h"
#include "../Services/sensor_service.h"
#include "alarm_logic.h"

/* Forward declares from other units */
extern void LCD_init(void);
extern SensorData_t myData;
extern volatile uint8_t is_buzzer_muted;

void App_Main(void)
{
    /* Minimal LCD + GUI init and initialise sensor/alarm subsystems */
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
        SensorService_Update(&myData);
        Alarm_SetUserMute();  // Đọc nút Mute từ GPIO
        
        AlarmLevel_t current_level = Alarm_ProcessLogic(&myData);
        Alarm_ExecuteAction(current_level);  // Bật LED, Buzzer dựa trên level

        uint8_t fire = myData.fire_detected ? 1 : 0;
        uint8_t trouble = (uint8_t)myData.temperature;
        uint8_t supervisor = (uint8_t)(myData.smoke_conc * 100.0f);
        uint8_t disable = is_buzzer_muted ? 1 : 0;

        Display_FirePanelStatus(&renderer, fire, trouble, supervisor, disable);

        // Log chi tiết mỗi 500ms
        static uint32_t last_app_log = 0;
        uint32_t current_tick = HAL_GetTick();
        if (current_tick - last_app_log >= 500) {
            printf("[UNG DUNG CHINH] Muc:%d | Lua:%d | Nhiet do:%.1fC | Khoi:%.2fppm | DO:%d | AO:%.2fV | Da tat:%d\r\n",
                   current_level, fire, myData.temperature, myData.smoke_conc,
                   myData.mh_sensor_do, myData.mh_sensor_ao_volt, disable);
            last_app_log = current_tick;
        }

        osDelay(300);
    }
}
