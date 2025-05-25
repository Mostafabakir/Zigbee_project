/**
  ******************************************************************************
  * @file    app_bsp.c
  * @author  MCD Application Team
  * @brief   Application to manage BSP.
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

/* Includes ------------------------------------------------------------------*/
#include "app_common.h"
#include "log_module.h"
#include "app_conf.h"
#include "app_bsp.h"
#include "main.h"

#include "stm32_rtos.h"
#include "stm32_timer.h"

/* Private includes -----------------------------------------------------------*/
#include "serial_cmd_interpreter.h"

/* Private typedef -----------------------------------------------------------*/
#if (CFG_BUTTON_SUPPORTED == 1)
typedef struct
{
  Button_TypeDef      button;
  UTIL_TIMER_Object_t longTimerId;
  uint8_t             longPressed;
  uint32_t            waitingTime;
} ButtonDesc_t;
#endif /* (CFG_BUTTON_SUPPORTED == 1) */

/* USER CODE BEGIN PTD */
#if (CFG_LED_SUPPORTED == 1)
typedef struct {
    LedPattern_t currentPattern;
    uint8_t      step;
    uint32_t     interval;
    UTIL_TIMER_Object_t patternTimer;
} LedControl_t;
#endif
/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/
#if (CFG_BUTTON_SUPPORTED == 1)
#define BUTTON_LONG_PRESS_SAMPLE_MS           (50u)     /* Sample button level rate in milli seconds. */
/* USER CODE BEGIN PD_BUTTON */
#define BUTTON_LONG_PRESS_THRESHOLD_MS        (10000u)  /* ESL: Long press time threshold 10 seconds. */
/* USER CODE END PD_BUTTON */
#ifdef CFG_BSP_ON_CEB
#define BUTTON_NB_MAX                         (B2 + 1u)
#else /* CFG_BSP_ON_CEB */
#define BUTTON_NB_MAX                         (B3 + 1u)
#endif /* CFG_BSP_ON_CEB */
#endif /* (CFG_BUTTON_SUPPORTED == 1) */

#ifndef CFG_BSP_ON_SEQUENCER
#if (CFG_BUTTON_SUPPORTED == 1)
/* Push Button B1 Task related defines */
#define TASK_STACK_SIZE_BUTTON_B1             TASK_STACK_SIZE_BUTTON_Bx
#define TASK_PRIO_BUTTON_B1                   TASK_PRIO_BUTTON_Bx
#define TASK_PREEMP_BUTTON_B1                 TASK_PREEMP_BUTTON_Bx

/* Push Button B2 Task related defines */
#define TASK_STACK_SIZE_BUTTON_B2             TASK_STACK_SIZE_BUTTON_Bx
#define TASK_PRIO_BUTTON_B2                   TASK_PRIO_BUTTON_Bx
#define TASK_PREEMP_BUTTON_B2                 TASK_PREEMP_BUTTON_Bx

/* Push Button B3 Task related defines */
#define TASK_STACK_SIZE_BUTTON_B3             TASK_STACK_SIZE_BUTTON_Bx
#define TASK_PRIO_BUTTON_B3                   TASK_PRIO_BUTTON_Bx
#define TASK_PREEMP_BUTTON_B3                 TASK_PREEMP_BUTTON_Bx

#endif /* (CFG_BUTTON_SUPPORTED == 1) */
#if (CFG_JOYSTICK_SUPPORTED == 1)
/* Push Joystick Up Task related defines */
#define TASK_STACK_SIZE_JOYSTICK_UP           TASK_STACK_SIZE_JOYSTICK_x
#define TASK_PRIO_JOYSTICK_UP                 TASK_PRIO_JOYSTICK_x
#define TASK_PREEMP_JOYSTICK_UP               TASK_PREEMP_JOYSTICK_x

/* Push Joystick Right Task related defines */
#define TASK_STACK_SIZE_JOYSTICK_RIGHT        TASK_STACK_SIZE_JOYSTICK_x
#define TASK_PRIO_JOYSTICK_RIGHT              TASK_PRIO_JOYSTICK_x
#define TASK_PREEMP_JOYSTICK_RIGHT            TASK_PREEMP_JOYSTICK_x

/* Push Joystick Down Task related defines */
#define TASK_STACK_SIZE_JOYSTICK_DOWN         TASK_STACK_SIZE_JOYSTICK_x
#define TASK_PRIO_JOYSTICK_DOWN               TASK_PRIO_JOYSTICK_x
#define TASK_PREEMP_JOYSTICK_DOWN             TASK_PREEMP_JOYSTICK_x

/* Push Joystick Left Task related defines */
#define TASK_STACK_SIZE_JOYSTICK_LEFT         TASK_STACK_SIZE_JOYSTICK_x
#define TASK_PRIO_JOYSTICK_LEFT               TASK_PRIO_JOYSTICK_x
#define TASK_PREEMP_JOYSTICK_LEFT             TASK_PREEMP_JOYSTICK_x

/* Push Joystick Select Task related defines */
#define TASK_STACK_SIZE_JOYSTICK_SELECT       TASK_STACK_SIZE_JOYSTICK_x
#define TASK_PRIO_JOYSTICK_SELECT             TASK_PRIO_JOYSTICK_x
#define TASK_PREEMP_JOYSTICK_SELECT           TASK_PREEMP_JOYSTICK_x

#endif /* (CFG_JOYSTICK_SUPPORTED == 1) */
#endif /* CFG_BSP_ON_SEQUENCER */

#if (CFG_JOYSTICK_SUPPORTED == 1)
#define JOYSTICK_PRESS_SAMPLE_MS              (100u)     /* Sample Joystick level rate in milli seconds. */
#endif /* (CFG_JOYSTICK_SUPPORTED == 1) */

/* USER CODE BEGIN PD */
#if (CFG_LED_SUPPORTED == 1)
#define LED_SHORT_BLINK_MS      100
#define LED_LONG_BLINK_MS       500
#define LED_FAST_BLINK_MS       250 // 1/(2Hz) / 2 = 250ms ON, 250ms OFF
#define LED_SLOW_BLINK_MS       1000 // 1/(0.5Hz) / 2 = 1000ms ON, 1000ms OFF
#endif
/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/

