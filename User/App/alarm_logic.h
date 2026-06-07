#ifndef ALARM_LOGIC_H_
#define ALARM_LOGIC_H_

#include "sensor_service.h"

typedef enum {
    ALARM_NONE = 0,
    ALARM_WARNING,
    ALARM_CRITICAL,          // Đồng bộ nhãn trạng thái cao nhất
    ALARM_SUPERVISORY,
    ALARM_DISABLED
} AlarmLevel_t;

// Khai báo extern để main.c hoặc alarm_logic.c đều hiểu chung một biến
extern volatile uint8_t is_buzzer_muted;

void AlarmLogic_Init(void);
void Alarm_SetUserMute(void);
AlarmLevel_t Alarm_ProcessLogic(SensorData_t *data);
void Alarm_ExecuteAction(AlarmLevel_t level);

#endif /* ALARM_LOGIC_H_ */