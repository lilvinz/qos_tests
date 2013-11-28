/**************************************************************************//**
\file  adc.c

\brief Implementation of ADC interface.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    3/09/07 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                   Includes section
******************************************************************************/
#include <halAdc.h>
#include <stddef.h>

/******************************************************************************
                   Types section
******************************************************************************/
typedef enum
{
  IDLE,      // idle
  DATA,      // performs request
  BUSY       // the module is ready to start conversion
} AdcStates_t;


AdcStates_t halAdcState = IDLE; // Monitors current state

typedef struct
{
  void (*callback)(); // address of callback
} AdcControl_t;

/******************************************************************************
                   Global variables section
******************************************************************************/
AdcControl_t halAdcControl;

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
Opens the ADC to make the measuring on a ADC channel.
Parameters:
  factor - division factor
  callback - a address of the function to notify about measurement completion.
Returns:
  -1 - unsupported channel number, there is uncompleted request.
   0 - on success.
******************************************************************************/
int HAL_OpenAdc(HAL_AdcParams_t *param)
{
  if (IDLE != halAdcState)
    return -1;
  if (NULL == param)
    return -1;
  if (NULL == param->bufferPointer)
    return -1;
  if (param->resolution > RESOLUTION_10_BIT)
    return -1;
  if ((RESOLUTION_10_BIT == param->resolution) && (param->sampleRate < ADC_369KSPS))
    return -1;

  halAdcState = BUSY;
  halOpenAdc(param);
  halAdcControl.callback = param->callback;
  return 0;
}

/******************************************************************************
Starts ADC with the parameters that were defined at HAL_OpenAdc.
Parameters:
  none.
Returns:
  -1 - the ADC was not opened, unsupported channel number.
   0 - on success.
******************************************************************************/
int HAL_ReadAdc(HAL_AdcChannelNumber_t channel)
{
  uint8_t i;
  uint8_t needReturn = 0;

  if (BUSY != halAdcState)
    return -1;
  /* incorrect channel number */
  if (channel > HAL_ADC_CHANNEL7)
    return -1;
  /* incorrect channel number */
  for (i=0; i<8; i++)
  {
    if (channel & (1 << i))
    {
      if (needReturn)
        return -1;
      else
        needReturn = 1;
    }
  }

  halAdcState = DATA;
  halStartAdc(channel);
  return 0;
}

/******************************************************************************
Closes the ADC.
Parameters:
  none.
Returns:
  -1  - the module was not opened to be used.
   0  - on success.
******************************************************************************/
int HAL_CloseAdc(void)
{
  if (IDLE == halAdcState)
    return -1;

  halAdcState = IDLE;
  halCloseAdc();
  return 0;
}

/******************************************************************************
 ADC interrupt handler.
******************************************************************************/
void halSigAdcHandler(void)
{
  if (DATA == halAdcState)
  {
    halAdcState = BUSY;
    if (NULL != halAdcControl.callback)
      halAdcControl.callback();
  }
}
// eof adc.c
