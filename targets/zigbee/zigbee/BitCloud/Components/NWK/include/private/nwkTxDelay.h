/**************************************************************************//**
  \file nwkTxDelay.h

  \brief Interface of transmission delays.

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
   History:
    2009-09-20 Max Gekk - Created.
   Last change:
    $Id: nwkTxDelay.h 17595 2011-06-16 15:55:27Z mgekk $
 ******************************************************************************/
#if !defined _NWK_TX_DELAY_H
#define _NWK_TX_DELAY_H

/******************************************************************************
                                Includes section
 ******************************************************************************/
#include <nwkConfig.h>
#include <appTimer.h>
#include <queue.h>
#include <nldeData.h>

/******************************************************************************
                                 Types section
 ******************************************************************************/
/** Further behavior after delay. */
typedef enum _NwkTxDelayStatus_t
{
  NWK_TX_NOW = 0x0,             /**< Immediately to send. */
  NWK_TX_DONE = 0x1,            /**< To complete transmission. */
  NWK_TX_DELAY_REQUIRED = 0x2,  /**< Delay before transmission. */
  NWK_TX_INDIRECT = 0x3,        /**< Indirect transmission to all children. */
  NWK_TX_PASSIVE_ACK_DONE = 0x4 /**< Check incoming packets from all neighbors. */
} NwkTxDelayStatus_t;

/** Internal variables of delay component. */
typedef struct _NwkTxDelay_t
{
  bool isTimerStarted;
  HAL_AppTimer_t timer;
  QueueDescriptor_t queue;
} NwkTxDelay_t;

/******************************************************************************
                              Prototypes section
 ******************************************************************************/
/**************************************************************************//**
  \brief Transmission delay request.

  \param[in] delayReq - pointer to delay request's parameters.
  \param[in] macStatus - pointer to MAC status of MLDE-DATA.confirm or NULL if
    it is first transmission.
  \return further behavior.
 ******************************************************************************/
NWK_PRIVATE NwkTxDelayStatus_t nwkTxDelayReq(NwkTxDelayReq_t *const delayReq,
  MAC_Status_t *const macStatus);

/**************************************************************************//**
  \brief The transmission delay has expired.

  \param[in] delayReq - pointer to delay request's parameters.
  \param[in] status - status of further behavior.
  \return None.
 ******************************************************************************/
NWK_PRIVATE void nwkTxDelayConf(NwkTxDelayReq_t *const delayReq,
  const NwkTxDelayStatus_t status);

/**************************************************************************//**
  \brief Flush all delays.
 ******************************************************************************/
NWK_PRIVATE void nwkFlushTxDelays(void);

/**************************************************************************//**
  \brief Reset delay component.
 ******************************************************************************/
NWK_PRIVATE void nwkResetTxDelays(void);

/******************************************************************************
  \brief nwkTxDelay idle checking.

  \return true, if nwkTxDelay performs no activity, false - otherwise.
 ******************************************************************************/
NWK_PRIVATE bool nwkTxDelayIsIdle(void);

/**************************************************************************//**
  \brief Delivery time of broadcast data transmission on a network.

  \return Current value of broadcast delivery time in milliseconds.
 ******************************************************************************/
NWK_PRIVATE uint32_t nwkGetBroadcastDeliveryTime(void);

#endif /* _NWK_TX_DELAY_H */
/** eof nwkTxDelay.h */

