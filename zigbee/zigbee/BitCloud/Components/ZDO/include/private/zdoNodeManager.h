/************************************************************************//**
  \file zdoNodeManager.h

  \brief The header file describes the constants, types and internal interface
  of ZDO node manager

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
******************************************************************************/

#ifndef ZDONODEMANAGER_H_
#define ZDONODEMANAGER_H_

#include <zdoZdpManager.h>
#include <zdoZdpFrameProcessor.h>
#include <zdo.h>
#include <aps.h>
#include <appTimer.h>


// ScanDuration field of Mgmt_NWK_Update_req
#define MGMT_NWK_UPDATE_DUR_0               ((uint8_t)0x00)
#define MGMT_NWK_UPDATE_DUR_1               ((uint8_t)0x01)
#define MGMT_NWK_UPDATE_DUR_2               ((uint8_t)0x02)
#define MGMT_NWK_UPDATE_DUR_3               ((uint8_t)0x03)
#define MGMT_NWK_UPDATE_DUR_4               ((uint8_t)0x04)
#define MGMT_NWK_UPDATE_DUR_5               ((uint8_t)0x05)
#define MGMT_NWK_UPDATE_DUR_CHANNEL_CHANGE  ((uint8_t)0xFE)
#define MGMT_NWK_UPDATE_DUR_PARAMS_CHANGE   ((uint8_t)0xFF)


typedef struct zdoNodeManager_t_
{
  NWK_EDScanReq_t         nwkEDScanReq;
  MAC_SetReq_t            macSetReq;
  HAL_AppTimer_t          zdpNodeWaitTimer;
  uint8_t                 edScanCount;
  uint8_t                 newChannel;
  uint8_t                 nwkUpdateId;
  NWK_EDScanConf_t        *nwkEDScanConf;
  ZDO_ZdpReq_t            zdpNwkUpdateNotify;
  ZDO_ZdpReq_t            *storedZdpRequest;
} ZdoNodeManager_t;

void zdoZdpNodeManagerReset(void);
void zdoExecuteEDScan(uint32_t scanChannels, uint8_t scanDuration);
void zdoZdpNodeRequestCreate(void);
void zdoZdpNodeMessageInd(void);
void zdpNodeWaitTimerFired(void);

/******************************************************************************
 Set timer on interval BROADCAST_DELIVERY_TIME upon fires which node will 
   change the channel 
 Parameters:
   scanChannels - channels to move to
 Returns:
   None.
******************************************************************************/
void zdoSetChannelChangeTimer(const uint32_t scanChannels);

#endif /*ZDONODEMANAGER_H_*/
