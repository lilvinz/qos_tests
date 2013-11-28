/**************************************************************************//**
  \file zdoStartNetwork.h

  \brief Interface for ZDO Start Network request routines

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
  History:
    17.09.09 A. Taradov - Created
******************************************************************************/
#ifndef _ZDOSTARTNETWORK_H
#define _ZDOSTARTNETWORK_H

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
typedef enum _ZdoStartNetworkState_t
{
  ZDO_START_NETWORK_IDLE_STATE,
  ZDO_START_NETWORK_BEGIN_STATE,
  ZDO_START_NETWORK_APS_RESET_STATE,
  ZDO_START_NETWORK_FORMATION_STATE,
  ZDO_START_NETWORK_JOIN_STATE,
  ZDO_START_NETWORK_REJOIN_STATE,
  ZDO_START_NETWORK_JOINED_STATE,
  ZDO_START_NETWORK_START_ROUTER_STATE,
  ZDO_START_NETWORK_PERMIT_JOIN_STATE,
  ZDO_START_NETWORK_APS_START_STATE,
  ZDO_START_NETWORK_ANNCE_STATE,
  ZDO_START_NETWORK_CONFIRM_STATE
} ZdoStartNetworkState_t;

typedef void (ZdoStartNetworkConf_t)(ZDO_Status_t status);

typedef struct _ZdoStartNetwork_t
{
  QueueDescriptor_t        queue;
  ZdoStartNetworkState_t   state;
  NWK_RejoinNetwork_t      startupType;
  ZdoStartNetworkConf_t    *confirm;
  NWK_PowerFailureControl_t powerFailureControl;
#if defined(_ROUTER_) || defined(_ENDDEVICE_)
  uint8_t                  joinAttempts;
  HAL_AppTimer_t           joinTimer;
#endif
} ZdoStartNetwork_t;

/******************************************************************************
                        Prototypes section
******************************************************************************/

/******************************************************************************
  Reset Start Network component.
  Parameters:
    none
  Return:
    none
******************************************************************************/
void zdoStartNetworkReset(void);

/******************************************************************************
  Start network rejoin
  Parameters:
    none
  Return:
    none
******************************************************************************/
void zdoRejoinNetwork(void);

/******************************************************************************
  Start Network request task handler.
  Parameters:
    none
  Return:
    none
******************************************************************************/
void zdoStartNetworkTaskHandler(void);

/******************************************************************************
  Allocate Start Network module
  Parameters:
    startNetworkConf - confirmation callback
  Return:
    true - if module was alocated, false otherwise
******************************************************************************/
bool zdoStartNetworkAlloc(ZdoStartNetworkConf_t *startNetworkConf);

/******************************************************************************
  Free Start Network module
  Parameters:
    none
  Return:
    none
******************************************************************************/
void zdoStartNetworkFree(void);

#ifdef _COORDINATOR_
/******************************************************************************
  NLME-NETWORK-FORMATION.request preparing and sending.
  Parameters:
    none
  Return:
    none
******************************************************************************/
void zdoNetworkFormationReq(void);
#endif /* _COORDINATOR_ */

#endif // _ZDOSTARTNETWORK_H

// eof zdoStartNetwork.h
