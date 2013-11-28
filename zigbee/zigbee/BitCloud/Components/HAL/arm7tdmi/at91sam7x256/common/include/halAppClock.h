/**************************************************************************//**
\file  halAppClock.h

\brief Declarations of appTimer hardware-dependent module.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    16/08/07 A. Khromykh - Created
******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/
#ifndef _HALAPPCLOCK_H
#define _HALAPPCLOCK_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <halClkCtrl.h>
#include <halTaskManager.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
/** \brief system timer interval in ms */
#define HAL_APPTIMERINTERVAL 10ul
/** \brief frequency prescaler for system timer */
#define TIMER_FREQUENCY_PRESCALER  8
/** \brief timer counter top value */
#define TOP_TIMER_COUNTER_VALUE  ((F_CPU/1000ul) / TIMER_FREQUENCY_PRESCALER) * HAL_APPTIMERINTERVAL

/******************************************************************************
                   Prototypes section
******************************************************************************/
/**************************************************************************//**
\brief configure, enable and start timer counter channel 0
******************************************************************************/
void halStartAppClock(void);

/**************************************************************************//**
\brief Stop and disable timer
******************************************************************************/
void halStopAppClock(void);

/**************************************************************************//**
\brief Synchronization system time which based on application timer.
******************************************************************************/
void halAppSystemTimeSynchronize(void);

/**************************************************************************//**
\brief Return time of sleep timer.

\return
  time in ms.
******************************************************************************/
uint32_t halGetTimeOfAppTimer(void);

#endif
// eof halAppClock.h
