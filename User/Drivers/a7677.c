#include "a7677.h"
#include "usart.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>

extern UART_HandleTypeDef huart5;

// Hàm khởi động module
void A7677_PowerOn(void) {
    // Lưu ý: Đổi lại tên Port/Pin bằng đúng thứ bạn cấu hình trong CubeMX
    // Ví dụ giả định bạn đặt tên chân là A7677_PWR
    HAL_GPIO_WritePin(A7677_PWRKEY_GPIO_Port, A7677_PWRKEY_Pin, GPIO_PIN_RESET);
    osDelay(1500); // Pulse 1.5s
    HAL_GPIO_WritePin(A7677_PWRKEY_GPIO_Port, A7677_PWRKEY_Pin, GPIO_PIN_SET);
    
    // Đợi 5 giây cho hệ thống Sim, Network kết nối
    osDelay(5000); 
}

// Gửi một lệnh AT Command cơ bản
void A7677_Send_AT(const char* cmd) {
    // Sử dụng UART5 cho module 4G
    HAL_UART_Transmit(&huart5, (uint8_t*)cmd, strlen(cmd), 1000);
}

// Lệnh thực hiện cuộc gọi
void A7677_Call(const char* phone_number) {
    char cmd[32];
    sprintf(cmd, "ATD%s;\r\n", phone_number);
    A7677_Send_AT(cmd);
}

// Lệnh gửi tin nhắn SMS
void A7677_SendSMS(const char* phone_number, const char* message) {
    char cmd[64];
    
    // Cài đặt SMS Text Mode
    A7677_Send_AT("AT+CMGF=1\r\n");
    osDelay(200); // Chờ phản hồi OK

    // Nhập số điện thoại
    sprintf(cmd, "AT+CMGS=\"%s\"\r\n", phone_number);
    A7677_Send_AT(cmd);
    osDelay(200); // Chờ dấu '>' để nhập liệu

    // Điền nội dung tin nhắn
    A7677_Send_AT(message);
    osDelay(100);

    // Bấm tổ hợp Ctrl+Z (Mã Hex là 0x1A) để gửi tin
    char ctrl_z[2] = {0x1A, '\0'};
    A7677_Send_AT(ctrl_z);
}
