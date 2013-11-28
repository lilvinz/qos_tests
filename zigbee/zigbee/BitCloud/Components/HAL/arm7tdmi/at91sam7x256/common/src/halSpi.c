/**************************************************************************//**
\file  halSpi.c

\brief SPI interface routines.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    24/08/07 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                   Includes section
******************************************************************************/
#include <halRfSpi.h>
#include <spi.h>
#include <halRfPio.h>
#include <halTaskManager.h>
#if defined(FREE_RTOS)
  #include <FreeRTOS.h>
  #include <task.h>
#endif

/******************************************************************************
                   Define(s) section
******************************************************************************/
// set cs pin
#define SPI_CS0_SELECT 0x0Eul
#define SPI_CS1_SELECT 0x0Dul
#define SPI_CS2_SELECT 0x0Bul
#define SPI_CS3_SELECT 0x07ul

#define RF_SPI_BAUDRATE  SPI_BAUDRATE(SPI_FREQUENCY)
#define RF_SPI_DLYBS  SPI_DLYBS(NPCS_TO_SPCK_RF230)
#define RF_SPI_DLYBCT  SPI_DLYBCT(DELAY_BETWEEN_CONSECUTIVE_TRANSFERS_RF230)
#define SPI_RF_CS_SELECT  SPI_CS2_SELECT
#define SPI_RF_CS         CS2

/******************************************************************************
                   Prototypes section
******************************************************************************/
#if defined(FREE_RTOS)
  void spi0Handler(void)  __attribute__((naked));
  void spi1Handler(void)  __attribute__((naked));
#endif

/******************************************************************************
                   Global variable section
******************************************************************************/
void (* tx_rf_callback)(void)   = NULL;
void (* rx_rf_callback)(void)   = NULL;
void (* mem_spi_callback)(void) = NULL;
volatile uint8_t halSpiCurrentState = BUS_NOINIT;

/******************************************************************************
                   Implementations section
******************************************************************************/
/*****************************************************************************
***************** SPI0 implementation for radio ******************************
******************************************************************************/
/******************************************************************************
  interrupt handler spi0 (spi for connection rf chip)
  Parameters:
   none
  Returns:
   none.
******************************************************************************/
void spi0Handler(void)
{
#if defined(FREE_RTOS)
  portSAVE_CONTEXT();
#endif

  volatile uint32_t status;

  // get spi status register
  status = AT91C_BASE_SPI0->SPI_SR;

  if ((status & AT91C_SPI_TXBUFE) && (AT91C_BASE_SPI0->SPI_IMR & AT91C_SPI_TXBUFE))
  {
    AT91C_BASE_SPI0->SPI_IDR = AT91C_SPI_TXBUFE;
    halPostTask(HAL_SPI0_TXBUFE);
  }
  else if ((status & AT91C_SPI_RXBUFF) && (AT91C_BASE_SPI0->SPI_IMR & AT91C_SPI_RXBUFF))
  {
    AT91C_BASE_SPI0->SPI_IDR = AT91C_SPI_RXBUFF;
    halPostTask(HAL_SPI0_RXBUFF);
  }

#if defined(FREE_RTOS)
  /* End the interrupt in the AIC. */
  AT91C_BASE_AIC->AIC_EOICR = 0;

  portRESTORE_CONTEXT();
#endif
}

