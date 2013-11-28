/**************************************************************************//**
\file  wdt.c

\brief Implementation of WDT interrupt handler.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    30/08/07 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                   Includes section
******************************************************************************/
#include <wdtCtrl.h>
#include <stddef.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define RESET_KEY           0xA5

/******************************************************************************
                   Global variables section
******************************************************************************/
void (*halWdtCallback)(void) = NULL;

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
register user callback function
Parameters:
  wdtCallback - pointer to user callback function
Returns:
  none.
******************************************************************************/
void HAL_RegisterWdtCallback(void (*wdtCallback)(void))
{
  halWdtCallback = wdtCallback;
}

/******************************************************************************
initialize and start wdt
Parameters:
  interval - time befor wdt fault
Returns:
  none.
******************************************************************************/
void HAL_StartWdt(HAL_WdtInterval_t interval)
{
  // this register can only be written once after a porcessor reset
  // this has to be kept in mind when starting bootloader / main program
  AT91C_BASE_WDTC->WDTC_WDMR = 
    AT91C_WDTC_WDDBGHLT |
    ((interval << 16) & AT91C_WDTC_WDD) |
    AT91C_WDTC_WDRSTEN |
	((interval << 0) & AT91C_WDTC_WDV);

  HAL_ResetWdt();
}

/******************************************************************************
wdt interrupt handler
Parameters:
  none
Returns:
  none.
******************************************************************************/
void wdtHandler(void)
{
  // start user function
  if (halWdtCallback != NULL)
    halWdtCallback();
  // reset core, wdt and perephireal
  AT91C_BASE_RSTC->RSTC_RCR = KEY | AT91C_RSTC_PROCRST | AT91C_RSTC_PERRST;
}

/******************************************************************************
Software reset.
******************************************************************************/
void halWarmReset(void)
{
  // software reset processor core and peripherals
  AT91C_BASE_RSTC->RSTC_RCR = AT91C_RSTC_PROCRST | AT91C_RSTC_PERRST | (RESET_KEY << 24);
}

/******************************************************************************
Read reset reason.
Returns:
  reset reason.
******************************************************************************/
uint8_t halReadResetReason(void)
{
  return (uint8_t)(AT91C_BASE_RSTC->RSTC_RSR >> 8);
}

//eof wdt.c
