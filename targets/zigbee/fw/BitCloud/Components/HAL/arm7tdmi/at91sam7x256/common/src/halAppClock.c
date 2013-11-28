/**************************************************************************//**
\file  halAppClock.c

\brief Implementation of appTimer hardware-dependent module.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    16/08/07 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/
/******************************************************************************
                   Includes section
******************************************************************************/
#include <halAppClock.h>
#include <halMacIsr.h>
#include <halDbg.h>
#if defined(FREE_RTOS)
  #include <FreeRTOS.h>
  #include <task.h>
#endif

/******************************************************************************
                   Prototypes section
******************************************************************************/
#if defined(FREE_RTOS)
  void timerHandler(void) __attribute__((naked));
#endif

/******************************************************************************
                     Global variables section
******************************************************************************/
static uint32_t halAppTime = 0ul;     // time of application timer
uint8_t halAppTimeOvfw = 0;
static volatile uint8_t halAppIrqCount = 0;

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
 Interrupt handler signal implementation
 Parameters:
   none
 Returns:
   none
 *****************************************************************************/
INLINE void halInterruptAppClock(void)
{
  halAppIrqCount++;
  halPostTask(HAL_TIMER4_COMPA);
  // infinity loop spy
  SYS_InfinityLoopMonitoring();
}

/**************************************************************************//**
Synchronization system time which based on application timer.
******************************************************************************/
void halAppSystemTimeSynchronize(void)
{
  uint8_t tmpCounter;
  uint32_t tmpValue;

  ATOMIC_SECTION_ENTER
    tmpCounter = halAppIrqCount;
    halAppIrqCount = 0;
  ATOMIC_SECTION_LEAVE

  tmpValue = tmpCounter * HAL_APPTIMERINTERVAL;
  halAppTime += tmpValue;
  if (halAppTime < tmpValue)
    halAppTimeOvfw++;
}

/******************************************************************
time counter interrupt handler
 Parameters:
   none
 Returns:
   none
******************************************************************/
void timerHandler(void)
{
#if defined(FREE_RTOS)
  portSAVE_CONTEXT();
#endif

  uint32_t status = AT91C_BASE_TC0->TC_SR;

  if ((status & AT91C_TC_CPCS) && (AT91C_BASE_TC0->TC_IMR & AT91C_TC_CPCS))
  {
    halInterruptAppClock();
  }
  if ((status & AT91C_TC_CPAS) && (AT91C_BASE_TC0->TC_IMR & AT91C_TC_CPAS))
  {
    halMacTimerHandler();
  }

#if defined(FREE_RTOS)
  /* End the interrupt in the AIC. */
  AT91C_BASE_AIC->AIC_EOICR = 0;

  portRESTORE_CONTEXT();
#endif
}

/******************************************************************
configure and enable timer counter channel 0
 Parameters:
   none
 Returns:
   none
******************************************************************/
void halStartAppClock(void)
{
  uint32_t dummy;

  /* Enable peripheral clock for the PIO controller */
  AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_TC0);

  /* Enable the peripheral */
  /* Disable the clock and the interrupts */
  AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKDIS;
  AT91C_BASE_TC0->TC_IDR = ALL_PERIPHERIAL_INTERRUPT_DISABLE;

  /* Clear status bit */
  dummy = AT91C_BASE_TC0->TC_SR;
  (void)dummy;

  /* Set the Mode of the Timer Counter */
  /* Waveform mode. Source TIMER_CLOCK2. RC Compare resets the counter and starts the counter clock.*/
  AT91C_BASE_TC0->TC_CMR = AT91C_TC_WAVE | AT91C_TC_CLKS_TIMER_DIV2_CLOCK | AT91C_TC_WAVESEL_UP_AUTO;
  AT91C_BASE_TC0->TC_RC = TOP_TIMER_COUNTER_VALUE;  /* MCKR divided by 8*100 */

  /* Enable interrupts */
  /* Disable the interrupt on the interrupt controller */
  AT91C_BASE_AIC->AIC_IDCR = (1ul << AT91C_ID_TC0);
  /* Save the interrupt handler routine pointer and the interrupt priority */
  AT91C_BASE_AIC->AIC_SVR[AT91C_ID_TC0] = (uint32_t)timerHandler;
  /* Store the Source Mode Register */
  AT91C_BASE_AIC->AIC_SMR[AT91C_ID_TC0] = AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL | AT91C_AIC_PRIOR_LOWEST;
  /* Clear the interrupt on the interrupt controller */
  AT91C_BASE_AIC->AIC_ICCR = (1 << AT91C_ID_TC0);
  /* enable interrupt from C compare */
  AT91C_BASE_TC0->TC_IER = AT91C_TC_CPCS;

  /* Enable the interrupt on the interrupt controller */
  AT91C_BASE_AIC->AIC_IECR = (1ul << AT91C_ID_TC0);

  /* Clock is started */
  AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKEN;
  /* Counter is reseted and the clock is started */
  AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG;
}

/******************************************************************
disable timer
 Parameters:
   none
 Returns:
   none
******************************************************************/
void halStopAppClock(void)
{
  /* Disable the clock */
  AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKDIS;
  /* Disable peripheral clock for the PIO controller */
  AT91C_BASE_PMC->PMC_PCDR = (1 << AT91C_ID_TC0);
}

/******************************************************************************
Return time of sleep timer.

Returns:
  time in ms.
******************************************************************************/
uint32_t halGetTimeOfAppTimer(void)
{
  halAppSystemTimeSynchronize();
  return halAppTime;
}

/**************************************************************************//**
\brief System clock.

\return
  system clock in Hz.
******************************************************************************/
uint32_t HAL_ReadFreq(void)
{
  return (uint32_t)F_CPU;
}

// eof halAppClock.c
