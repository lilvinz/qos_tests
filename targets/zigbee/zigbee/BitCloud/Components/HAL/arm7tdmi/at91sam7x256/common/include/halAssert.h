/**************************************************************************//**
\file  halAssert.h

\brief Implementation of arm assert algorithm.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    18/08/08 A. Khromykh - Created
******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

#ifndef _HALASSERT_H
#define _HALASSERT_H

#ifdef  __IAR_SYSTEMS_ICC__
#ifndef _SYSTEM_BUILD
#pragma system_include
#endif
#endif

/******************************************************************************
                   Includes section
******************************************************************************/
#include <AT91SAM7X256.h>
#include <atomic.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define DELAY_VALUE 0x000100ul

#ifdef _SYS_ASSERT_ON_

#define PIO_LEDS   AT91C_PIO_PB19 | AT91C_PIO_PB20 | AT91C_PIO_PB21 | AT91C_PIO_PB22
#define PORT_ID_LEDS   AT91C_ID_PIOB

#if defined(_HAL_ASSERT_INTERFACE_UART0_)
  #define BASE_ADDRESS AT91C_BASE_US0
  #define PIO_SERIAL  AT91C_PA0_RXD0 | AT91C_PA1_TXD0
  #define ID_PORT AT91C_ID_US0
  #define CR      US_CR
  #define IDR     US_IDR
  #define BRGR    US_BRGR
  #define MR      US_MR
  #define PTCR    US_PTCR
  #define CSR     US_CSR
  #define THR     US_THR
#elif defined(_HAL_ASSERT_INTERFACE_UART1_)
  #define BASE_ADDRESS AT91C_BASE_US1
  #define PIO_SERIAL  AT91C_PA5_RXD1 | AT91C_PA6_TXD1
  #define ID_PORT AT91C_ID_US1
  #define CR      US_CR
  #define IDR     US_IDR
  #define BRGR    US_BRGR
  #define MR      US_MR
  #define PTCR    US_PTCR
  #define CSR     US_CSR
  #define THR     US_THR
#elif defined(_HAL_ASSERT_INTERFACE_DBG_PORT_)
  #define BASE_ADDRESS AT91C_BASE_DBGU
  #define PIO_SERIAL  AT91C_PA27_DRXD | AT91C_PA28_DTXD;
  #define ID_PORT 33
  #define CR      DBGU_CR
  #define IDR     DBGU_IDR
  #define BRGR    DBGU_BRGR
  #define MR      DBGU_MR
  #define PTCR    DBGU_PTCR
  #define CSR     DBGU_CSR
  #define THR     DBGU_THR
#else
  #error " Unknown assert interface "
#endif

/******************************************************************************
                   Inline static functions section
******************************************************************************/
INLINE void halAssert(uint8_t condition, uint16_t dbgCode)
{
  ATOMIC_SECTION_ENTER
  if (!(condition))
  {
    uint32_t delay;

    // init led port
    AT91C_BASE_PIOB->PIO_OER = PIO_LEDS;
    // enable clock on port B
    AT91C_BASE_PMC->PMC_PCER = (1 << PORT_ID_LEDS);
    /* Init UART */
    /* Reset and disable receiver */
    /* Configure PIOs for serial port */
    AT91C_BASE_PIOA->PIO_ASR = PIO_SERIAL;
    AT91C_BASE_PIOA->PIO_BSR = 0;
    AT91C_BASE_PIOA->PIO_PDR = PIO_SERIAL;

#if defined(_HAL_ASSERT_INTERFACE_UART0_) || defined(_HAL_ASSERT_INTERFACE_UART1_)
    //enable the clock of serial port
    AT91C_BASE_PMC->PMC_PCER = (1ul << ID_PORT);
#endif

    BASE_ADDRESS->CR = AT91C_US_RSTRX | AT91C_US_RSTTX;

    /* Disable interrupts */
    BASE_ADDRESS->IDR = 0xFFFFFFFF;

    /* === Configure serial link === */
    /* Define the baud rate divisor register [BRGR = MCK / (38400 * 16)] */
    BASE_ADDRESS->BRGR = 78;
    /* Define the USART mode */
    BASE_ADDRESS->MR = AT91C_US_PAR_NONE | AT91C_US_CHMODE_NORMAL | AT91C_US_CHRL_8_BITS;

    /* Disable the RX and TX PDC transfer requests */
    BASE_ADDRESS->PTCR = AT91C_PDC_RXTDIS;
    BASE_ADDRESS->PTCR = AT91C_PDC_TXTDIS;

    /* Enable transmitter */
    BASE_ADDRESS->CR = AT91C_US_TXEN;

    while(1)
    {
      AT91C_BASE_PIOB->PIO_SODR = PIO_LEDS;
      /* Send high byte of message to UART */
      while (!(BASE_ADDRESS->CSR & AT91C_US_TXRDY));
      BASE_ADDRESS->THR = (dbgCode >> 8);
      /* Send low byte of message to UART */
      while (!(BASE_ADDRESS->CSR & AT91C_US_TXRDY));
      BASE_ADDRESS->THR = dbgCode;
      delay = DELAY_VALUE;
      while(delay--);

      AT91C_BASE_PIOB->PIO_CODR = PIO_LEDS;
      delay = (DELAY_VALUE/2);
      while(delay--);
    }
  }
  ATOMIC_SECTION_LEAVE
}

#else /* _SYS_ASSERT_ON_ */
  #define halAssert(condition, dbgCode)
#endif /* _SYS_ASSERT_ON_ */

#endif /* _HALASSERT_H */

// eof halAssert.h
