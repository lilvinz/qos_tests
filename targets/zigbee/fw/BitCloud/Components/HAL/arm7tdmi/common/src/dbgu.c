/**************************************************************************//**
\file  dbgu.c

\brief Implementation of debug message interface.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    10/11/08 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                   Includes section
******************************************************************************/
#include <dbgPort.h>

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
 Open dbgu port (115200 baud, 8N1 format).
 Parameters:
   none
 Returns:
   none
******************************************************************************/
void HAL_OpenDbgu(void)
{
  configure_dbgu();
}

/******************************************************************************
 Send string trough dbgu port.
 Parameters:
   none
 Returns:
   none
******************************************************************************/
void HAL_SendDbguMessage(const char *buffer)
{
  dbgu_print_ascii(buffer);
}
// eof dbgu.c
