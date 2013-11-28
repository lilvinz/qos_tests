/**************************************************************************//**
\file  halEeprom.c

\brief Implementation of chip-eeprom interface.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    5/12/07 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                   Includes section
******************************************************************************/
#include <halEeprom.h>
#include <spi.h>
#include <appTimer.h>
#include <halTaskManager.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define SPI_BUSY_POLL_PERIOD      10

/******************************************************************************
                   Prototypes section
******************************************************************************/
/******************************************************************************
Callback for spi.
******************************************************************************/
void halSpiCallback(void);

/******************************************************************************
Read status register. If eeprom busy again read register until eeprom
return "ready".
******************************************************************************/
void halReadStatusRegister(void);

/******************************************************************************
Start of data reading from eeprom
******************************************************************************/
void halReadSpiEeprom(void);

/******************************************************************************
                   External variables section
******************************************************************************/
extern volatile uint8_t halSpiCurrentState;

/******************************************************************************
                   Global variables section
******************************************************************************/
uint8_t halSpiEepromState = EEPROM_IDLE;
void (*halEepromCallback)() = NULL;
void (*eepromAction)() = NULL;
HAL_EepromParams_t localCopy;
uint32_t header;
uint32_t sreg;
HAL_AppTimer_t halBusyTimer;
HAL_SpiDescriptor_t spiDescrip;

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
Eeprom callback for correct work with spi
******************************************************************************/
void halSpiEepromDone(void)
{
  // clear chip select
  GPIO_EEPROM_CS_set();
  halSpiEepromState = EEPROM_IDLE;
  HAL_CloseSpi(&spiDescrip);
  if (NULL != halEepromCallback)
    halEepromCallback();
}

/******************************************************************************
Send read command and data address
******************************************************************************/
void halSendReadCommandSpiEeprom(void)
{
  // op-code & internal eeprom address
  header = READ;
  header |= (localCopy.address & 0xFF00);
  header |= ((uint32_t)(localCopy.address) << 16);

  // second step for reading
  eepromAction = halReadSpiEeprom;

  // set chip select
  GPIO_EEPROM_CS_clr();
  HAL_WriteSpi(&spiDescrip, (uint8_t *)&header, 3);
}

/******************************************************************************
Start of data reading from eeprom
******************************************************************************/
void halReadSpiEeprom(void)
{
  // third step for reading
  eepromAction = halSpiEepromDone;
  HAL_ReadSpi(&spiDescrip, localCopy.data, localCopy.length);
}

/******************************************************************************
send "write enable" instructions
******************************************************************************/
void halSendWriteEnable(void)
{
  uint32_t header = WREN;

  // set chip select
  GPIO_EEPROM_CS_clr();
  HAL_WriteSpi(&spiDescrip, (uint8_t *)&header, 1);
  // clear chip select
  GPIO_EEPROM_CS_set();
}

/******************************************************************************
Start write data to eeprom. Linear address map convert to page address map
Return:
  number of unsent data.
******************************************************************************/
uint16_t halWriteSpiEeprom(void)
{
  uint16_t localSize;
  uint8_t *localPointer;
  uint32_t header;

  localPointer = localCopy.data;
  localSize = WRITE_PAGE_SIZE - (localCopy.address & REMAINDER);

  // op-code & internal eeprom address
  header = WRITE;
  header |= (localCopy.address & 0xFF00);
  header |= ((uint32_t)(localCopy.address) << 16);

  if (localCopy.length < localSize)
  { // current data size less page remainder
    localSize = localCopy.length;
    localCopy.length = 0;
  }
  else
  {
    localCopy.length -= localSize;
    localCopy.data += localSize;
    localCopy.address += localSize;
    sreg = RDY << 8;
  }

  halSendWriteEnable();
  // set chip select
  GPIO_EEPROM_CS_clr();
  HAL_WriteSpi(&spiDescrip, (uint8_t *)&header, 3);
  HAL_WriteSpi(&spiDescrip, localPointer, localSize);
  GPIO_EEPROM_CS_set();

  return localCopy.length;
}

/******************************************************************************
read status register. If eeprom busy again read register until eeprom return "ready".
******************************************************************************/
void halReadStatusRegister(void)
{
  // clear previous transaction
  GPIO_EEPROM_CS_set();
  if ((sreg >> 8) & RDY) // eeprom busy
  {
    sreg = 0ul;
    sreg = RDSR;
    // chip select set
    GPIO_EEPROM_CS_clr();
    HAL_ReadSpi(&spiDescrip, (uint8_t *)&sreg, 2);
  }
  else
  {
    if (EEPROM_IS_READ == halSpiEepromState)
    {
      halSendReadCommandSpiEeprom();
      return;
    }
    ATOMIC_SECTION_ENTER
    HAL_CloseSpi(&spiDescrip);
    // Init spi bus for eeprom
    spiDescrip.tty = SPI_CHANNEL_1;
    spiDescrip.callback = NULL;
    spiDescrip.clockMode = SPI_CLOCK_MODE0;      // spi mode
    spiDescrip.symbolSize = SPI_8BITS_SYMBOL;
    spiDescrip.pack_parameter = HAL_SPI_PACK_PARAMETER(EEPROM_SPI_FREQUENCY,
                                                       NPCS_TO_SPCK_EEPROM,
                                                       DELAY_BETWEEN_CONSECUTIVE_TRANSFERS_EEPROM);  // spi timing parameter cs
    HAL_OpenSpi(&spiDescrip);
    ATOMIC_SECTION_LEAVE
    if (halWriteSpiEeprom())
    {
      HAL_CloseSpi(&spiDescrip);
      halSpiOpener();
    }
    else
    {
      HAL_CloseSpi(&spiDescrip);
      halSpiEepromDone();
    }
  }
}

/******************************************************************************
SPI opener.
******************************************************************************/
void halSpiOpener(void)
{
  // Init spi bus for eeprom
  spiDescrip.tty = SPI_CHANNEL_1;
  spiDescrip.callback = halSpiCallback;
  spiDescrip.clockMode = SPI_CLOCK_MODE0;      // spi mode
  spiDescrip.symbolSize = SPI_8BITS_SYMBOL;
  spiDescrip.pack_parameter = HAL_SPI_PACK_PARAMETER(EEPROM_SPI_FREQUENCY,
                                                     NPCS_TO_SPCK_EEPROM,
                                                     DELAY_BETWEEN_CONSECUTIVE_TRANSFERS_EEPROM);  // spi timing parameter cs
  eepromAction = halReadStatusRegister;
  if (0 != HAL_OpenSpi(&spiDescrip))
  {
    halBusyTimer.interval = SPI_BUSY_POLL_PERIOD;
    halBusyTimer.mode = TIMER_ONE_SHOT_MODE;
    halBusyTimer.callback = halSpiOpener;
    HAL_StartAppTimer(&halBusyTimer);
    return;
  }

  GPIO_EEPROM_CS_make_pullup();
  GPIO_EEPROM_CS_make_out();
  // It's special first busy station to start eeprom transaction
  sreg = RDY << 8;
  halReadStatusRegister();
}

/******************************************************************************
Callback for spi.
******************************************************************************/
void halSpiCallback(void)
{
  eepromAction();
}

// eof halEeprom.c
