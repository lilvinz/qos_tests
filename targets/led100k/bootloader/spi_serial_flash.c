/*
 * spi_serial_flash.c
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
#include "spi_serial_flash.h"

#include "spi_master.h"
#include "spi_memory.h"

//******************************************************************************************
//********** Note:                                                                **********
//********** The protocol of old and new serial spi flash is not identical!       **********
//********** There are differences in byte write command and AAI commands!        **********
//******************************************************************************************

uint8_t spi_serial_flash_write_buffer[SPI_SERIAL_FLASH_WRITE_BUFFER_SIZE];
uint32_t 	erase_sector_current_addr;
uint32_t 	erase_sector_end_addr;

/**
  * @brief  Initialize spi for serial flash.
  * @param  None
  * @retval None
  */
void SPI_SERIAL_FLASH_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);

    // Enable GPIO clock
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_GPIOA, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_GPIOF, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, DISABLE);

    //Connect alternate function pins
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

    // SPI 1 SCK
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // SPI 1 MISO
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // SPI 1 MOSI
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // FlashEEP-CS
    GPIO_InitStructure.GPIO_Pin = FLASH_EEP_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(FLASH_EEP_CS_PORT, &GPIO_InitStructure);

    // init chip select for spi serial flash
    DESELECT_FLASH();

    SPI_InitTypeDef SPI_InitStructure;
    SPI_StructInit(&SPI_InitStructure);

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    // APB1 = 30 MHz max.
    // APB2 = 60 MHz max.
    // SPI1 = APB2 / SPI_BaudRatePrescaler
    // SPI2 = APB1 / SPI_BaudRatePrescaler
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; // 7.5 MHz
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_MASTER_Init(SPI_SERIAL_FLASH_SPI_BUS, &SPI_InitStructure);
}

/**
  * @brief  Enables the write access to the FLASH.
  * @param  None
  * @retval None
  */
bool SPI_SERIAL_FLASH_WriteEnable(void)
{
    // select flash
    SELECT_FLASH();

    bool b_result;
    uint8_t buffer = SPI_DRIVER_MEMORY_CMD_WREN;   // send "Write Enable" instruction

    b_result = SPI_MASTER_TransmitByte(SPI_SERIAL_FLASH_SPI_BUS, &buffer);

    // deselect flash and wait for 100ns
    SPI_SERIAL_FLASH_Deselect_Flash_Wait();

    return b_result;
}

/**
  * @brief  Polls the "Write In Progress" bit of the external FLASH until
  * the write operation is finished or an error occurred.
  * @param  None
  * @retval bool: 	- true  : finished write operation
  * 				- false	: an error occurred
  */
bool SPI_SERIAL_FLASH_PollWIPBit(void)
{
    // select flash
    SELECT_FLASH();

    bool b_result = false;
    uint8_t mosi_buffer = SPI_DRIVER_MEMORY_CMD_READ_STATUS_REGISTER;   // send "Read Status Register" instruction

    b_result = SPI_MASTER_TransmitByte(SPI_SERIAL_FLASH_SPI_BUS, &mosi_buffer);

    while (b_result)
    {
    	// poll WIP bit until it is reset
    	uint8_t miso_buffer;
    	b_result = SPI_MASTER_ReceiveByte(SPI_SERIAL_FLASH_SPI_BUS, &miso_buffer);
    	if ((miso_buffer & SPI_SERIAL_FLASH_WIP_BIT) == 0)
    		break;
    }
    // deselect flash and wait for 100ns
    SPI_SERIAL_FLASH_Deselect_Flash_Wait();

    return b_result;
}

/**
  * @brief  Reads a block of data from the FLASH.
  * @param  addr: FLASH's internal address to read from.
  * @param  pBuffer: pointer to the buffer that receives the data read from the FLASH.
  * @param  len: number of bytes to read from the FLASH.
  * @retval None
  */
