/****************************************************************************//**
  \file vcpCdcProtocol.h

  \brief Implementation of communication device protocol command.

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
  History:
    11/09/08 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
                   Includes section
******************************************************************************/
#include <vcpCdcProtocol.h>
#include <vcpEnumeration.h>
#include <usart.h>
#include <usb.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
// request codes for communication interface class
#define SEND_ENCAPSULATED_COMMAND                   0x00
#define GET_ENCAPSULATED_RESPONSE                   0x01
#define SET_COMM_FEATURE                            0x02
#define GET_COMM_FEATURE                            0x03
#define CLEAR_COMM_FEATURE                          0x04
#define SET_AUX_LINE_STATE                          0x10
#define SET_HOOK_STATE                              0x11
#define PULSE_SETUP                                 0x12
#define SEND_PULSE                                  0x13
#define SET_PULSE_TIME                              0x14
#define RING_AUX_JACK                               0x15
#define SET_LINE_CODING                             0x20
#define GET_LINE_CODING                             0x21
#define SET_CONTROL_LINE_STATE                      0x22
#define SEND_BREAK                                  0x23
#define SET_RINGER_PARMS                            0x30
#define GET_RINGER_PARMS                            0x31
#define SET_OPERATION_PARMS                         0x32
#define GET_OPERATION_PARMS                         0x33
#define SET_LINE_PARMS                              0x34
#define GET_LINE_PARMS                              0x35
#define DIAL_DIGITS                                 0x36
#define SET_UNIT_PARAMETER                          0x37
#define GET_UNIT_PARAMETER                          0x38
#define CLEAR_UNIT_PARAMETER                        0x39
#define GET_PROFILE                                 0x3A
#define SET_ETHERNET_MULTICAST_FILTERS              0x40
#define SET_ETHERNET_POWER_MANAGEMENT_PATTERNFILTER 0x41
#define GET_ETHERNET_POWER_MANAGEMENT_PATTERNFILTER 0x42
#define SET_ETHERNET_PACKET_FILTER                  0x43
#define GET_ETHERNET_STATISTIC                      0x44
#define SET_ATM_DATA_FORMAT                         0x50
#define GET_ATM_DEVICE_STATISTICS                   0x51
#define SET_ATM_DEFAULT_VC                          0x52
#define GET_ATM_VC_STATISTICS                       0x53

// virtual uart speed
#define VU_1200   0x4B0
#define VU_2400   0x960
#define VU_4800   0x12C0
#define VU_9600   0x2580
#define VU_19200  0x4B00
#define VU_38400  0x9600
#define VU_57600  0xE100
#define VU_115200 0x1C200

// char size
#define VU_1STOPBIT    0
#define VU_1d5STOPBITS 1
#define VU_2STOPBITS   2

// parity
#define VU_NONE  0
#define VU_ODD   1
#define VU_EVEN  2
#define VU_MARK  3
#define VU_SPACE 4

// data bits number
#define VU_5DATABITS  5
#define VU_6DATABITS  6
#define VU_7DATABITS  7
#define VU_8DATABITS  8
#define VU_16DATABITS 16

/******************************************************************************
                   External global variables section
******************************************************************************/
extern HAL_UsartDescriptor_t *vcpPointDescrip;

/******************************************************************************
                   Global variables section
******************************************************************************/
UsbRequest_t  request;
UsbResponse_t response;

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
Get baud rate meaning for cdc response.

Parameters:
  baudRate - virtual uart baudRate
******************************************************************************/
void vcpGetBaudRate(uint32_t baudRate)
{
  (void)baudRate;
  response.getLineCoding.dwDTERate = VU_115200;
}

/******************************************************************************
Get number of stop bits meaning for cdc response.

Parameters:
  stopBits - virtual uart stop bits
******************************************************************************/
void vcpGetStopBits(uint8_t stopBits)
{
  (void)stopBits;
  response.getLineCoding.bCharFormat = VU_1STOPBIT;
}

/******************************************************************************
Get parity meaning for cdc response.

Parameters:
  parity - virtual uart parity
******************************************************************************/
void vcpGetParity(uint8_t parity)
{
  (void)parity;
  response.getLineCoding.bParityType = VU_NONE;
}

/******************************************************************************
Get data length meaning for cdc response.

Parameters:
  dataLength - virtual uart data length
******************************************************************************/
void vcpGetDataLength(uint8_t dataLength)
{
  (void)dataLength;
  response.getLineCoding.bParityType = VU_8DATABITS;
}

/******************************************************************************
Get virtual uart data and send answer to host.
******************************************************************************/
void vcpResponseGetLineCoding(void)
{
  vcpGetBaudRate(vcpPointDescrip->baudrate);
  vcpGetStopBits(vcpPointDescrip->stopbits);
  vcpGetParity(vcpPointDescrip->parity);
  vcpGetDataLength(vcpPointDescrip->dataLength);

  HAL_UsbWrite(0, (void *)&response, sizeof(GetLineCodingResponse_t), 0, 0);
}

/******************************************************************************
Set baud rate meaning to virtual port.

Parameters:
  baudRate - virtual uart baud rate
******************************************************************************/
void vcpSetBaudRate(uint32_t baudRate)
{
  (void)baudRate;
}

/******************************************************************************
Set number stop bits to virtual port.

Parameters:
  stopBits - virtual uart stop bits
******************************************************************************/
void vcpSetStopBits(uint8_t stopBits)
{
  (void)stopBits;
}

/******************************************************************************
Set parity meaning to virtual port.

Parameters:
  parity - virtual uart parity
******************************************************************************/
void vcpSetParity(uint8_t parity)
{
  (void)parity;
}

/******************************************************************************
Set data length to virtual port.

Parameters:
  dataLength - virtual uart data length
******************************************************************************/
void vcpSetDataLength(uint8_t dataLength)
{
  (void)dataLength;
}

/******************************************************************************
Set virtual uart data and send response to host.
******************************************************************************/
void vcpResponseSetLineCoding(void)
{
  vcpSetBaudRate(response.getLineCoding.dwDTERate);
  vcpSetStopBits(response.getLineCoding.bCharFormat);
  vcpSetParity(response.getLineCoding.bParityType);
  vcpSetDataLength(response.getLineCoding.bDataBits);

#if defined(AT91SAM7X256) || defined(AT91SAM3S4C)
  sendZLP();
#endif
}

/******************************************************************************
communication device request handler

Parameters:
  data - pointer to host's request
******************************************************************************/
void vcpRequestHandler(uint8_t *data)
{
  UsbRequest_t *pRequest = NULL;

  pRequest = (UsbRequest_t *)data;
  if (NULL == pRequest)
    return;

  // Check request code
  switch (pRequest->bRequest)
  {
    case SET_LINE_CODING:
        HAL_UsbRead(0, (void *)&response, sizeof(GetLineCodingResponse_t), (TransferCallback_t)vcpResponseSetLineCoding, 0);
      break;
    case GET_LINE_CODING:
        vcpResponseGetLineCoding();
      break;
    case SET_CONTROL_LINE_STATE:
        //vcpReadDataFromSetControlLineState(pRequest->wValue); // possible in the future
        #if defined(AT91SAM7X256) || defined(AT91SAM3S4C)
          sendZLP();
        #endif
      break;
    default:
        HAL_Stall(0);
      break;
  }
}

//eof vcpCdcProtocol.c
