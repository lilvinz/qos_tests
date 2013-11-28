/************************************************************//**
  \file rs232Controller.h

  \brief Functions for RS232 level converter interface.

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
  History:
    28.09.09 A. Taradov - Created
*****************************************************************/
#ifndef _RS232CONTROLLER_H
#define _RS232CONTROLLER_H

/*****************************************************************
                   Includes section
*****************************************************************/
#include <gpio.h>

/************************************************************//**
\brief Enable RS232 level converter.
****************************************************************/
static inline void BSP_EnableRs232(void)
{
#if BSP_ENABLE_RS232_CONTROL == 1
  #if defined(RCB_ATMEGA128RFA1)
    // ~EN
    GPIO_D4_make_out();
    GPIO_D4_clr();

    // FORCEON
    GPIO_D6_make_out();
    GPIO_D6_set();

    // ~FORCEOFF
    GPIO_D7_make_out();
    GPIO_D7_set();
  #elif defined(RCB230_V31) || defined(RCB230_V32) || defined(RCB230_V331) || defined(RCB231_V402) || \
        defined(RCB231_V411) || defined(RCB212_V532)
    // ~EN
    GPIO_13_make_out();
    GPIO_13_clr();

    // FORCEON
    GPIO_15_make_out();
    GPIO_15_set();

    // ~FORCEOFF
    GPIO_16_make_out();
    GPIO_16_set();
  #elif defined(BOARD_SAM3S_EK)
    // ~EN
    GPIO_PA23_make_out();
    GPIO_PA23_clr();
  #else
    #error 'Unsupported board.'
  #endif
#endif
};

/************************************************************//**
\brief Disable RS232 level converter.
****************************************************************/
static inline void BSP_DisableRs232(void)
{
#if BSP_ENABLE_RS232_CONTROL == 1
  #if defined(RCB_ATMEGA128RFA1)
    GPIO_D4_make_in();
    GPIO_D6_make_in();
    GPIO_D7_make_in();
  #elif defined(RCB230_V31) || defined(RCB230_V32) || defined(RCB230_V331) || defined(RCB231_V402) || \
        defined(RCB231_V411) || defined(RCB212_V532)
    GPIO_13_make_in();
    GPIO_15_make_in();
    GPIO_16_make_in();
  #elif defined(BOARD_SAM3S_EK)
    GPIO_PA23_make_in();
  #else
    #error 'Unsupported board.'
  #endif
#endif
};

#endif /* _RS232CONTROLLER_H */