SPI_SERIAL_FLASH_RESULT_T SPI_SERIAL_FLASH_ReadBuffer(uint32_t addr, void* pBuffer, uint16_t len)
{
    bool b_result;
    uint8_t mosi_buffer[SPI_SERIAL_FLASH_NUM_CMD_ADR_BYTES];

    // select flash
    SELECT_FLASH();

    mosi_buffer[0] = SPI_DRIVER_MEMORY_CMD_READ;  // send "Read from Memory " instruction
    mosi_buffer[1] = (addr & 0xFF0000) >> 16;  //Send addr high nibble address byte to read from
    mosi_buffer[2] = (addr & 0xFF00) >> 8;  // send addr medium nibble address byte to read from
    mosi_buffer[3] = (addr & 0xFF);  // send addr low nibble address byte to read from

    b_result = SPI_MASTER_Transmit(SPI_SERIAL_FLASH_SPI_BUS, mosi_buffer, SPI_SERIAL_FLASH_NUM_CMD_ADR_BYTES);
    if (b_result)
        b_result = SPI_MASTER_Receive(SPI_SERIAL_FLASH_SPI_BUS, pBuffer, len);

    // deselect flash
    DESELECT_FLASH();

    if (b_result)
    	return SPI_SERIAL_FLASH_RESULT_FINISHED;
    else
    	return SPI_SERIAL_FLASH_RESULT_ERROR;
}

/**
  * @brief  Erases the specified FLASH block.
  * @param  addr: address of the block to erase.
  * @retval None
  */
SPI_SERIAL_FLASH_RESULT_T SPI_SERIAL_FLASH_EraseSectionStart(uint32_t start_addr, uint32_t end_addr)
{
	erase_sector_current_addr = start_addr;
	erase_sector_end_addr = end_addr;

	return SPI_SERIAL_FLASH_RESULT_FINISHED;
}

/**
  * @brief  Erases the specified FLASH block.
  * @param  addr: address of the block to erase.
  * @retval None
  */
SPI_SERIAL_FLASH_RESULT_T SPI_SERIAL_FLASH_EraseSectionTick(void)
{
    // select flash
    SELECT_FLASH();

	// poll WIP bit
    bool b_result = false;
    uint8_t mosi_buffer[SPI_SERIAL_FLASH_NUM_CMD_ADR_BYTES];
	uint8_t miso_buffer;

	mosi_buffer[0] = SPI_DRIVER_MEMORY_CMD_READ_STATUS_REGISTER;   // send "Read Status Register" instruction

    b_result = SPI_MASTER_TransmitByte(SPI_SERIAL_FLASH_SPI_BUS, mosi_buffer);
    if (b_result)
    	b_result = SPI_MASTER_ReceiveByte(SPI_SERIAL_FLASH_SPI_BUS, &miso_buffer);

	// deselect flash and wait for 100ns
	SPI_SERIAL_FLASH_Deselect_Flash_Wait();

    if (!b_result)
    	return SPI_SERIAL_FLASH_RESULT_ERROR;
	if ((miso_buffer & SPI_SERIAL_FLASH_WIP_BIT) != 0)
		return SPI_SERIAL_FLASH_RESULT_BUSY;
	if (erase_sector_current_addr > erase_sector_end_addr)
	{
	    // deselect flash
	    DESELECT_FLASH();
		return SPI_SERIAL_FLASH_RESULT_FINISHED;
	}

	// erase next sector
    // send write enable instruction
    if (!SPI_SERIAL_FLASH_WriteEnable())
    {
        // deselect flash
        DESELECT_FLASH();
        return SPI_SERIAL_FLASH_RESULT_ERROR;
    }

    // select flash
    SELECT_FLASH();

    // block Erase
    mosi_buffer[0] = SPI_DRIVER_MEMORY_CMD_BE_64K;  // send "Block Erase 64k " instruction
    mosi_buffer[1] = (erase_sector_current_addr & 0xFF0000) >> 16;  //Send high nibble address byte to read from
    mosi_buffer[2] = (erase_sector_current_addr & 0xFF00) >> 8;  // send medium nibble address byte to read from
    mosi_buffer[3] = (erase_sector_current_addr & 0xFF);  // send low nibble address byte to read from

    // send Sector Erase instruction
    if (SPI_MASTER_Transmit(SPI_SERIAL_FLASH_SPI_BUS, mosi_buffer, SPI_SERIAL_FLASH_NUM_CMD_ADR_BYTES))
    {
    	erase_sector_current_addr += SPI_SERIAL_FLASH_SECTOR_SIZE_64K;
        // deselect flash
        DESELECT_FLASH();
    	return SPI_SERIAL_FLASH_RESULT_BUSY;
    }

    // deselect flash
    DESELECT_FLASH();
	return SPI_SERIAL_FLASH_RESULT_ERROR;
}

/**
  * @brief  Erases the specified FLASH block.
  * @param  addr: address of the block to erase.
  * @retval None
  */
