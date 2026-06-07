/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  * of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */
// Biến toàn cục lưu trạng thái Mute của còi (0: Kêu bình thường, 1: Tắt tiếng tạm thời)
static volatile uint8_t flag_buzzer_muted = 0;

/**
  * @brief Hàm kiểm tra trạng thái còi có đang bị tắt tiếng hay không
  * @note  Hàm này giải quyết dứt điểm lỗi Linker undefined reference tại alarm_logic.c
  */
uint8_t is_buzzer_muted(void) {
    return flag_buzzer_muted;
}

/**
  * @brief Hàm set trạng thái còi (Dùng khi bấm nút tắt tiếng tạm thời)
  */
void set_buzzer_mute_state(uint8_t state) {
    flag_buzzer_muted = state;
}
/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins
     PC14-OSC32_IN(OSC32_IN)   ------> RCC_OSC32_IN
     PC15-OSC32_OUT(OSC32_OUT)   ------> RCC_OSC32_OUT
     PH0-OSC_IN(PH0)   ------> RCC_OSC_IN
     PA11   ------> USB_DM
     PA12   ------> USB_DP
     PA13(JTMS/SWDIO)   ------> DEBUG_JTMS-SWDIO
     PA14(JTCK/SWCLK)   ------> DEBUG_JTCK-SWCLK
     PA15(JTDI)   ------> DEBUG_JTDI
     PB3(JTDO/TRACESWO)   ------> DEBUG_JTDO-SWO
     PB6   ------> LPUART1_TX
     PB7   ------> LPUART1_RX
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_DATA_DB12_Pin|LCD_CS_Pin|LCD_DC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GREEN_ON_GPIO_Port, LED_GREEN_ON_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, BUZZER_ALARM_Pin|A7677_RST_Pin|A7677_PWRKEY_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_RED_ALARM_GPIO_Port, LED_RED_ALARM_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : GPIO_DATA_DB12_Pin */
  GPIO_InitStruct.Pin = GPIO_DATA_DB12_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIO_DATA_DB12_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : GPIO_DO_MH_sensor_Pin */
  GPIO_InitStruct.Pin = GPIO_DO_MH_sensor_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIO_DO_MH_sensor_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_CS_Pin LCD_DC_Pin */
  GPIO_InitStruct.Pin = LCD_CS_Pin|LCD_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_RST_Pin */
  GPIO_InitStruct.Pin = LCD_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_GREEN_ON_Pin */
  GPIO_InitStruct.Pin = LED_GREEN_ON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GREEN_ON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BUZZER_ALARM_Pin A7677_RST_Pin A7677_PWRKEY_Pin */
  GPIO_InitStruct.Pin = BUZZER_ALARM_Pin|A7677_RST_Pin|A7677_PWRKEY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_RED_ALARM_Pin */
  GPIO_InitStruct.Pin = LED_RED_ALARM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_RED_ALARM_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : UCPD_FLT_Pin */
  GPIO_InitStruct.Pin = UCPD_FLT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(UCPD_FLT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : USB_FS_N_Pin USB_FS_P_Pin */
  GPIO_InitStruct.Pin = USB_FS_N_Pin|USB_FS_P_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_USB;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : ARD_D1_TX_Pin ARD_D0_RX_Pin */
  GPIO_InitStruct.Pin = ARD_D1_TX_Pin|ARD_D0_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF8_LPUART1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI13_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI13_IRQn);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
