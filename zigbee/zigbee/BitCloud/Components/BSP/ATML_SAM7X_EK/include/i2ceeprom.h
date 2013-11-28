/**************************************************************************//**
\file  eeprom.h

\brief  Provides the access to I2C EEPROM.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    17/09/07 A. Khromykh - Created
*******************************************************************************/

#ifndef _I2CEEPROM_H
#define _I2CEEPROM_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <i2cPacket.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define I2C_EEPROM_DATA_MEMORY_SIZE 0x10000u

/******************************************************************************
                   Types section
******************************************************************************/
typedef struct
{
  uint16_t address;        // eeprom address
  uint8_t *data;           // pointer to data memory
  uint16_t length;         // number of bytes
} BSP_I2c_EepromParams_t;

/******************************************************************************
                   Prototypes section
******************************************************************************/
/******************************************************************************
Open and init i2c bus for eeprom
Parameters:
  i2cMode - clock rate
Retrun
  result
******************************************************************************/
int BSP_OpenI2cForEeprom(HAL_i2cMode_t *i2cMode);

/******************************************************************************
Close i2c bus for eeprom
Retrun
  result
******************************************************************************/
int BSP_CloseI2cForEeprom(void);

/******************************************************************************
Reads some number of bytes defined by BSP_I2c_EepromParams_t from the EEPROM.
Parameters:
  params - address of BSP_I2c_EepromParams_t defined by user.
  readDone - address of callback.
Returns:
  -1 - the EEPROM has request that was not completed
******************************************************************************/
int BSP_ReadEeprom(BSP_I2c_EepromParams_t *params, void (*readDone)(bool Result));

/******************************************************************************
Writes number of bytes defined by BSP_I2c_EepromParams_t to EEPROM.
Parameters:
  params - address of BSP_I2c_EepromParams_t defined by user.
  writeDone - address of callback.
Returns:
  -1 - the EEPROM has request that was not completed.
******************************************************************************/
int BSP_WriteEeprom(BSP_I2c_EepromParams_t *params, void (*writeDone)(bool Result));

#endif /*_I2CEEPROM_H*/
