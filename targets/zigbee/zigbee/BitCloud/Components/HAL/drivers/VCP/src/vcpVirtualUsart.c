/****************************************************************************//**
  \file vcpVirtualUart.c

  \brief Implementation of virtual uart API.

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
#include <string.h>
#include <vcpCdcProtocol.h>
#include <vcpDescriptors.h>
#include <vcpEnumeration.h>
#include <vcpVirtualUsart.h>

/******************************************************************************
                   Types section
******************************************************************************/
// usart control
typedef struct
{
  uint16_t rxUnusedRemaining;
} VcpControl_t;

/******************************************************************************
                   Prototypes section
******************************************************************************/
/******************************************************************************
Tuning of received buffer and enable received pipe if there is free place
in the buffer.
Parameters:
  descriptor - pointer to HAL_UartDescriptor_t structure;
Returns:
  none.
******************************************************************************/
void vcpEnableReadPipe(void);

/******************************************************************************
                   External global variables section
******************************************************************************/
extern UsbRequest_t  request;
extern UsbResponse_t response;

/******************************************************************************
                   Global variables section
******************************************************************************/
// pointer to application uart descriptor
HAL_UsartDescriptor_t *vcpPointDescrip = NULL;
// structure for internal ring buffer
VcpControl_t vcpControl;

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
Registers usb end of bus reset handlers.
******************************************************************************/
void vcpBusResetAction(void)
{
  HAL_RegisterRequestHandler(runtimeRequestHandler);
}

/******************************************************************************
Transmitting callback of virtual uart
Parameters:
  pArg - pointer to some data.
  status - result of the USB transfer.
  transferred - how much data are transferred
  remaining - how much data are not transferred
Returns:
  none.
******************************************************************************/
void vcpTmtCallback(void *pArg, uint8_t status, uint16_t transferred, uint16_t remaining)
{
  (void)pArg;
  (void)status;
  (void)transferred;
  (void)remaining;
  uint16_t copyPor = vcpPointDescrip->service.txPointOfRead;
  uint16_t copyPow = vcpPointDescrip->service.txPointOfWrite;


  if (NULL != vcpPointDescrip)
  {
    if (NULL != vcpPointDescrip->txBuffer)
    { /* polling mode */
      copyPor += transferred;
      if (copyPor == vcpPointDescrip->txBufferLength)
        copyPor = 0;

      if (copyPor != copyPow)
      {
        if (copyPor > copyPow)
        {
          HAL_UsbWrite(TRANSMIT_PIPE, &vcpPointDescrip->txBuffer[copyPor],
                      (vcpPointDescrip->txBufferLength - copyPor), vcpTmtCallback, NULL);
        }
        else
        {
          HAL_UsbWrite(TRANSMIT_PIPE, &vcpPointDescrip->txBuffer[copyPor],
                      (copyPow - copyPor), vcpTmtCallback, NULL);
        }
      }
      vcpPointDescrip->service.txPointOfRead = copyPor;
    } /* polling mode */

    if (NULL != vcpPointDescrip->txCallback)
      vcpPointDescrip->txCallback();
  }
}

/******************************************************************************
Receiving callback of virtual uart
Parameters:
  pArg - pointer to something data.
  status - result of the USB transfer.
  transferred - how much data are transferred
  remaining - how much data are not transferred
Returns:
  none.
******************************************************************************/
void vcpRcvCallback(void *pArg, uint8_t status, uint16_t transferred, uint16_t remaining)
{
  (void)pArg;
  (void)status;
  (void)remaining;
  uint16_t number;
  uint16_t copyPor;
  uint16_t copyPow;
  uint16_t copyUnr;

  vcpPointDescrip->service.rxPointOfWrite += transferred;
  copyPor = vcpPointDescrip->service.rxPointOfRead;
  copyPow = vcpPointDescrip->service.rxPointOfWrite;
  copyUnr = vcpControl.rxUnusedRemaining;

  if (NULL != vcpPointDescrip)
  {
    if (NULL != vcpPointDescrip->rxCallback)
    {
      if (copyPow < copyPor)
        number = copyUnr - (copyPor - copyPow);
      else
        number = copyPow - copyPor;

      vcpPointDescrip->rxCallback(number);
      vcpEnableReadPipe();
    }
  }
}

