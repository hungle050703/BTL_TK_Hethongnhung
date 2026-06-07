/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : app_freertos.c
  ******************************************************************************
  */
/* USER CODE END Header */

#include "app_freertos.h"
#include "cmsis_os2.h"
#include "sensor_service.h"
#include "alarm_logic.h"
#include "display/display_panel.h"
#include "display/display_service.h"
#include <stdio.h>
#include <string.h>

/* Private variables */
extern SensorData_t myData;
AlarmLevel_t g_alarm_level = ALARM_NONE;

/* Task Handle */
osThreadId_t Sensor_TaskHandle;
osThreadId_t Alarm_Logic_TaskHandle;
osThreadId_t Comm_4G_TaskHandle;
osMutexId_t DataMutexHandle;

void MX_FREERTOS_Init(void) {
  DataMutexHandle = osMutexNew(NULL);
  Sensor_TaskHandle = osThreadNew(StartDefaultTask, NULL, NULL);
  Alarm_Logic_TaskHandle = osThreadNew(StartTask02, NULL, NULL);
  Comm_4G_TaskHandle = osThreadNew(StartTask03, NULL, NULL);
}

void StartDefaultTask(void *argument) {
  SensorService_Init();
  SensorData_t local_data;
  for(;;) {
    SensorService_Update(&local_data);
    if (osMutexAcquire(DataMutexHandle, 10) == osOK) {
        myData = local_data;
        osMutexRelease(DataMutexHandle);
    }
    osDelay(50);
  }
}

void StartTask02(void *argument) {
  // KHỞI TẠO CHỈ GỌI 1 LẦN TRƯỚC VÒNG LẶP
  AlarmLogic_Init();

  SensorData_t temp_sensor_data;
  DisplayRenderer renderer = {
      .Clear = Display_Clear,
      .DrawHeader = Display_DrawHeader,
      .DrawLine = Display_DrawLine,
      .DrawFooter = Display_DrawFooter,
  };

  for(;;) {
    if (osMutexAcquire(DataMutexHandle, 10) == osOK) {
        temp_sensor_data = myData;
        osMutexRelease(DataMutexHandle);
    }

    g_alarm_level = Alarm_ProcessLogic(&temp_sensor_data);
    Alarm_ExecuteAction(g_alarm_level);

    /* Simple status screen based on alarm level */
    uint8_t fire = (g_alarm_level == ALARM_CRITICAL);
    uint8_t trouble = (g_alarm_level == ALARM_WARNING);
    uint8_t supervisor = (g_alarm_level == ALARM_SUPERVISORY);
    uint8_t disable = (g_alarm_level == ALARM_DISABLED);
    Display_FirePanelStatus(&renderer, fire, trouble, supervisor, disable);

    osDelay(500); // Give SPI time and avoid redraw flooding
  }
}

void StartTask03(void *argument) {
  for(;;) {
    if (g_alarm_level == ALARM_CRITICAL) {
        printf("[4G] ALERT!\r\n");
        osDelay(10000);
    } else {
        osDelay(3000);
    }
  }
}
