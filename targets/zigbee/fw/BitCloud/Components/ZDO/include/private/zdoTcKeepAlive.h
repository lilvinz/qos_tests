/************************************************************************//**
  \file zdoTcKeepAlive.h

  \brief
    The TC keep-alive mechanism interface.

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
    20.01.10 A. Taradov - Created.
******************************************************************************/
#ifndef _ZDO_TC_KEEP_ALIVE_H
#define _ZDO_TC_KEEP_ALIVE_H

#ifdef _TC_SWAPOUT_

/******************************************************************************
                   Includes section
******************************************************************************/
#include <zdo.h>
#include <zcl.h>

/******************************************************************************
                        Types section
******************************************************************************/
typedef enum _ZdoKeepAliveState_t
{
  ZDO_TC_KEEP_ALIVE_STARTED_STATE,
  ZDO_TC_KEEP_ALIVE_STOPPED_STATE,
} ZdoTcKeepAliveState_t;

typedef enum _ZdoKeepAliveReqState_t
{
  ZDO_TC_KEEP_ALIVE_REQ_FREE_STATE,
  ZDO_TC_KEEP_ALIVE_REQ_BUSY_STATE,
} ZdoTcKeepAliveReqState_t;

typedef struct _ZdoTcKeepAlive_t
{
  ZdoTcKeepAliveState_t    state;
  ZdoTcKeepAliveReqState_t reqState;
  ZCL_Request_t            req;
  HAL_AppTimer_t           timer;
  uint8_t                  pingsFailed;
  Endpoint_t               localEndpoint;
  Endpoint_t               remoteEndpoint;
} ZdoTcKeepAlive_t;

/******************************************************************************
                        Prototypes section
******************************************************************************/

/******************************************************************************
  Reset TC keep-alive module
  Parameters:
    none
  Return:
    none
******************************************************************************/
void zdoTcKeepAliveReset(void);

#endif // _TC_SWAPOUT_

#endif // _ZDO_TC_KEEP_ALIVE_H

// eof zdoTcKeepAlive.h
