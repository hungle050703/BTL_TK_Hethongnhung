#ifndef ALARM_LOGIC_H_
#define ALARM_LOGIC_H_

#include "../Services/sensor_service.h" // Sửa lại đường dẫn tương đối cho chuẩn nếu cần
#include "ugui.h"

// Định nghĩa mã trạng thái số để đóng gói truyền UART sang ESP32
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

// 🟢 CHÚ Ý ĐỒNG BỘ TÊN BIẾN: Trong app_main.c bạn đang gọi `alarm_status_text` và `alarm_status_color`
// Nếu trong alarm_logic.c bạn khai báo mảng tĩnh, hãy để kiểu extern char[] như thế này:
extern char alarm_status_text[];
extern UG_COLOR alarm_status_color;

// Thêm các biến text/color cũ nếu bạn vẫn dùng song song cho logic nội bộ
extern char* status_text;
extern uint16_t status_color;

/* ==================================================================== */
/* CÁC NGUYÊN MẪU HÀM HỆ THỐNG                     */
/* ==================================================================== */

void AlarmLogic_Init(void);
void Alarm_SetUserMute(void);
AlarmLevel_t Alarm_ProcessLogic(SensorData_t *data);
void Alarm_ExecuteAction(AlarmLevel_t level);

// 🟢 BỔ SUNG DÒNG NÀY: Để app_main.c nhìn thấy hàm xử lý phân tầng và truyền UART sang ESP32
void Process_Alarm_System(float temperature, int smoke, int fire_detected);

#endif /* ALARM_LOGIC_H_ */
