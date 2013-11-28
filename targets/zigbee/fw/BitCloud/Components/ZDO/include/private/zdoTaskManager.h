/**************************************************************************//**
  \file zdoTaskManager.h

  \brief The header file describes the constants, types and internal interface
  of ZDO task manager

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
******************************************************************************/

#ifndef _ZDOTASKMANAGER_H
#define _ZDOTASKMANAGER_H

/******************************************************************************
                             Includes section
******************************************************************************/
#include <types.h>

/******************************************************************************
                            Definitions section
******************************************************************************/

/******************************************************************************
                                Types section
******************************************************************************/
typedef enum _ZdoTaskId_t
{
  ZDO_SECURITYMANAGER_TASK_ID,
  ZDO_ZDP_FRAME_PROCESSOR_TASK_ID,
  ZDO_ZDP_MANAGER_TASK_ID,
  ZDO_POWER_MANAGER_TASK_ID,
  ZDO_START_NETWORK_TASK_ID,
#ifdef _TC_SWAPOUT_
  ZDO_TC_SWAPOUT_TASK_ID,
#endif // _TC_SWAPOUT_
  ZDO_INIT_TASK_ID,
  ZDO_LAST_TASK_ID // Not a real task
} ZdoTaskId_t;

// Mask of posted ZDO tasks
typedef uint16_t ZdoTaskBitMask_t;

typedef void (*ZdoTaskHandler_t)(void);

/******************************************************************************
                            Prototypes section
******************************************************************************/

/******************************************************************************
  \brief Post ZDO subtask
******************************************************************************/
void zdoPostTask(ZdoTaskId_t task);

/******************************************************************************
  \brief Remove ZDO subtask from list of posted tasks
******************************************************************************/
void zdoUnpostTask(ZdoTaskId_t task);

#endif // _ZDOTASKMANAGER_H_