/******************************************************************************
Open virtual com port and register uart's event handlers.

Parameters:
  descriptor - pointer to HAL_UartDescriptor_t structure

Returns:
  Returns positive uart descriptor on success or -1 in cases:
    - bad uart channel;
    - there are not enough resources;
    - receiving buffer is less bulk endpoint size;
******************************************************************************/
int VCP_OpenUsart(HAL_UsartDescriptor_t *descriptor)
{
  if (NULL == descriptor)
    return -1;

  if (USART_CHANNEL_VCP != descriptor->tty)
    return -1;

  if (NULL != vcpPointDescrip)
    return -1; /* source was opened */

  vcpPointDescrip = descriptor;

  vcpPointDescrip->service.rxPointOfRead = 0;
  vcpPointDescrip->service.rxPointOfWrite = 0;
  vcpControl.rxUnusedRemaining = vcpPointDescrip->rxBufferLength;
  vcpPointDescrip->service.txPointOfRead = 0;
  vcpPointDescrip->service.txPointOfWrite = 0;
  HAL_RegisterEndOfBusResetHandler(vcpBusResetAction);
  HAL_UsbInit((uint8_t *)&request);

  return (int)descriptor->tty;
}

/******************************************************************************
Frees the virtual uart channel.
Parameters:
  descriptor - the uart descriptor.
Returns:
  Returns 0 on success or -1 if bad descriptor.
******************************************************************************/
int VCP_CloseUsart(HAL_UsartDescriptor_t *descriptor)
{
  if (NULL == descriptor)
    return -1;

  if (vcpPointDescrip != descriptor)
      return -1;

  vcpPointDescrip = NULL;

  return 0;
}

/******************************************************************************
Writes a number of bytes to a virtual uart channel.
txCallback function will be used to notify when the transmission is finished.
Parameters:
  descriptor - pointer to HAL_UartDescriptor_t structure;
  buffer - pointer to the application data buffer;
  length - number of bytes to transfer;
Returns:
  -1 - bad descriptor;
   Number of bytes placed to the buffer - success.
******************************************************************************/
int VCP_WriteUsart(HAL_UsartDescriptor_t *descriptor, uint8_t *buffer, uint16_t length)
{
  uint16_t copyPow = vcpPointDescrip->service.txPointOfWrite;
  uint16_t copyPor = vcpPointDescrip->service.txPointOfRead;
  uint16_t freePlace;
  uint16_t wasWrote = 0;

  if (NULL == descriptor)
    return -1;

  if (vcpPointDescrip != descriptor)
    return -1;

  if (!buffer || !length)
    return -1;

  if (NULL == descriptor->txBuffer)
  { /* callback mode */
    if (STATUS_SUCCESS != HAL_UsbWrite(TRANSMIT_PIPE, buffer, length, vcpTmtCallback, NULL))
      return -1; // there is unsent data
    return length;
  } /* callback mode */
  else
  { /* polling mode */
    if (copyPor > copyPow)
    {
      freePlace = copyPor - copyPow;
      if (freePlace < length)
        return -1; // there is unsent data
      memcpy(&descriptor->txBuffer[copyPow], buffer, length);
      HAL_UsbWrite(TRANSMIT_PIPE, buffer, length, vcpTmtCallback, NULL);
      copyPow += length;

    }
    else // point of write more or equal point of read
    {
      freePlace = descriptor->txBufferLength - copyPow + copyPor;
      if (freePlace < length)
        return -1; // there is unsent data

      uint16_t tempValue = descriptor->txBufferLength - copyPow;
      if (length > tempValue)
      {
        memcpy(&descriptor->txBuffer[copyPow], buffer, tempValue);
        HAL_UsbWrite(TRANSMIT_PIPE, buffer, tempValue, vcpTmtCallback, NULL);
        buffer += tempValue;
        length -= tempValue;
        memcpy(descriptor->txBuffer, buffer, length);
        copyPow = length;
      }
      else
      {
        memcpy(&descriptor->txBuffer[copyPow], buffer, length);
        HAL_UsbWrite(TRANSMIT_PIPE, buffer, length, vcpTmtCallback, NULL);
        copyPow += length;
      }
    }

    if (copyPow == descriptor->txBufferLength)
      copyPow = 0;
    vcpPointDescrip->service.txPointOfWrite = copyPow;
    wasWrote = length;

    return wasWrote;
  } /* polling mode */
}