/* Private constants ---------------------------------------------------------*/
#ifdef CFG_BSP_ON_FREERTOS
#if (CFG_JOYSTICK_SUPPORTED == 1)
/* FreeRtos Joystick Up stack attributes */
const osThreadAttr_t JoystickUpThreadAttributes =
{
  .name         = "Joystick Up Thread",
  .attr_bits    = TASK_DEFAULT_ATTR_BITS,
  .cb_mem       = TASK_DEFAULT_CB_MEM,
  .cb_size      = TASK_DEFAULT_CB_SIZE,
  .stack_mem    = TASK_DEFAULT_STACK_MEM,
  .priority     = TASK_PRIO_JOYSTICK_UP,
  .stack_size   = TASK_STACK_SIZE_JOYSTICK_UP
};

/* FreeRtos Joystick Right stack attributes */
const osThreadAttr_t JoystickRightThreadAttributes =
{
  .name         = "Joystick Up Thread",
  .attr_bits    = TASK_DEFAULT_ATTR_BITS,
  .cb_mem       = TASK_DEFAULT_CB_MEM,
  .cb_size      = TASK_DEFAULT_CB_SIZE,
  .stack_mem    = TASK_DEFAULT_STACK_MEM,
  .priority     = TASK_PRIO_JOYSTICK_RIGHT,
  .stack_size   = TASK_STACK_SIZE_JOYSTICK_RIGHT
};

/* FreeRtos Joystick Down stack attributes */
const osThreadAttr_t JoystickDownThreadAttributes =
{
  .name         = "Joystick Down Thread",
  .attr_bits    = TASK_DEFAULT_ATTR_BITS,
  .cb_mem       = TASK_DEFAULT_CB_MEM,
  .cb_size      = TASK_DEFAULT_CB_SIZE,
  .stack_mem    = TASK_DEFAULT_STACK_MEM,
  .priority     = TASK_PRIO_JOYSTICK_DOWN,
  .stack_size   = TASK_STACK_SIZE_JOYSTICK_DOWN
};

/* FreeRtos Joystick Left stack attributes */
const osThreadAttr_t JoystickLeftThreadAttributes =
{
  .name         = "Joystick Left Thread",
  .attr_bits    = TASK_DEFAULT_ATTR_BITS,
  .cb_mem       = TASK_DEFAULT_CB_MEM,
  .cb_size      = TASK_DEFAULT_CB_SIZE,
  .stack_mem    = TASK_DEFAULT_STACK_MEM,
  .priority     = TASK_PRIO_JOYSTICK_LEFT,
  .stack_size   = TASK_STACK_SIZE_JOYSTICK_LEFT
};

/* FreeRtos Joystick Select stack attributes */
const osThreadAttr_t JoystickSelectThreadAttributes =
{
  .name         = "Joystick Up Thread",
  .attr_bits    = TASK_DEFAULT_ATTR_BITS,
  .cb_mem       = TASK_DEFAULT_CB_MEM,
  .cb_size      = TASK_DEFAULT_CB_SIZE,
  .stack_mem    = TASK_DEFAULT_STACK_MEM,
  .priority     = TASK_PRIO_JOYSTICK_SELECT,
  .stack_size   = TASK_STACK_SIZE_JOYSTICK_SELECT
};

#endif /* (CFG_JOYSTICK_SUPPORTED == 1) */
#if (CFG_BUTTON_SUPPORTED == 1)

/* FreeRtos PushButton B1 stacks attributes */
const osThreadAttr_t ButtonB1ThreadAttributes =
{
  .name         = "PushButton B1 Thread",
  .attr_bits    = TASK_DEFAULT_ATTR_BITS,
  .cb_mem       = TASK_DEFAULT_CB_MEM,
  .cb_size      = TASK_DEFAULT_CB_SIZE,
  .stack_mem    = TASK_DEFAULT_STACK_MEM,
  .priority     = TASK_PRIO_BUTTON_B1,
  .stack_size   = TASK_STACK_SIZE_BUTTON_B1
};

/* FreeRtos PushButton B2 stacks attributes */
const osThreadAttr_t ButtonB2ThreadAttributes =
{
  .name         = "PushButton B2 Thread",
  .attr_bits    = TASK_DEFAULT_ATTR_BITS,
  .cb_mem       = TASK_DEFAULT_CB_MEM,
  .cb_size      = TASK_DEFAULT_CB_SIZE,
  .stack_mem    = TASK_DEFAULT_STACK_MEM,
  .priority     = TASK_PRIO_BUTTON_B2,
  .stack_size   = TASK_STACK_SIZE_BUTTON_B2
};

/* FreeRtos PushButton B3 stacks attributes */
const osThreadAttr_t ButtonB3ThreadAttributes =
{
  .name         = "PushButton B3 Thread",
  .attr_bits    = TASK_DEFAULT_ATTR_BITS,
  .cb_mem       = TASK_DEFAULT_CB_MEM,
  .cb_size      = TASK_DEFAULT_CB_SIZE,
  .stack_mem    = TASK_DEFAULT_STACK_MEM,
  .priority     = TASK_PRIO_BUTTON_B3,
  .stack_size   = TASK_STACK_SIZE_BUTTON_B3
};
#endif /* (CFG_BUTTON_SUPPORTED == 1) */
#endif /* CFG_BSP_ON_FREERTOS */

/* Private variables ---------------------------------------------------------*/
#if (CFG_BUTTON_SUPPORTED == 1)
/* Button management */
#ifdef CFG_BSP_ON_THREADX
TX_SEMAPHORE         ButtonB1Semaphore, ButtonB2Semaphore, ButtonB3Semaphore;
TX_THREAD            ButtonB1Thread, ButtonB2Thread, ButtonB3Thread;
#endif /* CFG_BSP_ON_THREADX */
#ifdef CFG_BSP_ON_FREERTOS
osSemaphoreId_t      ButtonB1Semaphore, ButtonB2Semaphore, ButtonB3Semaphore;
osThreadId_t         ButtonB1Thread, ButtonB2Thread, ButtonB3Thread;
#endif /* CFG_BSP_ON_FREERTOS */
#ifdef CFG_BSP_ON_CEB
static ButtonDesc_t         buttonDesc[BUTTON_NB_MAX] = { { B2, { 0 } , 0, 0 } };
#endif /* CFG_BSP_ON_CEB */
#ifdef CFG_BSP_ON_NUCLEO
/* Assuming B1 is the primary button for ESL */
static ButtonDesc_t         buttonDesc[BUTTON_NB_MAX] = { { B1, { 0 }, 0, 0 } , { B2, { 0 } , 0, 0 }, { B3, { 0 }, 0, 0 } };
#endif /* CFG_BSP_ON_NUCLEO */
#endif /* (CFG_BUTTON_SUPPORTED == 1) */

