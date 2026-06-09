#ifndef ALARM_LOGIC_H_
#define ALARM_LOGIC_H_

#include "sensor_service.h"
#include "ugui.h"

#define ST_AN_TOAN       1
#define ST_MAT_CAM_BIEN  2
#define ST_DE_PHONG      3
#define ST_HOA_HOAN      4

typedef enum {
    ALARM_NONE = 0,
    ALARM_WARNING,
    ALARM_CRITICAL,          // Đồng bộ nhãn trạng thái cao nhất
    ALARM_SUPERVISORY,
    ALARM_DISABLED
} AlarmLevel_t;

// Khai báo extern để main.c hoặc alarm_logic.c đều hiểu chung một biến
extern volatile uint8_t is_buzzer_muted;
extern char alarm_status_text[24];
extern UG_COLOR alarm_status_color;

void AlarmLogic_Init(void);
void Alarm_SetUserMute(void);
AlarmLevel_t Alarm_ProcessLogic(SensorData_t *data);
void Alarm_ExecuteAction(AlarmLevel_t level);

#endif /* ALARM_LOGIC_H_ */