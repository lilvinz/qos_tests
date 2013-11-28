/**************************************************************************//**
\file  halUart.c

\brief Implementation of uart hardware-dependent module.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    17/08/07 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/
/******************************************************************************
                   Includes section
******************************************************************************/
#include <usart.h>
#if defined(FREE_RTOS)
  #include <FreeRTOS.h>
  #include <task.h>
#endif

/******************************************************************************
                   Prototypes section
******************************************************************************/
#if defined(FREE_RTOS)
  void usart0Handler(void)  __attribute__((naked));
  void usart1Handler(void)  __attribute__((naked));
#endif

void halPostUsartTask(HalUsartTaskId_t taskId);

/*****************************************************************************
                   Implementations section
******************************************************************************/
/**************************************************************************//**
  \brief Interrupt handler usart0

  \param none
  \return none.
******************************************************************************/
void usart0Handler(void)
{
#if defined(FREE_RTOS)
  portSAVE_CONTEXT();
#endif

  uint32_t status;
  uint32_t data;

  // get usart status register
  status = AT91C_BASE_US0->US_CSR;

  // overrun, frame or parity error
  if (status & (AT91C_US_OVRE | AT91C_US_FRAME | AT91C_US_PARE))
  {
    AT91C_BASE_US0->US_CR = AT91C_US_RSTRX | AT91C_US_RSTTX | AT91C_US_RSTSTA;
    #if defined(_USE_USART_ERROR_EVENT_)
      halUsartSaveErrorReason(USART_CHANNEL_0, status);
      halPostUsartTask(HAL_USART_TASK_USART0_ERR);
    #endif
  }
  else if ((status & AT91C_US_RXRDY) && (AT91C_BASE_US0->US_IMR & AT91C_US_RXRDY))
  {
    data = AT91C_BASE_US0->US_RHR;
    halUsartRxBufferFiller(USART_CHANNEL_0, data);
    halPostUsartTask(HAL_USART_TASK_USART0_RXC);
  }
  else if ((status & AT91C_US_TXEMPTY) && (AT91C_BASE_US0->US_IMR & AT91C_US_TXEMPTY))
  {
    halDisableUsartTxcInterrupt(USART_CHANNEL_0);
    halPostUsartTask(HAL_USART_TASK_USART0_TXC);
  }
  else if ((status & AT91C_US_TXRDY) && (AT91C_BASE_US0->US_IMR & AT91C_US_TXRDY))
  {
    // We must disable the interrupt because we must "break" context.
    halDisableUsartDremInterrupt(USART_CHANNEL_0);
    halPostUsartTask(HAL_USART_TASK_USART0_DRE);
  }

#if defined(FREE_RTOS)
  /* End the interrupt in the AIC. */
  AT91C_BASE_AIC->AIC_EOICR = 0;

  portRESTORE_CONTEXT();
#endif
}

/**************************************************************************//**
  \brief Interrupt handler usart1

  \param none
  \return none.
******************************************************************************/
void usart1Handler(void)
{
#if defined(FREE_RTOS)
  portSAVE_CONTEXT();
#endif

  uint32_t status;
  uint32_t  data;

  // get usart status register
  status = AT91C_BASE_US1->US_CSR;

  // overrun, frame or parity error
  if (status & (AT91C_US_OVRE | AT91C_US_FRAME | AT91C_US_PARE))
  {
    AT91C_BASE_US1->US_CR = AT91C_US_RSTRX | AT91C_US_RSTTX | AT91C_US_RSTSTA;
    #if defined(_USE_USART_ERROR_EVENT_)
      halUsartSaveErrorReason(USART_CHANNEL_1, status);
      halPostUsartTask(HAL_USART_TASK_USART1_ERR);
    #endif
  }
  else if ((status & AT91C_US_RXRDY) && (AT91C_BASE_US1->US_IMR & AT91C_US_RXRDY))
  {
    data = AT91C_BASE_US1->US_RHR;
    halUsartRxBufferFiller(USART_CHANNEL_1, data);
    halPostUsartTask(HAL_USART_TASK_USART1_RXC);
  }
  else if ((status & AT91C_US_TXEMPTY) && (AT91C_BASE_US1->US_IMR & AT91C_US_TXEMPTY))
  {
    halDisableUsartTxcInterrupt(USART_CHANNEL_1);
    halPostUsartTask(HAL_USART_TASK_USART1_TXC);
  }
  else if ((status & AT91C_US_TXRDY) && (AT91C_BASE_US1->US_IMR & AT91C_US_TXRDY))
  {
    // We must disable the interrupt because we must "break" context.
    halDisableUsartDremInterrupt(USART_CHANNEL_1);
    halPostUsartTask(HAL_USART_TASK_USART1_DRE);
  }

#if defined(FREE_RTOS)
  /* End the interrupt in the AIC. */
  AT91C_BASE_AIC->AIC_EOICR = 0;

  portRESTORE_CONTEXT();
#endif
}

