/**************************************************************************//**
  \file tcDeviceTableCtrlHandler.h

  \brief Security Trust Centre devices' permission table control header file.

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      2008.03 - ALuzhetsky created.
******************************************************************************/
#ifndef _TCDEVICETABLECTRLHANDLERH
#define _TCDEVICETABLECTRLHANDLERH

/******************************************************************************
                             Includes section.
******************************************************************************/
#include <types.h>

#ifndef _HIGH_SECURITY_
/******************************************************************************
                             Functions prototypes section.
******************************************************************************/
/**************************************************************************//**
  \brief Initializes Trust Centre device permission table.

  \param none.
  \return none.
******************************************************************************/
#ifdef _TC_PERMISSION_TABLE_
  void tcResetDevicePermissionTable(void);
#endif // _TC_PERMISSION_TABLE_

/**************************************************************************//**
  \brief Finds device in Trust Centre device permission table.

  \param none.
  \return true if device presents, false - otherwise.
******************************************************************************/
#ifdef _TC_PERMISSION_TABLE_
  bool tcFindDeviceInPermissionTable(ExtAddr_t *extAddr);
#endif // _TC_PERMISSION_TABLE_

#else // _HIGH_SECURITY_
#endif // _HIGH_SECURITY_

#endif // _TCDEVICETABLECTRLHANDLERH
// eof tcDeviceTableCtrlHandler.h

