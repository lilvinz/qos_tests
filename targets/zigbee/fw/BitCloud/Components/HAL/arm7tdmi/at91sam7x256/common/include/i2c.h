/*****************************************************************************//**
\file  i2c.h

\brief Declarations of i2c interface.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    29/08/07 A. Khromykh - Created
**********************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

#ifndef _I2C_H
#define _I2C_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <i2cPacket.h>
#include <halTaskManager.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
// send only one byte
#define HAL_SINGLE_DATA_BYTE 1

/******************************************************************************
                   Types section
******************************************************************************/
typedef struct
{
  uint8_t* data;              // bytes to write to the i2c bus
  uint16_t length;            // length in bytes of the request
  uint16_t index;             // current index of read/write byte
  uint8_t addr;               // destination address
  uint32_t lengthAddr;        // Internal Device Address Size conditions
  uint32_t internalAddr;      // Internal Device Address
  void (*done)(bool result);  // callback
} I2cPacketControl_t;

// i2c virtual state
typedef enum
{
  I2C_PACKET_CLOSE = 0,
  I2C_PACKET_IDLE,
  I2C_PACKET_WRITE_ADDRESS,
  I2C_PACKET_WRITE_DATA,
  I2C_PACKET_READ_ADDRESS,
  I2C_PACKET_READ_DATA,
  I2C_PACKET_TRANSAC_SUCCESS,
  I2C_PACKET_TRANSAC_FAIL
} I2cPacketStates_t;

/******************************************************************************
                   Prototypes section
******************************************************************************/
/******************************************************************************
Inits TWI module.
Parameters:
  i2cMode - the speed of TWI.
Returns:
  none.
******************************************************************************/
void halInitI2c(HAL_i2cMode_t *i2cMode);

/******************************************************************************
start write transaction
Parameters:
  none.
Returns:
  none
******************************************************************************/
void halSendStartWriteI2c(void);

/******************************************************************************
start read transaction
Parameters:
  none.
Returns:
  none
******************************************************************************/
void halSendStartReadI2c(void);

#endif /* _I2C_H*/
// eof i2c.h
