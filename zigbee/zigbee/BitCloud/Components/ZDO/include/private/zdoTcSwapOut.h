/**************************************************************************//**
  \file zdoTcSwapOut.h

  \brief Interface to TC Swap-out implementation

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
  History:
    25.01.10 A. Taradov - Created
******************************************************************************/

#ifndef _ZDOTCSWAPOUT_H
#define _ZDOTCSWAPOUT_H

#ifdef _TC_SWAPOUT_

/******************************************************************************
                        Includes section
******************************************************************************/
#include <zdo.h>
#include <aps.h>
#include <queue.h>
#include <nwk.h>
#include <zclSecurityManager.h>

/******************************************************************************
                        Definitions section
******************************************************************************/

/******************************************************************************
                        Types section
******************************************************************************/
typedef enum _ZdoTcSwapoutState_t
{
  ZDO_TC_SWAPOUT_IDLE_STATE,
  ZDO_TC_SWAPOUT_BEGIN_STATE,
  ZDO_TC_SWAPOUT_LEAVE_BEFORE_REJOIN_STATE,
  ZDO_TC_SWAPOUT_REJOIN_STATE,
  ZDO_TC_SWAPOUT_LEAVE_BEFORE_CONFIRM_STATE,
  ZDO_TC_SWAPOUT_CONFIRM_STATE
} ZdoTcSwapoutState_t;

typedef struct _ZdoTcSwapOut_t
{
  QueueDescriptor_t      queue;
  ZdoTcSwapoutState_t    state;
  APS_Status_t           keyWasSecuredWith;
  ZCL_StartSecurityReq_t zclStartSecurityReq;
} ZdoTcSwapOut_t;

/******************************************************************************
                        Prototypes section
******************************************************************************/

/******************************************************************************
  Indication of Transport Key command receipt
  Parameters:
    ind - indication parameters
  Return:
    none
******************************************************************************/
void zdoTcSwapOutTransportKeyInd(APS_TransportKeyInd_t *ind);

/******************************************************************************
  TC Swap-out request task handler.
  Parameters:
    none
  Return:
    none
******************************************************************************/
void zdoTcSwapOutTaskHandler(void);

/******************************************************************************
  Reset Trust Centre Swap-out component.
  Parameters:
    none
  Return:
    none
******************************************************************************/
void zdoTcSwapOutReset(void);

#endif // _TC_SWAPOUT_

#endif // _ZDOTCSWAPOUT_H

// eof zdoTcSwapOut.h
