/*****************************************************************************//**
\file   halMemSpi.h

\brief  SPI1 interface routines header.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    19/10/07 A. Khromykh - Created
**********************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

#ifndef _HALMEMSPI_H
#define _HALMEMSPI_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <types.h>
#include <gpio.h>
#include <halClkCtrl.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define SPI_BAUDRATE(freq_value)  ((uint32_t)(F_CPU / freq_value))
#define SPI_DLYBS(delay_value)  ((uint32_t)(delay_value * F_CPU) + 1ul)
#define SPI_DLYBCT(delay_value)  ((uint32_t)(delay_value * F_CPU / 32) + 1ul)
#define HAL_SPI_PACK_PARAMETER(freq_value, valid_delay_value, cons_delay_value) \
                              ((SPI_BAUDRATE(freq_value) << 8) |                \
                               (SPI_DLYBS(valid_delay_value) << 16) |           \
                               (SPI_DLYBCT(cons_delay_value) << 24))

// spi mcu module is not initialize
#define BUS_NOINIT          0
// spi is ready to transaction
#define BUS_READY           1
// spi is busy
#define BUS_BUSY            2

#define CS0    0
#define CS1    1
#define CS2    2
#define CS3    3

#define DEFAULT_CS  CS0

#define SPI_CHANNEL_1   AT91C_ID_SPI1

/******************************************************************************
                   Types section
******************************************************************************/
// spi channel
typedef uint8_t SpiChannel_t;

// callback method
typedef void (*CallBack_t)();

// mode SPI work
typedef enum
{
  SPI_CLOCK_MODE1 = 0ul,
  SPI_CLOCK_MODE3 = AT91C_SPI_CPOL,
  SPI_CLOCK_MODE0 = AT91C_SPI_NCPHA,
  SPI_CLOCK_MODE2 = (AT91C_SPI_NCPHA | AT91C_SPI_CPOL)
} SpiClockMode_t;

typedef enum
{
  SPI_8BITS_SYMBOL  = AT91C_SPI_BITS_8,
  SPI_9BITS_SYMBOL  = AT91C_SPI_BITS_9,
  SPI_10BITS_SYMBOL = AT91C_SPI_BITS_10,
  SPI_11BITS_SYMBOL = AT91C_SPI_BITS_11,
  SPI_12BITS_SYMBOL = AT91C_SPI_BITS_12,
  SPI_13BITS_SYMBOL = AT91C_SPI_BITS_13,
  SPI_14BITS_SYMBOL = AT91C_SPI_BITS_14,
  SPI_15BITS_SYMBOL = AT91C_SPI_BITS_15,
  SPI_16BITS_SYMBOL = AT91C_SPI_BITS_16
} SpiSymbolSize_t;

// type of access
typedef enum
{
  SPI_READ,
  SPI_WRITE
} SpiAccessType_t;

// spi mode struct
typedef struct
{
  SpiClockMode_t clockMode;   // spi mode cs
  SpiSymbolSize_t symbolSize;   // symbol size (bit)
  uint32_t pack_parameter;    // spi timing parameter cs
} HAL_SpiMode_t;

/******************************************************************************
                   Prototypes section
******************************************************************************/
/******************************************************************************
starts spi data transaction
Parameters:
   accessType - access type
   data - pointer to data buffer.
   size - size of data buffer.
Return:
   0 - success (asynchronous mode);
   Number of written bytes - success (synchronous mode).
******************************************************************************/
int halStartDataMemSpi(SpiAccessType_t accessType, uint8_t *data, uint16_t size);

#endif /*_HALMEMSPI_H*/
