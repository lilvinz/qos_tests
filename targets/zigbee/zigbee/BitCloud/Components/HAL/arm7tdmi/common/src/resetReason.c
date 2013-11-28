/**************************************************************************//**
\file  resetReason.c

\brief Implementation of the reset reason interface.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    29/05/07 E. Ivanov - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                   Includes section
******************************************************************************/
#include <resetReason.h>
#include <halWdt.h>

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
Returns the reset reason.
Parameters:
  none.
Returns:
  The reason of reset.
******************************************************************************/
HAL_ResetReason_t HAL_ReadResetReason(void)
{
  return (HAL_ResetReason_t)halReadResetReason();
}

/******************************************************************************
Software reset.
******************************************************************************/
void HAL_WarmReset(void)
{
  halWarmReset();
}
//eof resetReason.c
