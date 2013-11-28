/*****************************************************************************//**
\file  halW1.h

\brief  Declarations of 1-wire hardware-dependent module.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    29/05/07 E. Ivanov - Created
    05/09/07 A. Khromykh - modified (for arm)
**********************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

#ifndef _HALW1_H
#define _HALW1_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <atomic.h>
#include <gpio.h>
#include <inttypes.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#if defined(PLATFORM_SAM7X_EK_RF2XX) || defined(PLATFORM_CUSTOM_1) || defined(PLATFORM_CUSTOM_2)
  #define ONE_WIRE_PORT       B
  #define BASE_ONE_WIRE_PORT  AT91C_BASE_PIOB
  #define ONE_WIRE_PIN        AT91C_PIO_PB27
  #define ONE_WIRE_PORT_ID    AT91C_ID_PIOB
#elif defined(PLATFORM_ZIGBIT_LAN) || defined(PLATFORM_ZIGBIT_TURBO)
  #define ONE_WIRE_PORT       B
  #define BASE_ONE_WIRE_PORT  AT91C_BASE_PIOB
  #define ONE_WIRE_PIN        AT91C_PIO_PB22
  #define ONE_WIRE_PORT_ID    AT91C_ID_PIOB
#endif

/******************************************************************************
                   Types section
******************************************************************************/
// w1 status
typedef enum
{
  W1_NO_DEVICE_STATUS, // There is no device on the bus
  W1_SUCCESS_STATUS,   // At least one device is on the bus
  W1_INVALID_CRC       // Invalid CRC was read during the device search operation
} W1Status_t;

/******************************************************************************
                   Inline static functions section
******************************************************************************/
// the macros for the manipulation by 1-wire
HAL_ASSIGN_PIN(ONE_WIRE, B, ONE_WIRE_PIN);

/******************************************************************************
                   Prototypes section
******************************************************************************/
/*******************************************************************************
Reads byte from the bus.
Parameters:
    none.
Returns:
  byte read from the bus.
*******************************************************************************/
uint8_t halReadW1(void);

/*******************************************************************************
Reads bit from the bus.
Parameters:
  none.
Returns:
  Read bit is placed to position of last significant bit.
*******************************************************************************/
uint8_t halReadW1Bit(void);

/*******************************************************************************
Writes bit to the bus.
Parameters:
  value - to write. The bit is placed to position of last significant bit.
Returns:
  none.
*******************************************************************************/
void halWriteW1bit(uint8_t value);

/*******************************************************************************
Writes byte to the bus
Parameters:
    value - byte to write.
Returns:
    none.
*******************************************************************************/
void halWriteW1(uint8_t value);

/*******************************************************************************
Resets all devices connected to the bus.
Parameters:
  none.
Returns:
  W1_SUCCESS_STATUS   - If device(s) was(were) detected.
  W1_NO_DEVICE_STATUS - If device(s) was(were) not detected.
*******************************************************************************/
W1Status_t halResetW1(void);

/*******************************************************************************
 Delay in microseconds.
 Parameters:
   us - delay time in microseconds
*******************************************************************************/
void halDelayUs(uint16_t us);

#endif /* _HALW1_H */
// eof halW1.h
