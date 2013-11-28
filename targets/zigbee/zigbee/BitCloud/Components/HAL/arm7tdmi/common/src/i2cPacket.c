/**************************************************************************//**
\file  i2cPacket.c

\brief Provides the functionality for the writing and the reading \n
       of packets through the TWI.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    29/08/07 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                   Includes section
******************************************************************************/
#include <i2cPacket.h>
#include <i2c.h>

/******************************************************************************
                   External global variables section
******************************************************************************/
extern volatile I2cPacketStates_t halI2cPacketState;  // current state of the i2c request
extern I2cPacketControl_t halI2cPacketControl;

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
Open and initialize resource.
Parameters:
  i2cMode - clock rate
Returns:
  -1 - resource was opened or pointer is NULL
   0 - success.
******************************************************************************/
int HAL_OpenI2cPacket(HAL_i2cMode_t *i2cMode)
{
  if ((I2C_PACKET_CLOSE == halI2cPacketState) && (NULL != i2cMode))
  {
    halInitI2c(i2cMode);
    halI2cPacketState = I2C_PACKET_IDLE;
    halI2cPacketControl.index = 0;
    return 0;
  }
  return -1;
}

/******************************************************************************
Closes resource.
Returns:
  -1 - resource was not opened.
  0 -  success.
******************************************************************************/
int HAL_CloseI2cPacket(void)
{
  if (I2C_PACKET_CLOSE != halI2cPacketState)
  {
    halI2cPacketControl.done = NULL;
    halI2cPacketState = I2C_PACKET_CLOSE;
    return 0;
  }
  return -1;
}

/******************************************************************************
Writes the series of bytes out to the TWI bus.
Parameters:
  param - pointer to HAL_I2cParams_t structure
Returns:
  0 - the bus is free and the request is accepted.
  -1 - other case.
******************************************************************************/
int HAL_WriteI2cPacket(HAL_I2cParams_t *param)
{
  if ((I2C_PACKET_IDLE == halI2cPacketState) && param)
  {
    halI2cPacketControl.addr = param->id;
    halI2cPacketControl.data = param->data;
    halI2cPacketControl.index = 0;
    halI2cPacketControl.length = param->length;
    halI2cPacketControl.lengthAddr = param->lengthAddr;
    halI2cPacketControl.internalAddr = param->internalAddr;
    halI2cPacketControl.done = param->f;
  }
  else
  {
    return -1;
  }

  halI2cPacketState = I2C_PACKET_WRITE_ADDRESS;
  halSendStartWriteI2c();
  return 0;
}

/******************************************************************************
Reads the series of bytes out to the TWI bus.
Parameters:
  param - pointer to HAL_I2cParams_t structure
Returns:
  0 - the bus is free and the request is accepted.
  -1 - other case.
******************************************************************************/
int HAL_ReadI2cPacket(HAL_I2cParams_t *param)
{
  if ((I2C_PACKET_IDLE == halI2cPacketState) && param)
  {
    halI2cPacketControl.addr = param->id;
    halI2cPacketControl.index = 0;
    halI2cPacketControl.length = param->length;
    halI2cPacketControl.data = param->data;
    halI2cPacketControl.lengthAddr = param->lengthAddr;
    halI2cPacketControl.internalAddr = param->internalAddr;
    halI2cPacketControl.done = param->f;
  }
  else
  {
    return -1;
  }

  halI2cPacketState = I2C_PACKET_READ_ADDRESS;
  halSendStartReadI2c();
  return 0;
}

/******************************************************************************
It's callback function for task manager
Parameters:
  none
Returns:
  none
******************************************************************************/
void halSig2WireSerialHandler(void)
{
  if (NULL != halI2cPacketControl.done)
  {
    if (I2C_PACKET_TRANSAC_SUCCESS == halI2cPacketState)
    {
      halI2cPacketState = I2C_PACKET_IDLE;
      halI2cPacketControl.done(true);
    }
    else
    {
      halI2cPacketState = I2C_PACKET_IDLE;
      halI2cPacketControl.done(false);
    }
  }
  else
  {
    halI2cPacketState = I2C_PACKET_IDLE;
  }

}
// eof i2cPacket.c
