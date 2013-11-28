/****************************************************************************//**
  \file vcpCdcProtocol.h

  \brief Declaration of communication device protocol command.

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
  History:
    05/09/08 A. Khromykh - Created
*******************************************************************************/
#ifndef _VCPCDCPROTOCOL_H
#define _VCPCDCPROTOCOL_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <types.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
// data size in request structure
#define REQUEST_DATA_SIZE           7
// data size in notification structure
#define NOTIFICATION_DATA_SIZE      2

/******************************************************************************
                   Types section
******************************************************************************/
BEGIN_PACK
// Usb host request
typedef struct PACK
{
  uint8_t  bmRequestType;
  uint8_t  bRequest;
  uint16_t wValue;
  uint16_t wIndex;
  uint16_t wLength;
  uint8_t  bData[REQUEST_DATA_SIZE];
} UsbRequest_t;

// Usb GetLineCoding device response
typedef struct PACK
{
  uint32_t dwDTERate;
  uint8_t  bCharFormat;
  uint8_t  bParityType;
  uint8_t  bDataBits;
} GetLineCodingResponse_t;

typedef union PACK
{
  GetLineCodingResponse_t getLineCoding;
} UsbResponse_t;

// Usb VCP notification
typedef struct PACK
{
  uint8_t  bmRequestType;
  uint8_t  bNotification;
  uint16_t wValue;
  uint16_t wIndex;
  uint16_t wLength;
  uint8_t  bData[NOTIFICATION_DATA_SIZE];
} UsbNotification_t;
END_PACK

/******************************************************************************
                   Prototypes section
******************************************************************************/
/******************************************************************************
communication device request handler

Parameters:
  data - pointer to host's request
******************************************************************************/
void vcpRequestHandler(uint8_t *data);

#endif /* _VCPCDCPROTOCOL_H */
