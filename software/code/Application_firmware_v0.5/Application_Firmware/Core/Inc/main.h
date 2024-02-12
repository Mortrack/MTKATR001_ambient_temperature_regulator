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

/* USER CODE BEGIN Private defines */

#define IIATR_LED_GPIO_Output_Pin GPIO_PIN_13                                   /**< @brief @ref GPIO_TypeDef Type of the GPIO Port towards which the Output Mode GPIO Pin PC13 will be used so that our MCU can turn On or Off the Is Internal Ambient Temperature Ready (IIATR) LED at will. */
#define IIATR_LED_GPIO_Output_GPIO_Port GPIOC                                   /**< @brief Label for Pin PC13 in Output Mode, which controls the Is Internal Ambient Temperature Ready (IIATR) LED of our MCU that the @ref main module will use in its program for indicating to the user whenever the MTKATR001 System has reached the Desired Internal Ambient Temperature (within the treshold specified at @ref INTERNAL_AMBIENT_TEMP_ERROR_ALLOWED) or not. @details The following are the output states to be taken into account:<br><br>* 0 (i.e., Low State and also IIATR LED turned Off) = The Desired Internal Ambient Temperature has not been reached.<br>* 1 (i.e., High State and also IIATR turned On) = The Desired Internal Ambient Temperature has been reached. */
#define HM10_is_default_settings_GPIO_Input_Pin GPIO_PIN_14                     /**< @brief @ref GPIO_TypeDef Type of the GPIO Port towards which the Input Mode GPIO Pin PC14 will be used so that our MCU can know whether the user wants it to set the default configuration settings in the HM-10 BT Device or not. */
#define HM10_is_default_settings_GPIO_Input_GPIO_Port GPIOC                     /**< @brief Label for the GPIO Pin 14 towards which the GPIO Pin PC14 in Input Mode is at, which is used so that our MCU can know whether the user wants it to set the default configuration settings in the HM-10 BT Device or not. @details The following are the possible values of this Pin:<br><br>* 0 (i.e., Low State) = Do not reset/change the configuration settings of the HM-10 BT Device.<br>* 1 (i.e., High State) = User requests to reset the configuration settings of the HM-10 BT Device to its default settings. */
#define Show_cold_fan_duty_cycle_GPIO_Input_Pin GPIO_PIN_15                     /**< @brief @ref GPIO_TypeDef Type of the GPIO Port towards which the Input Mode GPIO Pin PC15 will be used so that our MCU can know whether the user currently desires to have the Duty Cycle of the Cold Fan shown at the MTKATR001 System's 7-segment Display Device or not. */
#define Show_cold_fan_duty_cycle_GPIO_Input_GPIO_Port GPIOC                     /**< @brief Label for the GPIO Pin 15 towards which the GPIO Pin PC15 in Input Mode is at, which is used so that our MCU can know whether the user currently desires to have the Duty Cycle of the Cold Fan shown at the MTKATR001 System's 7-segment Display Device or not. @details The following are the possible values of this Pin:<br><br>* 0 (i.e., Low State) = The user does not want the Duty Cycle of the Cold Fan to be shown at the 7-Segment Display Device.<br>* 1 (i.e., High State) = The user wants the Duty Cycle of the Cold Fan to be shown at the 7-Segment Display Device. */
#define Cold_Water_Temp_Sensor_ADC1_IN0_Pin GPIO_PIN_0
#define Cold_Water_Temp_Sensor_ADC1_IN0_GPIO_Port GPIOA
#define Hot_Water_Temp_Sensor_ADC1_IN1_Pin GPIO_PIN_1
#define Hot_Water_Temp_Sensor_ADC1_IN1_GPIO_Port GPIOA
#define Display_C_terminal_GPIO_Output_Pin GPIO_PIN_2
#define Display_C_terminal_GPIO_Output_GPIO_Port GPIOA
#define Display_D_terminal_GPIO_Output_Pin GPIO_PIN_3
#define Display_D_terminal_GPIO_Output_GPIO_Port GPIOA
#define Internal_Ambient_Temp_Sensor_ADC1_IN4_Pin GPIO_PIN_4
#define Internal_Ambient_Temp_Sensor_ADC1_IN4_GPIO_Port GPIOA
#define Water_Heating_Resistor_GPIO_Output_Pin GPIO_PIN_5                       /**< @brief @ref GPIO_TypeDef Type of the GPIO Port towards which the Output Mode GPIO Pin PA5 will be used so that our MCU can turn On or Off the Water Heating Resistor at will. */
#define Water_Heating_Resistor_GPIO_Output_GPIO_Port GPIOA                      /**< @brief Label for Pin PA5 in Output Mode, which controls the Water Heating Resistor of our MCU that the @ref main module will use in its program for turning On or Off the Water Heating Resistor in order to heat the Hot Water of the MTKATR001 System. @details The following are the output states to be taken into account:<br><br>* 0 (i.e., Low State and also Water Heating Resistor turned Off) = The Water Heating Resistor has been turned Off; the Hot Water will not be heated anymore.<br>* 1 (i.e., High State and also Water Heating Resistor turned On) = The Water Heating Resistor has been turned On; the Hot Water will be heated more. */
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
#define Hot_Water_Pump_GPIO_Output_Pin GPIO_PIN_15                              /**< @brief @ref GPIO_TypeDef Type of the GPIO Port towards which the Output Mode GPIO Pin PB15 will be used so that our MCU can turn On or Off the Hot Water Pump at will. */
#define Hot_Water_Pump_GPIO_Output_GPIO_Port GPIOB                              /**< @brief Label for Pin PB15 in Output Mode, which controls the Hot Water Pump of our MCU that the @ref main module will use in its program for turning On or Off the Hot Water Pump to assist the Hot Fan to respectively throw in Heat or not inside the MTKATR001 System. @details The following are the output states to be taken into account:<br><br>* 0 (i.e., Low State and also Hot Water Pump turned Off) = The Hot Water Pump has been turned Off; no heated water will circulate around the outer side of the Hot Fan.<br>* 1 (i.e., High State and also Hot Water Pump turned On) = The Hot Water Pump has been turned On; heated water will circulate around the outer side of the Hot Fan. */
#define Cold_Water_Pump_GPIO_Output_Pin GPIO_PIN_8                              /**< @brief @ref GPIO_TypeDef Type of the GPIO Port towards which the Output Mode GPIO Pin PA8 will be used so that our MCU can turn On or Off the Cold Water Pump at will. */
#define Cold_Water_Pump_GPIO_Output_GPIO_Port GPIOA                             /**< @brief Label for Pin PA8 in Output Mode, which controls the Cold Water Pump of our MCU that the @ref main module will use in its program for turning On or Off the Cold Water Pump to assist the Cold Fan to respectively throw in Cold Air or not inside the MTKATR001 System. @details The following are the output states to be taken into account:<br><br>* 0 (i.e., Low State and also Cold Water Pump turned Off) = The Cold Water Pump has been turned Off; no cold water will circulate around the outer side of the Cold Fan.<br>* 1 (i.e., High State and also Cold Water Pump turned On) = The Cold Water Pump has been turned On; cold water will circulate around the outer side of the Cold Fan. */
#define Hot_Water_Shortcircuit_Indicator_GPIO_Input_Pin GPIO_PIN_9              /**< @brief @ref GPIO_TypeDef Type of the GPIO Port towards which the Input Mode GPIO Pin PA9 will be used so that our MCU can know whether the Hot Water Temperature Sensor is currently under a short-circuit or not. */
#define Hot_Water_Shortcircuit_Indicator_GPIO_Input_GPIO_Port GPIOA             /**< @brief Label for the GPIO Pin 9 towards which the GPIO Pin PA9 in Input Mode is at, which is used so that our MCU can know whether the Hot Water Temperature Sensor is currently under a short-circuit or not. @details The following are the possible values of this Pin:<br><br>* 0 (i.e., Low State) = The Hot Water Temperature Sensor is currently under a short-circuit.<br>* 1 (i.e., High State) = The Hot Water Temperature Sensor is not under a short-circuit. */
#define Cold_Water_Shortcircuit_Indicator_GPIO_Input_Pin GPIO_PIN_10            /**< @brief @ref GPIO_TypeDef Type of the GPIO Port towards which the Input Mode GPIO Pin PA10 will be used so that our MCU can know whether the Cold Water Temperature Sensor is currently under a short-circuit or not. */
#define Cold_Water_Shortcircuit_Indicator_GPIO_Input_GPIO_Port GPIOA            /**< @brief Label for the GPIO Pin 10 towards which the GPIO Pin PA10 in Input Mode is at, which is used so that our MCU can know whether the Cold Water Temperature Sensor is currently under a short-circuit or not. @details The following are the possible values of this Pin:<br><br>* 0 (i.e., Low State) = The Cold Water Temperature Sensor is currently under a short-circuit.<br>* 1 (i.e., High State) = The Cold Water Temperature Sensor is not under a short-circuit. */
#define Show_desired_internal_ambient_temperature_GPIO_Input_Pin GPIO_PIN_11    /**< @brief @ref GPIO_TypeDef Type of the GPIO Port towards which the Input Mode GPIO Pin PA11 will be used so that our MCU can know whether the user currently desires to have the Desired Internal Ambient Temperature shown at the MTKATR001 System's 7-segment Display Device or not. */
#define Show_desired_internal_ambient_temperature_GPIO_Input_GPIO_Port GPIOA    /**< @brief Label for the GPIO Pin 11 towards which the GPIO Pin PA11 in Input Mode is at, which is used so that our MCU can know whether the user currently desires to have the Desired Internal Ambient Temperature shown at the MTKATR001 System's 7-segment Display Device or not. @details The following are the possible values of this Pin:<br><br>* 0 (i.e., Low State) = The user does not want the Desired Internal Ambient Temperature to be shown at the 7-Segment Display Device.<br>* 1 (i.e., High State) = The user wants the Desired Internal Ambient Temperature to be shown at the 7-Segment Display Device. */
#define Show_current_firmware_version_GPIO_Input_Pin GPIO_PIN_12                /**< @brief @ref GPIO_TypeDef Type of the GPIO Port towards which the Input Mode GPIO Pin PA12 will be used so that our MCU can know whether the user currently desires to have the Application Firmware Version shown at the MTKATR001 System's 7-segment Display Device or not. */
#define Show_current_firmware_version_GPIO_Input_GPIO_Port GPIOA                /**< @brief Label for the GPIO Pin 12 towards which the GPIO Pin PA12 in Input Mode is at, which is used so that our MCU can know whether the user currently desires to have the Application Firmware Version shown at the MTKATR001 System's 7-segment Display Device or not. @details The following are the possible values of this Pin:<br><br>* 0 (i.e., Low State) = The user does not want the current Application Firmware Version of the MTKATR001 System to be shown at the 7-Segment Display Device.<br>* 1 (i.e., High State) = The user wants the current Application Firmware Version of the MTKATR001 System to be shown at the 7-Segment Display Device. */
#define Show_hot_fan_duty_cycle_GPIO_Input_Pin GPIO_PIN_15                      /**< @brief @ref GPIO_TypeDef Type of the GPIO Port towards which the Input Mode GPIO Pin PA15 will be used so that our MCU can know whether the user currently desires to have the Duty Cycle of the Hot Fan shown at the MTKATR001 System's 7-segment Display Device or not. */
#define Show_hot_fan_duty_cycle_GPIO_Input_GPIO_Port GPIOA                      /**< @brief Label for the GPIO Pin 15 towards which the GPIO Pin PA15 in Input Mode is at, which is used so that our MCU can know whether the user currently desires to have the Duty Cycle of the Hot Fan shown at the MTKATR001 System's 7-segment Display Device or not. @details The following are the possible values of this Pin:<br><br>* 0 (i.e., Low State) = The user does not want the Duty Cycle of the Hot Fan to be shown at the 7-Segment Display Device.<br>* 1 (i.e., High State) = The user wants the Duty Cycle of the Hot Fan to be shown at the 7-Segment Display Device. */
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

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
