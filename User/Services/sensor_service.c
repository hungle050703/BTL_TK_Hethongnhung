#include "sensor_service.h"
#include "ds18b20.h"
#include "mq2.h"
#include "main.h"

extern ADC_HandleTypeDef hadc2; // ADC2 để đọc MH-Sensor AO từ PB1 

static uint32_t last_ds18b20_tick = 0;
static uint8_t ds18b20_state = 0; 
static uint8_t ds18b20_fail_count = 0;

void SensorService_Init(void) {
    DS18B20_Init();
    MQ2_Init();
    last_ds18b20_tick = 0;
    ds18b20_state = 0;
    ds18b20_fail_count = 0;
}

void SensorService_Update(SensorData_t *data) {
    if (data == NULL) return;

    uint32_t current_tick = HAL_GetTick();

    // =================================================================
    // 1. --- XỬ LÝ DS18B20 KHÔNG DÙNG DELAY (NON-BLOCKING STATE MACHINE) ---
    // =================================================================
    if (ds18b20_state == 0) {
        if (DS18B20_Start()) {
            DS18B20_Write(0xCC); // Skip ROM
            DS18B20_Write(0x44); // Convert T
            last_ds18b20_tick = current_tick;
            ds18b20_state = 1;
        } else {
            ds18b20_fail_count++;
            if (ds18b20_fail_count >= 3) {
                data->temperature = -999.0f; // Xác định mất cảm biến
            }
        }
    } 
    else if (ds18b20_state == 1) {
        if (current_tick - last_ds18b20_tick >= 750) {
            float temp_read = DS18B20_ReadTemperature_NonBlocking();

            if (temp_read > -50.0f && temp_read < 125.0f) {
                data->temperature = temp_read; // Đọc chuẩn
                ds18b20_fail_count = 0;
            } else {
                ds18b20_fail_count++;
                if (ds18b20_fail_count >= 3) {
                    data->temperature = -999.0f; // Báo mất cảm biến hoặc đọc lỗi
                }
            }
            ds18b20_state = 0; // Quay về State 0
        }
    }
    
    // =================================================================
    // 2. --- XỬ LÝ KHÓI MQ2 TỪ ADC (POLLING THỦ CÔNG) ---
    // =================================================================
    data->smoke_raw = MQ2_ReadRawData();
    data->smoke_conc = MQ2_CalculateGasConcentration(data->smoke_raw);

    // =================================================================
    // 3. --- TÍCH HỢP CẢM BIẾN LỬA HỒNG NGOẠI ---
    // =================================================================
    if (HAL_GPIO_ReadPin(GPIO_DO_MH_sensor_GPIO_Port, GPIO_DO_MH_sensor_Pin) == GPIO_PIN_RESET) {
        data->fire_detected = 1;
        data->mh_sensor_do = 0;
    } else {
        data->fire_detected = 0;
        data->mh_sensor_do = 1;
    }
    
    HAL_ADC_Stop(&hadc2);
    HAL_ADC_Start(&hadc2);
    if (HAL_ADC_PollForConversion(&hadc2, 10) == HAL_OK) {
        uint32_t mh_raw = HAL_ADC_GetValue(&hadc2);
        HAL_ADC_Stop(&hadc2);
        data->mh_sensor_ao_volt = (float)mh_raw * (3.3f / 4095.0f);
    } else {
        data->mh_sensor_ao_volt = -1.0f;
    }

    // 🟢 ĐÃ XÓA KHỐI PRINTF TẠI ĐÂY ĐỂ TRÁNH TRANH CHẤP UART VỚI APP_MAIN
}