SPI_SERIAL_FLASH_RESULT_T SPI_SERIAL_FLASH_EraseSector64k(uint32_t addr)
{
    // send write enable instruction
    SPI_SERIAL_FLASH_WriteEnable();

    // select flash
    SELECT_FLASH();

    bool b_result;
    uint8_t mosi_buffer[SPI_SERIAL_FLASH_NUM_CMD_ADR_BYTES];

    // block Erase
    mosi_buffer[0] = SPI_DRIVER_MEMORY_CMD_BE_64K;  // send "Block Erase 64k " instruction
    mosi_buffer[1] = (addr & 0xFF0000) >> 16;  //Send addr high nibble address byte to read from
    mosi_buffer[2] = (addr & 0xFF00) >> 8;  // send addr medium nibble address byte to read from
    mosi_buffer[3] = (addr & 0xFF);  // send addr low nibble address byte to read from

    // send Sector Erase instruction
    b_result = SPI_MASTER_Transmit(SPI_SERIAL_FLASH_SPI_BUS, mosi_buffer, SPI_SERIAL_FLASH_NUM_CMD_ADR_BYTES);

    // deselect flash
    DESELECT_FLASH();
    // give flash time to execute erase command
    while (!SPI_SERIAL_FLASH_PollWIPBit()){}

    if (b_result)
    	return SPI_SERIAL_FLASH_RESULT_FINISHED;
    else
    	return SPI_SERIAL_FLASH_RESULT_ERROR;
}

/**
  * @brief  Erases the entire FLASH.
  * @param  None
  * @retval None
  */
SPI_SERIAL_FLASH_RESULT_T SPI_SERIAL_FLASH_EraseBulk(void)
{
    // send write enable instruction
    SPI_SERIAL_FLASH_WriteEnable();

    // select flash
    SELECT_FLASH();

    bool b_result;
    uint8_t buffer = SPI_DRIVER_MEMORY_CMD_CMD_BE;  // send "Bulk Erase" instruction

    // bulk Erase
    // send Sector Erase instruction
    b_result = SPI_MASTER_TransmitByte(SPI_SERIAL_FLASH_SPI_BUS, &buffer);

    // deselect flash
    DESELECT_FLASH();
    // give flash time to execute erase command
    while (!SPI_SERIAL_FLASH_PollWIPBit()){}

    if (b_result)
    	return SPI_SERIAL_FLASH_RESULT_FINISHED;
    else
    	return SPI_SERIAL_FLASH_RESULT_ERROR;
}

/**
  * @brief  Unlocks the flash for write access.
  *         The whole flash memory will be unlocked.
  *         If parts of the memory should be kept locked, this function can be altered.
  *         Look at the data sheet of the flash memory for further information.
  * @param  None
  * @retval None
  */
SPI_SERIAL_FLASH_RESULT_T SPI_SERIAL_FLASH_UnlockFlash(void)
{
    // send write enable instruction
    SPI_SERIAL_FLASH_WriteEnable();

    // select flash
    SELECT_FLASH();

    bool b_result;
    uint8_t mosi_buffer[SPI_SERIAL_MEMORY_NUM_BYTES_UNLOCK_MEM_CMD];

    // write status register to unlock flash
    mosi_buffer[0] = SPI_DRIVER_MEMORY_CMD_WRITE_STATUS_REGISTER;  // send "Write-Status-Register" instruction
    mosi_buffer[1] = SPI_SERIAL_MEMORY_UNLOCK_MEMORY_COMMAND;
    b_result = SPI_MASTER_Transmit(SPI_SERIAL_FLASH_SPI_BUS, mosi_buffer, SPI_SERIAL_MEMORY_NUM_BYTES_UNLOCK_MEM_CMD);

    // deselect flash and wait for 100ns
    SPI_SERIAL_FLASH_Deselect_Flash_Wait();

    if (b_result)
    	return SPI_SERIAL_FLASH_RESULT_FINISHED;
    else
    	return SPI_SERIAL_FLASH_RESULT_ERROR;
}

/**
  * @brief  Writes one byte to the FLASH.
  * @param  addr: FLASH's internal address to write to.
  * @param  pBuffer: pointer to the buffer containing the data to be written
  *         to the FLASH.
  * @retval None
  */
