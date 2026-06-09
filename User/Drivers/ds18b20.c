#include "ds18b20.h"

// Gọi extern cấu hình timer từ tim.c
extern TIM_HandleTypeDef htim2;

/* Tích hợp FreeRTOS để bảo vệ các mốc Timing nhạy cảm */
#include "FreeRTOS.h"
#include "task.h"

// Hàm delay microsecond ĐỘC LẬP - KHÔNG RESET COUNTER (Free-running)
void delay_us(uint32_t us) {
    uint32_t start = __HAL_TIM_GET_COUNTER(&htim2);
    uint32_t timeout_cnt = 0;

    while ((__HAL_TIM_GET_COUNTER(&htim2) - start) < us) {
        timeout_cnt++;
        if (timeout_cnt > 1000000) break; // Kháng treo nếu Timer lỗi
    }
}

// 1. Hàm Reset: Kiểm tra phản hồi (Presence) từ Sensor
uint8_t DS18B20_Start(void) {
    uint8_t response = 0;
    
    taskENTER_CRITICAL(); // --- KHÓA NGẮT ĐỂ GIỮ TIMING CHUẨN XÁC ---

    // Kéo chân xuống thấp trong 480us
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
    delay_us(480);
    
    // Thả chân lên cao (Open Drain) và đợi 80us
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
    delay_us(80);
    
    // Đọc Presence pulse từ DS18B20
    if (!(HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN))) {
        response = 1; // Khởi động tốt
    } else {
        response = 0; // Không thấy sensor
    }
    
    taskEXIT_CRITICAL(); // --- MỞ LẠI NGẮT CHO FREERTOS THỰC THI ---
    
    delay_us(400); // Đợi nốt chu kỳ hồi bus
    return response;
}

// 2. Hàm Ghi 1 Byte
void DS18B20_Write(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        taskENTER_CRITICAL(); // Khóa ngắt bảo vệ slot ghi bit đơn lẻ

        if ((data & (1 << i)) != 0) {
            // Ghi bit 1
            HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
            delay_us(2);
            HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
            delay_us(60);
        } else {
            // Ghi bit 0
            HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
            delay_us(60);
            HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
            delay_us(2);
        }

        taskEXIT_CRITICAL(); // Mở lại ngắt ngay sau khi kết thúc 1 bit slot
    }
}

// 3. Hàm Đọc 1 Byte
uint8_t DS18B20_Read(void) {
    uint8_t value = 0;
    for (int i = 0; i < 8; i++) {
        taskENTER_CRITICAL(); // Khóa ngắt chí mạng để lấy mẫu chuẩn micro giây thứ 12-15

        // Kéo chân xuống thấp 2us kích hoạt chu kỳ đọc
        HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
        delay_us(2);
        
        // Thả chân lên cao và đợi 10us để sensor đẩy data ra ổn định trên bus
        HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
        delay_us(10);
        
        // Đọc dữ liệu trả về từ cảm biến
        if (HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN) == GPIO_PIN_SET) {
            value |= (1 << i);
        }
        
        taskEXIT_CRITICAL(); // Mở ngắt nhường quyền xử lý cho các tác vụ khác

        delay_us(50); // Thời gian nghỉ hồi phục bus
    }
    return value;
}

void DS18B20_Init(void) {
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
}

// Hàm đọc nhiệt độ Non-Blocking tối ưu cho State Machine chạy ngầm trong FreeRTOS Task
float DS18B20_ReadTemperature_NonBlocking(void) {
    uint8_t LSB = 0, MSB = 0;
    int16_t temp = 0;
    
    if (DS18B20_Start()) {
        DS18B20_Write(0xCC); // Skip ROM
        DS18B20_Write(0xBE); // Read Scratchpad
        
        LSB = DS18B20_Read();
        MSB = DS18B20_Read();
        
        temp = (MSB << 8) | LSB;
        return (float)temp / 16.0f;
    }
    
    return -999.0f; // Lỗi mất kết nối vật lý
}
