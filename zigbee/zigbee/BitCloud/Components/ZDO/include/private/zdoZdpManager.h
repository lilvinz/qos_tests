/************************************************************************//**
  \file zdoZdpManager.h

  \brief The header file describes the constants, types and internal interface
  of ZDP manager. This component manages ZDP requests and responses.

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
******************************************************************************/

#ifndef _ZDOZDPMANAGER_H
#define _ZDOZDPMANAGER_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <queue.h>
#include <appTimer.h>
#include <zdoZdpFrameProcessor.h>

/******************************************************************************
                   Definitions section
******************************************************************************/

/******************************************************************************
                   Types section
******************************************************************************/
typedef enum _ZDO_ZdpRequestState_t
{
  ZDO_IDLE_ZDP_REQUEST_STATE,
  ZDO_SEND_ZDP_REQUEST_STATE,
  ZDO_WAIT_RESP_CONF_ZDP_REQUEST_STATE,
  ZDO_WAIT_CONF_ZDP_REQUEST_STATE,
  ZDO_WAIT_RESP_ZDP_REQUEST_STATE,
  ZDO_CONFIRM_MDR_ZDP_REQUEST_STATE,
  ZDO_CONFIRM_MDR1_ZDP_REQUEST_STATE,
  ZDO_CONFIRM_ZDP_REQUEST_STATE,
  ZDO_WAIT_CONFIRM_UPDATE_ZDP_REQUEST_STATE
} ZDO_ZdpRequestState_t;

typedef struct _ZdoZdpManager_t
{
  QueueDescriptor_t requestQueue;
  HAL_AppTimer_t    ttlUpdateTimer;
  ZDO_ZdpFrame_t    zdpFrame;
} ZdoZdpManager_t;


/******************************************************************************
                   Prototypes section
******************************************************************************/
void zdoZdpResetReq(void);

void zdoZdpManagerHandler(void);

void zdoInitZdpRequestTTL(ZDO_ZdpReq_t *const zdpReq, const uint32_t ttl);

#endif // _ZDOZDPMANAGER_H_