SPI_SERIAL_FLASH_RESULT_T SPI_SERIAL_FLASH_WriteByte(uint32_t addr, void* pBuffer)
{
    // send write enable instruction
    SPI_SERIAL_FLASH_WriteEnable();

    // select flash
    SELECT_FLASH();

    bool b_result;
    uint8_t mosi_buffer[SPI_SERIAL_FLASH_NUM_CMD_ADR_BYTES + 1];

    // write a single byte
    mosi_buffer[0] = SPI_DRIVER_MEMORY_CMD_WRITE;  // send "Write to Memory" instruction
    mosi_buffer[1] = (addr & 0xFF0000) >> 16;  //Send addr high nibble address byte to read from
    mosi_buffer[2] = (addr & 0xFF00) >> 8;  // send addr medium nibble address byte to read from
    mosi_buffer[3] = (addr & 0xFF);  // send addr low nibble address byte to read from
    mosi_buffer[4] = ((uint8_t *)pBuffer)[0];  // byte to write into flash
    b_result = SPI_MASTER_Transmit(SPI_SERIAL_FLASH_SPI_BUS, mosi_buffer, SPI_SERIAL_FLASH_NUM_CMD_ADR_BYTES);

    // deselect flash and wait for 100ns
    SPI_SERIAL_FLASH_Deselect_Flash_Wait();

    // give flash time to execute write command
    while (!SPI_SERIAL_FLASH_PollWIPBit()){}

    if (b_result)
    	return SPI_SERIAL_FLASH_RESULT_FINISHED;
    else
    	return SPI_SERIAL_FLASH_RESULT_ERROR;
}

/**
  * @brief  Writes a block of data to the flash. This function uses Auto Address Increment Word-Program.
  * @param  addr: FLASH's internal address to write to. Attention! The address has to be even!
  * @param  pBuffer: pointer to the buffer containing the data to be written to the FLASH.
  * @param  len: number of bytes to write to the FLASH.
  * @retval None
  */
SPI_SERIAL_FLASH_RESULT_T SPI_SERIAL_FLASH_WriteBuffer(uint32_t addr, void* pBuffer, uint16_t len)
{
    if (len == 0)
        return SPI_SERIAL_FLASH_RESULT_FINISHED;

    // write more than one byte
    bool b_result;
    uint8_t *p_src = (uint8_t *)pBuffer;  // send two bytes of data
    uint8_t mosi_buffer[SPI_SERIAL_FLASH_NUM_CMD_ADR_BYTES];  // send write command and address

    // Writing to the external flash is organized as writing pages of 256 bytes.
    // If a pagebreak is necessary (because the number of bytes which should be
    // written will not fit into the page which contains the start address) the
    // write operation has to separated into multiple write operations to avoid
    // unwanted behaviour.
    while (len > 0)
    {
        uint16_t start_page = addr / SPI_SERIAL_FLASH_PAGE_SIZE;
        uint16_t end_page = (addr + len) / SPI_SERIAL_FLASH_PAGE_SIZE;
        uint16_t num_bytes;

        if (start_page != end_page)
        	num_bytes = SPI_SERIAL_FLASH_PAGE_SIZE - (addr % SPI_SERIAL_FLASH_PAGE_SIZE);
        else
        	num_bytes = len;

        // send write enable instruction
        SPI_SERIAL_FLASH_WriteEnable();
        // select flash
        SELECT_FLASH();
        mosi_buffer[0] = SPI_DRIVER_MEMORY_CMD_WRITE;  // send "Write to Memory" instruction
        mosi_buffer[1] = (addr & 0xFF0000) >> 16;  // Send addr high nibble address byte to write to
        mosi_buffer[2] = (addr & 0xFF00) >> 8;  // Send addr medium nibble address byte to write to
        mosi_buffer[3] = addr & 0xFF;  // Send addr low nibble address byte to write to

        // send command and address
        b_result = SPI_MASTER_Transmit(SPI_SERIAL_FLASH_SPI_BUS, mosi_buffer, SPI_SERIAL_FLASH_NUM_CMD_ADR_BYTES);

		// send two bytes of data
		b_result = SPI_MASTER_Transmit(SPI_SERIAL_FLASH_SPI_BUS, p_src, num_bytes);

		p_src += num_bytes;
		addr += num_bytes;
		len -= num_bytes;

		// deselect flash and wait for 100ns
		SPI_SERIAL_FLASH_Deselect_Flash_Wait();
		// give flash time to execute write command
		while (!SPI_SERIAL_FLASH_PollWIPBit()){}
    }

    if (b_result)
    	return SPI_SERIAL_FLASH_RESULT_FINISHED;
    else
    	return SPI_SERIAL_FLASH_RESULT_ERROR;
}
