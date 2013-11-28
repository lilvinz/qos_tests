/*
 * coredump.c
 *
 *  Created on: 27.05.2011
 *      Author: cba
 */

#include "stdheader.h"

#include "coredump.h"
#include "flash_memory_map.h"




bool COREDUMP_FLASH_WriteBuffer(uint32_t addr, void* pBuffer, uint16_t len);
bool COREDUMP_FLASH_EraseBlock64k(uint32_t addr);
bool COREDUMP_FLASH_WriteEnable(void);
void COREDUMP_SPI_MASTER_Init(void);
void COREDUMP_SPI_MASTER_Reinit(void);
void COREDUMP_SPI_MASTER_Disable(void);
void COREDUMP_SPI_MASTER_Enable(void);
void COREDUMP_SPI_MASTER_ContinuousReadEnable(void);
void COREDUMP_SPI_MASTER_ContinuousReadDisable(void);
bool COREDUMP_SPI_MASTER_Transmit(const uint8_t *pDataMOSI, uint16_t Length);
bool COREDUMP_SPI_MASTER_ReceiveByte(uint8_t *pDataMISO);
uint8_t COREDUMP_SPI_MASTER_ReceiveContinuousByte(void);
void COREDUMP_SPI_MASTER_ReceiverClear(void);

void COREDUMP_PerformCoreDump(void)
{
#define MEM_RAM_START_ADDR  0x20000000
#define MEM_RAM_END_ADDR    MEM_RAM_START_ADDR + (96 * 1024)
#define COREDUMP_BLOCKSIZE  (8 * 1024)

    uint32_t    src_addr;
    uint32_t    dst_addr;

    COREDUMP_SPI_MASTER_Reinit();

    src_addr = MEM_RAM_START_ADDR;
    dst_addr = FLASH_COREDUMP_START_ADDRESS;

    while (src_addr < MEM_RAM_END_ADDR && dst_addr < FLASH_COREDUMP_END_ADDRESS)
    {
        COREDUMP_FLASH_WriteBuffer(dst_addr, (void *)src_addr, COREDUMP_BLOCKSIZE);
        src_addr += COREDUMP_BLOCKSIZE;
        dst_addr += COREDUMP_BLOCKSIZE;
    }
}


void COREDUMP_Erase(void)
{
    uint32_t addr = FLASH_COREDUMP_START_ADDRESS;

    COREDUMP_SPI_MASTER_Reinit();

    while (addr < FLASH_COREDUMP_END_ADDRESS)
    {
        COREDUMP_FLASH_EraseBlock64k(addr);
        addr += (64 * 1024);
    }

    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
}


bool COREDUMP_IsEmpty(void)
{
    uint32_t addr = FLASH_COREDUMP_START_ADDRESS;
    uint8_t mosi_buffer[COREDUMP_FLASH_NUM_CMD_ADR_BYTES + 1];
    bool b_result;

    COREDUMP_SPI_MASTER_Reinit();

    COREDUMP_SELECT_FLASH();

    mosi_buffer[0] = COREDUMP_MEMORY_CMD_HSREAD;  // send "High-Speed-Read" instruction
    mosi_buffer[1] = (addr & 0xFF0000) >> 16;  // Send addr high nibble address byte to write to
    mosi_buffer[2] = (addr & 0xFF00) >> 8;  // Send addr medium nibble address byte to write to
    mosi_buffer[3] = addr & 0xFF;  // Send addr low nibble address byte to write to
    mosi_buffer[4] = 0;  // dummy byte of data
    b_result = COREDUMP_SPI_MASTER_Transmit(mosi_buffer, COREDUMP_FLASH_NUM_CMD_ADR_BYTES + 1);

    COREDUMP_SPI_MASTER_ReceiverClear();

    COREDUMP_SPI_MASTER_ContinuousReadEnable();
    volatile uint8_t byte = 0;
    while (addr < FLASH_COREDUMP_END_ADDRESS && b_result == TRUE)
    {
        byte = COREDUMP_SPI_MASTER_ReceiveContinuousByte();
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        if (0xFF != byte)
            b_result = FALSE;
//        if (0xFF != COREDUMP_SPI_MASTER_ReceiveContinuousByte())
//            b_result = FALSE;

        ++addr;
    }
    COREDUMP_SPI_MASTER_ContinuousReadDisable();

    COREDUMP_DESELECT_FLASH();  // this needs about 2us

    return b_result;
}


bool COREDUMP_FLASH_WriteEnable(void)
{
    COREDUMP_SELECT_FLASH();

    bool b_result;
    uint8_t buffer = COREDUMP_MEMORY_CMD_WREN;   // send "Write Enable" instruction

    b_result = COREDUMP_SPI_MASTER_Transmit(&buffer, 1);

    COREDUMP_FLASH_Deselect_Flash_Wait();

    return b_result;
}