#if (CFG_JOYSTICK_SUPPORTED == 1)
/* Joystick management */
static UTIL_TIMER_Object_t  joystickTimer;
#ifdef CFG_BSP_ON_THREADX
TX_SEMAPHORE         JoystickUpSemaphore, JoystickRightSemaphore, JoystickDownSemaphore, JoystickLeftSemaphore, JoystickSelectSemaphore;
TX_THREAD            JoystickUpThread, JoystickRightThread, JoystickDownThread, JoystickLeftThread, JoystickSelectThread;
#endif /* CFG_BSP_ON_THREADX */
#ifdef CFG_BSP_ON_FREERTOS
osSemaphoreId_t      JoystickUpSemaphore, JoystickRightSemaphore, JoystickDownSemaphore, JoystickLeftSemaphore, JoystickSelectSemaphore;
osThreadId_t         JoystickUpThread, JoystickRightThread, JoystickDownThread, JoystickLeftThread, JoystickSelectThread;
#endif /* CFG_BSP_ON_FREERTOS */
#endif /* (CFG_JOYSTICK_SUPPORTED == 1) */

/* USER CODE BEGIN PV */
#if (CFG_LED_SUPPORTED == 1)
static LedControl_t ledControl = { .currentPattern = LED_PATTERN_NONE };
#endif
/* USER CODE END PV */

/* Global variables ----------------------------------------------------------*/

/* Private functions prototypes-----------------------------------------------*/
#if (CFG_BUTTON_SUPPORTED == 1)
static void Button_LongTimerElapsed     ( void * arg );
static void Button_TriggerActions         ( void * arg );
#endif /* (CFG_BUTTON_SUPPORTED == 1) */

/* USER CODE BEGIN PFP */
#if (CFG_LED_SUPPORTED == 1)
static void LedPatternTimerCallback(void *arg);
#endif
/* USER CODE END PFP */

/* External variables --------------------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/
/**
 * @brief   Initialisation of all used BSP and their Task if needed.
 */
void APP_BSP_Init( void )
{
#if (CFG_LED_SUPPORTED == 1)
  APP_BSP_LedInit();
#endif /* (CFG_LED_SUPPORTED == 1) */

#ifdef CFG_BSP_ON_DISCOVERY
#if (CFG_LCD_SUPPORTED == 1)
  APP_BSP_LcdInit();
#endif /* (CFG_LCD_SUPPORTED == 1) */

#if (CFG_JOYSTICK_SUPPORTED == 1)
  APP_BSP_JoystickInit();
#endif /* (CFG_JOYSTICK_SUPPORTED == 1) */
#endif /* CFG_BSP_ON_DISCOVERY */

#if (CFG_BUTTON_SUPPORTED == 1)
  APP_BSP_ButtonInit();
#endif /* (CFG_BUTTON_SUPPORTED == 1) */
}

/**
 * @brief   Verify if Wakeup is not done by a button (or Joystick)
 */
void APP_BSP_PostIdle( void )
{
#if (CFG_LPM_STDBY_SUPPORTED != 0)
#if (CFG_BUTTON_SUPPORTED == 1)
  /* Treatment of WakeUp Button */
  if ( ( PWR->WUSR & PWR_WAKEUP_PIN2 ) != 0 )
  {
    PWR->WUSCR = PWR_WAKEUP_PIN2;
    BSP_PB_Callback( B1 );
  }
#endif /* (CFG_BUTTON_SUPPORTED == 1) */
#if (CFG_JOYSTICK_SUPPORTED == 1)
  if ( JOY_StandbyExitFlag == 1u )
  {
    /* Could reconfigure Joystick here */
    JOY_StandbyExitFlag = 0;
  }
#endif /* CFG_JOYSTICK_SUPPORTED */
#endif /* (CFG_LPM_STDBY_SUPPORTED != 0) */
}

/**
 * @brief   Re-Initialisation of all used BSP after a StandBy.
 */
void APP_BSP_StandbyExit( void )
{
#if (CFG_LED_SUPPORTED == 1)
  /* Leds Initialization */
  BSP_LED_Init(LED_BLUE);
#ifdef CFG_BSP_ON_NUCLEO
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_RED);
#endif /* CFG_BSP_ON_NUCLEO */
#endif /* (CFG_LED_SUPPORTED == 1) */

#if (CFG_BUTTON_SUPPORTED == 1)
#ifdef CFG_BSP_ON_DISCOVERY
  /* Joystick HW Initialization */
  BSP_JOY_Init( JOY1, JOY_MODE_EXTI, JOY_ALL );
#endif /* CFG_BSP_ON_DISCOVERY */

#ifdef CFG_BSP_ON_CEB
  /* Button HW Initialization */
  BSP_PB_Init( B2, BUTTON_MODE_EXTI );
#endif /* CFG_BSP_ON_CEB */

#ifdef CFG_BSP_ON_NUCLEO
  /* Buttons HW Initialization */
  BSP_PB_Init( B1, BUTTON_MODE_EXTI );
  BSP_PB_Init( B2, BUTTON_MODE_EXTI );
  BSP_PB_Init( B3, BUTTON_MODE_EXTI );
#endif /* CFG_BSP_ON_NUCLEO */
#endif /* (CFG_BUTTON_SUPPORTED == 1) */
}

#if ( CFG_BUTTON_SUPPORTED == 1 )

/**
 * @brief   Indicate if the selected button was pressed during a 'long time' or not.
 *
 * @param   btnIdx    Button to test, listed in enum Button_TypeDef
 * @return  '1' if pressed during a 'long time', else '0'.
 */
uint8_t APP_BSP_ButtonIsLongPressed( uint16_t btnIdx )
{
  uint8_t pressStatus = 0;

  if ( btnIdx < BUTTON_NB_MAX )
  {
    pressStatus = buttonDesc[btnIdx].longPressed;
    buttonDesc[btnIdx].longPressed = 0; /* Consume the flag */
  }

  return pressStatus;
}

