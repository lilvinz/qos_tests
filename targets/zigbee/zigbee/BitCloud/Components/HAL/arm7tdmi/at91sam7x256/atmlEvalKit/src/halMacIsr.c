/**************************************************************************//**
\file  halMacIsr.c

\brief   mac timer interface implementation.

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
/******************************************************************************
                        Includes section.
******************************************************************************/
#include <halClkCtrl.h>
#include <halRfCtrl.h>
#include <halMacIsr.h>
#if defined(FREE_RTOS)
  #include <FreeRTOS.h>
  #include <task.h>
#endif

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define HAL_NUMBER_OF_TICKS_IN_USEC    (F_CPU/(TIMER_FREQUENCY_PRESCALER * 1000000))

/******************************************************************************
                   Global variables section
******************************************************************************/
RTimerDescr_t __rtimer;

/******************************************************************************
                    Prototypes section
******************************************************************************/
/******************************************************************************
  Initializes Rtimer and RF ext. interrupts.
******************************************************************************/
void HAL_InitMacIsr(void);

/******************************************************************************
  Redirect interrupt event depending on the TrxState.
  Parameters: none.
  Returns: none.
******************************************************************************/
void phyDispatcheRTimerEvent(void);

/******************************************************************************
  Redirect interrupt event depending on the TrxState.
  Parameters: none.
  Returns: none.
******************************************************************************/
void phyDispatcheRfInterrupt(void);

/******************************************************************************
                    Implementations section
******************************************************************************/
/******************************************************************************
  Initializes Rtimer.
******************************************************************************/
void HAL_InitMacIsr(void)
{
  __rtimer.mode = HAL_RTIMER_STOPPED_MODE;
  HAL_InitRfIrq();
}

/******************************************************************************
  Starts RTimer. Function should be invoked in critical section.
  Parameters:
    mode    - RTimer mode.
    period  - RTimer period.
******************************************************************************/
bool HAL_StartRtimer(HAL_RTimerMode_t mode, uint16_t period)
{
  if (HAL_RTIMER_STOPPED_MODE != __rtimer.mode)
    return false;

  __rtimer.period    = (uint16_t)(period * HAL_NUMBER_OF_TICKS_IN_USEC);
  __rtimer.mode      = mode;
  __rtimer.nextEvent = AT91C_BASE_TC0->TC_CV + __rtimer.period;
  if (__rtimer.nextEvent > TOP_TIMER_COUNTER_VALUE)
    __rtimer.nextEvent -= TOP_TIMER_COUNTER_VALUE;
  AT91C_BASE_TC0->TC_RA = __rtimer.nextEvent;
  /* Clear status bit(possible interrupt) */
  uint32_t dummy;
  dummy = AT91C_BASE_TC0->TC_SR;
  (void)dummy;
  /* enable interrupt from A compare */
  AT91C_BASE_TC0->TC_IER = AT91C_TC_CPAS;

  return true;
}

/******************************************************************************
  Stops RTimer. Function should be invoked in critical section.
******************************************************************************/
void HAL_StopRtimer(void)
{
  /* disable interrupt from A compare */
  AT91C_BASE_TC0->TC_IDR = AT91C_TC_CPAS;
  __rtimer.mode = HAL_RTIMER_STOPPED_MODE;
}

/******************************************************************************
  MAC timer handler.
******************************************************************************/
void halMacTimerHandler(void)
{
  if (HAL_RTIMER_ONE_SHOT_MODE == __rtimer.mode)
  {
    /* disable interrupt from A compare */
    AT91C_BASE_TC0->TC_IDR = AT91C_TC_CPAS;
    __rtimer.mode = HAL_RTIMER_STOPPED_MODE;
  }
  else
  {
    __rtimer.nextEvent += __rtimer.period;
    if (__rtimer.nextEvent > TOP_TIMER_COUNTER_VALUE)
      __rtimer.nextEvent -= TOP_TIMER_COUNTER_VALUE;
    AT91C_BASE_TC0->TC_RA = __rtimer.nextEvent;
  }
  phyDispatcheRTimerEvent();
}

/****************************************************************
  RF interrupt service routine.
****************************************************************/
void irqRfHandler(void)
{
#if defined(PLATFORM_SAM7X_EK_RF2XX) || defined(PLATFORM_CUSTOM_1) || defined(PLATFORM_CUSTOM_2)
  #if defined(FREE_RTOS)
    portSAVE_CONTEXT();
  #endif
#endif

  phyDispatcheRfInterrupt();

#if defined(PLATFORM_SAM7X_EK_RF2XX) || defined(PLATFORM_CUSTOM_1) || defined(PLATFORM_CUSTOM_2)
  #if defined(FREE_RTOS)
    /* End the interrupt in the AIC. */
    AT91C_BASE_AIC->AIC_EOICR = 0;

    portRESTORE_CONTEXT();
  #endif
#endif
}

// eof halMacIsr.c
