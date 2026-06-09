/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os2.h"
#include "adc.h"
#include "icache.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sensor_service.h"
#include "alarm_logic.h"
#include "display/display_panel.h"
#include "mq2.h"
#include "ds18b20.h"
#include <stdio.h>
#include <string.h>

/* Khai bao header FreeRTOS nhan dien kieu du lieu Static */
#include "FreeRTOS.h"
#include "task.h"

/* Dinh nghia chan Mute tuong thich cho nut nhan B1 (PC13) */
#ifndef BTN_MUTE_Pin
#define BTN_MUTE_Pin GPIO_PIN_13
#endif
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
/* USER CODE BEGIN PV */
SensorData_t myData;

/* BIẾN TOÀN CỤC ĐỘC LẬP TÁCH KHỎI RTOS ĐỂ LƯU NHIỆT ĐỘ DS18B20 */
volatile float global_ds18b20_temp = 25.0f;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
void MX_FREERTOS_Init(void);

/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern UART_HandleTypeDef huart3; 

int __io_putchar(int ch) {
    if (huart3.Instance != NULL) { 
        HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 10);
    }
    return ch;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_ICACHE_Init();

  /* Khởi tạo phần cứng TIM2 đầu tiên */
  MX_TIM2_Init();

  MX_USART3_UART_Init();
  MX_ADC2_Init();
  MX_SPI1_Init();
  MX_UART4_Init();

  /* USER CODE BEGIN 2 */
  printf("\r\n==================================================\r\n");
  printf("BTL EMBEDDED: THIET BI TRUYEN TIN BAO CHAY STARTING...\r\n");
  printf("==================================================\r\n");

  /* 1. KÍCH HOẠT ĐẾM COUNTER CỦA TIM2 ĐỂ LÀM GỐC THỜI GIAN DELAY_US TRƯỚC */
  HAL_TIM_Base_Start(&htim2);

  // Trễ một khoảng nhỏ bằng vòng lặp thô để thanh ghi TIM2->CNT kịp ổn định nhảy số
  for(volatile int i = 0; i < 5000; i++);

  memset(&myData, 0, sizeof(SensorData_t));

  /* 2. BÂY GIỜ MỚI KHỞI TẠO CÁC DỊCH VỤ CẢM BIẾN (SẼ KHÔNG BỊ TREO KHI ĐỌC DS18B20 CHU KỲ ĐẦU) */
  extern void LCD_init(void);
  extern void SensorService_Init(void);
  extern void AlarmLogic_Init(void);
  LCD_init();
  SensorService_Init();
  AlarmLogic_Init();

  printf("Hardware Peripheral Init Done! Activating FreeRTOS Kernel...\r\n");

  // Đọc tần số PCLK1 chuẩn xác từ thư viện HAL
  uint32_t pclk1_freq = HAL_RCC_GetPCLK1Freq();
  uint32_t tim_clock = pclk1_freq;

  /* Cấu trúc cho dòng STM32H5: Sử dụng CFGR2 và định nghĩa chuẩn RCC_CFGR2_PPRE1 */
  if ((RCC->CFGR2 & RCC_CFGR2_PPRE1) != 0) {
      tim_clock = pclk1_freq * 2;
  }

  printf("\r\n--- KIỂM TRA PHẦN CỨNG TIMER ---\r\n");
  printf("[TIM2_CHECK] Tan so nguon cap cho Timer: %lu Hz (%lu MHz)\r\n", tim_clock, tim_clock / 1000000);
  printf("[TIM2_CHECK] Prescaler hien tai: %lu\r\n", (uint32_t)htim2.Init.Prescaler);
  printf("[TIM2_CHECK] Thoi gian 1 tick cua TIM2 = %.3f us\r\n", (float)(htim2.Init.Prescaler + 1) / ((float)tim_clock / 1000000.0f));
  printf("--------------------------------\r\n");
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* Call init function for freertos objects */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here */
  while (1)
  {
  }
}

/**
  * @brief System Clock Configuration
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLL1_SOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1_VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1_VCORANGE_WIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_FLASH_SET_PROGRAM_DELAY(FLASH_PROGRAMMING_DELAY_1);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/* MPU Configuration */
void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};
  MPU_Attributes_InitTypeDef MPU_AttributesInit = {0};

  HAL_MPU_Disable();
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x08FFF000;
  MPU_InitStruct.LimitAddress = 0x08FFFFFF;
  MPU_InitStruct.AttributesIndex = MPU_ATTRIBUTES_NUMBER0;
  MPU_InitStruct.AccessPermission = MPU_REGION_ALL_RO;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  MPU_AttributesInit.Number = MPU_ATTRIBUTES_NUMBER0;
  MPU_AttributesInit.Attributes = INNER_OUTER(MPU_NOT_CACHEABLE);

  HAL_MPU_ConfigMemoryAttributes(&MPU_AttributesInit);
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
  * @brief  Callback phục vụ xử lý ngắt định kỳ cho cả SysTick (TIM6) và Luồng quét Bare-metal (TIM2)
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM6)
	  {
	    HAL_IncTick();
	  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
