/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : app_freertos.c
  * @brief          : FreeRTOS Task Management for Fire Alarm System
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "sensor_service.h"
#include "alarm_logic.h"
#include "mq2.h"
#include "display/display_panel.h"
#include "../Services/display/display_service.h"

extern SensorData_t myData;
extern volatile uint8_t is_buzzer_muted;
extern char alarm_status_text[];
extern UG_COLOR alarm_status_color;

static const DisplayRenderer renderer = {
    .Clear = Display_Clear,
    .DrawHeader = Display_DrawHeader,
    .DrawLine = Display_DrawLine,
    .DrawFooter = Display_DrawFooter,
    .DrawString = Display_DrawString,
};
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
/* USER CODE END Variables */

osThreadId_t Sensor_TaskHandle;
const osThreadAttr_t Sensor_Task_attributes = {
  .name = "Sensor_Task",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 512 * 8  /* Increased from 2KB to 4KB for sensor updates */
};

osThreadId_t Alarm_Logic_TaskHandle;
const osThreadAttr_t Alarm_Logic_Task_attributes = {
  .name = "Alarm_Logic_Task",
  .priority = (osPriority_t) osPriorityAboveNormal,
  .stack_size = 512 * 8  /* Increased from 2KB to 4KB for display updates */
};

osThreadId_t Comm_4G_TaskHandle;
const osThreadAttr_t Comm_4G_Task_attributes = {
  .name = "Comm_4G_Task",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 512 * 8  /* Increased from 2KB to 4KB for logging */
};

osMutexId_t DataMutexHandle;
const osMutexAttr_t DataMutex_attributes = {
  .name = "DataMutex"
};

osMessageQueueId_t myQueue01Handle;
const osMessageQueueAttr_t myQueue01_attributes = {
  .name = "myQueue01"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void StartDefaultTask(void *argument);
void StartTask02(void *argument);
void StartTask03(void *argument);

/* SUA: Thay doi uint8_t thanh int de trung khop voi file alarm_logic.h */
extern void Process_Alarm_System(float temperature, int smoke, int fire_detected);
/* USER CODE END FunctionPrototypes */

/**
  * @brief  FreeRTOS initialization
  */
void MX_FREERTOS_Init(void) {
  DataMutexHandle = osMutexNew(&DataMutex_attributes);
  myQueue01Handle = osMessageQueueNew (16, sizeof(uint16_t), &myQueue01_attributes);

  /* Thiet lap Thread vao dung con tro ham da khai bao prototype */
  Sensor_TaskHandle = osThreadNew(StartDefaultTask, NULL, &Sensor_Task_attributes);
  Alarm_Logic_TaskHandle = osThreadNew(StartTask02, NULL, &Alarm_Logic_Task_attributes);
  Comm_4G_TaskHandle = osThreadNew(StartTask03, NULL, &Comm_4G_Task_attributes);
}

/**
* @brief Function implementing the Sensor_Task thread.
*/
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN Sensor_Task */
  /* NOTE: LCD_init(), SensorService_Init(), AlarmLogic_Init() moved to main.c */
  /* They are now executed before osKernelStart() to avoid stack overflow */

  for(;;)
  {
    if (osMutexAcquire(DataMutexHandle, osWaitForever) == osOK) {
        SensorService_Update(&myData);
        osMutexRelease(DataMutexHandle);
    }
    osDelay(300);
  }
  /* USER CODE END Sensor_Task */
}

/**
* @brief Function implementing the Alarm_Logic_Task thread.
*/
void StartTask02(void *argument)
{
  /* USER CODE BEGIN StartTask02 */
  SensorData_t localData;
  for(;;)
  {
    Alarm_SetUserMute();

    if (osMutexAcquire(DataMutexHandle, osWaitForever) == osOK) {
        memcpy(&localData, &myData, sizeof(SensorData_t));
        osMutexRelease(DataMutexHandle);
    }

    AlarmLevel_t current_level = Alarm_ProcessLogic(&localData);
    Alarm_ExecuteAction(current_level);

    uint8_t fire = localData.fire_detected ? 1 : 0;
    float smoke_v = localData.smoke_conc;
    float smoke_percentage = (smoke_v / 5.0f) * 100.0f;
    int supervisor = (int)smoke_percentage;
    uint8_t disable = is_buzzer_muted ? 1 : 0;

    Display_FirePanelStatus(&renderer, alarm_status_text, alarm_status_color,
                             fire, (uint8_t)localData.temperature, supervisor, disable);

    osDelay(100);
  }
  /* USER CODE END StartTask02 */
}

/**
* @brief Function implementing the Comm_4G_Task thread.
*/
void StartTask03(void *argument)
{
  /* USER CODE BEGIN StartTask03 */
  SensorData_t localData;
  for(;;)
  {
    if (osMutexAcquire(DataMutexHandle, osWaitForever) == osOK) {
        memcpy(&localData, &myData, sizeof(SensorData_t));
        osMutexRelease(DataMutexHandle);
    }

    uint8_t fire = localData.fire_detected ? 1 : 0;
    float smoke_v = localData.smoke_conc;
    float smoke_percentage = (smoke_v / 5.0f) * 100.0f;
    int supervisor = (int)smoke_percentage;
    float base_v = MQ2_GetBaseVoltage();
    bool fire_danger = (localData.mh_sensor_do == 0);

    Process_Alarm_System(localData.temperature, supervisor, fire);

    const char* smoke_status;
    if (smoke_v < base_v + 0.2f) {
        smoke_status = "SACH";
    } else if (smoke_v < base_v + 0.6f) {
        smoke_status = "CO KHOI NHE";
    } else {
        smoke_status = "NGUY HIEM!";
    }
    const char* mh_status = fire_danger ? "CO LUA/VAT CAN!!" : "BINH THUONG";

    printf("[RTOS LUONG 4G] STATUS PANEL: %s\r\n", alarm_status_text);
    if (localData.temperature == -999.0f) {
        printf("[RTOS LUONG 4G] Nhiet do:  MAT CAM BIEN\r\n");
    } else {
        printf("[RTOS LUONG 4G] Nhiet do: %5.2f C\r\n", localData.temperature);
    }
    printf("[RTOS LUONG 4G]   Khoi MQ-2:   %4.1f %% - %s (Base: %.2fV | Now: %.2fV)\r\n",
           smoke_percentage, smoke_status, base_v, smoke_v);
    printf("[RTOS LUONG 4G]   Lua MH-SENS: %s (AO: %.2fV)\r\n",
           mh_status, localData.mh_sensor_ao_volt);
    printf("[RTOS LUONG 4G] ----------------------------------------\r\n");

    osDelay(1000);
  }
  /* USER CODE END StartTask03 */
}
