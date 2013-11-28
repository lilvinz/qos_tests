/*****************************************************************************//**
\file  halUsart.h

\brief Declarations of usart hardware-dependent module.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    17/08/07 A. Khromykh - Created
**********************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/
#ifndef _HAL_USART_H
#define _HAL_USART_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <halClkCtrl.h>
#include <halTaskManager.h>
#include <stddef.h>
#include <types.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define USART_CHANNEL_1     AT91C_BASE_US1
#define USART_CHANNEL_0     AT91C_BASE_US0

#define HAL_GET_INDEX_BY_CHANNEL(channel)  ((channel - USART_CHANNEL_0) >> 14)

/******************************************************************************
                   Types section
******************************************************************************/
// usart channel
typedef AT91PS_USART UsartChannel_t;

// usart data speed
typedef enum
{
  USART_BAUDRATE_1200 = F_CPU / (8ul*(2ul-1ul)*1200ul), // 1200 baud rate
  USART_BAUDRATE_2400 = F_CPU / (8ul*(2ul-1ul)*2400ul), // 2400 baud rate
  USART_BAUDRATE_4800 = F_CPU / (8ul*(2ul-1ul)*4800ul), // 4800 baud rate
  USART_BAUDRATE_9600 = F_CPU / (8ul*(2ul-1ul)*9600ul), // 9600 baud rate
  USART_BAUDRATE_19200 = F_CPU / (8ul*(2ul-1ul)*19200ul), // 19200 baud rate
  USART_BAUDRATE_38400 = F_CPU / (8ul*(2ul-1ul)*38400ul), // 38400 baud rate
  USART_BAUDRATE_57600 = F_CPU / (8ul*(2ul-1ul)*57600ul), // 57600 baud rate
  USART_BAUDRATE_115200 = F_CPU / (8ul*(2ul-1ul)*115200ul), // 115200 baud rate
  USART_SYNC_BAUDRATE_1200 = F_CPU / 1200ul,
  USART_SYNC_BAUDRATE_2400 = F_CPU / 2400ul,
  USART_SYNC_BAUDRATE_4800 = F_CPU / 4800ul,
  USART_SYNC_BAUDRATE_9600  = F_CPU / 9600ul,
  USART_SYNC_BAUDRATE_38400 = F_CPU / 38400ul,
  USART_SYNC_BAUDRATE_57600 = F_CPU / 57600ul,
  USART_SYNC_BAUDRATE_115200 = F_CPU / 115200ul
} UsartBaudRate_t;

// USART data length
typedef enum
{
  USART_DATA5 = AT91C_US_CHRL_5_BITS, // 5 bits data length
  USART_DATA6 = AT91C_US_CHRL_6_BITS, // 6 bits data length
  USART_DATA7 = AT91C_US_CHRL_7_BITS, // 7 bits data length
  USART_DATA8 = AT91C_US_CHRL_8_BITS, // 8 bits data length
} UsartData_t;

// parity mode
typedef enum
{
  USART_PARITY_NONE  = AT91C_US_PAR_NONE,  // Non parity mode
  USART_PARITY_EVEN  = AT91C_US_PAR_EVEN,  // Even parity mode
  USART_PARITY_ODD   = AT91C_US_PAR_ODD,   // Odd parity mode
  USART_PARITY_SPACE = AT91C_US_PAR_SPACE, // Space parity mode
  USART_PARITY_MARK  = AT91C_US_PAR_MARK,  // Mark parity mode
} UsartParity_t;

// number of stop bits
typedef enum
{
  USART_STOPBIT_1  = AT91C_US_NBSTOP_1_BIT,   // 1 stop bits mode
  USART_STOPBIT_15 = AT91C_US_NBSTOP_15_BIT, // 1.5 stop bits mode
  USART_STOPBIT_2  = AT91C_US_NBSTOP_2_BIT    // 2 stop bits mode
} UsartStopBits_t;

// dummy parameter. only for avr compatibility.
typedef enum
{
  USART_EDGE_MODE_FALLING = 0,
  USART_EDGE_MODE_RISING  = 1
} UsartEdgeMode_t;

// clock is output in master mode else input
typedef enum
{
  USART_CLK_MODE_MASTER = AT91C_US_CKLO,
  USART_CLK_MODE_SLAVE  = AT91C_US_CLKS_EXT
} UsartClkMode_t;

// USART mode
typedef enum
{
  USART_MODE_ASYNC = AT91C_US_USMODE_NORMAL,
  USART_MODE_RS485 = AT91C_US_USMODE_RS485,
  USART_MODE_SYNC  = AT91C_US_SYNC
} UsartMode_t;

#if defined(_USE_USART_ERROR_EVENT_)
  // usart receiver error reason
  typedef enum
  {
    FRAME_ERROR,
    DATA_OVERRUN,
    PARITY_ERROR
  } UsartErrorReason_t;
#endif

