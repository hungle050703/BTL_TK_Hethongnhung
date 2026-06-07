/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h5xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ADC ADC1
#define STLK_MCO_Pin GPIO_PIN_0
#define STLK_MCO_GPIO_Port GPIOH
#define ADC_MQ2_Pin GPIO_PIN_0
#define ADC_MQ2_GPIO_Port GPIOC
#define GPIO_DATA_DB12_Pin GPIO_PIN_0
#define GPIO_DATA_DB12_GPIO_Port GPIOA
#define GPIO_DO_MH_sensor_Pin GPIO_PIN_3
#define GPIO_DO_MH_sensor_GPIO_Port GPIOA
#define LCD_CS_Pin GPIO_PIN_4
#define LCD_CS_GPIO_Port GPIOA
#define LCD_DC_Pin GPIO_PIN_6
#define LCD_DC_GPIO_Port GPIOA
#define LCD_RST_Pin GPIO_PIN_4
#define LCD_RST_GPIO_Port GPIOC
#define LED_GREEN_ON_Pin GPIO_PIN_0
#define LED_GREEN_ON_GPIO_Port GPIOB
#define ADC2_AO_MH_sensor_Pin GPIO_PIN_1
#define ADC2_AO_MH_sensor_GPIO_Port GPIOB
#define BUZZER_ALARM_Pin GPIO_PIN_15
#define BUZZER_ALARM_GPIO_Port GPIOD
#define LED_RED_ALARM_Pin GPIO_PIN_4
#define LED_RED_ALARM_GPIO_Port GPIOG
#define UCPD_FLT_Pin GPIO_PIN_7
#define UCPD_FLT_GPIO_Port GPIOG
#define USB_FS_N_Pin GPIO_PIN_11
#define USB_FS_N_GPIO_Port GPIOA
#define USB_FS_P_Pin GPIO_PIN_12
#define USB_FS_P_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define T_JTDI_Pin GPIO_PIN_15
#define T_JTDI_GPIO_Port GPIOA
#define A7677_UART5_TX_Pin GPIO_PIN_12
#define A7677_UART5_TX_GPIO_Port GPIOC
#define A7677_RST_Pin GPIO_PIN_0
#define A7677_RST_GPIO_Port GPIOD
#define A7677_PWRKEY_Pin GPIO_PIN_1
#define A7677_PWRKEY_GPIO_Port GPIOD
#define A7677_UART5_RX_Pin GPIO_PIN_2
#define A7677_UART5_RX_GPIO_Port GPIOD
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define ARD_D1_TX_Pin GPIO_PIN_6
#define ARD_D1_TX_GPIO_Port GPIOB
#define ARD_D0_RX_Pin GPIO_PIN_7
#define ARD_D0_RX_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
