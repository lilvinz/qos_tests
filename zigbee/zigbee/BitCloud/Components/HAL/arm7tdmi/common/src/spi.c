/**************************************************************************//**
\file  spi.c

\brief Implementation of spi interface.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    5/12/07 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                   Includes section
******************************************************************************/
#include <spi.h>
#include <stddef.h>

/******************************************************************************
                   Prototypes section
******************************************************************************/
/******************************************************************************
  Performs hardware initialization of SPI1(Mem) interface.
  Parameters:
   descrp - pointer to the spi descriptor.
******************************************************************************/
void halInitMemSpi(HAL_SpiDescriptor_t *descrp);

/******************************************************************************
                   External variables section
******************************************************************************/
extern volatile uint8_t halSpiCurrentState;
extern void (* mem_spi_callback)(void);

/******************************************************************************
                   Implementations section
******************************************************************************/
/**************************************************************************//**
\brief Opens SPI1.
\param[in]
  descriptor - pointer to spi descriptor
\return
   -1 - there are no free resources. \n
    0 - SPI channel is ready.
******************************************************************************/
int HAL_OpenSpi(HAL_SpiDescriptor_t *descriptor)
{
  if (NULL == descriptor)
    return -1;
  if (SPI_CHANNEL_1 != descriptor->tty)
    return -1;
  if (BUS_NOINIT != halSpiCurrentState)
    return -1;
  mem_spi_callback = descriptor->callback;
  halInitMemSpi(descriptor);
  halSpiCurrentState = BUS_READY;
  return 0;
}

/**************************************************************************//**
\brief Releases the channel and pins.
\param[in]
  descriptor - pointer to spi descriptor
\return
  -1 - channel was not opened. \n
   0 - success.
******************************************************************************/
int HAL_CloseSpi(HAL_SpiDescriptor_t *descriptor)
{
  if (NULL == descriptor)
    return -1;
  if (SPI_CHANNEL_1 != descriptor->tty)
    return -1;
  if (BUS_READY == halSpiCurrentState)
  {
    halSpiCurrentState = BUS_NOINIT;
    return 0;
  }
  return -1;
}

/**************************************************************************//**
\brief Writes a number of bytes to the SPI.\n
 Callback function will be used to notify when the activity is finished. \n
\param[in]
  descriptor - pointer to HAL_SpiDescriptor_t structure
\param[in]
  buffer -  pointer to the application data buffer
\param[in]
  length -  number of bytes to transfer
\return
  -1 - spi module was not opened, or there is unsent data, or the pointer to
   data or the length are NULL; \n
   0 - success; \n
   Number of written bytes - success (synchronous mode).
******************************************************************************/
int HAL_WriteSpi(HAL_SpiDescriptor_t *descriptor, uint8_t *buffer, uint16_t length)
{
  if (NULL == descriptor)
    return -1;
  if (SPI_CHANNEL_1 != descriptor->tty)
    return -1;
  if (BUS_READY != halSpiCurrentState)
    return -1;
  if ((NULL == buffer) || (0 == length))
    return -1;

  halSpiCurrentState = BUS_BUSY;
  return halStartDataMemSpi(SPI_WRITE, buffer, length);
}

/**************************************************************************//**
\brief Writes a number of bytes to the SPI.\n
 Callback function will be used to notify when the activity is finished.\n
 The read data is placed to the buffer.
\param[in]
  descriptor - pointer to HAL_SpiDescriptor_t structure
\param[in]
  buffer -  pointer to the application data buffer
\param[in]
  length -  number of bytes to transfer
\return
  -1 - spi module was not opened, or there is unsent data, or the pointer to
   data or the length are NULL; \n
   0 - success; \n
   Number of written bytes - success (synchronous mode).
******************************************************************************/
int HAL_ReadSpi(HAL_SpiDescriptor_t *descriptor, uint8_t *buffer, uint16_t length)
{
  if (NULL == descriptor)
    return -1;
  if (SPI_CHANNEL_1 != descriptor->tty)
    return -1;
  if (BUS_READY != halSpiCurrentState)
    return -1;
  if ((NULL == buffer) || (0 == length))
    return -1;

  halSpiCurrentState = BUS_BUSY;
  return halStartDataMemSpi(SPI_READ, buffer, length);
}

//eof spi.c
