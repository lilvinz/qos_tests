/**************************************************************************//**
\file   isdImageStorage.h

\brief  The public API of image storage driver.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    25.05.11 A. Khromykh - Created
*******************************************************************************/

#ifndef _ISDIMAGESTORAGE_H
#define _ISDIMAGESTORAGE_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <types.h>
#include <zclOTAUCluster.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/

/******************************************************************************
                   Types section
******************************************************************************/
/** \brief Status which is returned by driver */
typedef enum
{
  ISD_SUCCESS,
  ISD_HARDWARE_FAULT,
  ISD_COMMUNICATION_LOST
} ISD_Status_t;

typedef void (* IsdOpenCb_t)(ISD_Status_t);
typedef void (* IsdQueryNextImageCb_t)(ZCL_OtauQueryNextImageResp_t *);
typedef void (* IsdImageBlockCb_t)(ZCL_OtauImageBlockResp_t *);
typedef void (* IsdUpgradeEndCb_t)(ZCL_OtauUpgradeEndResp_t *);

/******************************************************************************
                   Prototypes section
******************************************************************************/
/**************************************************************************//**
\brief Open image storage driver

\param[in] cb - callback about driver actions
******************************************************************************/
void ISD_Open(IsdOpenCb_t cb);

/**************************************************************************//**
\brief Close image storage driver
******************************************************************************/
void ISD_Close(void);

/**************************************************************************//**
\brief Send query next image request to storage system

\param[in] addressing - pointer to structure that include client network information; \n
\param[in] data - data payload; \n
\param[in] cd - callback about response receiving from storage system.
******************************************************************************/
void ISD_QueryNextImageReq(ZCL_Addressing_t *addressing, ZCL_OtauQueryNextImageReq_t *data, IsdQueryNextImageCb_t cb);

/**************************************************************************//**
\brief Send image block request to storage system

\param[in] addressing - pointer to structure that include client network information; \n
\param[in] data - data payload; \n
\param[in] cd - callback about response receiving from storage system.
******************************************************************************/
void ISD_ImageBlockReq(ZCL_Addressing_t *addressing, ZCL_OtauImageBlockReq_t *data, IsdImageBlockCb_t cb);

/**************************************************************************//**
\brief Send upgrade end request to storage system

\param[in] addressing - pointer to structure that include client network information; \n
\param[in] data - data payload; \n
\param[in] cd - callback about response receiving from storage system.
******************************************************************************/
void ISD_UpgradeEndReq(ZCL_Addressing_t *addressing, ZCL_OtauUpgradeEndReq_t *data, IsdUpgradeEndCb_t cb);

#endif /* _ISDIMAGESTORAGE_H */
