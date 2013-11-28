/**************************************************************************//**
\file  eeprom.c

\brief Implementation of the I2C EEPROM interface.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    17/09/07 A. Khromykh - Created
*******************************************************************************/
#if APP_DISABLE_BSP != 1

/******************************************************************************
                   Includes section
******************************************************************************/
#include <i2ceeprom.h>
#include <string.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define I2C_EEPROM_ADDRESS   0x50

/******************************************************************************
                   Types section
******************************************************************************/
typedef enum
{
  EEPROM_IDLE = 0,
  EEPROM_BUSY
} BSP_EepromState_t;

/******************************************************************************
                   Global variables section
******************************************************************************/
BSP_EepromState_t bspEepromState = EEPROM_IDLE;
BSP_I2c_EepromParams_t bspEepromParams;
void (*bspUserCallbackFunctions)(bool) = NULL;

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
Open and init i2c bus for eeprom
Parameters:
  i2cMode - clock rate
Retrun
  result
******************************************************************************/
int BSP_OpenI2cForEeprom(HAL_i2cMode_t *i2cMode)
{
  return HAL_OpenI2cPacket(i2cMode);
}

/******************************************************************************
Close i2c bus for eeprom
Retrun
  result
******************************************************************************/
int BSP_CloseI2cForEeprom(void)
{
  return HAL_CloseI2cPacket();
}

/******************************************************************************
BSP callback for correct work with i2c
Parameters:
  Result - result i2c transaction
           successful TRUE
           other FALSE
******************************************************************************/
void bspEepromDone(bool Result)
{
  if (NULL == bspUserCallbackFunctions)
    return;
  bspUserCallbackFunctions(Result);
  HAL_CloseI2cPacket();
  bspEepromState = EEPROM_IDLE;
}

/******************************************************************************
Reads some number of bytes defined by BSP_I2c_EepromParams_t from the EEPROM.
Parameters:
  params - address of BSP_I2c_EepromParams_t defined by user.
  readDone - address of callback.
Returns:
  -1 - the EEPROM has request that was not completed
******************************************************************************/
int BSP_ReadEeprom(BSP_I2c_EepromParams_t *params, void (*readDone)(bool Result))
{
  HAL_I2cParams_t i2cParam;

  if (EEPROM_IDLE != bspEepromState)
    return -1;
  if (((uint32_t)params->address + (uint32_t)params->length) > I2C_EEPROM_DATA_MEMORY_SIZE)
    return -1;

  i2cParam.id = I2C_EEPROM_ADDRESS;
  i2cParam.length = params->length;
  i2cParam.data = params->data;
  i2cParam.lengthAddr = HAL_TWO_BYTE_SIZE;
  i2cParam.internalAddr = params->address;
  i2cParam.f = bspEepromDone;

  bspUserCallbackFunctions = readDone;
  bspEepromState = EEPROM_BUSY;
  HAL_ReadI2cPacket(&i2cParam);
  return 0;
}

/******************************************************************************
Writes number of bytes defined by BSP_I2c_EepromParams_t to EEPROM.
Parameters:
  params - address of BSP_I2c_EepromParams_t defined by user.
  writeDone - address of callback.
Returns:
  -1 - the EEPROM has request that was not completed.
******************************************************************************/
int BSP_WriteEeprom(BSP_I2c_EepromParams_t *params, void (*writeDone)(bool Result))
{
  HAL_I2cParams_t i2cParam;

  if (EEPROM_IDLE != bspEepromState)
    return -1;
  if (((uint32_t)params->address + (uint32_t)params->length) > I2C_EEPROM_DATA_MEMORY_SIZE)
    return -1;

  i2cParam.id = I2C_EEPROM_ADDRESS;
  i2cParam.length = params->length;
  i2cParam.data = params->data;
  i2cParam.lengthAddr = HAL_TWO_BYTE_SIZE;
  i2cParam.internalAddr = params->address;
  i2cParam.f = bspEepromDone;

  bspUserCallbackFunctions = writeDone;
  bspEepromState = EEPROM_BUSY;
  HAL_WriteI2cPacket(&i2cParam);
  return 0;
}

#endif // APP_DISABLE_BSP != 1

//eof i2ceeprom.c
