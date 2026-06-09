#include <stdio.h>
#include "sensor_service.h"
#include "ds18b20.h"
#include "mq2.h"
#include "main.h"

extern ADC_HandleTypeDef hadc2; // ADC2 để đọc MH-Sensor AO từ PB1 

// Biến tĩnh nội bộ quản lý State-Machine đọc DS18B20 không dùng delay
static uint32_t last_ds18b20_tick = 0;
static uint8_t ds18b20_state = 0; 

void SensorService_Init(void) {
    DS18B20_Init();
    MQ2_Init();
}

void SensorService_Update(SensorData_t *data) {
    if (data == NULL) return;

    uint32_t current_tick = HAL_GetTick();

    // =================================================================
    // 1. --- XỬ LÝ DS18B20 KHÔNG DÙNG DELAY (NON-BLOCKING) ---
    // =================================================================
    static uint8_t ds18b20_fail_count = 0;
    if (ds18b20_state == 0) {
        if (DS18B20_Start()) {
            DS18B20_Write(0xCC); // Skip ROM
            DS18B20_Write(0x44); // Convert T
            last_ds18b20_tick = current_tick;
            ds18b20_state = 1;
        } else {
            // Không gán ngay -999 nếu chỉ bị văng một lần, giữ giá trị cũ để tránh loạn xạ
            ds18b20_fail_count++;
            if (ds18b20_fail_count >= 5) {
                data->temperature = -999.0f;
            }
            HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
            delay_us(5);
        }
    } 
    else if (ds18b20_state == 1) {
        if (current_tick - last_ds18b20_tick >= 750) {
            float temp_read = DS18B20_ReadTemperature_NonBlocking();
            if (temp_read > -50.0f && temp_read < 125.0f) { // Bộ lọc giới hạn vật lý
                data->temperature = temp_read;
                ds18b20_fail_count = 0;
            } else {
                ds18b20_fail_count++;
                if (ds18b20_fail_count >= 5) {
                    data->temperature = -999.0f;
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
    // Đọc DO (Digital Output) - PA3
    if (HAL_GPIO_ReadPin(GPIO_DO_MH_sensor_GPIO_Port, GPIO_DO_MH_sensor_Pin) == GPIO_PIN_RESET) {
        data->fire_detected = 1; // Phát hiện ngọn lửa trực diện
        data->mh_sensor_do = 0;  // DO kích hoạt = 0
    } else {
        data->fire_detected = 0; // An toàn
        data->mh_sensor_do = 1;  // DO bình thường = 1
    }
    
    // Đọc AO (Analog Output) từ ADC2 - PB1
    HAL_ADC_Stop(&hadc2);
    HAL_ADC_Start(&hadc2);
    if (HAL_ADC_PollForConversion(&hadc2, 10) == HAL_OK) {
        uint32_t mh_raw = HAL_ADC_GetValue(&hadc2);
        HAL_ADC_Stop(&hadc2);
        data->mh_sensor_ao_volt = (float)mh_raw * (3.3f / 4095.0f); // Đổi sang Volt
    } else {
        data->mh_sensor_ao_volt = -1.0f; // Báo lỗi
    }

    // =================================================================
    // 4. --- XUẤT LOG BÁO CẢM BIẾN (DEBUG QUA UART - ĐỊNH KỲ 1000MS) ---
    // =================================================================
    static uint32_t last_log_tick = 0;
    if (current_tick - last_log_tick >= 1000) { 
        printf("[CANH BAO CAM BIEN] Nhiet do: %.1fC | Khoi: %.2fppm | Lua: %s | DO:%d | AO:%.2fV\r\n", 
                data->temperature, data->smoke_conc, 
                data->fire_detected ? "CO LUA!" : "OK",
                data->mh_sensor_do, data->mh_sensor_ao_volt);
        last_log_tick = current_tick;
    }
}