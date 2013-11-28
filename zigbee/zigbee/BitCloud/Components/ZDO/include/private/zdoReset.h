/**************************************************************************//**
  \file zdoReset.h

  \brief Interface for ZDO reset routine

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
  History:
    29.10.09 A. Taradov - Created
******************************************************************************/

#ifndef _ZDORESET_H
#define _ZDORESET_H

/******************************************************************************
                        Definitions section
******************************************************************************/
typedef struct
{
  ZDO_Status_t status;
} ZDO_SoftwareResetConf_t;

typedef struct
{
  ZDO_SoftwareResetConf_t confirm;
  void (*ZDO_SoftwareResetConf)(ZDO_Status_t status);
} ZDO_SoftwareResetReq_t;

typedef struct
{
  void (*stackResetCallback)(ZDO_Status_t status);
} ZdoReset_t;

/******************************************************************************
                   Prototypes section
******************************************************************************/
/**************************************************************************//**
  \brief Stack software reset routine.

  \param[in] req - pointer to request parameters.

  \return None.
 ******************************************************************************/
void ZDO_SoftwareResetReq(ZDO_SoftwareResetReq_t *req);

/******************************************************************************
  \brief ZDO reset routine
******************************************************************************/
void zdoReset(void);

/**************************************************************************//**
  \brief Stack resetting start routine.

  \param[in] powerFailureControl - stack restoring after power failure control bitfield;
                                  affects on initialization procedure.
  \param[out] callback - callback to stack reset finished handler.

  \return None.
 ******************************************************************************/
void zdoStackReset(NWK_PowerFailureControl_t powerFailureControl, void (*callback)(ZDO_Status_t status));

/**************************************************************************//**
  \brief Reset component initialization routine.
 ******************************************************************************/
void zdoInitResetComponent(void);

#endif // _ZDORESET_H

// eof zdoReset.h
