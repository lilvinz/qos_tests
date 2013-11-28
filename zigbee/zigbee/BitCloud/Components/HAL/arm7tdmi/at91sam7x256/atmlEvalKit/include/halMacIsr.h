/**************************************************************************//**
\file  halMacIsr.h

\brief Implementation mac timer.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    14/11/07 A. Khromykh - Modified for arm.
******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

#ifndef _HALMACISR_H
#define _HALMACISR_H

/******************************************************************************
                        Includes section.
******************************************************************************/
#include <types.h>
#include <AT91SAM7X256.h>
#include <halAppClock.h>

/******************************************************************************
                        Types section.
******************************************************************************/
typedef enum
{
  HAL_RTIMER_REPEAT_MODE   = 0,
  HAL_RTIMER_ONE_SHOT_MODE = 1,
  HAL_RTIMER_STOPPED_MODE  = 2
} HAL_RTimerMode_t;

typedef struct // Timer description.
{
  volatile HAL_RTimerMode_t    mode;       // Mode.
  volatile uint16_t            period;     // Period of the timer.
  volatile uint16_t            nextEvent;  // Counter of periods.
} RTimerDescr_t;

/******************************************************************************
                        External variables.
******************************************************************************/
extern RTimerDescr_t __rtimer;

/******************************************************************************
                        Prototypes section.
******************************************************************************/
/******************************************************************************
  Initializes Rtimer and RF ext. interrupts.
******************************************************************************/
void HAL_InitMacIsr(void);

/******************************************************************************
  MAC timer handler.
******************************************************************************/
void halMacTimerHandler(void);

/****************************************************************
  RF interrupt service routine.
****************************************************************/
#if defined(PLATFORM_SAM7X_EK_RF2XX) || defined(PLATFORM_CUSTOM_1) || defined(PLATFORM_CUSTOM_2)
  #if defined(FREE_RTOS)
    void irqRfHandler(void) __attribute__((naked));
  #else
    void irqRfHandler(void);
  #endif
#elif defined(PLATFORM_ZIGBIT_LAN) || defined(PLATFORM_ZIGBIT_TURBO)
  void irqRfHandler(void);
#endif

/******************************************************************************
  Starts RTimer. Function should be invoked in critical section.
  Parameters:
    mode    - RTimer mode.
    period  - RTimer period.
******************************************************************************/
bool HAL_StartRtimer(HAL_RTimerMode_t mode, uint16_t period);

/******************************************************************************
  Stops RTimer. Function should be invoked in critical section.
******************************************************************************/
void HAL_StopRtimer(void);

#endif /* _HALMACISR_H */

// eof halMacIsr.h