// USART task IDs.
typedef enum
{
  #if defined(HAL_USE_USART_CHANNEL_0)
    HAL_USART_TASK_USART0_DRE,
    HAL_USART_TASK_USART0_TXC,
    HAL_USART_TASK_USART0_RXC,
    #if defined(_USE_USART_ERROR_EVENT_)
      HAL_USART_TASK_USART0_ERR,
    #endif
  #endif

  #if defined(HAL_USE_USART_CHANNEL_1)
    HAL_USART_TASK_USART1_DRE,
    HAL_USART_TASK_USART1_TXC,
    HAL_USART_TASK_USART1_RXC,
    #if defined(_USE_USART_ERROR_EVENT_)
      HAL_USART_TASK_USART1_ERR,
    #endif
  #endif

  HAL_USART_TASKS_NUMBER
} HalUsartTaskId_t;

// usart control
typedef struct
{
  volatile uint16_t txPointOfRead;
  volatile uint16_t txPointOfWrite;
  volatile uint16_t rxPointOfRead;
  volatile uint16_t rxPointOfWrite;
  volatile uint16_t rxBytesInBuffer;
  uint8_t  usartShiftRegisterEmpty;
#if defined(_USE_USART_ERROR_EVENT_)
  uint8_t  errorReason;
#endif
} HalUsartService_t;

/******************************************************************************
                   Prototypes section
******************************************************************************/
/**************************************************************************//**
\brief Puts the byte received to the cyclic buffer.

\param[in]
  tty - channel number.
\param[in]
  data - data to put.
******************************************************************************/
void halUsartRxBufferFiller(UsartChannel_t tty, uint8_t data);

/**************************************************************************//**
\brief Checks the channel number.

\param[in]
  channel - channel to be verified.

\return
  true if channel is possible, \n
  false otherwise.
******************************************************************************/
bool halIsUsartChannelCorrect(UsartChannel_t channel);

#if defined(_USE_USART_ERROR_EVENT_)
/**************************************************************************//**
\brief Save status register for analyzing of the error reason.

\param[in]
  tty - channel number.
\param[in]
  status - usart status register.
******************************************************************************/
void halUsartSaveErrorReason(UsartChannel_t tty, uint8_t status);
#endif


/******************************************************************************
                   Inline static functions section
******************************************************************************/
/**************************************************************************//**
  \brief Disables USART channel

  \param tty - number of USART channel.
  \return none.
******************************************************************************/
INLINE void halCloseUsart(UsartChannel_t tty)
{
  //disable the clock of USART
  if (USART_CHANNEL_0 == tty)
    AT91C_BASE_PMC->PMC_PCDR = 1ul << AT91C_ID_US0;
  else
    AT91C_BASE_PMC->PMC_PCDR = 1ul << AT91C_ID_US1;
}

/**************************************************************************//**
  \brief Enables data register empty interrupt

  \param tty - number of USART channel.
  \return none.
******************************************************************************/
INLINE void halEnableUsartDremInterrupt(UsartChannel_t tty)
{
  tty->US_IER = AT91C_US_TXRDY;
}

/**************************************************************************//**
  \brief Disables data register empty interrupt

  \param tty - number of USART channel.
  \return none.
******************************************************************************/
INLINE void halDisableUsartDremInterrupt(UsartChannel_t tty)
{
  tty->US_IDR = AT91C_US_TXRDY;
}

/**************************************************************************//**
  \brief Enables transmit complete interrupt

  \param tty - number of USART channel.
  \return none.
******************************************************************************/
INLINE void halEnableUsartTxcInterrupt(UsartChannel_t tty)
{
  tty->US_IER = AT91C_US_TXEMPTY;
}

/**************************************************************************//**
  \brief Disables transmit complete interrupt

  \param tty - number of USART channel.
  return none.
******************************************************************************/
INLINE void halDisableUsartTxcInterrupt(UsartChannel_t tty)
{
  tty->US_IDR = AT91C_US_TXEMPTY;
}

/**************************************************************************//**
  \brief Enables receive complete interrupt

  \param tty - number of USART channel.
  \return none.
******************************************************************************/
INLINE void halEnableUsartRxcInterrupt(UsartChannel_t tty)
{
  tty->US_IER = AT91C_US_RXRDY;
}

/**************************************************************************//**
  \brief Disables receive complete interrupt

  \param tty - number of USART channel.
  \return none.
******************************************************************************/
INLINE void halDisableUsartRxcInterrupt(UsartChannel_t tty)
{
  tty->US_IDR = AT91C_US_RXRDY;
}

/**************************************************************************//**
  \brief Puts byte to data register of USART

  \param tty - number of USART channel.
         data - byte to send.
  \return none.
******************************************************************************/
INLINE void halSendUsartByte(UsartChannel_t tty, uint8_t data)
{
  tty->US_THR = data;
}

#endif // _HAL_USART_H
//eof halUsart.h