/**
  * @brief  Polls the "Write In Progress" bit of the external FLASH until
  * the write operation is finished or an error occurred.
  * @param  None
  * @retval bool:   - true  : finished write operation
  *                 - false : an error occurred
  */
bool COREDUMP_FLASH_PollWIPBit(void)
{
    // select flash
    COREDUMP_SELECT_FLASH();

    bool b_result = false;
    uint8_t mosi_buffer = COREDUMP_MEMORY_CMD_READ_STATUS_REGISTER;   // send "Read Status Register" instruction

    b_result = COREDUMP_SPI_MASTER_Transmit(&mosi_buffer, 1);

    while (b_result)
    {
        // poll WIP bit until it is reset
        uint8_t miso_buffer;
        b_result = COREDUMP_SPI_MASTER_ReceiveByte(&miso_buffer);
        if ((miso_buffer & COREDUMP_FLASH_WIP_BIT) == 0)
            break;
        DELAY_MS(COREDUMP_FLASH_POLL_WIP_BIT_DELAY);
    }

    // deselect flash and wait for 100ns
    COREDUMP_DESELECT_FLASH();

    return b_result;
}

bool COREDUMP_FLASH_EraseBlock64k(uint32_t addr)
{
    COREDUMP_FLASH_WriteEnable();
    COREDUMP_SELECT_FLASH();

    bool b_result;
    uint8_t mosi_buffer[COREDUMP_FLASH_NUM_CMD_ADR_BYTES];

    mosi_buffer[0] = COREDUMP_MEMORY_CMD_BE_64K;  // send "Block Erase 64k " instruction
    mosi_buffer[1] = (addr & 0xFF0000) >> 16;  //Send addr high nibble address byte to read from
    mosi_buffer[2] = (addr & 0xFF00) >> 8;  // send addr medium nibble address byte to read from
    mosi_buffer[3] = (addr & 0xFF);  // send addr low nibble address byte to read from
    b_result = COREDUMP_SPI_MASTER_Transmit(mosi_buffer, COREDUMP_FLASH_NUM_CMD_ADR_BYTES);

    COREDUMP_DESELECT_FLASH();
    // give flash time to execute erase command
    while (!COREDUMP_FLASH_PollWIPBit()) {}

    return b_result;
}


bool COREDUMP_FLASH_WriteBuffer(uint32_t addr, void* pBuffer, uint16_t len)
{
    if (len == 0)
        return true;

    bool b_result;
    uint8_t *p_src = (uint8_t *)pBuffer;  // send two bytes of data
    uint8_t mosi_buffer[COREDUMP_FLASH_NUM_CMD_ADR_BYTES];  // send write command and address

    // Writing to the external flash is organized as writing pages of 256 bytes.
    // If a pagebreak is necessary (because the number of bytes which should be
    // written will not fit into the page which contains the start address) the
    // write operation has to separated into multiple write operations to avoid
    // unwanted behaviour.
    while (len > 0)
    {
        uint16_t start_page = addr / COREDUMP_FLASH_PAGE_SIZE;
        uint16_t end_page = (addr + len) / COREDUMP_FLASH_PAGE_SIZE;
        uint16_t num_bytes;

        if (start_page != end_page)
            num_bytes = COREDUMP_FLASH_PAGE_SIZE - (addr % COREDUMP_FLASH_PAGE_SIZE);
        else
            num_bytes = len;

        // send write enable instruction
        COREDUMP_FLASH_WriteEnable();

        // select flash
        COREDUMP_SELECT_FLASH();
        mosi_buffer[0] = COREDUMP_MEMORY_CMD_WRITE;  // send "Write to Memory" instruction
        mosi_buffer[1] = (addr & 0xFF0000) >> 16;  // Send addr high nibble address byte to write to
        mosi_buffer[2] = (addr & 0xFF00) >> 8;  // Send addr medium nibble address byte to write to
        mosi_buffer[3] = addr & 0xFF;  // Send addr low nibble address byte to write to

        // send command and address
        b_result = COREDUMP_SPI_MASTER_Transmit(mosi_buffer, COREDUMP_FLASH_NUM_CMD_ADR_BYTES);

        // send two bytes of data
        b_result = COREDUMP_SPI_MASTER_Transmit(p_src, num_bytes);

        p_src += num_bytes;
        addr += num_bytes;
        len -= num_bytes;

        // deselect flash and wait for 100ns
        COREDUMP_FLASH_Deselect_Flash_Wait();

        // give flash time to execute write command
        while (!COREDUMP_FLASH_PollWIPBit()) {}
    }

    return b_result;
}


