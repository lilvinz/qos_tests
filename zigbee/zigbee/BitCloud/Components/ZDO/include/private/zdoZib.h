/************************************************************************//**
  \file zdoZib.h

  \brief The header file describes the ZDO information base

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
******************************************************************************/
#ifndef _ZDOZIB_H
#define _ZDOZIB_H

/******************************************************************************
                        Includes section.
******************************************************************************/
#include <appFramework.h>
#include <nwkCommon.h>

/******************************************************************************
                        Definitions section.
******************************************************************************/
#define LUXOFT_MANUFACTURER_CODE        0x100c
#define CT_EPID_MASK                    0xFFFFFFFFFFFF0000 // ???
#define COMMISSIONING_EPID              0x0050C27710000000 // ???
#define NUM_ACTIVE_EP                   1 //FIXME
#define MAX_DISCOVERED_NETWORKS_NUMBER  3
#define NETWORK_SELECT_CRITERIES        (CHOOSENET_EXTPANID | CHOOSENET_STACKPROFILE | CHOOSENET_ZBVERSION)
#define BEACON_ORDER                    0x0f
#define SUPER_FRAME_ORDER               0x0f
#define BATTERY_LIFE_EXTENTION          true

/******************************************************************************
                   Types section
******************************************************************************/
typedef struct
{
  //scannig channel mask
  uint32_t             channelMask;
  //extended PAN ID
  ExtPanId_t           extPanId;
  //short address
  ShortAddr_t          shortAddr;
  //current channel
  uint8_t              channel;
  //parent extended address
  ExtAddr_t            parentExtAddress;
  //parent short address
  ShortAddr_t          parentNetworkAddress;
  //current PAN ID
  PanId_t              panID;
  //node descriptor
  NodeDescriptor_t     nodeDescriptor;
  //node power descriptor
  PowerDescriptor_t    powerDescriptor;
  //simple descriptor
  //SimpleDescriptor_t   configSimpleDescriptors[NUM_ACTIVE_EP];
  //scanning duration in seconds
  uint8_t              scanDuration;
  //number of failure parent sync retries
  uint8_t              parentLinkRetryThreshold;
  //
  uint8_t              permitJoinDuration;
  //ZDP response wait timeout
  uint16_t             zdpResponseTimeout;
} ZIB_t;

extern ZIB_t csZIB;

/******************************************************************************
                        Prototypes section
******************************************************************************/

/******************************************************************************
  Reload ZIB from ConfigServer
  Parameters:
    none
  Return:
    none
******************************************************************************/
void zdoReloadZibFromConfigServer(void);

/******************************************************************************
  Set actual network parameters to ZIB
  Parameters:
    none
  Return:
    none
******************************************************************************/
void setNetworkParametersToZib(void);

#endif // _ZDOZIB_H

// eof zdoZib.h
