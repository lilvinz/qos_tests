/************************************************************************//**
  \file zdoZdpFrameProcessor.h

  \brief The header file describes the constants, types and internal interface
  of ZDP parser component. This component sends/receives ZDP commands.

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
    28/11/07 A. Zemlyanov - Created.
******************************************************************************/

#ifndef _ZDP_FRAME_PROCESSOR_H
#define _ZDP_FRAME_PROCESSOR_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <types.h>
#include <queue.h>
#include <zdo.h>
#include <zdoDbg.h>
#include <aps.h>

/******************************************************************************
                   Definitions section
******************************************************************************/

/******************************************************************************
                   Types section
******************************************************************************/
typedef void (*ZdoAPSDataConf_t)(APS_DataConf_t *conf);

typedef enum
{
  ZDO_ZDP_DATA_REQ_FREE_STATE,
  ZDO_ZDP_DATA_REQ_BUSY_STATE,
  ZDO_ZDP_DATA_REQ_SEND_STATE,
  ZDO_ZDP_DATA_REQ_WAIT_CONF_STATE
} ZDO_ZdpDataReqState_t;

typedef enum
{
  ZDO_ZDP_IND_FREE_STATE,
  ZDO_ZDP_IND_BUSY_STATE
} ZDO_ZdpIndState_t;


typedef struct zdoFrameProc_t_
{
  uint8_t               requestSeqNum;
  ZDO_ZdpReq_t          *zdpRequest;                    // Currently processed zdpRequest

  ZDO_ZdpDataReqState_t apsDataReqState;
  APS_DataReq_t         apsDataReq;                     // Common for all requests to APS

  // Incoming data from APS
  ZDO_ZdpIndState_t     apsDataIndState;
  APS_DataInd_t         apsDataInd;
  uint8_t               apsAsduBuff[APS_MAX_ASDU_SIZE]; // Buffer for inconing APS ASDU
} ZdoFrameProc_t;


/******************************************************************************
                   Prototypes section
******************************************************************************/
/******************************************************************************
 Reset the ZDP Frame Processor into initial state
 Parameters:
   None
 Returns:
   None.
******************************************************************************/
void zdoZdpFrameProcessorReset(void);

/******************************************************************************
 Send ZDP request
 Parameters:
   req - pointer to request object
 Returns:
   None.
******************************************************************************/
void zdoZdpSendRequest(ZDO_ZdpReq_t *req);

/******************************************************************************
  TODO
******************************************************************************/
void zdoSetZdpDataReqState(ZDO_ZdpDataReqState_t state);

/******************************************************************************
  TODO
******************************************************************************/
void zdoSetZdpIndState(ZDO_ZdpIndState_t state);

/******************************************************************************
 Process incoming ZDP frames data from APS layer
 Parameters:
   ind - Pointer to APS_DataInd_t type object
 Returns:
   None.
******************************************************************************/
void APS_ZdoDataInd(APS_DataInd_t *ind);

/******************************************************************************
 To be called when gets frame from APS
 Parameters:
   apsInd - Pointer to APS_DataInd_t type object
 Returns:
   None.
******************************************************************************/
void zdoZdpMessageInd(APS_DataInd_t *dataInd);

void zdoZdpFrameProcessorHandler(void);

#endif /* _ZDP_FRAME_PROCESSOR_H */
