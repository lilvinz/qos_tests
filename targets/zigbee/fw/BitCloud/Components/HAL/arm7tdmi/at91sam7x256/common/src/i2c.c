/**************************************************************************//**
\file  i2c.c

\brief Provides the functionality of TWI.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    29/08/07 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                   Includes section
******************************************************************************/
#include <i2c.h>
#include <halTaskManager.h>
#if defined(FREE_RTOS)
  #include <FreeRTOS.h>
  #include <task.h>
#endif

/******************************************************************************
                   Prototypes section
******************************************************************************/
#if defined(FREE_RTOS)
  void twiHandler(void) __attribute__((naked));
#endif
void halTransactionCompleteI2c(void);
void halReadByteDoneI2c(void);
void halWriteByteDoneI2c(void);
void halI2cBusError(void);

/******************************************************************************
                   Global variables section
******************************************************************************/
I2cPacketControl_t halI2cPacketControl;
// current state of the i2c request
volatile I2cPacketStates_t halI2cPacketState = I2C_PACKET_CLOSE;

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
TWI interrupt handler.
******************************************************************************/
void twiHandler(void)
{
#if defined(FREE_RTOS)
  portSAVE_CONTEXT();
#endif

  if (I2C_PACKET_READ_DATA == halI2cPacketState)
  { // It is necessary for problem-solving NACK Status Bit Lost.
    // Reading status register is not allowed.
    if (AT91C_BASE_TWI->TWI_IMR & AT91C_TWI_TXCOMP_MASTER)
      halTransactionCompleteI2c();
    else
      halReadByteDoneI2c();
  }
  else
  {
    volatile uint32_t status;

    status = AT91C_BASE_TWI->TWI_SR;
    if ((status & AT91C_TWI_TXCOMP_MASTER) && (AT91C_BASE_TWI->TWI_IMR & AT91C_TWI_TXCOMP_MASTER))
    {
      halTransactionCompleteI2c();
    }
    else if ((status & AT91C_TWI_TXRDY_MASTER) && (AT91C_BASE_TWI->TWI_IMR & AT91C_TWI_TXRDY_MASTER))
    {
      halWriteByteDoneI2c();
    }
    else if ((status & AT91C_TWI_NACK_MASTER) && (AT91C_BASE_TWI->TWI_IMR & AT91C_TWI_NACK_MASTER))
    {
      halI2cBusError();
    }
  }

#if defined(FREE_RTOS)
  /* End the interrupt in the AIC. */
  AT91C_BASE_AIC->AIC_EOICR = 0;

  portRESTORE_CONTEXT();
#endif
}

/******************************************************************************
Inits TWI module.
Parameters:
  i2cMode - the speed of TWI.
Returns:
  none.
******************************************************************************/
void halInitI2c(HAL_i2cMode_t *i2cMode)
{
  if ((uint32_t)twiHandler != AT91C_BASE_AIC->AIC_SVR[AT91C_ID_TWI])
  { /* first init */
    /* Configure PIOs for TWI */
    AT91C_BASE_PIOA->PIO_ASR = AT91C_PA10_TWD | AT91C_PA11_TWCK;
    AT91C_BASE_PIOA->PIO_PDR = AT91C_PA10_TWD | AT91C_PA11_TWCK;
    // enable open drain
    AT91C_BASE_PIOA->PIO_MDER = AT91C_PA10_TWD | AT91C_PA11_TWCK;
    // disable pull-up
    AT91C_BASE_PIOA->PIO_PPUDR = AT91C_PA10_TWD | AT91C_PA11_TWCK;

    //enable the clock of TWI
    AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_TWI;

    /* Enable interrupts */
    /* Disable the interrupt on the interrupt controller */
    AT91C_BASE_AIC->AIC_IDCR = (1 << AT91C_ID_TWI);
    /* Save the interrupt handler routine pointer and the interrupt priority */
    AT91C_BASE_AIC->AIC_SVR[AT91C_ID_TWI] = (uint32_t)twiHandler;
    /* Store the Source Mode Register */
    AT91C_BASE_AIC->AIC_SMR[AT91C_ID_TWI] = AT91C_AIC_SRCTYPE_HIGH_LEVEL | AT91C_AIC_PRIOR_LOWEST;
    /* Clear the interrupt on the interrupt controller */
    AT91C_BASE_AIC->AIC_ICCR = (1 << AT91C_ID_TWI);
    /* Enable the interrupt on the interrupt controller */
    AT91C_BASE_AIC->AIC_IECR = (1 << AT91C_ID_TWI);
  }

  // initialization twi
  // reset and disable all interrupts
  AT91C_BASE_TWI->TWI_CR = AT91C_TWI_SWRST;
  AT91C_BASE_TWI->TWI_IDR = ALL_PERIPHERIAL_INTERRUPT_DISABLE;

  // set clock rate. CLDIV = CHDIV (50% duty cycle)
  AT91C_BASE_TWI->TWI_CWGR =((uint32_t)(i2cMode->clockrate) << 8) | (uint32_t)(i2cMode->clockrate);

  // set master mode
  AT91C_BASE_TWI->TWI_CR = AT91C_TWI_MSEN;
}

