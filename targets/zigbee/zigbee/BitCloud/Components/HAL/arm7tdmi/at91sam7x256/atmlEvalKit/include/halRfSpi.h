/*****************************************************************************//**
\file  halRfSpi.h

\brief SPI interface routines header.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    24/08/07 A. Khromykh - Created
**********************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

#ifndef _HALRFSPI_H
#define _HALRFSPI_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <types.h>
#include <AT91SAM7X256.h>
#include <atomic.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
// serial clock baud rate
// speed - spi frequency in Hz
#if defined(PLATFORM_SAM7X_EK_RF2XX) || defined(PLATFORM_CUSTOM_1) || defined(PLATFORM_CUSTOM_2)
  #define SPI_FREQUENCY  6000000ul
#elif defined(PLATFORM_ZIGBIT_LAN) || defined(PLATFORM_ZIGBIT_TURBO)
  #ifdef RF
    #define SPI_FREQUENCY  8000000ul
  #else
    #define SPI_FREQUENCY  6000000ul
  #endif
#endif
// DLYBS = Delay * Master Clock. Delay from NPCS valid to the first valid SPCK transition
// When DLYBS equals zero, the NPCS valid to SPCK transition is 1/2 the SPCK clock period
// rf230 180 ns (t1 in manual)
#define NPCS_TO_SPCK_RF230  0.00000018
// Delay Between Consecutive Transfers DLYBC = Delay * Master Clock / 32
// rf230 250 ns (t5 in manual)
#define DELAY_BETWEEN_CONSECUTIVE_TRANSFERS_RF230  0.00000025

/******************************************************************************
                   Types section
******************************************************************************/
// mode select cs
typedef enum {
  HAL_SPI_HARDWARE_CS,
  HAL_SPI_SOFTWARE_CS
} CsMode_t;

// mode spi dma
typedef enum {
  HAL_SPI_BYTE,
  HAL_SPI_DMA
} DmaMode_t;

// spi mode struct
typedef struct
{
  CsMode_t csmode;
  DmaMode_t dmamode;
} HAL_RfSpiMode_t;

/******************************************************************************
                   Prototypes section
******************************************************************************/
/******************************************************************************
Performs hwi initialization of SPI0(RF) interface.
  Parameters:
  SpiMode - the address of a structure including spi work mode.
******************************************************************************/
void HAL_InitRfSpi(HAL_RfSpiMode_t *SpiMode);

/******************************************************************************
  Writes data to SPI.
  parameters:
    data - pointer to data
    size - size data in bytes
    callback - the address of a function if a application wishes to be notified about
                a spi transaction complete event or NULL.
******************************************************************************/
void HAL_WriteDataRfSpi(uint8_t *data, uint16_t size, void (*callback)());

/******************************************************************************
  Enable reads data from SPI.
  parameters:
    data - pointer to data
    size - size data in bytes
    callback - the address of a function if a application wishes to be notified about
                a spi transaction complete event or NULL.
******************************************************************************/
void HAL_EnableReadDataRfSpi(uint8_t *data, uint16_t size, void (*callback)());

/******************************************************************************
  Writes/reads byte to/from SPI.
  parameters: value - byte to write.
  Returns:    the byte which was read.
******************************************************************************/
uint32_t HAL_WriteByteRfSpi(uint8_t value);

/******************************************************************************
  Deselects a slave device.
******************************************************************************/
void HAL_DeselectRfSpi(void);

/******************************************************************************
  Selects a slave device.
******************************************************************************/
void HAL_SelectRfSpi(void);

/******************************************************************************
                   Inline static functions section
******************************************************************************/
/******************************************************************************
  Inline function (to use in critical sections)
  Writes/reads byte to/from SPI.
  parameters: value - byte to write.
  Returns:    the byte which was read.
******************************************************************************/
static inline uint8_t HAL_WriteByteInlineRfSpi(uint8_t value)
{
  uint8_t tempValue;
  ATOMIC_SECTION_ENTER
    tempValue = HAL_WriteByteRfSpi(value);
  ATOMIC_SECTION_LEAVE
  return tempValue;
}

#endif /* _HALRFSPI_H */

//eof halRfSpi.h
