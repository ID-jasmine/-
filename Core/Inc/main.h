/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

void Sending(void);
void clamp_values(uint8_t *temperature, uint8_t *humidity, float *light, uint8_t *marks_LED, uint8_t *marks_BEEP);
uint8_t Json_getValue(char *str);
uint8_t Json_getTarget(char *str,char* toBuf);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BEEP_Pin GPIO_PIN_1
#define BEEP_GPIO_Port GPIOA
#define ESP_RX_Pin GPIO_PIN_2
#define ESP_RX_GPIO_Port GPIOA
#define ESP_TX_Pin GPIO_PIN_3
#define ESP_TX_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_4
#define LED_GPIO_Port GPIOA
#define OLEDSCL_Pin GPIO_PIN_5
#define OLEDSCL_GPIO_Port GPIOA
#define OLEDSDA_Pin GPIO_PIN_7
#define OLEDSDA_GPIO_Port GPIOA
#define SW1_Pin GPIO_PIN_0
#define SW1_GPIO_Port GPIOB
#define SW1_EXTI_IRQn EXTI0_IRQn
#define SW2_Pin GPIO_PIN_1
#define SW2_GPIO_Port GPIOB
#define SW2_EXTI_IRQn EXTI1_IRQn
#define DHT11_SDA_Pin GPIO_PIN_8
#define DHT11_SDA_GPIO_Port GPIOA
#define BH1750_SCL_Pin GPIO_PIN_6
#define BH1750_SCL_GPIO_Port GPIOB
#define BH1750_SDA_Pin GPIO_PIN_7
#define BH1750_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define BEEP_1 HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET)
#define BEEP_0 HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET)
#define LED_1 HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET)
#define LED_0 HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
