/**************************************************************************//**
\file  halAdc.c

\brief Implementation of hardware depended ADC interface.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    26/11/07 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                   Includes section
******************************************************************************/
#include <halAdc.h>
#if defined(FREE_RTOS)
  #include <FreeRTOS.h>
  #include <task.h>
#endif

/******************************************************************************
                   Define(s) section
******************************************************************************/
// Prescaler rate selection. ADC Clock Frequency 5MHz for 10-bit resolution mode.
#define MANUAL_CLOCK 5000000ul
//ADCClock = MCK / ( (PRESCAL+1) * 2 )
#define PRESCAL   ((uint32_t)(F_CPU / (MANUAL_CLOCK*2) - 1) + 1)
#define ADC_PRESCAL   (PRESCAL <<  8)
#define ADC_CLOCK    (F_CPU / ((PRESCAL+1) * 2))
// manual max startup time
#define MANUAL_STARTUP 0.00002
//Startup Time = (STARTUP+1) * 8 / ADCClock. Max 20 us
#define ADC_STARTUP  (((uint32_t)(MANUAL_STARTUP * ADC_CLOCK / 8 - 1) + 1) << 16)
// manual min track and hold acquisition time
#define MANUAL_SHTIM 0.0000006
//Sample & Hold Time = (SHTIM+1) / ADCClock. Min 600 ns
#define ADC_SHTIM  (((uint32_t)(MANUAL_SHTIM * ADC_CLOCK - 1) + 1) << 24)

/******************************************************************************
                   Prototypes section
******************************************************************************/
#if defined(FREE_RTOS)
  void adcHandler(void) __attribute__((naked));
#endif

/******************************************************************************
                   Global variables section
******************************************************************************/
const uint8_t halAdcDevider[7]   = {2, 3, 4, 7, 15, 31, 63};
const uint8_t halAdcHoldTick[7]  = {4, 3, 2, 1, 1, 0, 0};
const uint8_t halAdcStartTick[7] = {20, 15, 12, 8, 4, 2, 1};
uint16_t selectionsAmount;
void *bufferPointer;

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
ADC conversion complete interrupt handler.
******************************************************************************/
void adcHandler(void)
{
#if defined(FREE_RTOS)
  portSAVE_CONTEXT();
#endif

  uint32_t status = AT91C_BASE_ADC->ADC_SR;

  if (status & AT91C_ADC_RXBUFF)
  {
     // Disables data ready Interrupt
     AT91C_BASE_ADC->ADC_IDR = AT91C_ADC_RXBUFF | AT91C_ADC_DRDY;
     // disables all channels
     AT91C_BASE_ADC->ADC_CHDR = ALL_CHANNEL_DISABLE;
     halSigAdcInterrupt();
  } else if (status & AT91C_ADC_DRDY)
  {
    uint32_t temp = AT91C_BASE_ADC->ADC_RCR;

    AT91C_BASE_ADC->ADC_PTCR = AT91C_PDC_RXTEN;
    while((temp == AT91C_BASE_ADC->ADC_RCR) && (0 != temp));
    AT91C_BASE_ADC->ADC_PTCR = AT91C_PDC_RXTDIS;
    // Starts conversion
    AT91C_BASE_ADC->ADC_CR = AT91C_ADC_START;
  }

#if defined(FREE_RTOS)
  /* End the interrupt in the AIC. */
  AT91C_BASE_AIC->AIC_EOICR = 0;

  portRESTORE_CONTEXT();
#endif
}

/******************************************************************************
Inits the ADC.
Parameters:
  none.
Returns:
  none.
******************************************************************************/
void  halOpenAdc(HAL_AdcParams_t *param)
{
  /* initialization adc
     reset adc  */
  AT91C_BASE_ADC->ADC_CR = AT91C_ADC_SWRST;

  /* disable receiver PDC transfer requests */
  AT91C_BASE_ADC->ADC_PTCR = AT91C_PDC_RXTDIS | AT91C_PDC_TXTDIS;

  if (RESOLUTION_8_BIT == param->resolution)
    /* Starting a conversion is only possible by software. 8-bit resolution mode. */
    AT91C_BASE_ADC->ADC_MR = ((uint32_t)halAdcDevider[param->sampleRate] << 8) |
                             ((uint32_t)halAdcHoldTick[param->sampleRate] << 24) |
                             ((uint32_t)halAdcStartTick[param->sampleRate] << 16) |
                             AT91C_ADC_LOWRES;
  else
    /* Starting a conversion is only possible by software. 10-bit resolution mode. */
    AT91C_BASE_ADC->ADC_MR = ((uint32_t)halAdcDevider[param->sampleRate] << 8) |
                             ((uint32_t)halAdcHoldTick[param->sampleRate] << 24) |
                             ((uint32_t)halAdcStartTick[param->sampleRate] << 16);

  /* saves pdc pointer and counter */
  selectionsAmount = param->selectionsAmount;
  bufferPointer = param->bufferPointer;

  /* disables all channels */
  AT91C_BASE_ADC->ADC_CHDR = ALL_CHANNEL_DISABLE;
  /* disable all adc interrupt */
  AT91C_BASE_ADC->ADC_IDR = ALL_PERIPHERIAL_INTERRUPT_DISABLE;

  if (AT91C_BASE_AIC->AIC_SVR[AT91C_ID_ADC] != (uint32_t)adcHandler)
  { /* first start */
    /* Enable interrupts */
    /* Disable the interrupt on the interrupt controller */
    AT91C_BASE_AIC->AIC_IDCR = (1 << AT91C_ID_ADC);
    /* Save the interrupt handler routine pointer and the interrupt priority */
    AT91C_BASE_AIC->AIC_SVR[AT91C_ID_ADC] = (uint32_t)adcHandler;
    /* Store the Source Mode Register */
    AT91C_BASE_AIC->AIC_SMR[AT91C_ID_ADC] = AT91C_AIC_SRCTYPE_HIGH_LEVEL | AT91C_AIC_PRIOR_LOWEST;
    /* Clear the interrupt on the interrupt controller */
    AT91C_BASE_AIC->AIC_ICCR = (1 << AT91C_ID_ADC);
    /* Enable the interrupt on the interrupt controller */
    AT91C_BASE_AIC->AIC_IECR = (1 << AT91C_ID_ADC);
  }
}

/******************************************************************************
Starts convertion on the ADC channel.
Parameters:
  channel - channel number.
Returns:
  none.
******************************************************************************/
void halStartAdc(HAL_AdcChannelNumber_t channel)
{
  /* set pdc pointer and counter */
  AT91C_BASE_ADC->ADC_RCR = selectionsAmount;
  AT91C_BASE_ADC->ADC_RPR = (uint32_t)bufferPointer;
  /* enable current channel */
  AT91C_BASE_ADC->ADC_CHER = channel;
  /* enable reciever buffer full interrupt and data ready interrupt */
  AT91C_BASE_ADC->ADC_IER = AT91C_ADC_RXBUFF | AT91C_ADC_DRDY;
  /* Starts conversion */
  AT91C_BASE_ADC->ADC_CR = AT91C_ADC_START;
}

/******************************************************************************
Closes the ADC.
******************************************************************************/
void halCloseAdc(void)
{
  AT91C_BASE_ADC->ADC_MR = AT91C_ADC_SLEEP_MODE;
}

// eof halAdc.c