/******************************************************************************
The Handler of transmite hold register ready
Parameters:
  none
Returns:
  none.
******************************************************************************/
void halWriteByteDoneI2c(void)
{
  if ((I2C_PACKET_WRITE_DATA == halI2cPacketState) && (halI2cPacketControl.index < halI2cPacketControl.length))
  {
    if ((halI2cPacketControl.index + 1) == halI2cPacketControl.length)
    {
      AT91C_BASE_TWI->TWI_CR = AT91C_TWI_STOP;
      // disable interrupt TXRDY and NACK
      AT91C_BASE_TWI->TWI_IDR = AT91C_TWI_TXRDY_MASTER | AT91C_TWI_NACK_MASTER;
    }
    else
      AT91C_BASE_TWI->TWI_THR = halI2cPacketControl.data[++halI2cPacketControl.index];
  }
  else
  { // dasable interrupt TXRDY and NACK
    AT91C_BASE_TWI->TWI_IDR = AT91C_TWI_TXRDY_MASTER | AT91C_TWI_NACK_MASTER;
  }
}

/******************************************************************************
The Handler of receive hold register ready
Parameters:
  none
Returns:
  none.
******************************************************************************/
void halReadByteDoneI2c(void)
{
  halI2cPacketControl.data[halI2cPacketControl.index++] = AT91C_BASE_TWI->TWI_RHR;
  // read last byte now
  if (halI2cPacketControl.index == (halI2cPacketControl.length-1))
  {
    AT91C_BASE_TWI->TWI_CR = AT91C_TWI_STOP;
    AT91C_BASE_TWI->TWI_IDR = AT91C_TWI_RXRDY;
    AT91C_BASE_TWI->TWI_IER = AT91C_TWI_TXCOMP_MASTER;
  }
}

/******************************************************************************
start write transaction
Parameters:
  none.
Returns:
  none
******************************************************************************/
void halSendStartWriteI2c(void)
{
  // set slave address, internal address size and master write direction
  AT91C_BASE_TWI->TWI_MMR = ((uint32_t)(halI2cPacketControl.addr) << 16) | halI2cPacketControl.lengthAddr;
  AT91C_BASE_TWI->TWI_MMR &= ~AT91C_TWI_MREAD;
  // set internal address
  AT91C_BASE_TWI->TWI_IADR = halI2cPacketControl.internalAddr;
  AT91C_BASE_TWI->TWI_THR = halI2cPacketControl.data[0];
  // set i2c virtual state
  halI2cPacketState = I2C_PACKET_WRITE_DATA;
  if (HAL_SINGLE_DATA_BYTE == halI2cPacketControl.length)
    AT91C_BASE_TWI->TWI_CR = AT91C_TWI_START | AT91C_TWI_STOP;
  else
    AT91C_BASE_TWI->TWI_CR = AT91C_TWI_START;

  // enable transmit hold register empty, transmit complete and NACK detect (error bus)
  AT91C_BASE_TWI->TWI_IER = AT91C_TWI_TXCOMP_MASTER | AT91C_TWI_TXRDY_MASTER | AT91C_TWI_NACK_MASTER;
}

/******************************************************************************
start read transaction
Parameters:
  none.
Returns:
  none
******************************************************************************/
void halSendStartReadI2c(void)
{
  uint32_t dummy;

  AT91C_BASE_TWI->TWI_MMR = ((uint32_t)(halI2cPacketControl.addr) << 16) | halI2cPacketControl.lengthAddr | AT91C_TWI_MREAD;
  // set internal address
  AT91C_BASE_TWI->TWI_IADR = halI2cPacketControl.internalAddr;
  // set i2c virtual state
  halI2cPacketState = I2C_PACKET_READ_DATA;
  // reset receiver
  dummy = AT91C_BASE_TWI->TWI_RHR;
  (void)dummy;
  if (HAL_SINGLE_DATA_BYTE == halI2cPacketControl.length)
  {
    // enable transmit complete interrupt
    AT91C_BASE_TWI->TWI_IER = AT91C_TWI_TXCOMP_MASTER;
    AT91C_BASE_TWI->TWI_CR = AT91C_TWI_START | AT91C_TWI_STOP;
  }
  else
  {
    // enable receiv data ready interrupt
    AT91C_BASE_TWI->TWI_IER = AT91C_TWI_RXRDY;
    AT91C_BASE_TWI->TWI_CR = AT91C_TWI_START;
  }
}

/******************************************************************************
The Handler of NACK
Parameters:
  none.
Returns:
  none.
******************************************************************************/
void halI2cBusError(void)
{
  AT91C_BASE_TWI->TWI_IDR = ALL_PERIPHERIAL_INTERRUPT_DISABLE;
  halI2cPacketState = I2C_PACKET_TRANSAC_FAIL;
  halPostTask(HAL_TWI);
}

/******************************************************************************
The Handler of transaction complete
Parameters:
  none.
Returns:
  none.
******************************************************************************/
void halTransactionCompleteI2c(void)
{
  // disable transaction complete interrupt
  if (I2C_PACKET_READ_DATA == halI2cPacketState)
    halI2cPacketControl.data[halI2cPacketControl.index] = AT91C_BASE_TWI->TWI_RHR;
  AT91C_BASE_TWI->TWI_IDR = AT91C_TWI_TXCOMP_MASTER;
  halI2cPacketState = I2C_PACKET_TRANSAC_SUCCESS;
  halPostTask(HAL_TWI);
}
// eof i2c.c