/******************************************************************************
  Performs hardware initialization of SPI0 interface.
  Parameters:
   SpiMode - the address of a structure including spi0 work mode.
******************************************************************************/
void halInitRfSpi(HAL_RfSpiMode_t *SpiMode)
{
  /* Configure PIOs for SPI */
  AT91C_BASE_PIOA->PIO_ASR = AT91C_PA16_SPI0_MISO | AT91C_PA17_SPI0_MOSI |
                             AT91C_PA18_SPI0_SPCK;
  AT91C_BASE_PIOA->PIO_PDR = AT91C_PA16_SPI0_MISO | AT91C_PA17_SPI0_MOSI |
                             AT91C_PA18_SPI0_SPCK;
  if (HAL_SPI_HARDWARE_CS == SpiMode->csmode)
  {
    /* enable hardware cs */
    AT91C_BASE_PIOA->PIO_ASR = SPI_RF_NPC;
    AT91C_BASE_PIOA->PIO_PDR = SPI_RF_NPC;
  }
  else
  {
    /* disable hardware cs */
    AT91C_BASE_PIOA->PIO_OER = SPI_RF_NPC;
    AT91C_BASE_PIOA->PIO_PER = SPI_RF_NPC;
    AT91C_BASE_PIOA->PIO_SODR = SPI_RF_NPC;
  }

  //enable the clock of SPI
  AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_SPI0;

  /* initialization spi
     reset and disable */
  AT91C_BASE_SPI0->SPI_CR = AT91C_SPI_SPIDIS | AT91C_SPI_SWRST;
  /* setup spi mode master, first peripheral active */
  AT91C_BASE_SPI0->SPI_MR = AT91C_SPI_MSTR | (SPI_RF_CS_SELECT << 16) | AT91C_SPI_MODFDIS;
  /* CPOL = 0, NCPHA = 1. Read bit rising edge. Set bit falling edge. SPI mode 0.
     speed spi 8 MHz, Delay between NPCS and SPCK transition 300 ns.  */
  AT91C_BASE_SPI0->SPI_CSR[SPI_RF_CS] = AT91C_SPI_NCPHA | AT91C_SPI_BITS_8 |
                                       (RF_SPI_BAUDRATE <<  8) | (RF_SPI_DLYBS << 16) | (RF_SPI_DLYBCT << 24);
  /* disable all spi interrupt */
  AT91C_BASE_SPI0->SPI_IDR = ALL_PERIPHERIAL_INTERRUPT_DISABLE;

  /* Enable interrupts */
  /* Disable the interrupt on the interrupt controller */
  AT91C_BASE_AIC->AIC_IDCR = (1 << AT91C_ID_SPI0);
  /* Save the interrupt handler routine pointer and the interrupt priority */
  AT91C_BASE_AIC->AIC_SVR[AT91C_ID_SPI0] = (uint32_t)spi0Handler;
  /* Store the Source Mode Register */
  AT91C_BASE_AIC->AIC_SMR[AT91C_ID_SPI0] = AT91C_AIC_SRCTYPE_HIGH_LEVEL | AT91C_AIC_PRIOR_LOWEST;
  /* Clear the interrupt on the interrupt controller */
  AT91C_BASE_AIC->AIC_ICCR = (1 << AT91C_ID_SPI0);
  /* Enable the interrupt on the interrupt controller */
  AT91C_BASE_AIC->AIC_IECR = (1 << AT91C_ID_SPI0);

  if (HAL_SPI_DMA == SpiMode->dmamode)
  {
    /* enable spi DMA */
    AT91C_BASE_SPI0->SPI_PTCR = AT91C_PDC_RXTEN | AT91C_PDC_TXTEN;
  }
  else
  {
    /* disable spi DMA */
    AT91C_BASE_SPI0->SPI_PTCR = AT91C_PDC_RXTDIS | AT91C_PDC_TXTDIS;
  }
  /* enable spi */
  AT91C_BASE_SPI0->SPI_CR = AT91C_SPI_SPIEN;
}

/******************************************************************************
  Performs hwi initialization of rf SPI0 interface.
  Parameters:
    SpiMode - the address of a structure including spi work mode.
******************************************************************************/
void HAL_InitRfSpi(HAL_RfSpiMode_t *SpiMode)
{
  halInitRfSpi(SpiMode);
}

/******************************************************************************
  Writes/reads byte to/from SPI.
  parameters: value - byte to write.
  Returns:    the dword which was read.
******************************************************************************/
uint32_t HAL_WriteByteRfSpi(uint8_t value)
{
  AT91C_BASE_SPI0->SPI_TDR = (uint32_t)value; // Write data.
  while (!((AT91C_BASE_SPI0->SPI_SR) & AT91C_SPI_RDRF));
  return AT91C_BASE_SPI0->SPI_RDR;
}

