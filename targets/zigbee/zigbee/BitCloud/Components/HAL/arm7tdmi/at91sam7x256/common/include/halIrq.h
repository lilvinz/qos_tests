/*****************************************************************************//**
\file   halirq.h

\brief  Declaration of HWD IRQ interface.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    04/09/07 A. Khromykh - Created
**********************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

#ifndef _HALIRQ_H
#define _HALIRQ_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <AT91SAM7X256.h>
#include <halTaskManager.h>
#include <irq.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#if defined(PLATFORM_SAM7X_EK_RF2XX) || defined(PLATFORM_CUSTOM_1) || defined(PLATFORM_CUSTOM_2)
  #define IRQ0   AT91C_ID_FIQ   // Valid for use
  #define IRQ1   AT91C_ID_IRQ1  // Valid for use
#elif defined(PLATFORM_ZIGBIT_LAN) || defined(PLATFORM_ZIGBIT_TURBO)
  #define IRQ0   AT91C_ID_IRQ0  // Valid for use
  #define IRQ1   AT91C_ID_IRQ1  // Valid for use
#endif

// interrupt activation condition.
// The low level generates an interrupt request.
#define LOW_LEVEL       AT91C_AIC_SRCTYPE_EXT_LOW_LEVEL
// Any edge generates an interrupt request.
#define HIGH_LEVEL      AT91C_AIC_SRCTYPE_HIGH_LEVEL
// Falling edge generates an interrupt request.
#define FALLING_EDGE    AT91C_AIC_SRCTYPE_EXT_NEGATIVE_EDGE
// Rising edge generates an interrupt request.
#define RISING_EDGE     AT91C_AIC_SRCTYPE_POSITIVE_EDGE

/* number valid irq */
#define HAL_NUM_IRQ_LINES 2
/* number of first valid irq */
#define HAL_FIRST_VALID_IRQ 0ul
/* number of second valid irq */
#define HAL_SECOND_VALID_IRQ 1ul

/******************************************************************************
                   Types section
******************************************************************************/
typedef void (* IrqCallback_t)(void);

/******************************************************************************
                   Static functions prototypes section
******************************************************************************/
/******************************************************************************
Enables external interrupt
Parameters:
  irqNumber - number of external interrupt.
Returns:
  none.
******************************************************************************/
INLINE void halEnableIrqInterrupt(uint32_t irqNumber)
{
  /* Enable the interrupt on the interrupt controller */
  AT91C_BASE_AIC->AIC_IECR = (1 << irqNumber);
}

/******************************************************************************
Disables external interrupt
Parameters:
  irqNumber - number of external interrupt.
Returns:
  none.
******************************************************************************/
INLINE void halDisableIrqInterrupt(uint32_t irqNumber)
{
  /* Disable the interrupt on the interrupt controller */
  AT91C_BASE_AIC->AIC_IDCR = (1 << irqNumber);
}

/******************************************************************************
                   Prototypes section
******************************************************************************/
/******************************************************************************
Sets configuration of pins and the registers.
Parameters:
  irqNumber - number of interrupt.
Returns:
  none.
******************************************************************************/
void halSetIrqConfig(uint32_t irqNumber, HAL_IrqMode_t irqMode);

/******************************************************************************
Clears configuration of pins and the registers.
Parameters:
  irqNumber - number of interrupt.
Returns:
  none.
******************************************************************************/
void halClrIrqConfig(uint32_t irqNumber);

#endif /* _HALIRQ_H */
//eof halirq.h

