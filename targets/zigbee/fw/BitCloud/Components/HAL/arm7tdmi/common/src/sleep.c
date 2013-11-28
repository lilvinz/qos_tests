/**************************************************************************//**
\file  sleep.c

\brief The module to make power off mode.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    2/06/08 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                   Includes section
******************************************************************************/
#include <sleep.h>
#include <appTimer.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define HAL_NULL_POINTER                      -1
#define HAL_SLEEP_TIMER_HAS_ALREADY_STARTED   -1
#define HAL_SLEEP_TIMER_IS_BUSY               -2
#define HAL_SLEEP_SYSTEM_HAS_ALREADY_STARTED  -3

/******************************************************************************
                   Implementations section
******************************************************************************/
/**************************************************************************//**
\brief Prepares mcu for power-save, power-down.
  Power-down the mode is possible only when internal RC is used
\return
  -1 - there is no possibility to sleep.
******************************************************************************/
int HAL_Sleep(void)
{
  return 0;
}

/**************************************************************************//**
\brief Starts sleep timer and HAL sleep. When system is wake up send callback
\param[in]
    sleepParam - pointer to sleep structure.
\return
    -1 - bad parameters,   \n
    -2 - sleep timer busy, \n
    -3 - sleep system has been started.
     0 - success.
******************************************************************************/
int HAL_StartSystemSleep(HAL_Sleep_t *sleepParam)
{
  static HAL_AppTimer_t wakeupTimer;

  if (!sleepParam)
    return HAL_NULL_POINTER;

  wakeupTimer.interval = sleepParam->sleepTime;
  wakeupTimer.mode = TIMER_ONE_SHOT_MODE;
  wakeupTimer.callback = sleepParam->callback;

  if (HAL_SLEEP_TIMER_HAS_ALREADY_STARTED == HAL_StartAppTimer(&wakeupTimer))
    return HAL_SLEEP_TIMER_IS_BUSY;

  if (-1 == HAL_Sleep())
    return HAL_SLEEP_SYSTEM_HAS_ALREADY_STARTED;

  return 0;
}

/***************************************************************************//**
\brief Makes MCU enter Idle mode.
*******************************************************************************/
void HAL_IdleMode(void)
{
}

// eof sleep.c