/******************************************************************************
Tuning of received buffer and enable received pipe if there is free place
in the buffer.
Parameters:
  none;
Returns:
  none.
******************************************************************************/
void vcpEnableReadPipe(void)
{
  uint16_t copyPor = vcpPointDescrip->service.rxPointOfRead;
  uint16_t copyPow = vcpPointDescrip->service.rxPointOfWrite;
  uint16_t copyUnr = vcpControl.rxUnusedRemaining;
  uint8_t usbResult = STATUS_SUCCESS;

  do
  {
    if (copyPor < copyPow)
    {
      if ((vcpPointDescrip->rxBufferLength - copyPow) < BULK_SIZE)
      {
        copyUnr = copyPow;
        if (0 == copyPor)
        {
          usbResult = STATUS_BUSY;
          break;
        }
        copyPow = 0;
      }
      else
      {
        usbResult = HAL_UsbRead(RECEIVE_PIPE, &vcpPointDescrip->rxBuffer[copyPow], BULK_SIZE, vcpRcvCallback, NULL);
        break;
      }
    }
    else
    {
      // empty buffer
      if (copyPow == copyPor)
      {
        if ((vcpPointDescrip->rxBufferLength - copyPow) < BULK_SIZE)
        {
          copyUnr = vcpPointDescrip->rxBufferLength;
          copyPor = 0;
          copyPow = 0;
        }
        usbResult = HAL_UsbRead(RECEIVE_PIPE, &vcpPointDescrip->rxBuffer[copyPow], BULK_SIZE, vcpRcvCallback, NULL);
      } // copyPor > copyPow
      else
      {
        if ((copyPor - copyPow) >= BULK_SIZE)
          usbResult = HAL_UsbRead(RECEIVE_PIPE, &vcpPointDescrip->rxBuffer[copyPow], BULK_SIZE, vcpRcvCallback, NULL);
        else
          usbResult = STATUS_BUSY;
      }
      break;
    }
  } while(1);

  if (STATUS_SUCCESS == usbResult)
  {
    vcpPointDescrip->service.rxPointOfRead = copyPor;
    vcpPointDescrip->service.rxPointOfWrite = copyPow;
    vcpControl.rxUnusedRemaining = copyUnr;
  }
}

/*****************************************************************************
Reads length bytes from uart and places ones to buffer.
Parameters:
  descriptor - uart descriptor;
  buffer - pointer to a application buffer;
  length - the number of bytes which should be placed to buffer
Returns:
  -1 - bad descriptor, bad number to read;
  number of bytes that were placed to buffer.
*****************************************************************************/
int VCP_ReadUsart(HAL_UsartDescriptor_t *descriptor, uint8_t *buffer, uint16_t length)
{
  uint16_t wasRead = 0;
  uint16_t firstPart = 0;
  uint16_t copyPor = vcpPointDescrip->service.rxPointOfRead;
  uint16_t copyPow = vcpPointDescrip->service.rxPointOfWrite;
  uint16_t copyUnr = vcpControl.rxUnusedRemaining;

  if (NULL == descriptor)
    return -1;

  if (vcpPointDescrip != descriptor)
    return -1;

  if (!buffer || !length)
    return -1;

  do
  {
    // buffer is empty
    if (copyPow == copyPor)
      break;

    // [----**********---------]   --- empty   **** busy
    //      por       pow
    // [*****---------*******###]  ### unused at the current moment
    //       pow      por    ur
    // por - point of read(user read from buffer)
    // pow - point of write(HAL write to buffer)
    // ur - unused remaining(because remaining less then max packet size at the start read moment.)
    if (copyPor < copyPow)
    {
      wasRead = copyPow - copyPor;

      if (wasRead > length)
      {
        wasRead = length;
        memcpy(buffer, &vcpPointDescrip->rxBuffer[copyPor], wasRead);
        copyPor += length;
      }
      else
      {
        memcpy(buffer, &vcpPointDescrip->rxBuffer[copyPor], wasRead);
        copyPor = copyPow;
      }
      break;
    }
    else //copyPor > copyPow
    {
      if ((copyPor + length) < copyUnr)
      {
        wasRead = length;
        memcpy(buffer, &vcpPointDescrip->rxBuffer[copyPor], wasRead);
        copyPor += length;
        break;
      }
      else
      {
        firstPart = copyUnr - copyPor;
        memcpy(buffer, &vcpPointDescrip->rxBuffer[copyPor], firstPart);
        buffer += firstPart;
        length -= firstPart;
        copyUnr = vcpPointDescrip->rxBufferLength;
        copyPor = 0;
      }
    }
  } while(1);

  vcpPointDescrip->service.rxPointOfRead = copyPor;
  vcpPointDescrip->service.rxPointOfWrite = copyPow;
  vcpControl.rxUnusedRemaining = copyUnr;

  vcpEnableReadPipe();
  wasRead += firstPart;

  return wasRead;
}

/**************************************************************************//**
\brief Checks the status of tx buffer.

\param[in] descriptor - pointer to HAL_UsartDescriptor_t structure;
\return -1 - bad descriptor, no tx buffer; \n
         1 - tx buffer is empty; \n
         0 - tx buffer is not empty;
******************************************************************************/
int VCP_IsTxEmpty(HAL_UsartDescriptor_t *descriptor)
{
  HalUsartService_t *halUsartControl;
  uint16_t copyPow;
  uint16_t copyPor;

  if (NULL == descriptor)
    return -1;

  if (vcpPointDescrip != descriptor)
    return -1;

  halUsartControl = &descriptor->service;
  copyPow = halUsartControl->txPointOfWrite;
  copyPor = halUsartControl->txPointOfRead;

  if (copyPow == copyPor)
    return 1;
  else
    return 0;
}

// eof vcpVirtualUart.c
