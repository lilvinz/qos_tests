/**************************************************************************//**
\file  dbgPort.h

\brief Declaration of dbg interface.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    17/12/07 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/
#ifndef _DBGPORT_H
#define _DBGPORT_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <types.h>

/******************************************************************************
 This function is used to send a string through the DBGU channel
 (Very low level debugging)
 Parameters:
   buffer - char string.
 Returns:
   none
******************************************************************************/
void dbgu_print_ascii(const char *buffer);

/******************************************************************************
default handler for spurious interrupt
*******************************************************************************/
#if defined(__GNUC__)
  void default_spurious_handler(void) __attribute__((interrupt ("IRQ")));
#elif defined(__ICCARM__)
  /*__irq __arm*/ void default_spurious_handler(void);
#else
  #error 'Compiler not supported'
#endif

/******************************************************************************
default handler for fast interrupt
*******************************************************************************/
#if defined(__GNUC__)
  void default_fiq_handler(void) __attribute__((interrupt ("FIQ")));
#elif defined(__ICCARM__)
  /*__fiq __arm*/ void default_fiq_handler(void);
#else
  #error 'Compiler not supported'
#endif

/******************************************************************************
default handler for irq
*******************************************************************************/
#if defined(__GNUC__)
  void default_irq_handler(void) __attribute__((interrupt ("IRQ")));
#elif defined(__ICCARM__)
  /*__irq __arm*/ void default_irq_handler(void);
#else
  #error 'Compiler not supported'
#endif

/******************************************************************************
 This function is used to send a uint8_t buffer through the DBGU channel
 (Very low level debugging)
 Parameters:
   buffer - pointer to buffer
   size - size buffer in bytes
 Returns:
   none
******************************************************************************/
void dbgu_print_buff(uint8_t *buffer, uint8_t size);

/******************************************************************************
 This function is used to print a 32-bit value in hex
 Parameters:
   value - parameter for dbgu translation
 Returns:
   none
******************************************************************************/
void dbgu_print_hex8(uint32_t value);

/******************************************************************************
 Configure DBGU
 Parameters:
   none
 Returns:
   none
******************************************************************************/
void configure_dbgu(void);

#endif /* _DBGPORT_H */