void COREDUMP_SPI_MASTER_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);

    SPI_I2S_ITConfig(COREDUMP_FLASH_SPI_BUS, SPI_I2S_IT_RXNE, DISABLE);
    SPI_I2S_ITConfig(COREDUMP_FLASH_SPI_BUS, SPI_I2S_IT_TXE, DISABLE);


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
    GPIO_InitStructure.GPIO_Pin = COREDUMP_FLASH_EEP_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(COREDUMP_FLASH_EEP_CS_PORT, &GPIO_InitStructure);


    // init chip select for spi serial flash
    COREDUMP_DESELECT_FLASH();

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
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; // 9 MHz
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;

    SPI_Cmd(COREDUMP_FLASH_SPI_BUS, DISABLE);

    SPI_Init(COREDUMP_FLASH_SPI_BUS, &SPI_InitStructure);

    SPI_I2S_ITConfig(COREDUMP_FLASH_SPI_BUS, SPI_I2S_IT_RXNE, DISABLE);
    SPI_I2S_ITConfig(COREDUMP_FLASH_SPI_BUS, SPI_I2S_IT_TXE, DISABLE);

    SPI_Cmd(COREDUMP_FLASH_SPI_BUS, ENABLE);
}


void COREDUMP_SPI_MASTER_Reinit(void)
{
    COREDUMP_SPI_MASTER_Disable();
    COREDUMP_SPI_MASTER_Init();
    COREDUMP_SPI_MASTER_Enable();
}


void COREDUMP_SPI_MASTER_Enable(void)
{
    SPI_Cmd(COREDUMP_FLASH_SPI_BUS, ENABLE);
}


void COREDUMP_SPI_MASTER_Disable(void)
{
    SPI_Cmd(COREDUMP_FLASH_SPI_BUS, DISABLE);
}


void COREDUMP_SPI_MASTER_ContinuousReadEnable(void)
{
    //COREDUMP_FLASH_SPI_BUS->CR1 &= ~COREDUMP_SPI_CR1_SPE;
    COREDUMP_FLASH_SPI_BUS->CR1 |= SPI_Direction_2Lines_RxOnly;
    //COREDUMP_FLASH_SPI_BUS->CR1 |= COREDUMP_SPI_CR1_SPE;

}


void COREDUMP_SPI_MASTER_ContinuousReadDisable(void)
{
    COREDUMP_FLASH_SPI_BUS->CR1 &= ~COREDUMP_SPI_CR1_SPE;
    COREDUMP_FLASH_SPI_BUS->CR1 &= ~SPI_Direction_2Lines_RxOnly;
    COREDUMP_FLASH_SPI_BUS->CR1 |= COREDUMP_SPI_CR1_SPE;
}


bool COREDUMP_SPI_MASTER_Transmit(const uint8_t *pDataMOSI, uint16_t Length)
{
    while (Length > 0)
    {
        while (SPI_I2S_GetFlagStatus(COREDUMP_FLASH_SPI_BUS, SPI_I2S_FLAG_TXE) != SET) {}

        COREDUMP_FLASH_SPI_BUS->DR = *(uint16_t*)pDataMOSI;
        pDataMOSI += 1;
        Length -= 1;
    }
    while (!(COREDUMP_FLASH_SPI_BUS->SR & SPI_I2S_FLAG_TXE)) {}
    while (!(COREDUMP_FLASH_SPI_BUS->SR & SPI_I2S_FLAG_RXNE)) {}
    while ((COREDUMP_FLASH_SPI_BUS->SR & SPI_I2S_FLAG_BSY)) {}
    return TRUE;
}


bool COREDUMP_SPI_MASTER_ReceiveByte(uint8_t *pDataMISO)
{
    uint8_t DataMOSI = 0;
    if (COREDUMP_SPI_MASTER_Transmit(&DataMOSI, 1) != TRUE)
        return false;
    while (!(COREDUMP_FLASH_SPI_BUS->SR & SPI_I2S_FLAG_RXNE)) {}
    *pDataMISO = (uint8_t)COREDUMP_FLASH_SPI_BUS->DR;
    return TRUE;
}


uint8_t COREDUMP_SPI_MASTER_ReceiveContinuousByte(void)
{
    while (!(COREDUMP_FLASH_SPI_BUS->SR & SPI_I2S_FLAG_RXNE)) {}
    return (uint8_t)COREDUMP_FLASH_SPI_BUS->DR;
}

void COREDUMP_SPI_MASTER_ReceiverClear(void)
{
    uint16_t devnull = COREDUMP_FLASH_SPI_BUS->DR;
    devnull = devnull;
}
