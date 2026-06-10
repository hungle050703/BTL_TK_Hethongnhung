#ifndef ESP32_SERVICE_H
#define ESP32_SERVICE_H

#include "main.h" // Chứa khai báo huart3 và các hàm HAL

typedef enum {
    SYS_STATE_AN_TOAN,
    SYS_STATE_DE_PHONG
} SystemState_t;

void ESP32Service_Init(void);
void ESP32Service_Process(void);

#endif
