/**************************************************************************//**
\file  halirq.c

\brief Implementation of HWD IRQ interface.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    04/09/07 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                   Includes section
******************************************************************************/
#include <halIrq.h>
#if defined(FREE_RTOS)
  #include <FreeRTOS.h>
  #include <task.h>
#endif

/******************************************************************************
                   Define(s) section
******************************************************************************/
#if defined(PLATFORM_SAM7X_EK_RF2XX) || defined(PLATFORM_CUSTOM_1) || defined(PLATFORM_CUSTOM_2)
  #define IRQ0_PIO   AT91C_PIO_PA29
  #define IRQ1_PIO   AT91C_PIO_PA14
#elif defined(PLATFORM_ZIGBIT_LAN) || defined(PLATFORM_ZIGBIT_TURBO)
  #define IRQ0_PIO   AT91C_PIO_PA30
  #define IRQ1_PIO   AT91C_PIO_PA14
#endif

/******************************************************************************
                   Prototypes section
******************************************************************************/
#if defined(PLATFORM_SAM7X_EK_RF2XX) || defined(PLATFORM_CUSTOM_1) || defined(PLATFORM_CUSTOM_2)
  void fiqHandler(void);
  #define IRQ0HANDLER  fiqHandler
#elif defined(PLATFORM_ZIGBIT_LAN) || defined(PLATFORM_ZIGBIT_TURBO)
  #if defined(FREE_RTOS)
    void irq0Handler(void)  __attribute__((naked));
  #else
    void irq0Handler(void);
  #endif
  #define IRQ0HANDLER  irq0Handler
#endif

#if defined(FREE_RTOS)
  void irq1Handler(void)  __attribute__((naked));
#endif

/******************************************************************************
                   Global variables section
******************************************************************************/
IrqCallback_t IrqCallbackList[HAL_NUM_IRQ_LINES] = {NULL, NULL};

/******************************************************************************
                   Implementations section
******************************************************************************/
#if defined(PLATFORM_SAM7X_EK_RF2XX) || defined(PLATFORM_CUSTOM_1) || defined(PLATFORM_CUSTOM_2)
/******************************************************************************
 External fast interrupt handler
******************************************************************************/
void fiqHandler(void)
{
  if (NULL != IrqCallbackList[HAL_FIRST_VALID_IRQ])
    IrqCallbackList[HAL_FIRST_VALID_IRQ]();
}
#elif defined(PLATFORM_ZIGBIT_LAN) || defined(PLATFORM_ZIGBIT_TURBO)
/******************************************************************************
 External interrupt 0 handler
******************************************************************************/
void irq0Handler(void)
{
#if defined(FREE_RTOS)
  portSAVE_CONTEXT();
#endif

  if (NULL != IrqCallbackList[HAL_FIRST_VALID_IRQ])
    IrqCallbackList[HAL_FIRST_VALID_IRQ]();

#if defined(FREE_RTOS)
  /* End the interrupt in the AIC. */
  AT91C_BASE_AIC->AIC_EOICR = 0;

  portRESTORE_CONTEXT();
#endif
}
#endif

/******************************************************************************
 External interrupt 1 handler
******************************************************************************/
void irq1Handler(void)
{
#if defined(FREE_RTOS)
  portSAVE_CONTEXT();
#endif

  if (NULL != IrqCallbackList[HAL_SECOND_VALID_IRQ])
    IrqCallbackList[HAL_SECOND_VALID_IRQ]();

#if defined(FREE_RTOS)
  /* End the interrupt in the AIC. */
  AT91C_BASE_AIC->AIC_EOICR = 0;

  portRESTORE_CONTEXT();
#endif
}

/******************************************************************************
Sets configuration of pins and the registers.
Parameters:
  irqNumber - number of interrupt.
Returns:
  none.
******************************************************************************/
void halSetIrqConfig(uint32_t irqNumber, HAL_IrqMode_t irqMode)
{
  /* Disable the interrupt on the interrupt controller */
  AT91C_BASE_AIC->AIC_IDCR = (1 << irqNumber);
  /* Save the interrupt handler routine pointer and the interrupt priority */
  switch (irqNumber)
  {
    case IRQ0:
      AT91C_BASE_AIC->AIC_SVR[irqNumber] = (uint32_t)IRQ0HANDLER;
      /* enable pio peripheral functions fiq*/
      AT91C_BASE_PIOA->PIO_ASR = IRQ0_PIO;
      AT91C_BASE_PIOA->PIO_PDR = IRQ0_PIO;
      break;
    case IRQ1:
      AT91C_BASE_AIC->AIC_SVR[irqNumber] = (uint32_t)irq1Handler;
      /* enable pio peripheral functions irq1*/
      AT91C_BASE_PIOA->PIO_BSR = IRQ1_PIO;
      AT91C_BASE_PIOA->PIO_PDR = IRQ1_PIO;
      break;
    default:
      break;
  }
  /* Store the Source Mode Register */
  AT91C_BASE_AIC->AIC_SMR[irqNumber] = irqMode | AT91C_AIC_PRIOR_LOWEST;
  /* Clear the interrupt on the interrupt controller */
  AT91C_BASE_AIC->AIC_ICCR = (1 << irqNumber);
}

/******************************************************************************
Clears configuration of pins and the registers.
Parameters:
  irqNumber - number of interrupt.
Returns:
  none.
******************************************************************************/
void halClrIrqConfig(uint32_t irqNumber)
{
  /* Disable the interrupt on the interrupt controller */
  AT91C_BASE_AIC->AIC_IDCR = (1 << irqNumber);
  switch (irqNumber)
  {
    case IRQ0:
      /* enable pio peripheral functions fiq*/
      AT91C_BASE_PIOA->PIO_PER = IRQ0_PIO;
      break;
    case IRQ1:
      /* enable pio peripheral functions irq1*/
      AT91C_BASE_PIOA->PIO_PER = IRQ1_PIO;
      break;
    default:
      break;
  }
}

// eof irq.c
