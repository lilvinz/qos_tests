/**************************************************************************//**
\file  halAdc.h

\brief Declaration of hardware depended ADC interface.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    03/09/07 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

#ifndef _HALADC_H
#define _HALADC_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <halTaskManager.h>
#include <halClkCtrl.h>
#include <adc.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define ALL_CHANNEL_DISABLE  ALL_PERIPHERIAL_INTERRUPT_DISABLE

/* adc resolution */
#define RESOLUTION_8_BIT  0
#define RESOLUTION_10_BIT 1

/******************************************************************************
                   Prototypes section
******************************************************************************/
/******************************************************************************
Inits the ADC.
******************************************************************************/
void halOpenAdc(HAL_AdcParams_t *param);

/******************************************************************************
Starts convertion on the ADC channel.
Parameters:
  channel - channel number.
Returns:
  none.
******************************************************************************/
void halStartAdc(HAL_AdcChannelNumber_t channel);

/******************************************************************************
Closes the ADC.
******************************************************************************/
void halCloseAdc(void);

/******************************************************************************
                   Inline static functions section
******************************************************************************/
/******************************************************************************
  adcHandler interrupt handler signal implementation
******************************************************************************/
static inline void halSigAdcInterrupt(void)
{
  halPostTask(HAL_ADC);
}
#endif /* _HALADC_H */

// eof halAdc.h
