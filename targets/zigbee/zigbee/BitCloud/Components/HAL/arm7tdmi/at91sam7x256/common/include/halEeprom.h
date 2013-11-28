/**************************************************************************//**
\file   halEeprom.h

\brief  The implementation interface of SPI EEPROM.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    3/12/07 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

#ifndef _HALEEPROM_H
#define _HALEEPROM_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <eeprom.h>
#include <gpio.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define EEPROM_SPI_FREQUENCY   10000000ul
#define NPCS_TO_SPCK_EEPROM  0.0000001
#define DELAY_BETWEEN_CONSECUTIVE_TRANSFERS_EEPROM  0
// eeprom instruction format (see at25080 manual)
#define WREN        0x06
#define WRDI        0x04
#define RDSR        0x05
#define WRSR        0x01
#define READ        0x03
#define WRITE       0x02

// status register bits
#define RDY         0x01
#define WEN         0x02
#define BP0         0x04
#define BP1         0x08
#define WPEN        0x80

#define WRITE_PAGE_SIZE        32
#define REMAINDER              0x001F

// eeprom current state
#define  EEPROM_IDLE         0
#define  EEPROM_IS_READ      1
#define  EEPROM_IS_WRITTEN   2

/******************************************************************************
                   Prototypes section
******************************************************************************/
/******************************************************************************
SPI opener.
******************************************************************************/
void halSpiOpener(void);

/******************************************************************************
                 Inline static functions prototypes section.
******************************************************************************/
// Macros for the EEPROM_CS pin manipulation.
HAL_ASSIGN_PIN(EEPROM_CS, A, AT91C_PIO_PA21);

#endif /*_HALEEPROM_H*/
