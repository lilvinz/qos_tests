/*
 * spi_serial_fram.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 15.06.2010
 *      Author: DHA
 *
 * Purpose:
 *
 */

#include "stdheader.h"
#include "spi_serial_fram.h"

#include "spi_master.h"
#include "spi_memory.h"


/**
  * @brief  Initialize spi for serial flash.
  * @param  None
  * @retval None
  */
void SPI_SERIAL_FRAM_Init(void)
{
    // init chip select for spi serial flash
    DESELECT_FRAM();

    SPI_InitTypeDef SPI_InitStructure;
    SPI_StructInit(&SPI_InitStructure);

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    // APB1 = 36 MHz max.
    // APB2 = 72 MHz max.
    // SPI1 = APB2 / SPI_BaudRatePrescaler
    // SPI2 = APB1 / SPI_BaudRatePrescaler
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4; // 9 MHz
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_MASTER_Init(SPI_SERIAL_FRAM_SPI_BUS, &SPI_InitStructure);
}

/**
  * @brief  Enables the write access to the FRAM.
  * @param  None
  * @retval None
  */
bool SPI_SERIAL_FRAM_WriteEnable(void)
{
    // select fram
    SELECT_FRAM();

    bool b_result;
    uint8_t buffer = SPI_DRIVER_MEMORY_CMD_WREN;   // send "Write Enable" instruction

    b_result = SPI_MASTER_TransmitByte(SPI_SERIAL_FRAM_SPI_BUS, &buffer);

    // deselect fram and wait for 40ns
    SPI_SERIAL_FRAM_Deselect_Fram_Wait();

    return b_result;
}

/**
  * @brief  Reads a block of data from the FRAM.
  * @param  addr: FRAM's internal address to read from.
  * @param  pBuffer: pointer to the buffer that receives the data read from the FRAM.
  * @param  len: number of bytes to read from the FRAM.
  * @retval None
  */
bool FRAM_Read(uint16_t addr, void* pBuffer, uint16_t len)
{
    // wait to get mutex
    SPI_MASTER_Lock(SPI_SERIAL_FRAM_SPI_BUS);

    bool b_result;
    uint8_t mosi_buffer[SPI_SERIAL_FRAM_NUM_CMD_ADR_BYTES];

    // select fram
    SELECT_FRAM();

    mosi_buffer[0] = SPI_DRIVER_MEMORY_CMD_READ;  // send "Read from Memory " instruction
    mosi_buffer[1] = (addr & 0xFF00) >> 8;  // send addr high nibble address byte to read from
    mosi_buffer[2] = (addr & 0xFF);  // send addr low nibble address byte to read from

    b_result = SPI_MASTER_Transmit(SPI_SERIAL_FRAM_SPI_BUS, mosi_buffer, SPI_SERIAL_FRAM_NUM_CMD_ADR_BYTES);
    if (b_result)
    {
        b_result = SPI_MASTER_Receive(SPI_SERIAL_FRAM_SPI_BUS, pBuffer, len);
    }

    // deselect fram
    DESELECT_FRAM();
    // give mutex back
    SPI_MASTER_Unlock(SPI_SERIAL_FRAM_SPI_BUS);

    return b_result;
}

/**
  * @brief  Unlocks the fram for write access.
  *         The whole fram memory will be unlocked.
  *         If parts of the memory should be kept locked, this function can be altered.
  *         Look at the data sheet of the fram memory for further information.
  * @param  None
  * @retval None
  */
bool SPI_SERIAL_FRAM_UnlockFram(void)
{
    // wait to get mutex
    SPI_MASTER_Lock(SPI_SERIAL_FRAM_SPI_BUS);

    // send write enable instruction
    SPI_SERIAL_FRAM_WriteEnable();

    // write status register to unlock fram
    // select the FRAM: Chip Select low
    SELECT_FRAM();

    bool b_result;
    uint8_t mosi_buffer[SPI_SERIAL_MEMORY_NUM_BYTES_UNLOCK_MEM_CMD];

    // block Erase
    mosi_buffer[0] = SPI_DRIVER_MEMORY_CMD_WRITE_STATUS_REGISTER;  // send "Write-Status-Register" instruction
    mosi_buffer[1] = SPI_SERIAL_MEMORY_UNLOCK_MEMORY_COMMAND;

    // write status register to unlock flash
    b_result = SPI_MASTER_Transmit(SPI_SERIAL_FRAM_SPI_BUS, mosi_buffer, SPI_SERIAL_MEMORY_NUM_BYTES_UNLOCK_MEM_CMD);

    DESELECT_FRAM();

    // give mutex back
    SPI_MASTER_Unlock(SPI_SERIAL_FRAM_SPI_BUS);

    return b_result;
}

/**
  * @brief  Writes a block of data to the fram. This function uses Auto Address Increment Word-Program.
  * @param  addr: FRAM's internal address to write to.
  * @param  pBuffer: pointer to the buffer containing the data to be written to the FRAM.
  * @param  len: number of bytes to write to the FRAM.
  * @retval None
  */
bool FRAM_Write(uint16_t addr, const void* pBuffer, uint16_t len)
{
    if (len == 0)
    {
        return TRUE;
    }

    // wait to get mutex
    SPI_MASTER_Lock(SPI_SERIAL_FRAM_SPI_BUS);

    // send write enable instruction
    SPI_SERIAL_FRAM_WriteEnable();

    // select the FRAM: Chip Select low
    SELECT_FRAM();

    // write bytes
    bool b_result;
    uint8_t mosi_buffer[SPI_SERIAL_FRAM_NUM_CMD_ADR_BYTES];

    mosi_buffer[0] = SPI_DRIVER_MEMORY_CMD_WRITE;  // send "Write to Memory" instruction
    mosi_buffer[1] = (addr & 0xFF00) >> 8;  // Send addr high nibble address byte to write to
    mosi_buffer[2] = addr & 0xFF;  // Send addr low nibble address byte to write to

    // send command and address
    b_result = SPI_MASTER_Transmit(SPI_SERIAL_FRAM_SPI_BUS, mosi_buffer, SPI_SERIAL_FRAM_NUM_CMD_ADR_BYTES);

    if (b_result == TRUE)
    {
        // send data
        b_result = SPI_MASTER_Transmit(SPI_SERIAL_FRAM_SPI_BUS, pBuffer, len);
    }

    // deselect fram and wait for 40ns
    SPI_SERIAL_FRAM_Deselect_Fram_Wait();

    // give mutex back
    SPI_MASTER_Unlock(SPI_SERIAL_FRAM_SPI_BUS);

    return b_result;
}
