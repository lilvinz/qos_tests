/**************************************************************************//**
\file  irq.c

\brief Implementation of IRQ interface.

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
#include <stddef.h>

/******************************************************************************
                   External global variables section
******************************************************************************/
extern IrqCallback_t IrqCallbackList[HAL_NUM_IRQ_LINES];

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
 Registers user's irqNumber interrupt
 Parameters:
   irqNumber - IRQ number
   irqMode   - Interrupt sence control
   f         - user's interrupt handler.  Handler must be executed less than 100 us.
 Returns:
   -1    - if irqNumber is out of range or such interrupt has
             been already registered.
   0 - otherwise.
******************************************************************************/
int HAL_RegisterIrq(HAL_IrqNumber_t irqNumber, HAL_IrqMode_t irqMode, void (*f)(void))
{
  uint32_t tempNumber;

  // undefined irq
  if ((IRQ_0 != irqNumber) && (IRQ_1 != irqNumber))
    return -1;

  // not valid irq mode
  if (IRQ_ANY_EDGE == irqMode)
    return -1;

  // Such interrupt has been already register
  if (IRQ_0 == irqNumber)
  {
    if (NULL != IrqCallbackList[HAL_FIRST_VALID_IRQ])
      return -1;
    else
      IrqCallbackList[HAL_FIRST_VALID_IRQ] = f;

    tempNumber = IRQ0;
  }

  if (IRQ_1 == irqNumber)
  {
    if (NULL != IrqCallbackList[HAL_SECOND_VALID_IRQ])
      return -1;
    else
      IrqCallbackList[HAL_SECOND_VALID_IRQ] = f;

    tempNumber = IRQ1;
  }

  /* send user data to hwd */
  switch(irqMode)
  {
    case IRQ_LOW_LEVEL:
      halSetIrqConfig(tempNumber, LOW_LEVEL);
    return 0;
    case IRQ_HIGH_LEVEL:
      halSetIrqConfig(tempNumber, HIGH_LEVEL);
    return 0;
    case IRQ_FALLING_EDGE:
      halSetIrqConfig(tempNumber, FALLING_EDGE);
    return 0;
    case IRQ_RISING_EDGE:
      halSetIrqConfig(tempNumber, RISING_EDGE);
    return 0;
    default:
    return -1;
  }
}

/******************************************************************************
 Enables irqNumber interrupt
 Parameters:
   irqNumber - IRQ number
 Returns:
   -1    - if irqNumber is out of range or has not been
             registered yet.
   0 - otherwise.
******************************************************************************/
int HAL_EnableIrq(HAL_IrqNumber_t irqNumber)
{
  // undefined irq
  if ((IRQ_0 != irqNumber) && (IRQ_1 != irqNumber))
    return -1;

  // Interrupt has not been opened yet
  if (IRQ_0 == irqNumber)
  {
    if (NULL == IrqCallbackList[HAL_FIRST_VALID_IRQ])
      return -1;
    else
      halEnableIrqInterrupt(IRQ0);
  }

  if (IRQ_1 == irqNumber)
  {
    if (NULL == IrqCallbackList[HAL_SECOND_VALID_IRQ])
      return -1;
    else
      halEnableIrqInterrupt(IRQ1);
  }

  return 0;
}

/******************************************************************************
 Disables irqNumber interrupt
 Parameters:
   irqNumber - IRQ number
 Returns:
   -1    - if irqNumber is out of range or has not been
             registered yet.
   0 - otherwise.
******************************************************************************/
int HAL_DisableIrq(HAL_IrqNumber_t irqNumber)
{
  // undefined irq
  if ((IRQ_0 != irqNumber) && (IRQ_1 != irqNumber))
    return -1;

  // Interrupt has not been opened yet
  if (IRQ_0 == irqNumber)
  {
    if (NULL == IrqCallbackList[HAL_FIRST_VALID_IRQ])
      return -1;
    else
      halDisableIrqInterrupt(IRQ0);
  }

  if (IRQ_1 == irqNumber)
  {
    if (NULL == IrqCallbackList[HAL_SECOND_VALID_IRQ])
      return -1;
    else
      halDisableIrqInterrupt(IRQ1);
  }

  return 0;
}

/******************************************************************************
 Unregisters user's irqNumber interrupt
 Parameters:
   irqNumber - IRQ number
 Returns:
   -1    - if irqNumber is out of range or has not been
             registered yet.
   0 - otherwise.
******************************************************************************/
int HAL_UnregisterIrq(HAL_IrqNumber_t irqNumber)
{
  // undefined irq
  if ((IRQ_0 != irqNumber) && (IRQ_1 != irqNumber))
    return -1;

  // Interrupt has not been opened yet
  if (IRQ_0 == irqNumber)
  {
    if (NULL == IrqCallbackList[HAL_FIRST_VALID_IRQ])
      return -1;
    else
      IrqCallbackList[HAL_FIRST_VALID_IRQ] = NULL;

    // Disable external interrupt request
    halDisableIrqInterrupt(IRQ0);
    halClrIrqConfig(IRQ0);
  }

  if (IRQ_1 == irqNumber)
  {
    if (NULL == IrqCallbackList[HAL_SECOND_VALID_IRQ])
      return -1;
    else
      IrqCallbackList[HAL_SECOND_VALID_IRQ] = NULL;

    // Disable external interrupt request
    halDisableIrqInterrupt(IRQ1);
    halClrIrqConfig(IRQ1);
  }

  return 0;
}

// eof irq.c
