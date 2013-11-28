/**************************************************************************//**
  \file zdoResetNetwork.h

  \brief Interface for ZDO Reset Network request routine

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
  History:
    17.09.09 A. Taradov - Created
******************************************************************************/
#ifndef _ZDORESETNETWORK_H
#define _ZDORESETNETWORK_H

/******************************************************************************
                        Includes section
******************************************************************************/
#include <zdo.h>
#include <queue.h>
#include <nwk.h>

/******************************************************************************
                        Definitions section
******************************************************************************/

/******************************************************************************
                        Types section
******************************************************************************/
typedef enum _ZdoResetNetworkState_t
{
  ZDO_RESET_NETWORK_IDLE_STATE,
  ZDO_RESET_NETWORK_APS_RESET_STATE,
} ZdoResetNetworkState_t;

typedef struct _ZdoResetNetwork_t
{
  ZdoResetNetworkState_t state;
  ZDO_ResetNetworkReq_t *req;
} ZdoResetNetwork_t;

/******************************************************************************
                        Prototypes section
******************************************************************************/

/******************************************************************************
  Reset Reset Network component.
  Parameters:
    none
  Return:
    none
******************************************************************************/
void zdoResetNetworkReset(void);

#endif // _ZDORESETNETWORK_H

// eof zdoResetNetwork.h