/**************************************************************************//**
  \brief Sets USART module parameters.

  \param descriptor - pointer to usart descriptor.
  \return none.
******************************************************************************/
void halFoundationsInit(HAL_UsartDescriptor_t *descriptor)
{
  register uint32_t handler;
  uint8_t usartID;

  if (USART_CHANNEL_0 == descriptor->tty)
  {
    /* Configure PIOs for USART0 */
    AT91C_BASE_PIOA->PIO_ASR = AT91C_PA0_RXD0 | AT91C_PA1_TXD0;
    AT91C_BASE_PIOA->PIO_PDR = AT91C_PA0_RXD0 | AT91C_PA1_TXD0;
    switch (descriptor->mode)
    {
      case USART_MODE_SYNC:
          /* Configure PIOs for SCK0 */
          AT91C_BASE_PIOA->PIO_ASR = AT91C_PA2_SCK0;
          AT91C_BASE_PIOA->PIO_PDR = AT91C_PA2_SCK0;
        break;
      case USART_MODE_RS485:
          /* Configure PIOs for RTS0 */
          AT91C_BASE_PIOA->PIO_ASR = AT91C_PA3_RTS0;
          AT91C_BASE_PIOA->PIO_PDR = AT91C_PA3_RTS0;
        break;
      default:
        break;
    }
    handler = (uint32_t)usart0Handler;
    usartID = AT91C_ID_US0;
  }
  else
  {
    /* Configure PIOs for USART1 */
    AT91C_BASE_PIOA->PIO_ASR = AT91C_PA5_RXD1 | AT91C_PA6_TXD1;
    AT91C_BASE_PIOA->PIO_PDR = AT91C_PA5_RXD1 | AT91C_PA6_TXD1;
    switch (descriptor->mode)
    {
      case USART_MODE_SYNC:
          /* Configure PIOs for SCK1 */
          AT91C_BASE_PIOA->PIO_ASR = AT91C_PA7_SCK1;
          AT91C_BASE_PIOA->PIO_PDR = AT91C_PA7_SCK1;
        break;
      case USART_MODE_RS485:
          /* Configure PIOs for RTS1 */
          AT91C_BASE_PIOA->PIO_ASR = AT91C_PA8_RTS1;
          AT91C_BASE_PIOA->PIO_PDR = AT91C_PA8_RTS1;
        break;
      default:
        break;
    }
    handler = (uint32_t)usart1Handler;
    usartID = AT91C_ID_US1;
  }

  /* enable the clock of USART */
  AT91C_BASE_PMC->PMC_PCER = 1ul << usartID;
  /* AIC initialization for USART */
  /* Disable the interrupt on the interrupt controller */
  AT91C_BASE_AIC->AIC_IDCR = 1ul << usartID;
  /* Save the interrupt handler routine pointer and the interrupt priority */
  AT91C_BASE_AIC->AIC_SVR[usartID] = handler;
  /* Store the Source Mode Register */
  AT91C_BASE_AIC->AIC_SMR[usartID] = AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL | AT91C_AIC_PRIOR_LOWEST;
  /* Clear the interrupt on the interrupt controller */
  AT91C_BASE_AIC->AIC_ICCR = 1ul << usartID;
  /* Enable the interrupt on the interrupt controller */
  AT91C_BASE_AIC->AIC_IECR = 1ul << usartID;
}

/**************************************************************************//**
  \brief Sets USART module parameters.

  \param
    descriptor - USART module descriptor.
  \return
   none.
******************************************************************************/
void halSetUsartConfig(HAL_UsartDescriptor_t *descriptor)
{
  uint32_t usartModeRegister = 0ul;

  halFoundationsInit(descriptor);

  usartModeRegister = descriptor->parity | AT91C_US_CHMODE_NORMAL | descriptor->dataLength |
                      descriptor->stopbits | descriptor->mode | AT91C_US_OVER;
  if (USART_MODE_SYNC == descriptor->mode)
    usartModeRegister |= descriptor->syncMode;

  #ifdef ENABLE_LOCAL_LOOP_BACK
    usartModeRegister |= AT91C_US_CHMODE_LOCAL;
  #endif
  descriptor->tty->US_MR = usartModeRegister;
  descriptor->tty->US_BRGR = descriptor->baudrate;
  descriptor->tty->US_IDR = ALL_PERIPHERIAL_INTERRUPT_DISABLE;

  // disable USART DMA
  descriptor->tty->US_PTCR = AT91C_PDC_RXTDIS | AT91C_PDC_TXTDIS;
  // enable Rx interrupt
  descriptor->tty->US_IER = AT91C_US_RXRDY;
  descriptor->tty->US_CR =  AT91C_US_RSTRX | AT91C_US_RSTTX;
  descriptor->tty->US_CR = AT91C_US_TXEN | AT91C_US_RXEN;
}

// eof halUsart.c
