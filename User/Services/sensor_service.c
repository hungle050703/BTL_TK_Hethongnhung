#include "sensor_service.h"
#include "ds18b20.h"
#include "mq2.h"

extern ADC_HandleTypeDef hadc2; // Thêm ADC2 để dùng cho MH-Sensor

// Không dùng DMA nữa
// uint16_t adc_dma_buffer[1];
// extern ADC_HandleTypeDef hadc1;

static uint32_t last_ds18b20_tick = 0;
static uint8_t ds18b20_state = 0; // 0: Idle/Request, 1: Waiting for conversion

void SensorService_Init(void) {
    DS18B20_Init();
    MQ2_Init();
    // Đã chuyển phần đọc ADC của MQ-2 sang chế độ thủ công (Polling) trong mq2.c
    // Không dùng DMA để giải quyết triệt để lỗi HardFault trên dòng STM32H5
}

void SensorService_Update(SensorData_t *data) {
    uint32_t current_tick = HAL_GetTick();

    // --- Xử lý DS18B20 không dùng Delay (Non-blocking) ---
    if (ds18b20_state == 0) {
        // Giai đoạn 1: Ra lệnh cho DS18B20 bắt đầu đo
        if (DS18B20_Start()) {
            DS18B20_Write(0xCC); // Skip ROM
            DS18B20_Write(0x44); // Gửi lệnh Convert T
            last_ds18b20_tick = current_tick;
            ds18b20_state = 1;   // Chuyển sang trạng thái chờ
        }
    } 
    else if (ds18b20_state == 1) {
        // Giai đoạn 2: Kiểm tra xem đã đủ 750ms chưa
        if (current_tick - last_ds18b20_tick >= 750) {
            data->temperature = DS18B20_ReadTemperature_NonBlocking();
            ds18b20_state = 0;   // Quay lại chu kỳ tiếp theo
        }
    }
    
    // --- Xử lý MQ2 từ ADC1 (Polling) ---
    data->smoke_raw = MQ2_ReadRawData();
    data->smoke_conc = MQ2_CalculateGasConcentration(data->smoke_raw);

    // --- Xử lý MH-Sensor (Hồng ngoại cảnh báo lửa/vật cản) ---
    // 1. Đọc tín hiệu Digital (DO) trên PA3
    if (HAL_GPIO_ReadPin(GPIO_DO_MH_sensor_GPIO_Port, GPIO_DO_MH_sensor_Pin) == GPIO_PIN_RESET) {
        data->mh_sensor_do = 0; // Kích hoạt (Có ngọn lửa/vật cản)
        data->fire_detected = 1; // Cập nhật cờ fire_detected
    } else {
        data->mh_sensor_do = 1; // Bình thường
        data->fire_detected = 0;
    }

    // 2. Đọc tín hiệu Analog (AO) trên PB1 từ ADC2 (Polling)
    HAL_ADC_Stop(&hadc2);  // Xóa cờ OVR nếu có
    HAL_ADC_Start(&hadc2); // Bấm máy đo
    if (HAL_ADC_PollForConversion(&hadc2, 10) == HAL_OK) {
        uint32_t mh_raw = HAL_ADC_GetValue(&hadc2);
        HAL_ADC_Stop(&hadc2); // Khóa lại
        
        // Đổi ra Volt (Chân PB1 chịu tối đa 3.3V)
        data->mh_sensor_ao_volt = (float)mh_raw * (3.3f / 4095.0f);
    } else {
        data->mh_sensor_ao_volt = -1.0f; // Báo lỗi
    }
}