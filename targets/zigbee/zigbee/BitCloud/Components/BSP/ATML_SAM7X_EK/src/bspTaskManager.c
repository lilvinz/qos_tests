/***************************************************************************//**
\file  bspTaskManager.c

\brief Implemenattion of BSP task manager.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    31/10/07 A. Khromykh - Created
*******************************************************************************/

/******************************************************************************
                   Includes section
******************************************************************************/
#include <bspTaskManager.h>
#include <joystick.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/

/******************************************************************************
                   Global variables section
******************************************************************************/
volatile uint8_t bspTaskFlags = 0;

/******************************************************************************
                   Prototypes section
******************************************************************************/
/**************************************************************************//**
\brief BSP joystick handler.
******************************************************************************/
void bspJoystickHandler(void);

/**************************************************************************//**
\brief BSP temperature sensor handler.
******************************************************************************/
void bspTemperatureSensorHandler(void);

/**************************************************************************//**
\brief BSP light sensor handler.
******************************************************************************/
void bspLightSensorHandler(void);

/**************************************************************************//**
\brief BSP battery handler.
******************************************************************************/
void bspEmptyBatteryHandler(void);

/******************************************************************************
                   Implementations section
******************************************************************************/
/**************************************************************************//**
\brief BSP task handler.
******************************************************************************/
void BSP_TaskHandler(void)
{
#if APP_DISABLE_BSP != 1
  if (bspTaskFlags & BSP_JOYSTICK)
  {
    bspTaskFlags &= ~BSP_JOYSTICK;
    bspJoystickHandler();
  }
#endif //APP_DISABLE_BSP != 1
  if (bspTaskFlags & BSP_TEMPERATURE)
  {
    bspTaskFlags &= (~BSP_TEMPERATURE);
    bspTemperatureSensorHandler();
  }
  if (bspTaskFlags & BSP_LIGHT)
  {
    bspTaskFlags &= (~BSP_LIGHT);
    bspLightSensorHandler();
  }
  if (bspTaskFlags & BSP_BATTERY)
  {
    bspTaskFlags &= (~BSP_BATTERY);
    bspEmptyBatteryHandler();
  }
  if (bspTaskFlags)
  {
    SYS_PostTask(BSP_TASK_ID);
  }
}

// eof bspTaskManager.c