/**
 * @brief   'Manually' set the selected button as pressed during a 'long time'.
 *
 * @param   btnIdx    Button to test, listed in enum Button_TypeDef
 */
void APP_BSP_SetButtonIsLongPressed( uint16_t btnIdx )
{
  if ( btnIdx < BUTTON_NB_MAX )
  {
      buttonDesc[btnIdx].longPressed = 1;
  }
}

/* USER CODE BEGIN FD_BUTTON_ACTIONS */
/* Weak definitions for button actions. Override these in application files (e.g., app_zigbee_endpoint.c) */
__WEAK void APP_BSP_Button1ShortPressAction( void )
{
  LOG_INFO_APP("Button 1 Short Press Action (Default)");
}

__WEAK void APP_BSP_Button1LongPressAction( void )
{
  LOG_INFO_APP("Button 1 Long Press Action (Default)");
}

/* Keep original weak definitions for other buttons if they exist and are needed */
__WEAK void APP_BSP_Button2Action( void )
{
  LOG_INFO_APP("Button 2 Action (Default)");
}

__WEAK void APP_BSP_Button3Action( void )
{
  LOG_INFO_APP("Button 3 Action (Default)");
}
/* USER CODE END FD_BUTTON_ACTIONS */

#endif /* ( CFG_BUTTON_SUPPORTED == 1 )  */
#if ( CFG_JOYSTICK_SUPPORTED == 1 )

/**
 * @brief  Action of Joystick UP when pressed, to be implemented by user.
 * @param  None
 * @retval None
 */
__WEAK void APP_BSP_JoystickUpAction( void )
{
}

/**
 * @brief  Action of Joystick RIGHT when pressed, to be implemented by user.
 * @param  None
 * @retval None
 */
__WEAK void APP_BSP_JoystickRightAction( void )
{
}

/**
 * @brief  Action of Joystick DOWN when pressed, to be implemented by user.
 * @param  None
 * @retval None
 */
__WEAK void APP_BSP_JoystickDownAction( void )
{
}

/**
 * @brief  Action of Joystick LEFT when pressed, to be implemented by user.
 * @param  None
 * @retval None
 */
__WEAK void APP_BSP_JoystickLeftAction( void )
{
}

/**
 * @brief  Action of Joystick SELECT when pressed, to be implemented by user.
 * @param  None
 * @retval None
 */
__WEAK void APP_BSP_JoystickSelectAction( void )
{
}

#endif /* ( CFG_JOYSTICK_SUPPORTED == 1 ) */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

#if ( CFG_LED_SUPPORTED == 1 )

/**
 * @brief  Initialisation of all LED on used boards.
 */
void APP_BSP_LedInit( void )
{
  /* Leds Initialization */
#ifdef CFG_BSP_ON_DISCOVERY
#ifdef STM32WBA65xx
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_RED);
#else // STM32WBA65xx
  BSP_LED_Init(LED_BLUE);
#endif // STM32WBA65xx
#endif // CFG_BSP_ON_DISCOVERY

#if CFG_BSP_ON_NUCLEO
  BSP_LED_Init(LED_BLUE);
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_RED);
#endif /* CFG_BSP_ON_NUCLEO */

#if CFG_BSP_ON_CEB // Assuming CEB might be similar to Nucleo or Discovery
  BSP_LED_Init(LED_BLUE);
#endif // CFG_BSP_ON_CEB

  /* Initialize LED pattern timer */
  UTIL_TIMER_Create(&ledControl.patternTimer, 0, UTIL_TIMER_ONESHOT, LedPatternTimerCallback, NULL);
  ledControl.currentPattern = LED_PATTERN_NONE;
  ledControl.step = 0;
  BSP_LED_Off(INDICATOR_LED);
}

/* USER CODE BEGIN FD_LED */
/**
 * @brief Timer callback for executing LED patterns.
 */
static void LedPatternTimerCallback(void *arg)
{
    ledControl.step++;
    uint32_t next_interval = 0;

    switch (ledControl.currentPattern)
    {
        case LED_PATTERN_MSG_RECEIVED: // 2 short blinks: ON-OFF-ON-OFF
            if (ledControl.step == 1) { BSP_LED_On(INDICATOR_LED); next_interval = LED_SHORT_BLINK_MS; }
            else if (ledControl.step == 2) { BSP_LED_Off(INDICATOR_LED); next_interval = LED_SHORT_BLINK_MS; }
            else if (ledControl.step == 3) { BSP_LED_On(INDICATOR_LED); next_interval = LED_SHORT_BLINK_MS; }
            else if (ledControl.step == 4) { BSP_LED_Off(INDICATOR_LED); ledControl.currentPattern = LED_PATTERN_NONE; } // Pattern finished
            break;

        case LED_PATTERN_STATUS_SENT: // 3 short blinks: ON-OFF-ON-OFF-ON-OFF
            if (ledControl.step == 1) { BSP_LED_On(INDICATOR_LED); next_interval = LED_SHORT_BLINK_MS; }
            else if (ledControl.step == 2) { BSP_LED_Off(INDICATOR_LED); next_interval = LED_SHORT_BLINK_MS; }
            else if (ledControl.step == 3) { BSP_LED_On(INDICATOR_LED); next_interval = LED_SHORT_BLINK_MS; }
            else if (ledControl.step == 4) { BSP_LED_Off(INDICATOR_LED); next_interval = LED_SHORT_BLINK_MS; }
            else if (ledControl.step == 5) { BSP_LED_On(INDICATOR_LED); next_interval = LED_SHORT_BLINK_MS; }
            else if (ledControl.step == 6) { BSP_LED_Off(INDICATOR_LED); ledControl.currentPattern = LED_PATTERN_NONE; } // Pattern finished
            break;

        case LED_PATTERN_SEARCHING: // Fast blink (2Hz): ON-OFF repeat
            BSP_LED_Toggle(INDICATOR_LED);
            next_interval = LED_FAST_BLINK_MS;
            break;

        case LED_PATTERN_OPERATIONAL: // Slow blink (0.5Hz): ON-OFF repeat
            BSP_LED_Toggle(INDICATOR_LED);
            next_interval = LED_SLOW_BLINK_MS;
            break;

        case LED_PATTERN_ERROR: // Solid ON - handled by APP_BSP_LedPatternStart
            // No timer needed, stays ON until stopped.
            break;

        case LED_PATTERN_NONE:
        default:
            BSP_LED_Off(INDICATOR_LED);
            break;
    }

    if (next_interval > 0)
    {
        UTIL_TIMER_SetPeriod(&ledControl.patternTimer, next_interval);
        UTIL_TIMER_Start(&ledControl.patternTimer);
    }
}

