/**************************************************************************//**
  \file nlmePermitJoining.h

  \brief NLME-PERMIT-JOINING interface

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
   History:
    2009-04-12 Max Gekk - Created.
   Last change:
    $Id: nlmePermitJoining.h 17448 2011-06-09 13:53:59Z ataradov $
 ******************************************************************************/
#if !defined _NLME_PERMIT_JOINING_H
#define _NLME_PERMIT_JOINING_H

/******************************************************************************
                                Includes section
 ******************************************************************************/
#include <types.h>
#include <nwkCommon.h>

/******************************************************************************
                                 Types section
 ******************************************************************************/
/**//**
 * \brief NLME-PERMIT-JOINING confirm primitive's parameters structure.
 * ZigBee Specification r17, 3.2.2.6 NLME-PERMIT-JOINING.confirm, page 276.
 */
typedef struct _NWK_PermitJoiningConf_t
{
  /** The status of the corresponding request. Valid Range: NWK_SUCCESS_STATUS
   * or any status returned from the MLME-SET.confirm primitive. */
  NWK_Status_t status;
} NWK_PermitJoiningConf_t;

/**//**
 * \brief NLME-PERMIT-JOINING request primitive's parameters structure.
 * Zigbee Specification r17, 3.2.2.5 NLME-PERMIT-JOINING.request, page 274.
 */
typedef struct _NWK_PermitJoiningReq_t
{
 /** Service fields - for internal needs. */
  struct
  {
    QueueElement_t qelem; /**< link used for queuing */
  } service;
  /** The length of time in seconds during which the ZigBee coordinator or
   * router will allow associations. The value 0x00 and 0xff indicate that
   * permission is disabled or enabled, respectively,
   * without a specified time limit. */
  uint8_t permitDuration;
  /** NLME-PERMIT-JOINING confirm callback function's pointer. */
  void (*NWK_PermitJoiningConf)(NWK_PermitJoiningConf_t *conf);
  NWK_PermitJoiningConf_t confirm;
} NWK_PermitJoiningReq_t;

/******************************************************************************
                               Prototypes section
 ******************************************************************************/
/**************************************************************************//**
  \brief NLME-PERMIT-JOINING request primitive's prototype.

  \param[in] req - NLME-PERMIT-JOINING request parameters' structure pointer.
  \return None.
 ******************************************************************************/
void NWK_PermitJoiningReq(NWK_PermitJoiningReq_t *const req);

#endif /* _NLME_PERMIT_JOINING_H */
/** eof nlmePermitJoining.h */

