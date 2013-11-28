/**************************************************************************//**
  \file zdoBinding.h

  \brief Internal interface of ZDO binding functionality.

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
   History:
    2011-03-17 Max Gekk - Created.
   Last change:
    $Id: zdoBinding.h 17448 2011-06-09 13:53:59Z ataradov $
 ******************************************************************************/
#if !defined _ZDO_BINDING_H
#define _ZDO_BINDING_H

/******************************************************************************
                               Prototypes section
 ******************************************************************************/
/**************************************************************************//**
  \brief Prepare Bind or Unbind ZDP request.
 ******************************************************************************/
void zdoUnbindBindRequestCreate(void);

/**************************************************************************//**
  \brief Process an incoming Bind or Unbind request and send response.
 ******************************************************************************/
void zdoUnbindBindInd(void);

/**************************************************************************//**
  \brief Prepare ZDP Mgmt Bind request. Init APS_DataReq_t structure.
 ******************************************************************************/
void zdoMgmtBindRequestCreate(void);

/**************************************************************************//**
  \brief Process an incoming Mgmt Bind request and send response.
 ******************************************************************************/
void zdoMgmtBindInd(void);

#endif /* _ZDO_BINDING_H */
/** eof zdoBinding.h */
