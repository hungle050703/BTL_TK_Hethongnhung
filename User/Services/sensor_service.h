#ifndef SENSOR_SERVICE_H_
#define SENSOR_SERVICE_H_

#include "main.h"

typedef struct {
    float temperature;     // Lưu nhiệt độ từ DS18B20
    uint32_t smoke_raw;    // Giá trị ADC thô từ MQ2
    float smoke_conc;      // Nồng độ PPM khói sau tính toán
    uint8_t fire_detected; // Cờ báo lửa hồng ngoại (0: an toàn, 1: có lửa)
    uint8_t mh_sensor_do;  // Tín hiệu số DO của hồng ngoại (0 = Có lửa, 1 = An toàn)
    float mh_sensor_ao_volt; // Tín hiệu điện áp AO từ ADC2 (Volt)
} SensorData_t;

void SensorService_Init(void);
void SensorService_Update(SensorData_t *data);

#endif /* SENSOR_SERVICE_H_ */