/************************************************************************//**
  \file zdoMem.h

  \brief The header file describes the internal ZDO memory

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
******************************************************************************/

#ifndef _ZDOMEM_H
#define _ZDOMEM_H

#include <zdo.h>
#include <zdoNodeManager.h>
#include <zdoNwkManager.h>
#include <zdoPowerManager.h>
#include <zdoStartNetwork.h>
#include <zdoResetNetwork.h>
#include <zdoLeaveNetwork.h>
#include <zdoZdpManager.h>
#include <zdoZdpFrameProcessor.h>
#include <zdoSecurityManager.h>
#include <zdoTcSwapOut.h>
#include <zdoTcKeepAlive.h>
#include <zdoReset.h>
#include <zdoEndDeviceBinding.h>

typedef union _ZdoStackReq_t
{
  // Requests from ZDO to NLME
#if defined(_COORDINATOR_)
  NWK_NetworkFormationReq_t nwkFormationReq;
#endif

#if defined(_ROUTER_) || defined(_ENDDEVICE_)
  NWK_JoinReq_t             nwkJoinReq;
#endif

#if defined(_COORDINATOR_) || defined(_ROUTER_)
  NWK_PermitJoiningReq_t    nwkPermitJoiningReq;
#endif

#if defined(_ROUTER_)
  NWK_StartRouterReq_t      nwkStartRouterReq;
#endif

  NWK_LeaveReq_t            nwkLeaveReq;

  // Requests from ZDO to APSME
  APS_ResetReq_t            apsResetReq;
  APS_StartReq_t            apsStartReq;
  APS_StopReq_t             apsStopReq;
} ZdoStackReq_t;

typedef struct _ZdoMem_t
{
  ZdoStackReq_t             stackReq;
  ZdoStartNetwork_t         zdoStartNetwork;
  ZdoLeaveNetwork_t         zdoLeaveNetwork;
  ZdoReset_t                zdoReset;
  ZdoResetNetwork_t         zdoResetNetwork;
  ZdoPowerManager_t         powerManager;
  ZdoNodeManager_t          nodeManager;
  ZdoNwkManager_t           nwkManager;
  ZdoZdpManager_t           zdoZdpManager;
  ZdoFrameProc_t            zdpFrameProcessor;
#ifdef _SECURITY_
  ZdoSecurityManager_t      zdoSecurityManager;
#endif
#ifdef _TC_SWAPOUT_
  ZdoTcSwapOut_t            zdoTcSwapOut;
  ZdoTcKeepAlive_t          zdoTcKeepAlive;
#endif
  uint16_t                  enabledModules;
  RF_RandomReq_t            randReq;
  HAL_AppTimer_t            timerReq;
#ifdef _END_DEVICE_BINDING_
  ZdoEndDeviceBinding_t     endDevBinding;
#endif
} ZdoMem_t;

extern ZdoMem_t zdoMem;

static inline ZdoStackReq_t * zdoMemStackRequest(void)
{
  return &zdoMem.stackReq;
}

static inline HAL_AppTimer_t * zdoMemTimerRequest(void)
{
  return &zdoMem.timerReq;
}

static inline RF_RandomReq_t * zdoMemRandRequest(void)
{
  return &zdoMem.randReq;
}

static inline ZdoPowerManager_t * zdoMemPowerManager(void)
{
  return &zdoMem.powerManager;
}

static inline ZdoNodeManager_t * zdoMemNodeManager(void)
{
  return &zdoMem.nodeManager;
}

static inline ZdoNwkManager_t * zdoMemNwkManager(void)
{
  return &zdoMem.nwkManager;
}

static inline ZdoZdpManager_t * zdoMemZdpManager(void)
{
  return &zdoMem.zdoZdpManager;
}

static inline ZdoFrameProc_t * zdoMemZdpFrameProcessor(void)
{
  return &zdoMem.zdpFrameProcessor;
}

static inline ZdoStartNetwork_t * zdoMemStartNetwork(void)
{
  return &zdoMem.zdoStartNetwork;
}

static inline ZdoResetNetwork_t * zdoMemResetNetwork(void)
{
  return &zdoMem.zdoResetNetwork;
}

static inline ZdoReset_t * zdoMemReset(void)
{
  return &zdoMem.zdoReset;
}

static inline ZdoLeaveNetwork_t * zdoMemLeaveNetwork(void)
{
  return &zdoMem.zdoLeaveNetwork;
}

#ifdef _SECURITY_
static inline ZdoSecurityManager_t * zdoMemSecurityManager(void)
{
  return &zdoMem.zdoSecurityManager;
}
#endif

#ifdef _TC_SWAPOUT_
static inline ZdoTcSwapOut_t * zdoMemTcSwapOut(void)
{
  return &zdoMem.zdoTcSwapOut;
}

static inline ZdoTcKeepAlive_t * zdoMemTcKeepAlive(void)
{
  return &zdoMem.zdoTcKeepAlive;
}
#endif

#ifdef _END_DEVICE_BINDING_
static inline ZdoEndDeviceBinding_t * zdoMemEndDeviceBinding(void)
{
  return &zdoMem.endDevBinding;
}
#endif
#endif // _ZDOMEM_H