/**
 * @brief Start displaying an LED pattern.
 * @param pattern The pattern to display.
 */
void APP_BSP_LedPatternStart(LedPattern_t pattern)
{
    UTIL_TIMER_Stop(&ledControl.patternTimer); // Stop any current pattern
    ledControl.currentPattern = pattern;
    ledControl.step = 0;

    switch (pattern)
    {
        case LED_PATTERN_MSG_RECEIVED:
        case LED_PATTERN_STATUS_SENT:
        case LED_PATTERN_SEARCHING:
        case LED_PATTERN_OPERATIONAL:
            LedPatternTimerCallback(NULL); // Start the first step
            break;
        case LED_PATTERN_ERROR:
            BSP_LED_On(INDICATOR_LED);
            break;
        case LED_PATTERN_NONE:
        default:
            BSP_LED_Off(INDICATOR_LED);
            break;
    }
}

/**
 * @brief Stop the currently active LED pattern.
 */
void APP_BSP_LedPatternStop(void)
{
    UTIL_TIMER_Stop(&ledControl.patternTimer);
    ledControl.currentPattern = LED_PATTERN_NONE;
    ledControl.step = 0;
    BSP_LED_Off(INDICATOR_LED);
    // Optionally, start the operational blink if needed after stopping another pattern
    // APP_BSP_LedPatternStart(LED_PATTERN_OPERATIONAL);
}
/* USER CODE END FD_LED */

#endif /* (CFG_LED_SUPPORTED == 1) */

#ifdef CFG_BSP_ON_DISCOVERY
#if (CFG_LCD_SUPPORTED == 1)

/**
 * @brief  Initialisation of the LCD screen on used Discovery board.
 */
void APP_BSP_LcdInit( void )
{
  int32_t   iStatus;

  /* LCD Initialisation */
  iStatus = BSP_LCD_Init( LCD1, LCD_ORIENTATION_LANDSCAPE );
  if ( iStatus == BSP_ERROR_NONE )
  {
    iStatus = BSP_LCD_DisplayOn( LCD1 );
  }

  if ( iStatus == BSP_ERROR_NONE )
  {
    /* LCD Management Initialisation */
    UTIL_LCD_SetFuncDriver( &LCD_Driver );

    /* Clear the Background Layer */
    UTIL_LCD_Clear( LCD_COLOR_BLACK );

    /* Select font and Color */
    UTIL_LCD_SetFont( &Font12 );
    UTIL_LCD_SetBackColor( LCD_COLOR_BLACK );
    UTIL_LCD_SetTextColor( LCD_COLOR_WHITE );
  }
}

#endif /* (CFG_LCD_SUPPORTED == 1) */
#if (CFG_JOYSTICK_SUPPORTED == 1)

/**
 * @brief  Launch the Task selected by the joystick.
 *
 * @param  joystickState  position of joystick.
 * @retval None
 */
static void Joystick_LaunchActionTask( JOYPin_TypeDef joystickState )
{
  switch ( joystickState )
  {
    case JOY_UP:
#ifdef CFG_BSP_ON_THREADX
      tx_semaphore_put( &JoystickUpSemaphore );
#endif /* CFG_BSP_ON_THREADX */
#ifdef CFG_BSP_ON_FREERTOS
      osSemaphoreRelease( JoystickUpSemaphore );
#endif /* CFG_BSP_ON_FREERTOS */
      break;

    case JOY_RIGHT:
#ifdef CFG_BSP_ON_THREADX
      tx_semaphore_put( &JoystickRightSemaphore );
#endif /* CFG_BSP_ON_THREADX */
#ifdef CFG_BSP_ON_FREERTOS
      osSemaphoreRelease( JoystickRightSemaphore );
#endif /* CFG_BSP_ON_FREERTOS */
      break;

    case JOY_DOWN:
#ifdef CFG_BSP_ON_THREADX
      tx_semaphore_put( &JoystickDownSemaphore );
#endif /* CFG_BSP_ON_THREADX */
#ifdef CFG_BSP_ON_FREERTOS
      osSemaphoreRelease( JoystickDownSemaphore );
#endif /* CFG_BSP_ON_FREERTOS */
      break;

    case JOY_LEFT:
#ifdef CFG_BSP_ON_THREADX
      tx_semaphore_put( &JoystickLeftSemaphore );
#endif /* CFG_BSP_ON_THREADX */
#ifdef CFG_BSP_ON_FREERTOS
      osSemaphoreRelease( JoystickLeftSemaphore );
#endif /* CFG_BSP_ON_FREERTOS */
      break;

    case JOY_SEL:
#ifdef CFG_BSP_ON_THREADX
      tx_semaphore_put( &JoystickSelectSemaphore );
#endif /* CFG_BSP_ON_THREADX */
#ifdef CFG_BSP_ON_FREERTOS
      osSemaphoreRelease( JoystickSelectSemaphore );
#endif /* CFG_BSP_ON_FREERTOS */
      break;

    default:
      break;
  }
}

/**
 * @brief  Joystick Timer Callback.
 *
 * @param  arg  Not used
 * @retval None
 */
static void Joystick_TimerCallback( void * arg )
{
  JOYPin_TypeDef joystickState = BSP_JOY_GetState( JOY1 );

  if ( joystickState != JOY_NONE )
  {
    Joystick_LaunchActionTask( joystickState );
  }
}

/**
 * @brief  Initialisation of the Joystick on used Discovery board.
 */