/******************************************************************************
  Writes data to SPI0.
  parameters:
    data - pointer to data
    size - size data in bytes
    callback - the address of a function if a application wishes to be notified about
             a bus transaction complete the event or NULL.
******************************************************************************/
void HAL_WriteDataRfSpi(uint8_t *data, uint16_t size, void (*callback)())
{
  tx_rf_callback = callback;
  AT91C_BASE_SPI0->SPI_TPR = (uint32_t)data;
  AT91C_BASE_SPI0->SPI_TCR = size;
  AT91C_BASE_SPI0->SPI_IER = AT91C_SPI_TXBUFE;
}

/******************************************************************************
  Enable reads data from SPI0.
  parameters:
    data - pointer to data
    size - size data in bytes
    callback - the address of a function if a application wishes to be notified about
             a bus transaction complete the event or NULL.
******************************************************************************/
void HAL_EnableReadDataRfSpi(uint8_t *data, uint16_t size, void (*callback)())
{
  rx_rf_callback = callback;
  AT91C_BASE_SPI0->SPI_RPR = (uint32_t)data;
  AT91C_BASE_SPI0->SPI_RCR = size;
  AT91C_BASE_SPI0->SPI_IER = AT91C_SPI_RXBUFF;
}

/******************************************************************************
  Interrupt handler of SPI0 dma buffer empty has occured.
******************************************************************************/
void halRfTxCompleteHandler(void)
{
   if (NULL != tx_rf_callback)
     tx_rf_callback();
}

/******************************************************************************
  Interrupt handler of SPI0 dma buffer full has occured.
******************************************************************************/
void halRfRxCompleteHandler(void)
{
   if (NULL != rx_rf_callback)
     rx_rf_callback();
}

/*****************************************************************************
***************** SPI1 implementation for the others *************************
******************************************************************************/
/******************************************************************************
  interrupt handler spi1 (spi for peripherial memory chip)
******************************************************************************/
void spi1Handler(void)
{
#if defined(FREE_RTOS)
  portSAVE_CONTEXT();
#endif

  // get spi status register
  uint32_t status = AT91C_BASE_SPI1->SPI_SR;

  if ((status & AT91C_SPI_RXBUFF) && (AT91C_BASE_SPI1->SPI_IMR & AT91C_SPI_RXBUFF))
  {
    AT91C_BASE_SPI1->SPI_IDR = AT91C_SPI_RXBUFF;
    // transaction is ended
    halPostTask(HAL_SPI1_RXBUFF);
  } else if ((status & AT91C_SPI_TXBUFE) && (AT91C_BASE_SPI1->SPI_IMR & AT91C_SPI_TXBUFE))
  {
    AT91C_BASE_SPI1->SPI_IDR = AT91C_SPI_TXBUFE;
    AT91C_BASE_SPI1->SPI_IER = AT91C_SPI_TXEMPTY;
  }
  else if ((status & AT91C_SPI_TXEMPTY) && (AT91C_BASE_SPI1->SPI_IMR & AT91C_SPI_TXEMPTY))
  {
    AT91C_BASE_SPI1->SPI_IDR = AT91C_SPI_TXEMPTY;
    // transmit buffer and shift register are empty
    halPostTask(HAL_SPI1_TXBUFE);
  }

#if defined(FREE_RTOS)
  /* End the interrupt in the AIC. */
  AT91C_BASE_AIC->AIC_EOICR = 0;

  portRESTORE_CONTEXT();
#endif
}

