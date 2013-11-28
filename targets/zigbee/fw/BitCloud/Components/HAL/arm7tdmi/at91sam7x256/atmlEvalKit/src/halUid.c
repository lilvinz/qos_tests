/**************************************************************************//**
  \file   halUid.c

  \brief  Implementation of UID interface.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      30.12.10 A. Khromykh - Created
 ******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                   Includes section
******************************************************************************/
#include <halRfPio.h>
#include <halRfSpi.h>
#include <uid.h>

/******************************************************************************
                   Types section
******************************************************************************/
/** \brief uid type. */
typedef union
{
  uint64_t uid;
  uint8_t array[sizeof(uint64_t)];
} HalUid_t;

/******************************************************************************
                   Global variables section
******************************************************************************/
static HalUid_t halUid = {.uid = 0ull};

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
 Reads uid from external spi eeprom at25010a.
******************************************************************************/
void halReadUid(void)
{
  uint8_t command = 0x03;
  uint8_t address = 0;
  uint8_t itr;

  GPIO_RF_RST_make_out();
  GPIO_RF_RST_clr();
  GPIO_SPI_CS_clr();

  HAL_WriteByteRfSpi(command);
  HAL_WriteByteRfSpi(address);
  for (itr = 0; itr < sizeof(uint64_t); itr++)
  {
    halUid.array[itr] = HAL_WriteByteRfSpi(address);
  }

  GPIO_SPI_CS_set();
}

/******************************************************************************
 Returns number which was read from external eeprom.
 Parameters:
   id - UID buffer pointer.
 Returns:
   0 - if unique ID has been found without error;
  -1 - if there are some erros during UID discovery.
******************************************************************************/
int HAL_ReadUid(uint64_t *id)
{
  if (!id)
    return -1;

  *id = halUid.uid;
  return 0;
}

// eof uid.c