void APP_BSP_JoystickInit( void )
{
  /* Joystick HW Initialization */
  BSP_JOY_Init( JOY1, JOY_MODE_EXTI, JOY_ALL );

  /* Joystick Timer Initialization */
  UTIL_TIMER_Create( &joystickTimer, 0, UTIL_TIMER_PERIODIC, Joystick_TimerCallback, NULL );
  UTIL_TIMER_SetPeriod( &joystickTimer, JOYSTICK_PRESS_SAMPLE_MS );
  UTIL_TIMER_Start( &joystickTimer );

#ifdef CFG_BSP_ON_THREADX
  /* Joystick Semaphores Initialization */
  tx_semaphore_create( &JoystickUpSemaphore, "Joystick Up Semaphore", 0 );
  tx_semaphore_create( &JoystickRightSemaphore, "Joystick Right Semaphore", 0 );
  tx_semaphore_create( &JoystickDownSemaphore, "Joystick Down Semaphore", 0 );
  tx_semaphore_create( &JoystickLeftSemaphore, "Joystick Left Semaphore", 0 );
  tx_semaphore_create( &JoystickSelectSemaphore, "Joystick Select Semaphore", 0 );

  /* Joystick Tasks Initialization */
  THREAD_INIT( JoystickUp, JoystickUpThread, APP_BSP_JoystickUpAction, 0, TASK_STACK_SIZE_JOYSTICK_UP, TASK_PRIO_JOYSTICK_UP, TASK_PREEMP_JOYSTICK_UP, TX_AUTO_START );
  THREAD_INIT( JoystickRight, JoystickRightThread, APP_BSP_JoystickRightAction, 0, TASK_STACK_SIZE_JOYSTICK_RIGHT, TASK_PRIO_JOYSTICK_RIGHT, TASK_PREEMP_JOYSTICK_RIGHT, TX_AUTO_START );
  THREAD_INIT( JoystickDown, JoystickDownThread, APP_BSP_JoystickDownAction, 0, TASK_STACK_SIZE_JOYSTICK_DOWN, TASK_PRIO_JOYSTICK_DOWN, TASK_PREEMP_JOYSTICK_DOWN, TX_AUTO_START );
  THREAD_INIT( JoystickLeft, JoystickLeftThread, APP_BSP_JoystickLeftAction, 0, TASK_STACK_SIZE_JOYSTICK_LEFT, TASK_PRIO_JOYSTICK_LEFT, TASK_PREEMP_JOYSTICK_LEFT, TX_AUTO_START );
  THREAD_INIT( JoystickSelect, JoystickSelectThread, APP_BSP_JoystickSelectAction, 0, TASK_STACK_SIZE_JOYSTICK_SELECT, TASK_PRIO_JOYSTICK_SELECT, TASK_PREEMP_JOYSTICK_SELECT, TX_AUTO_START );
#endif /* CFG_BSP_ON_THREADX */
#ifdef CFG_BSP_ON_FREERTOS
  /* Joystick Semaphores Initialization */
  JoystickUpSemaphore = osSemaphoreNew( 1, 0, NULL );          /* Semaphore counting to 1, not available at creation. */
  JoystickRightSemaphore = osSemaphoreNew( 1, 0, NULL );       /* Semaphore counting to 1, not available at creation. */
  JoystickDownSemaphore = osSemaphoreNew( 1, 0, NULL );        /* Semaphore counting to 1, not available at creation. */
  JoystickLeftSemaphore = osSemaphoreNew( 1, 0, NULL );        /* Semaphore counting to 1, not available at creation. */
  JoystickSelectSemaphore = osSemaphoreNew( 1, 0, NULL );      /* Semaphore counting to 1, not available at creation. */

  /* Joystick Tasks Initialization */
  JoystickUpThread = osThreadNew( (osThreadFunc_t)APP_BSP_JoystickUpAction, &JoystickUpSemaphore, &JoystickUpThreadAttributes );
  JoystickRightThread = osThreadNew( (osThreadFunc_t)APP_BSP_JoystickRightAction, &JoystickRightSemaphore, &JoystickRightThreadAttributes );
  JoystickDownThread = osThreadNew( (osThreadFunc_t)APP_BSP_JoystickDownAction, &JoystickDownSemaphore, &JoystickDownThreadAttributes );
  JoystickLeftThread = osThreadNew( (osThreadFunc_t)APP_BSP_JoystickLeftAction, &JoystickLeftSemaphore, &JoystickLeftThreadAttributes );
  JoystickSelectThread = osThreadNew( (osThreadFunc_t)APP_BSP_JoystickSelectAction, &JoystickSelectSemaphore, &JoystickSelectThreadAttributes );
#endif /* CFG_BSP_ON_FREERTOS */
}

#endif /* (CFG_JOYSTICK_SUPPORTED == 1) */
#endif /* CFG_BSP_ON_DISCOVERY */

#if ( CFG_BUTTON_SUPPORTED == 1 )

/**
 * @brief  Button Timer Callback.
 *
 * @param  arg  index of button in buttonDesc array
 * @retval None
 */
static void Button_LongTimerElapsed( void * arg )
{
  uint32_t btnIdx = (uint32_t)arg;

  if ( btnIdx < BUTTON_NB_MAX )
  {
    buttonDesc[btnIdx].longPressed = 1;
    buttonDesc[btnIdx].waitingTime = 0;

    /* Trigger actions now for long press */
    Button_TriggerActions(arg);
  }
}

/**
 * @brief  Button Trigger Actions.
 *
 * @param  arg  index of button in buttonDesc array
 * @retval None
 */
static void Button_TriggerActions( void * arg )
{
  uint32_t btnIdx = (uint32_t)arg;

  if ( btnIdx < BUTTON_NB_MAX )
  {
    /* USER CODE BEGIN Button_TriggerActions */
    /* Check if it was a long press (flag set by timer) or short press (timer stopped) */
    if (buttonDesc[btnIdx].longPressed)
    {
        // Long press already detected and flag set by timer callback
        // Call the specific long press action function
        if (buttonDesc[btnIdx].button == B1)
        {
            APP_BSP_Button1LongPressAction();
        }
        // Add else if for B2, B3 if needed for long press

        buttonDesc[btnIdx].longPressed = 0; // Consume the flag
    }
    else
    {
        // Short press detected (timer was stopped before expiring)
        // Call the specific short press action function
        if (buttonDesc[btnIdx].button == B1)
        {
            APP_BSP_Button1ShortPressAction();
        }
        else if (buttonDesc[btnIdx].button == B2)
        {
            APP_BSP_Button2Action(); // Keep original behavior for B2 if needed
        }
        else if (buttonDesc[btnIdx].button == B3)
        {
            APP_BSP_Button3Action(); // Keep original behavior for B3 if needed
        }
    }
    /* USER CODE END Button_TriggerActions */
  }
}

