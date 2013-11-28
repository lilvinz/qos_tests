/*****************************************************************************//**
\file   halRfPio.h

\brief  AT86RF230 control pins declarations.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    29/05/07 A. Luzhetsky - Created
    24/08/07 A. Khromykh - modified (for arm)
**********************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

#ifndef _HALRFPIO_H
#define _HALRFPIO_H

/******************************************************************************
                        Includes section.
******************************************************************************/
#include <gpio.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
// RF chip select pin
#define SPI_RF_NPC        AT91C_PIO_PA14
// RF sleep pin
#define SPI_RF_SLP        AT91C_PIO_PA8
// RF reset pin
#define SPI_RF_RST        AT91C_PIO_PA9

/******************************************************************************
                 Inline static functions prototypes section.
******************************************************************************/
// Macros for the RF_SLP_TR pin manipulation.
HAL_ASSIGN_PIN(RF_SLP_TR, A, SPI_RF_SLP);
// Macros for the RF_RST pin manipulation.
HAL_ASSIGN_PIN(RF_RST, A, SPI_RF_RST);
// Macros for the SPI_CS pin manipulation.
HAL_ASSIGN_PIN(SPI_CS, A, SPI_RF_NPC);

#endif // _HALRFPIO_H
// eof halRfPio.h
