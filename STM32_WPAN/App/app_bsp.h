/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_bsp.h
  * @author  MCD Application Team
  * @brief   Interface to manage BSP.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#ifndef APP_BSP_H
#define APP_BSP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "app_conf.h"
#include "stm32_rtos.h"

#ifdef STM32WBA55xx
#ifdef CFG_BSP_ON_DISCOVERY
#include "stm32wba55g_discovery.h"
#if (CFG_LCD_SUPPORTED == 1)
#include "stm32wba55g_discovery_lcd.h"
#include "stm32_lcd.h"
#endif /* (CFG_LCD_SUPPORTED == 1) */
#endif /* CFG_BSP_ON_DISCOVERY */
#endif /* STM32WBA55xx */

#ifdef STM32WBA65xx
#ifdef CFG_BSP_ON_DISCOVERY
#include "stm32wba65i_discovery.h"
#if (CFG_LCD_SUPPORTED == 1)
#include "stm32wba65i_discovery_lcd.h"
#include "stm32_lcd.h"
#endif /* (CFG_LCD_SUPPORTED == 1) */
#endif /* CFG_BSP_ON_DISCOVERY */
#endif /* STM32WBA65xx */

#ifdef CFG_BSP_ON_CEB
#include "b_wba5m_wpan.h"
#endif /* CFG_BSP_ON_CEB */
#ifdef CFG_BSP_ON_NUCLEO
#include "stm32wbaxx_nucleo.h"
#endif /* CFG_BSP_ON_NUCLEO */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN PI */
#include "stm32_timer.h"
/* USER CODE END PI */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum
{
  LED_PATTERN_NONE,
  LED_PATTERN_MSG_RECEIVED,   /* 2 short blinks */
  LED_PATTERN_STATUS_SENT,    /* 3 short blinks */
  LED_PATTERN_SEARCHING,      /* Fast blink (2Hz) */
  LED_PATTERN_OPERATIONAL,    /* Slow blink (0.5Hz) */
  LED_PATTERN_ERROR           /* Solid ON */
} LedPattern_t;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
#if (CFG_LCD_SUPPORTED == 1)
#define    LCD1                           (0u)
#endif /* (CFG_LCD_SUPPORTED == 1) */

#if (defined CFG_BSP_ON_DISCOVERY) && (defined STM32WBA65xx)
/* No Led Blue on Discovery for STM32WBA65I. Replaced by Red Led */
#define    LED_BLUE                       LED_RED
#endif /* (defined CFG_BSP_ON_DISCOVERY) && (defined STM32WBA65xx) */

/* USER CODE BEGIN EC */
/* Assuming LED_BLUE is the primary indicator LED */
#define INDICATOR_LED LED_BLUE
/* USER CODE END EC */

/* Exported variables --------------------------------------------------------*/
#if (CFG_BUTTON_SUPPORTED == 1)
#ifdef CFG_BSP_ON_THREADX
extern TX_SEMAPHORE         ButtonB1Semaphore, ButtonB2Semaphore, ButtonB3Semaphore;
#endif /* CFG_BSP_ON_THREADX */
#ifdef CFG_BSP_ON_FREERTOS
extern osSemaphoreId_t      ButtonB1Semaphore, ButtonB2Semaphore, ButtonB3Semaphore;
#endif /* CFG_BSP_ON_FREERTOS */
#endif /* (CFG_BUTTON_SUPPORTED == 1) */

#if (CFG_JOYSTICK_SUPPORTED == 1)
#ifdef CFG_BSP_ON_THREADX
extern TX_SEMAPHORE         JoystickUpSemaphore, JoystickRightSemaphore, JoystickDownSemaphore, JoystickLeftSemaphore, JoystickSelectSemaphore;
#endif /* CFG_BSP_ON_THREADX */
#ifdef CFG_BSP_ON_FREERTOS
extern osSemaphoreId_t      JoystickUpSemaphore, JoystickRightSemaphore, JoystickDownSemaphore, JoystickLeftSemaphore, JoystickSelectSemaphore;
#endif /* CFG_BSP_ON_FREERTOS */
#endif /* (CFG_JOYSTICK_SUPPORTED == 1) */

/* Exported macros ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void      APP_BSP_Init                    ( void );
void      APP_BSP_PostIdle                ( void );
void      APP_BSP_StandbyExit             ( void );
uint8_t   APP_BSP_SerialCmdExecute        ( uint8_t * pRxBuffer, uint16_t iRxBufferSize );

#if (CFG_LED_SUPPORTED == 1)
void      APP_BSP_LedInit                 ( void );
/* USER CODE BEGIN EFP_LED */
void      APP_BSP_LedPatternStart         ( LedPattern_t pattern );
void      APP_BSP_LedPatternStop          ( void );
/* USER CODE END EFP_LED */
#endif /* (CFG_LED_SUPPORTED == 1) */

#if (CFG_LCD_SUPPORTED == 1)
void      APP_BSP_LcdInit                 ( void );
#endif /* (CFG_LCD_SUPPORTED == 1) */

#if ( CFG_BUTTON_SUPPORTED == 1 )
void      APP_BSP_ButtonInit              ( void );
uint8_t   APP_BSP_ButtonIsLongPressed     ( uint16_t btnIdx );
void      APP_BSP_SetButtonIsLongPressed  ( uint16_t btnIdx );

/* USER CODE BEGIN EFP_BUTTON */
/* Weak definitions in app_bsp.c, override in app_zigbee_endpoint.c or similar */
void      APP_BSP_Button1ShortPressAction ( void );
void      APP_BSP_Button1LongPressAction  ( void );
/* Keep original actions for other buttons if they exist */
void      APP_BSP_Button2Action           ( void );
void      APP_BSP_Button3Action           ( void );
/* USER CODE END EFP_BUTTON */

void      BSP_PB_Callback                 ( Button_TypeDef button );

#endif /* ( CFG_BUTTON_SUPPORTED == 1 )  */

#if ( CFG_JOYSTICK_SUPPORTED == 1 )
void      APP_BSP_JoystickInit            ( void );

void      APP_BSP_JoystickUpAction        ( void );
void      APP_BSP_JoystickRightAction     ( void );
void      APP_BSP_JoystickDownAction      ( void );
void      APP_BSP_JoystickLeftAction      ( void );
void      APP_BSP_JoystickSelectAction    ( void );

void      BSP_JOY_Callback                ( JOY_TypeDef joyNb, JOYPin_TypeDef joyPin );

#endif /* CFG_JOYSTICK_SUPPORTED */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*APP_BSP_H */