/**
 * @brief  Initialisation of the Buttons on used boards.
 */
void APP_BSP_ButtonInit( void )
{
  uint32_t btnIdx;

#ifdef CFG_BSP_ON_CEB
  /* Button HW Initialization */
  BSP_PB_Init( B2, BUTTON_MODE_EXTI );
#endif /* CFG_BSP_ON_CEB */

#ifdef CFG_BSP_ON_NUCLEO
  /* Buttons HW Initialization */
  BSP_PB_Init( B1, BUTTON_MODE_EXTI );
  BSP_PB_Init( B2, BUTTON_MODE_EXTI );
  BSP_PB_Init( B3, BUTTON_MODE_EXTI );
#endif /* CFG_BSP_ON_NUCLEO */

  /* Button Timer Initialization */
  for ( btnIdx = 0; btnIdx < BUTTON_NB_MAX; btnIdx++ )
  {
    UTIL_TIMER_Create( &buttonDesc[btnIdx].longTimerId, 0, UTIL_TIMER_ONESHOT, Button_LongTimerElapsed, (void *)btnIdx );
    buttonDesc[btnIdx].longPressed = 0;
    buttonDesc[btnIdx].waitingTime = 0;
  }

#ifdef CFG_BSP_ON_THREADX
  /* Button Semaphores Initialization */
  tx_semaphore_create( &ButtonB1Semaphore, "Button B1 Semaphore", 0 );
  tx_semaphore_create( &ButtonB2Semaphore, "Button B2 Semaphore", 0 );
  tx_semaphore_create( &ButtonB3Semaphore, "Button B3 Semaphore", 0 );

  /* Button Tasks Initialization */
  THREAD_INIT( ButtonB1, ButtonB1Thread, APP_BSP_Button1Action, 0, TASK_STACK_SIZE_BUTTON_B1, TASK_PRIO_BUTTON_B1, TASK_PREEMP_BUTTON_B1, TX_AUTO_START );
  THREAD_INIT( ButtonB2, ButtonB2Thread, APP_BSP_Button2Action, 0, TASK_STACK_SIZE_BUTTON_B2, TASK_PRIO_BUTTON_B2, TASK_PREEMP_BUTTON_B2, TX_AUTO_START );
  THREAD_INIT( ButtonB3, ButtonB3Thread, APP_BSP_Button3Action, 0, TASK_STACK_SIZE_BUTTON_B3, TASK_PRIO_BUTTON_B3, TASK_PREEMP_BUTTON_B3, TX_AUTO_START );
#endif /* CFG_BSP_ON_THREADX */
#ifdef CFG_BSP_ON_FREERTOS
  /* Button Semaphores Initialization */
  ButtonB1Semaphore = osSemaphoreNew( 1, 0, NULL );          /* Semaphore counting to 1, not available at creation. */
  ButtonB2Semaphore = osSemaphoreNew( 1, 0, NULL );          /* Semaphore counting to 1, not available at creation. */
  ButtonB3Semaphore = osSemaphoreNew( 1, 0, NULL );          /* Semaphore counting to 1, not available at creation. */

  /* Button Tasks Initialization - Note: These tasks might not be needed if actions are handled directly in Button_TriggerActions */
  /* ButtonB1Thread = osThreadNew( (osThreadFunc_t)APP_BSP_Button1Action, &ButtonB1Semaphore, &ButtonB1ThreadAttributes ); */
  /* ButtonB2Thread = osThreadNew( (osThreadFunc_t)APP_BSP_Button2Action, &ButtonB2Semaphore, &ButtonB2ThreadAttributes ); */
  /* ButtonB3Thread = osThreadNew( (osThreadFunc_t)APP_BSP_Button3Action, &ButtonB3Semaphore, &ButtonB3ThreadAttributes ); */
#endif /* CFG_BSP_ON_FREERTOS */
}

/**
 * @brief  BSP Push Button callback
 * @param  Button Specifies the pin connected EXTI line
 * @retval None
 */
void BSP_PB_Callback( Button_TypeDef button )
{
  uint32_t btnIdx;

  for ( btnIdx = 0; btnIdx < BUTTON_NB_MAX; btnIdx++ )
  {
    if ( buttonDesc[btnIdx].button == button )
    {
      break;
    }
  }

  if ( btnIdx < BUTTON_NB_MAX )
  {
    if ( BSP_PB_GetState( button ) == GPIO_PIN_RESET ) /* Button is pressed */
    {
      /* Start sampling timer for long press detection */
      buttonDesc[btnIdx].waitingTime = BUTTON_LONG_PRESS_THRESHOLD_MS;
      UTIL_TIMER_SetPeriod( &buttonDesc[btnIdx].longTimerId, buttonDesc[btnIdx].waitingTime );
      UTIL_TIMER_Start( &buttonDesc[btnIdx].longTimerId );
      buttonDesc[btnIdx].longPressed = 0; // Reset long press flag
    }
    else /* Button is released */
    {
      if ( buttonDesc[btnIdx].waitingTime != 0 ) /* Timer was running (i.e., press was shorter than threshold) */
      {
        UTIL_TIMER_Stop( &buttonDesc[btnIdx].longTimerId );
        buttonDesc[btnIdx].waitingTime = 0;
        buttonDesc[btnIdx].longPressed = 0; // Ensure flag is clear for short press

        /* Trigger actions now for short press */
        Button_TriggerActions( (void *)btnIdx );
      }
      // If waitingTime is 0, it means the long press timer already elapsed and triggered the action.
      // The longPressed flag would have been set by the timer callback.
      // We don't trigger actions again on release for long press.
    }
  }
}

#endif /* ( CFG_BUTTON_SUPPORTED == 1 ) */
/**
 * @brief  Launch the Task selected by the button.
 *
 * @param  button  ID of pressed button.
 * @retval None
 */
