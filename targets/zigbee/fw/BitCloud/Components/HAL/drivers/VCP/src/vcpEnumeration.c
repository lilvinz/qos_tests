/****************************************************************************//**
  \file vcpEnumeration.c

  \brief Implementation of enumeration proccess.

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
#include <vcpDescriptors.h>
#include <vcpVirtualUsart.h>
#include <usb.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
// request codes for runtime work
#define USB_GET_STATUS             0
#define USB_CLEAR_FEATURE          1
#define USB_SET_FEATURE            3
#define USB_SET_ADDRESS            5
#define USB_GET_DESCRIPTOR         6
#define USB_SET_DESCRIPTOR         7
#define USB_GET_CONFIGURATION      8
#define USB_SET_CONFIGURATION      9
#define USB_GET_INTERFACE          10
#define USB_SET_INTERFACE          11
#define USB_SYNCH_FRAME            12

// Descriptor type definitions.
#define DESCRIPTOR_TYPE_DEVICE           0x01
#define DESCRIPTOR_TYPE_CONFIGURATION    0x02
#define DESCRIPTOR_TYPE_STRING           0x03
#define DESCRIPTOR_TYPE_INTERFACE        0x04
#define DESCRIPTOR_TYPE_CS_INTERFACE     0x24
#define DESCRIPTOR_TYPE_ENDPOINT         0x05
#define DESCRIPTOR_TYPE_DEVICE_QUALIFIER 0x06
#define DESCRIPTOR_TYPE_OTHER_SPEED_CFG  0x07
#define DESCRIPTOR_TYPE_INTERFACE_POWER  0x08

/******************************************************************************
                   External global variables section
******************************************************************************/
extern const ConfigurationFrameResponse_t vcpConfigDescriptor;
extern const DeviceDescriptor_t deviceDescrptr;
extern HAL_UsartDescriptor_t *vcpPointDescrip;

/******************************************************************************
                   Prototypes section
******************************************************************************/
/******************************************************************************
usb bulk out receiving callback

Parameters:
  pArg   - argument
  status  - index of the requested descriptor
  length - maximum number of bytes to return
******************************************************************************/
void vcpRcvCallback(void *pArg, uint8_t status, uint16_t transferred, uint16_t remaining);

/******************************************************************************
                   Global variables section
******************************************************************************/
// virtual communication port address on usb
static uint16_t vcpAddress = 0;

/******************************************************************************
                   Implementations section
******************************************************************************/
#if defined(AT91SAM7X256) || defined(AT91SAM3S4C)
/******************************************************************************
send zero-length packet through control pipe
******************************************************************************/
void sendZLP(void)
{
  // Acknowledge the request
  HAL_UsbWrite(0, NULL, 0, NULL, NULL);
}
#endif

/******************************************************************************
Configures the device by setting it into the Configured state.

Parameters:
  cfgnum - configuration number to set
******************************************************************************/
void runtimeSetConfiguration(uint8_t cfgnum)
{
  // Set & save the desired configuration
  HAL_SetConfiguration(cfgnum);

  #if defined(AT91SAM7X256) || defined(AT91SAM3S4C)
    // Acknowledge the request
    sendZLP();
  #endif
}

/******************************************************************************
runtime get descriptor command handler

Parameters:
  type   - type of the requested descriptor
  index  - index of the requested descriptor
  length - maximum number of bytes to return
******************************************************************************/
void runtimeGetDescriptorHandler(uint8_t type, uint8_t index, uint16_t length)
{
  (void)index;

  // Check the descriptor type
  switch (type)
  {
    case DESCRIPTOR_TYPE_DEVICE:
        // Adjust length and send descriptor
        if (length > deviceDescrptr.bLength)
          length = deviceDescrptr.bLength;
        HAL_UsbWrite(0, (void *)&deviceDescrptr, length, 0, 0);
      break;
    case DESCRIPTOR_TYPE_CONFIGURATION:
        // Adjust length and send descriptor
        if (length > vcpConfigDescriptor.config.wTotalLength)
          length = vcpConfigDescriptor.config.wTotalLength;
        HAL_UsbWrite(0, (void *)&vcpConfigDescriptor, length, 0, 0);
      break;
    case DESCRIPTOR_TYPE_INTERFACE:
        HAL_Stall(0);
      break;
    case DESCRIPTOR_TYPE_DEVICE_QUALIFIER:
        HAL_Stall(0);
      break;
    case DESCRIPTOR_TYPE_OTHER_SPEED_CFG:
        HAL_Stall(0);
      break;
    case DESCRIPTOR_TYPE_ENDPOINT:
        HAL_Stall(0);
      break;
    case DESCRIPTOR_TYPE_STRING:
        HAL_Stall(0);
      break;
    default:
        HAL_Stall(0);
      break;
  }
}

/******************************************************************************
Standard usb request handler

Parameters:
  data - pointer to host's request
******************************************************************************/
void runtimeRequestHandler(uint8_t *data)
{
  UsbRequest_t *pRequest = NULL;

  pRequest = (UsbRequest_t *)data;
  if (NULL == pRequest)
    return;

  // Check request code
  switch (pRequest->bRequest)
  {
    case USB_GET_DESCRIPTOR:
        // Send the requested descriptor
        runtimeGetDescriptorHandler((uint8_t)(pRequest->wValue >> 8), (uint8_t)(pRequest->wValue), pRequest->wLength);
      break;
    case USB_SET_ADDRESS:
        vcpAddress = pRequest->wValue & 0x7F;
        #if defined(AT91SAM7X256) || defined(AT91SAM3S4C)
          HAL_UsbWrite(0, 0, 0, (TransferCallback_t) HAL_SetAddress, (void *)&vcpAddress);
        #elif defined(AT90USB1287)
          HAL_SetAddress((uint8_t *)&vcpAddress);
        #endif
      break;
    case USB_SET_CONFIGURATION:
        // Set the requested configuration
        runtimeSetConfiguration((uint8_t)pRequest->wValue);
        HAL_RegisterRequestHandler(vcpRequestHandler);
        HAL_ConfigureEndpoint((void *)&(vcpConfigDescriptor.endpointIfc1));
        HAL_ConfigureEndpoint((void *)&(vcpConfigDescriptor.endpointIfc2[0]));
        HAL_ConfigureEndpoint((void *)&(vcpConfigDescriptor.endpointIfc2[1]));
        HAL_UsbRead(RECEIVE_PIPE, vcpPointDescrip->rxBuffer, BULK_SIZE, vcpRcvCallback, NULL);
      break;
    default:
        HAL_Stall(0);
      break;
  }
}

// eof vcpEnumeration.c
