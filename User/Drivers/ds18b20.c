#include "ds18b20.h"
#include "cmsis_os2.h" // Thư viện RTOS để sử dụng osDelay
#include "FreeRTOS.h"  // FreeRTOS kernel
#include "task.h"      // Để dùng taskENTER_CRITICAL() / taskEXIT_CRITICAL()

// Khai báo extern timer để dùng cho hàm delay_us
extern TIM_HandleTypeDef htim2;

// ============================================
// Hàm cấu hình chân GPIO: Output vs Input
// ============================================
static inline void DS18B20_Set_Pin_Output(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DS18B20_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;  // Open-Drain Output
    GPIO_InitStruct.Pull = GPIO_PULLUP;           // Bật Pull-up nội
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DS18B20_PORT, &GPIO_InitStruct);
}

static inline void DS18B20_Set_Pin_Input(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DS18B20_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;      // Input (cho trở kéo ngoài hoạt động)
    GPIO_InitStruct.Pull = GPIO_PULLUP;           // Kích hoạt Pull-up nội
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DS18B20_PORT, &GPIO_InitStruct);
}

// Hàm delay microsecond sử dụng TIM2 (Có chống treo MCU)
void delay_us(uint32_t us) {
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    uint32_t timeout_cnt = 0;
    while (__HAL_TIM_GET_COUNTER(&htim2) < us) {
        timeout_cnt++;
        if (timeout_cnt > 1000000) break; // Thoát hiểm nếu TIM2 lỗi
    }
}

// 1. Hàm Reset: Kiểm tra xem sensor có "sống" không
uint8_t DS18B20_Start(void) {
    uint8_t presence = 0;
    
    // 🔐 BẢO VỆ TIMING: Vào Critical Section để ngắt RTOS không chen ngang
    taskENTER_CRITICAL();

    // Step 1: Cấu hình chân thành Output (để có thể kéo LOW)
    DS18B20_Set_Pin_Output();
    
    // Step 2: STM32 kéo chân xuống LOW trong 480us (Gửi xung Reset)
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
    delay_us(480);
    
    // Step 3: Chuyển chân thành Input để trở kéo ngoài kéo lên HIGH
    DS18B20_Set_Pin_Input();
    
    // Step 4: Đợi 70µs để lọt vào giữa cửa sổ Presence Pulse của DS18B20 (60-240µs)
    delay_us(70);
    
    // Step 5: Đọc trạng thái chân để xác định có cảm biến không
    if (HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN) == GPIO_PIN_RESET) {
        presence = 1;  // ✅ CÓ CẢM BIẾN PHẢN HỒI!
    } else {
        presence = 0;  // ❌ THẬT SỰ MẤT CẢM BIẾN
    }
    
    // Step 6: Đợi nốt thời gian còn lại của chu kỳ reset (70 + 410 = 480µs tổng)
    delay_us(410);
    
    // 🔓 THOÁT CRITICAL SECTION: Mở lại ngắt cho RTOS làm việc
    taskEXIT_CRITICAL();

    return presence;
} // Đã sửa lỗi thừa dấu ngoặc '}' tại đây!

// 2. Hàm Ghi 1 Byte
void DS18B20_Write(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        // 🔐 Vào Critical Section để bảo vệ timing của mỗi bit slot (~60-120µs)
        taskENTER_CRITICAL();

        if ((data & (1 << i)) != 0) { // Ghi bit 1
            HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
            delay_us(1); 
            HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
            delay_us(60);
        } else { // Ghi bit 0
            HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
            delay_us(60);
            HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
            delay_us(1);
        }

        // 🔓 Thoát Critical Section
        taskEXIT_CRITICAL();
    }
}

// 3. Hàm Đọc 1 Byte
uint8_t DS18B20_Read(void) {
    uint8_t value = 0;
    for (int i = 0; i < 8; i++) {
        // 🔐 Vào Critical Section để bảo vệ cửa sổ đọc dữ liệu cực kỳ nhạy cảm
        taskENTER_CRITICAL();

        HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
        delay_us(2);
        
        HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
        delay_us(8); 
        
        if (HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN) == GPIO_PIN_SET) {
            value |= (1 << i);
        }
        
        delay_us(50);

        // 🔓 Thoát Critical Section
        taskEXIT_CRITICAL();
    }
    return value;
}

void DS18B20_Init(void) {
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
}

// Hàm đọc nhiệt độ chặn (Blocking) cải tiến phù hợp với RTOS
float DS18B20_ReadTemperature(void) {
    uint8_t LSB, MSB;
    int16_t temp;
    
    // 🔐 CRITICAL SECTION CHỈ CHO PHẦN RESET + GHI LỆNH CONVERT T
    taskENTER_CRITICAL();
    
    if (!DS18B20_Start()) {
        taskEXIT_CRITICAL();
        return -999.0f;  // Sensor không phản hồi
    }
    
    DS18B20_Write(0xCC); // Skip ROM
    DS18B20_Write(0x44); // Convert T
    
    taskEXIT_CRITICAL();
    // 🔓 ĐÃ THOÁT CRITICAL SECTION
    
    // ⏳ Sử dụng osDelay để giải phóng CPU cho luồng 4G, LCD, Alarm chạy trong 750ms
    osDelay(750);
    
    // 🔐 VÀO CRITICAL SECTION LẦN 2 CHỈ ĐỂ ĐỌC SCRATCHPAD
    taskENTER_CRITICAL();
    
    if (!DS18B20_Start()) {
        taskEXIT_CRITICAL();
        return -999.0f;
    }
    
    DS18B20_Write(0xCC); // Skip ROM
    DS18B20_Write(0xBE); // Read Scratchpad

    LSB = DS18B20_Read();
    MSB = DS18B20_Read();

    taskEXIT_CRITICAL();
    // 🔓 THOÁT CRITICAL SECTION SAU KHI ĐỌC XONG
    
    temp = (MSB << 8) | LSB;
    return (float)temp / 16.0f;
}

float DS18B20_ReadTemperature_NonBlocking(void) {
    uint8_t LSB, MSB;
    int16_t temp;
    
    // 🔐 CRITICAL SECTION: Bảo vệ toàn bộ quá trình đọc dữ liệu có sẵn
    taskENTER_CRITICAL();
    
    if (!DS18B20_Start()) {
        taskEXIT_CRITICAL();
        return -999.0f;
    }
    
    DS18B20_Write(0xCC);
    DS18B20_Write(0xBE);
    
    LSB = DS18B20_Read();
    MSB = DS18B20_Read();
    
    taskEXIT_CRITICAL();
    // 🔓 THOÁT CRITICAL SECTION
    
    temp = (MSB << 8) | LSB;
    return (float)temp / 16.0f;
}