static void Button_LaunchActionTask( Button_TypeDef button )
{
#ifdef CFG_BSP_ON_CEB
  if ( button == B2 )
  {
#ifdef CFG_BSP_ON_FREERTOS
    osSemaphoreRelease( ButtonB2Semaphore );
#endif /* CFG_BSP_ON_FREERTOS */
#ifdef CFG_BSP_ON_THREADX
    tx_semaphore_put( &ButtonB2Semaphore );
#endif /* CFG_BSP_ON_THREADX */
#ifdef CFG_BSP_ON_SEQUENCER
    UTIL_SEQ_SetTask( 1U << CFG_TASK_BSP_BUTTON_B2, CFG_SEQ_PRIO_0 );
#endif /* CFG_BSP_ON_SEQUENCER */
  }
#endif /* CFG_BSP_ON_CEB */
#ifdef CFG_BSP_ON_NUCLEO
  switch ( button )
  {
#ifdef CFG_BSP_ON_FREERTOS
    case B1:
        osSemaphoreRelease( ButtonB1Semaphore );
        break;

    case B2:
        osSemaphoreRelease( ButtonB2Semaphore );
        break;

    case B3:
        osSemaphoreRelease( ButtonB3Semaphore );
        break;
#endif /* CFG_BSP_ON_FREERTOS */
#ifdef CFG_BSP_ON_THREADX
    case B1:
        tx_semaphore_put( &ButtonB1Semaphore );
        break;

    case B2:
        tx_semaphore_put( &ButtonB2Semaphore );
        break;

    case B3:
        tx_semaphore_put( &ButtonB3Semaphore );
        break;
#endif /* CFG_BSP_ON_THREADX */
#ifdef CFG_BSP_ON_SEQUENCER
    case B1:
        UTIL_SEQ_SetTask( 1U << CFG_TASK_BSP_BUTTON_B1, CFG_SEQ_PRIO_0 );
        break;

    case B2:
        UTIL_SEQ_SetTask( 1U << CFG_TASK_BSP_BUTTON_B2, CFG_SEQ_PRIO_0 );
        break;

    case B3:
        UTIL_SEQ_SetTask( 1U << CFG_TASK_BSP_BUTTON_B3, CFG_SEQ_PRIO_0 );
        break;
#endif /* CFG_BSP_ON_SEQUENCER */

    default:
        break;
  }
#endif /* CFG_BSP_ON_NUCLEO */
}
uint8_t APP_BSP_SerialCmdExecute( uint8_t * pRxBuffer, uint16_t iRxBufferSize )
{
  uint8_t   cReturn = 0;
  uint16_t  iUserChoice = UINT16_MAX;

  /* Parse received frame */
#if ( CFG_BUTTON_SUPPORTED == 1 )
#ifdef CFG_BSP_ON_CEB
  /* For Board B_WBA5M_WPAN, the only button (same as B1 on other board) is named B2. */
  if ( (strcmp( (char const*)pRxBuffer, "B2" ) == 0) ||
       (strcmp( (char const*)pRxBuffer, "SW2" ) == 0) )
  {
    iUserChoice = B2;
  }
  else if ( (strcmp( (char const*)pRxBuffer, "B2L" ) == 0) ||
            (strcmp( (char const*)pRxBuffer, "SW2L" ) == 0) )
  {
    APP_BSP_SetButtonIsLongPressed(B2);
    iUserChoice = B2;
  }
#else /* CFG_BSP_ON_CEB */
  if ( (strcmp( (char const*)pRxBuffer, "B1" ) == 0) ||
       (strcmp( (char const*)pRxBuffer, "SW1" ) == 0) )
  {
    iUserChoice = B1;
  }
  else if ( (strcmp( (char const*)pRxBuffer, "B1L" ) == 0) ||
            (strcmp( (char const*)pRxBuffer, "SW1L" ) == 0) )
  {
    APP_BSP_SetButtonIsLongPressed(B1);
    iUserChoice = B1;
  }
  else if ( (strcmp( (char const*)pRxBuffer, "B2" ) == 0) ||
            (strcmp( (char const*)pRxBuffer, "SW2" ) == 0) )
  {
    iUserChoice = B2;
  }
  else if ( (strcmp( (char const*)pRxBuffer, "B2L" ) == 0) ||
            (strcmp( (char const*)pRxBuffer, "SW2L" ) == 0) )
  {
    APP_BSP_SetButtonIsLongPressed(B2);
    iUserChoice = B2;
  }
  else if ( (strcmp( (char const*)pRxBuffer, "B3" ) == 0) ||
            (strcmp( (char const*)pRxBuffer, "SW3" ) == 0) )
  {
    iUserChoice = B3;
  }
  else if ( (strcmp( (char const*)pRxBuffer, "B3L" ) == 0) ||
            (strcmp( (char const*)pRxBuffer, "SW3L" ) == 0) )
  {
    APP_BSP_SetButtonIsLongPressed(B3);
    iUserChoice = B3;
  }
#endif /* CFG_BSP_ON_CEB */
#endif /* ( CFG_BUTTON_SUPPORTED == 1 )  */
#if ( CFG_JOYSTICK_SUPPORTED == 1 )
  if (strcmp( (char const*)pRxBuffer, "UP" ) == 0)
  {
    iUserChoice = JOY_UP;
  }
  else if (strcmp( (char const*)pRxBuffer, "RIGHT" ) == 0)
  {
    iUserChoice = JOY_RIGHT;
  }
  else if (strcmp( (char const*)pRxBuffer, "DOWN" ) == 0 )
  {
    iUserChoice = JOY_DOWN;
  }
  else if (strcmp( (char const*)pRxBuffer, "LEFT" ) == 0)
  {
    iUserChoice = JOY_LEFT;
  }
  else if (strcmp( (char const*)pRxBuffer, "SELECT" ) == 0)
  {
    iUserChoice = JOY_SEL;
  }
#endif /* ( CFG_JOYSTICK_SUPPORTED == 1 )  */

  if ( iUserChoice != UINT16_MAX )
  {
    /* Launch Command */
    LOG_INFO_APP( "%s pressed by Serial Command.", pRxBuffer );
#if ( CFG_BUTTON_SUPPORTED == 1 )
    Button_LaunchActionTask( (Button_TypeDef)iUserChoice );
#endif /* ( CFG_BUTTON_SUPPORTED == 1 ) */
#if ( CFG_JOYSTICK_SUPPORTED == 1 )
    Joystick_LaunchActionTask( (JOYPin_TypeDef)iUserChoice );
#endif /* ( CFG_JOYSTICK_SUPPORTED == 1 ) */
    cReturn = 1;
  }

  return cReturn;
}
