/**************************************************************************//**
  \file sspManager.h

  \brief Security Service Provider header file.

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      06/12/2007 - ALuzhetsky API corrected.
******************************************************************************/
#ifndef _SSPMANAGER_H
#define _SSPMANAGER_H

/******************************************************************************
                             Includes section.
******************************************************************************/
#include <queue.h>

/******************************************************************************
                             Definition section.
******************************************************************************/
#ifdef _SSP_USE_FLASH_FOR_CONST_DATA
  #include <types.h>
  #define HANDLERS_MEM  FLASH_VAR
  #define HANDLERS_GET(A, I) memcpy_P(A, &sspHandlers[I], sizeof(SspTask_t))
#else // _SSP_USE_FLASH_FOR_CONST_DATA
  #define HANDLERS_MEM
  #define HANDLERS_GET(A, I) (((A)->task) = sspHandlers[I].task)
#endif // _SSP_USE_FLASH_FOR_CONST_DATA

#if defined(_MAC_HW_AES_) || defined(_HAL_HW_AES_)
  #ifdef _HIGH_SECURITY_
    #define SSP_TASKS_LIST                      \
      {sspCalculateAuthenticMacTag1ReqHandler}, \
      {sspCalculateAuthenticMacTag2ReqHandler}, \
      {sspCalculateSkkeParamsReqHandler},       \
      {sspKeyedHashReqHandler},                 \
      {sspEncryptFrameReqHandler},              \
      {sspDecryptFrameReqHandler}
  #else // _HIGH_SECURITY_
    #define SSP_TASKS_LIST                      \
      {sspEncryptFrameReqHandler},              \
      {sspDecryptFrameReqHandler}
  #endif // _HIGH_SECURITY_

#elif defined(_SSP_SW_AES_)
  #ifdef _HIGH_SECURITY_
    #define SSP_TASKS_LIST                      \
      {sspCalculateAuthenticMacTag1ReqHandler}, \
      {sspCalculateAuthenticMacTag2ReqHandler}, \
      {sspCalculateSkkeParamsReqHandler},       \
      {sspKeyedHashReqHandler},                 \
      {sspEncryptFrameReqHandler},              \
      {sspDecryptFrameReqHandler},              \
      {sspAesReqHandler},
  #else // _HIGH_SECURITY_
    #define SSP_TASKS_LIST                      \
      {sspEncryptFrameReqHandler},              \
      {sspDecryptFrameReqHandler},              \
      {sspAesReqHandler}
  #endif // _HIGH_SECURITY_

#else
  #error Unknown AES routine provider
#endif

/******************************************************************************
                             Types section.
******************************************************************************/
/**************************************************************************//**
  \brief TBD.

  TBD
******************************************************************************/
typedef struct
{
  void (*task)(void);
} SspTask_t;

/**************************************************************************//**
  \brief TBD.

  TBD
******************************************************************************/
typedef enum
{
#ifdef _HIGH_SECURITY_
  SSP_TASK_AUTHENTIC_MAC_TAG1,
  SSP_TASK_AUTHENTIC_MAC_TAG2,
  SSP_TASK_SKKE,
  SSP_TASK_KEYED_HASH,
#endif // _HIGH_SECURITY_
  SSP_TASK_ENCRYPT_FRAME,
  SSP_TASK_DECRYPT_FRAME,
#ifdef _SSP_SW_AES_
  SSP_TASK_AES,
#endif // _SSP_SW_AES_

  SSP_TASKS_SIZE,
} SspTaskId_t;

/**************************************************************************//**
  \brief TBD.

  TBD
******************************************************************************/
typedef enum  // ssp possible requests' types.
{
#ifdef _HIGH_SECURITY_
  SSP_REQ_ID_AUTHENTIC_MAC_TAG1 = SSP_TASK_AUTHENTIC_MAC_TAG1,
  SSP_REQ_ID_AUTHENTIC_MAC_TAG2 = SSP_TASK_AUTHENTIC_MAC_TAG2,
  SSP_REQ_ID_SKKE               = SSP_TASK_SKKE,
  SSP_REQ_ID_KEYED_HASH         = SSP_TASK_KEYED_HASH,
#endif // _HIGH_SECURITY_
  SSP_REQ_ID_SFP_ENCRYPT_FRAME  = SSP_TASK_ENCRYPT_FRAME,
  SSP_REQ_ID_SFP_DECRYPT_FRAME  = SSP_TASK_DECRYPT_FRAME,

} SspRequestId_t;

/**************************************************************************//**
  \brief TBD.

  TBD
******************************************************************************/
typedef uint8_t SspTaskBitMask_t;

/**************************************************************************//**
  \brief TBD.

  TBD
******************************************************************************/
typedef struct
{
  SspTaskBitMask_t  taskBitMask;
  QueueDescriptor_t reqQueueDescr;
} SspManagerMem_t;

/******************************************************************************
                        Functions prototypes section.
******************************************************************************/
/**************************************************************************//**
  \brief TBD.

  \param TBD.
  \return TBD.
******************************************************************************/
void sspPostTask(SspTaskId_t taskID);

/**************************************************************************//**
  \brief TBD.

  \param TBD.
  \return TBD.
******************************************************************************/
void sspResetTaskManager(void);

/******************************************************************************
 Sends confirmation to the SSP user.
 Parameters:
   request - parameters of the request to be confirmed.
 Returns:
   none.
******************************************************************************/
void sspSendConfToUpperLayer(void *request);

#endif // _SSPMANAGER_H

// eof sspManager.h
