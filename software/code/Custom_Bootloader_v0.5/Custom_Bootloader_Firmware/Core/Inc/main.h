/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define IATR_LED_GPIO_Output_Pin GPIO_PIN_13
#define IATR_LED_GPIO_Output_GPIO_Port GPIOC
#define HM10_is_default_settings_GPIO_Input_Pin GPIO_PIN_14
#define HM10_is_default_settings_GPIO_Input_GPIO_Port GPIOC
#define Display_C_terminal_GPIO_Output_Pin GPIO_PIN_2
#define Display_C_terminal_GPIO_Output_GPIO_Port GPIOA
#define Display_D_terminal_GPIO_Output_Pin GPIO_PIN_3
#define Display_D_terminal_GPIO_Output_GPIO_Port GPIOA
#define Water_Heating_Resistor_GPIO_Output_Pin GPIO_PIN_5
#define Water_Heating_Resistor_GPIO_Output_GPIO_Port GPIOA
#define Cold_Fan_PWM_TIM3_CH1_Pin GPIO_PIN_6
#define Cold_Fan_PWM_TIM3_CH1_GPIO_Port GPIOA
#define Hot_Fan_PWM_TIM3_CH2_Pin GPIO_PIN_7
#define Hot_Fan_PWM_TIM3_CH2_GPIO_Port GPIOA
#define Display_A_terminal_GPIO_Output_Pin GPIO_PIN_0
#define Display_A_terminal_GPIO_Output_GPIO_Port GPIOB
#define Display_B_terminal_GPIO_Output_Pin GPIO_PIN_1
#define Display_B_terminal_GPIO_Output_GPIO_Port GPIOB
#define BLE_TX_Pin GPIO_PIN_10
#define BLE_TX_GPIO_Port GPIOB
#define BLE_RX_Pin GPIO_PIN_11
#define BLE_RX_GPIO_Port GPIOB
#define Display_K2_terminal_GPIO_Output_Pin GPIO_PIN_12
#define Display_K2_terminal_GPIO_Output_GPIO_Port GPIOB
#define Display_K3_terminal_GPIO_Output_Pin GPIO_PIN_13
#define Display_K3_terminal_GPIO_Output_GPIO_Port GPIOB
#define Display_K4_terminal_GPIO_Output_Pin GPIO_PIN_14
#define Display_K4_terminal_GPIO_Output_GPIO_Port GPIOB
#define Hot_Water_Pump_GPIO_Output_Pin GPIO_PIN_15
#define Hot_Water_Pump_GPIO_Output_GPIO_Port GPIOB
#define Cold_Water_Pump_GPIO_Output_Pin GPIO_PIN_8
#define Cold_Water_Pump_GPIO_Output_GPIO_Port GPIOA
#define Display_E_terminal_GPIO_Output_Pin GPIO_PIN_5
#define Display_E_terminal_GPIO_Output_GPIO_Port GPIOB
#define Display_F_terminal_GPIO_Output_Pin GPIO_PIN_6
#define Display_F_terminal_GPIO_Output_GPIO_Port GPIOB
#define Display_G_terminal_GPIO_Output_Pin GPIO_PIN_7
#define Display_G_terminal_GPIO_Output_GPIO_Port GPIOB
#define Display_Dp_terminal_GPIO_Output_Pin GPIO_PIN_8
#define Display_Dp_terminal_GPIO_Output_GPIO_Port GPIOB
#define Display_K1_terminal_GPIO_Output_Pin GPIO_PIN_9
#define Display_K1_terminal_GPIO_Output_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
