/************************************************************************//**
  \file zdoNwkManager.h

  \brief The header file describes the constants, types and internal interface
  of ZDO network manager

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
    07/06/07 I. Kalganova - Modified
******************************************************************************/

#ifndef _ZDONWKMANAGER_H
#define _ZDONWKMANAGER_H

/******************************************************************************
                    Includes section
******************************************************************************/
#include <macAddr.h>
#include <nwk.h>
#include <zdo.h>
#include <zdoTaskManager.h>
#include <aps.h>

/******************************************************************************
                    Types section
******************************************************************************/
/********************************************************************************//**
\brief States of ZDO NWKUpdateState functionality
***********************************************************************************/
typedef enum
{ 
  ZDO_IDLE_NWK_UPDATE_STATE, 
  ZDO_INTERNAL_NWK_UPDATE_STATE,
  ZDO_EXTERNAL_NWK_UPDATE_STATE 
} ZdoNWKUpdateState_t;

typedef struct _ZdoNwkManager_t
{
  uint8_t               statusIndCounter: 4;
  bool                  resendAnnce: 1;
  bool                  zdpDataReqBusy: 1;
  bool                  updateDeviceReqBusy: 1;
  bool                  updateDeviceReqPending: 1;
  ZDO_ZdpReq_t          zdpDataReq;
  void                  (*deviceAnnceConf)(ZDO_ZdpResp_t *zdpResp);
  void                  (*resendAnnceConf)(ZDO_ZdpResp_t *zdpResp);
  MAC_SetReq_t          macSetReq;
  ZDO_SetTxPowerReq_t   *txPowerReq;
#ifdef _SECURITY_
  APS_UpdateDeviceReq_t apsUpdateDeviceReq;
#endif //_SECURITY_
  uint16_t              totalTransactions;
  uint16_t              failTransactions;
  bool                  badLink;
  HAL_AppTimer_t        updateNwkWaitTimer;
  ZdoNWKUpdateState_t   updateState; 
} ZdoNwkManager_t;

/******************************************************************************
                    Prototypes section
******************************************************************************/
void zdoNwkManagerReset(void);
void zdoSendDeviceAnnce(void (*deviceAnnceConf)(ZDO_ZdpResp_t *zdpResp));

/******************************************************************************
  Check if short address is available for given extended address and send
  appropriate request if not.
  Parameters:
    addr - extended address to check
  Return:
    true - short address already available
    false - short address currently is not available, request was sent
******************************************************************************/
bool zdoDiscoverNwkAddress(const ExtAddr_t *const addr);

#endif // _ZDONWKMANAGER_H

// eof zdoNwkManager.h