/******************************************************************************
  Performs hardware initialization of SPI1(Mem) interface.
  Parameters:
   descrp - pointer to the spi descriptor.
******************************************************************************/
void halInitMemSpi(HAL_SpiDescriptor_t *descrp)
{
  /* Configure PIOs for SPI */
  AT91C_BASE_PIOA->PIO_BSR = AT91C_PA23_SPI1_MOSI | AT91C_PA24_SPI1_MISO |
                             AT91C_PA22_SPI1_SPCK;
  AT91C_BASE_PIOA->PIO_PDR = AT91C_PA23_SPI1_MOSI | AT91C_PA24_SPI1_MISO |
                             AT91C_PA22_SPI1_SPCK;

  /* enable the clock of SPI */
  AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_SPI1;

  /* initialization spi */
  /* reset and disable  */
  AT91C_BASE_SPI1->SPI_CR = AT91C_SPI_SPIDIS | AT91C_SPI_SWRST;
  /* setup spi mode master, fixed mode */
  AT91C_BASE_SPI1->SPI_MR = AT91C_SPI_MSTR | AT91C_SPI_MODFDIS;
  /* setup parameters for default chip select */
  AT91C_BASE_SPI1->SPI_CSR[DEFAULT_CS] = (uint32_t)(descrp->clockMode) | (uint32_t)(descrp->symbolSize) | (uint32_t)(descrp->pack_parameter);

  /* disable all spi interrupt */
  AT91C_BASE_SPI1->SPI_IDR = ALL_PERIPHERIAL_INTERRUPT_DISABLE;

  /* Enable interrupts */
  /* Disable the interrupt on the interrupt controller */
  AT91C_BASE_AIC->AIC_IDCR = (1 << AT91C_ID_SPI1);
  /* Save the interrupt handler routine pointer and the interrupt priority */
  AT91C_BASE_AIC->AIC_SVR[AT91C_ID_SPI1] = (uint32_t)spi1Handler;
  /* Store the Source Mode Register */
  AT91C_BASE_AIC->AIC_SMR[AT91C_ID_SPI1] = AT91C_AIC_SRCTYPE_HIGH_LEVEL | AT91C_AIC_PRIOR_LOWEST;
  /* Clear the interrupt on the interrupt controller */
  AT91C_BASE_AIC->AIC_ICCR = (1 << AT91C_ID_SPI1);
  /* Enable the interrupt on the interrupt controller */
  AT91C_BASE_AIC->AIC_IECR = (1 << AT91C_ID_SPI1);

  /* enable spi DMA */
  AT91C_BASE_SPI1->SPI_PTCR = AT91C_PDC_RXTEN | AT91C_PDC_TXTEN;

  /* enable spi */
  AT91C_BASE_SPI1->SPI_CR = AT91C_SPI_SPIEN;
}

/******************************************************************************
starts spi data transaction
Parameters:
   data - pointer to data buffer.
   size - size of data buffer.
Return:
   0 - success (asynchronous mode); \n
   Number of written bytes - success (synchronous mode).
******************************************************************************/
int halStartDataMemSpi(SpiAccessType_t accessType, uint8_t *data, uint16_t size)
{
  uint32_t dummy;

  /* sometimes that bit is not clear. */
  while (AT91C_BASE_SPI1->SPI_SR & AT91C_SPI_RDRF)
  dummy = AT91C_BASE_SPI1->SPI_RDR;
  (void)dummy;

  if (SPI_READ == accessType)
  {
    /* pointer to received buffer */
    AT91C_BASE_SPI1->SPI_RPR = (uint32_t)data;
    /* data size */
    AT91C_BASE_SPI1->SPI_RCR = size;
  }
  /* pointer to transmitted buffer */
  AT91C_BASE_SPI1->SPI_TPR = (uint32_t)data;
  /* data size. Start bus transaction. */
  AT91C_BASE_SPI1->SPI_TCR = size;

  if (NULL == mem_spi_callback)
  { /* synchronous method. */
    /* wait for end of transaction. */
    while (!(AT91C_BASE_SPI1->SPI_SR & AT91C_SPI_TXBUFE));
    /* wait for last byte. */
    while (!(AT91C_BASE_SPI1->SPI_SR & AT91C_SPI_TXEMPTY));
    halSpiCurrentState = BUS_READY;
    return size;
  }
  else
  { /* asynchronous method (through interrupt). */
    if (SPI_READ == accessType)
    {
      /* enable interrupt when received buffer is full. */
      AT91C_BASE_SPI1->SPI_IER = AT91C_SPI_RXBUFF;
    }
    else
    {
      /* enable interrupt when transmitted buffer is empty. */
      AT91C_BASE_SPI1->SPI_IER = AT91C_SPI_TXBUFE;
    }
    return 0;
  }
}

/******************************************************************************
  Interrupt handler of SPI1 dma buffer full has occurred.
******************************************************************************/
void halMemIrqCompleteHandler(void)
{
  if (NULL != mem_spi_callback)
  {
    halSpiCurrentState = BUS_READY;
    mem_spi_callback();
  }
}

// eof halrfSpi.c
