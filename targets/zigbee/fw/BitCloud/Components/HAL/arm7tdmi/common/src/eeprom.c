/**************************************************************************//**
\file  eeprom.c

\brief Implementation of eeprom interface.

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
#include <halEeprom.h>
#include <spi.h>

/******************************************************************************
                   External variables section
******************************************************************************/
extern void (*halEepromCallback)();
extern HAL_EepromParams_t localCopy;
extern void (*eepromAction)();
extern uint8_t halSpiEepromState;

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
Reads some number of bytes defined by BSP_Spi_EepromParams_t from the EEPROM.
Parameters:
  params - address of BSP_Spi_EepromParams_t defined by user.
  readDone - address of callback.
Returns:
  -1 - wrong parameters
  -2 - EEPROM is busy
   0 - success
******************************************************************************/
int HAL_ReadEeprom(HAL_EepromParams_t *params, void (*readDone)())
{
  if (EEPROM_IDLE != halSpiEepromState)
    return -2;
  if (NULL == params)
    return -1;
  if (((uint32_t)params->address + (uint32_t)params->length) > EEPROM_DATA_MEMORY_SIZE)
    return -1;

  halEepromCallback = readDone;
  halSpiEepromState = EEPROM_IS_READ;
  localCopy = *params;
  halSpiOpener();
  return 0;
}

/******************************************************************************
Writes number of bytes defined by BSP_Spi_EepromParams_t to EEPROM.
Parameters:
  params - address of BSP_Spi_EepromParams_t defined by user.
  writeDone - address of callback.
Returns:
  -1 - the EEPROM has request that was not completed.
  -2 - EEPROM is busy
   0 - success
******************************************************************************/
int HAL_WriteEeprom(HAL_EepromParams_t *params, void (*writeDone)())
{
  if (EEPROM_IDLE != halSpiEepromState)
    return -2;
  if (NULL == params)
    return -1;
  if (((uint32_t)params->address + (uint32_t)params->length) > EEPROM_DATA_MEMORY_SIZE)
    return -1;

  halEepromCallback = writeDone;
  halSpiEepromState = EEPROM_IS_WRITTEN;
  localCopy = *params;
  halSpiOpener();
  return 0;
}

/******************************************************************************
Checks the eeprom state.

Returns:
  true  - eeprom is busy;
  false - eeprom is free;
******************************************************************************/
bool HAL_IsEepromBusy(void)
{
  if (EEPROM_IDLE != halSpiEepromState)
    return true;
  else
    return false;
}
