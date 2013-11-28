/**************************************************************************//**
\file  bspTaslManager.h

\brief Declarations of enums and functions of BSP task manager.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    31/10/07 A. Khromykh - Created
*******************************************************************************/

#ifndef _BSPTASKHANDLER_H
#define _BSPTASKHANDLER_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <inttypes.h>
#include <taskManager.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
enum
{
  BSP_JOYSTICK     = (uint8_t)1 << 0,
  BSP_TEMPERATURE  = (uint8_t)1 << 1,
  BSP_LIGHT        = (uint8_t)1 << 2,
  BSP_BATTERY      = (uint8_t)1 << 3
};

/******************************************************************************
                   External global variables section
******************************************************************************/
extern volatile uint8_t bspTaskFlags;

/******************************************************************************
                   Inline static functions section
******************************************************************************/
/***************************************************************************//**
\brief posting bsp task for task manager.
\param[in]
  flag - task number
*******************************************************************************/
INLINE void bspPostTask(uint8_t flag)
{
  bspTaskFlags |= flag;
  SYS_PostTask(BSP_TASK_ID);
}

#endif /* _BSPTASKHANDLER_H */
// eof bspTaskManager.h
