/**************************************************************************//**
\file  gpio.h

\brief This module contains a set of functions to manipulate GPIO pins.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    17/08/07 A. Khromykh - Created
******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/
#ifndef _GPIO_H
#define _GPIO_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <types.h>
#include <AT91SAM7X256.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
/******************************************************************************
* void gpioX_set() sets GPIOX pin to logical 1 level.
* void gpioX_clr() clears GPIOX pin to logical 0 level.
* void gpioX_make_in makes GPIOX pin as input.
* void gpioX_make_in makes GPIOX pin as output.
* uint8_t gpioX_read() returns logical level GPIOX pin.
* uint8_t gpioX_state() returns configuration of GPIOX port.
*******************************************************************************/
#define HAL_ASSIGN_PIN(name, port, bit) \
INLINE void  GPIO_##name##_set()         {AT91C_BASE_PIO##port->PIO_SODR = bit;} \
INLINE void  GPIO_##name##_clr()         {AT91C_BASE_PIO##port->PIO_CODR = bit;} \
INLINE uint8_t  GPIO_##name##_read()     {return ((AT91C_BASE_PIO##port->PIO_PDSR & bit) != 0l);} \
INLINE uint8_t  GPIO_##name##_state()    {return ((AT91C_BASE_PIO##port->PIO_OSR & bit) != 0l);} \
INLINE void  GPIO_##name##_make_out()    {AT91C_BASE_PIO##port->PIO_OER = bit;} \
INLINE void  GPIO_##name##_make_in()     {AT91C_BASE_PIO##port->PIO_ODR = bit; AT91C_BASE_PIO##port->PIO_PPUDR = bit;} \
INLINE void  GPIO_##name##_make_pullup() {AT91C_BASE_PIO##port->PIO_PPUER = bit;}\
INLINE void  GPIO_##name##_toggle()      {if (AT91C_BASE_PIO##port->PIO_ODSR & bit) \
                                            AT91C_BASE_PIO##port->PIO_CODR = bit; \
                                          else \
                                            AT91C_BASE_PIO##port->PIO_SODR = bit;}

/******************************************************************************
                   Inline static functions section
******************************************************************************/
#if defined(PLATFORM_SAM7X_EK_RF2XX)
  // USART RTS manipulation macros
  HAL_ASSIGN_PIN(USART_RTS, A, AT91C_PIO_PA3);
  // USART CTS manipulation macros
  HAL_ASSIGN_PIN(USART_CTS, A, AT91C_PIO_PA4);
  // USART DTR manipulation macros
  HAL_ASSIGN_PIN(USART_DTR, B, AT91C_PIO_PB25);
  // USART0------------------------------------
  // USART0 TXD manipulation macros
  HAL_ASSIGN_PIN(USART0_TXD, A, AT91C_PIO_PA1);
  // USART1------------------------------------
  // USART1 TXD manipulation macros
  HAL_ASSIGN_PIN(USART1_TXD, A, AT91C_PIO_PA6);

#elif defined(PLATFORM_ZIGBIT_LAN) || defined(PLATFORM_ZIGBIT_TURBO)
  // USART RTS manipulation macros
  HAL_ASSIGN_PIN(USART_RTS, A, AT91C_PIO_PA8);
  // USART CTS manipulation macros
  HAL_ASSIGN_PIN(USART_CTS, A, AT91C_PIO_PA9);
  // USART DTR manipulation macros
  HAL_ASSIGN_PIN(USART_DTR, B, AT91C_PIO_PB25);
  // USART0------------------------------------
  // USART0 TXD manipulation macros
  HAL_ASSIGN_PIN(USART0_TXD, A, AT91C_PIO_PA1);
  // USART1------------------------------------
  // USART1 TXD manipulation macros
  HAL_ASSIGN_PIN(USART1_TXD, A, AT91C_PIO_PA6);

  // -----Use interrupts pins as gpio--------
  // ---interrupts must be unregistered--------
  // the macros for the manipulation by FIQ pin
  HAL_ASSIGN_PIN(FIQ,  A, AT91C_PIO_PA29);
  // the macros for the manipulation by IRQ0 pin
  HAL_ASSIGN_PIN(IRQ0, A, AT91C_PIO_PA30);
  // the macros for the manipulation by IRQ1 pin
  HAL_ASSIGN_PIN(IRQ1, A, AT91C_PIO_PA14);

#elif defined(PLATFORM_CUSTOM_1) || defined(PLATFORM_CUSTOM_2)
  // USART RTS manipulation macros
  HAL_ASSIGN_PIN(USART_RTS, A, AT91C_PIO_PA3);
  // USART CTS manipulation macros
  HAL_ASSIGN_PIN(USART_CTS, A, AT91C_PIO_PA4);
  // USART DTR manipulation macros
  HAL_ASSIGN_PIN(USART_DTR, B, AT91C_PIO_PB25);
  // USART0------------------------------------
  // USART0 TXD manipulation macros
  HAL_ASSIGN_PIN(USART0_TXD, A, AT91C_PIO_PA1);
  // USART1------------------------------------
  // USART1 TXD manipulation macros
  HAL_ASSIGN_PIN(USART1_TXD, A, AT91C_PIO_PA6);

#endif // PLATFORM__SAM7X_EK_RF2XX

// the macros for first led
HAL_ASSIGN_PIN(FIRST_EK, B, AT91C_PIO_PB19);
// the macros for second led
HAL_ASSIGN_PIN(SECOND_EK, B, AT91C_PIO_PB20);
// the macros for third led
HAL_ASSIGN_PIN(THIRD_EK, B, AT91C_PIO_PB21);
// the macros for fourth led
HAL_ASSIGN_PIN(FOURTH_EK, B, AT91C_PIO_PB22);

// the macros for first led
HAL_ASSIGN_PIN(FIRST_HON, B, AT91C_PIO_PB16);

// the macros for the manipulation by GPIO0
HAL_ASSIGN_PIN(0, A, AT91C_PIO_PA2);
// the macros for the manipulation by GPIO1
HAL_ASSIGN_PIN(1, A, AT91C_PIO_PA3);
// the macros for the manipulation by GPIO2
HAL_ASSIGN_PIN(2, A, AT91C_PIO_PA4);
// the macros for the manipulation by GPIO3
HAL_ASSIGN_PIN(3, A, AT91C_PIO_PA26);
// the macros for the manipulation by GPIO4
HAL_ASSIGN_PIN(4, A, AT91C_PIO_PA27);
// the macros for the manipulation by GPIO5
HAL_ASSIGN_PIN(5, A, AT91C_PIO_PA28);
// the macros for the manipulation by GPIO6
HAL_ASSIGN_PIN(6, B, AT91C_PIO_PB27);
// the macros for the manipulation by GPIO7
HAL_ASSIGN_PIN(7, B, AT91C_PIO_PB28);
// the macros for the manipulation by GPIO8
HAL_ASSIGN_PIN(8, B, AT91C_PIO_PB29);
// the macros for the manipulation by GPIO9
HAL_ASSIGN_PIN(9, B, AT91C_PIO_PB30);

#endif
//eof gpio.h
