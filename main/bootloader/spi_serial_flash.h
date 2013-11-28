/*
 * spi_serial_flash.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 15.06.2010
 *      Author: DHA
 *
 * Purpose:
 *
 */

#ifndef SPI_SERIAL_FLASH_H_
#define SPI_SERIAL_FLASH_H_


// SPI SERIAL FLASH
#define FLASH_EEP_CS_PORT           GPIOF
#define FLASH_EEP_CS_PIN            GPIO_Pin_15

#define SELECT_FLASH()              GPIO_WriteBit(FLASH_EEP_CS_PORT, FLASH_EEP_CS_PIN , Bit_RESET)
#define DESELECT_FLASH()            GPIO_WriteBit(FLASH_EEP_CS_PORT, FLASH_EEP_CS_PIN , Bit_SET)

#define SPI_SERIAL_FLASH_SPI_BUS                SPI_MASTER_BUS_1
#define SPI_SERIAL_FLASH_NUM_CMD_ADR_BYTES      4
#define SPI_SERIAL_FLASH_WRITE_BUFFER_SIZE      2

#define SPI_SERIAL_FLASH_SECTOR_SIZE_64K        65536UL
#define SPI_SERIAL_FLASH_PAGE_SIZE              256

#define SPI_SERIAL_FLASH_WIP_BIT                0x01
#define SPI_SERIAL_FLASH_Deselect_Flash_Wait() DESELECT_FLASH()

typedef enum
{
    SPI_SERIAL_FLASH_RESULT_ERROR = -1,
    SPI_SERIAL_FLASH_RESULT_BUSY = 0,
    SPI_SERIAL_FLASH_RESULT_FINISHED = 1
} SPI_SERIAL_FLASH_RESULT_T;

void SPI_SERIAL_FLASH_Init(void);
SPI_SERIAL_FLASH_RESULT_T SPI_SERIAL_FLASH_UnlockFlash(void);
SPI_SERIAL_FLASH_RESULT_T SPI_SERIAL_FLASH_ReadBuffer(uint32_t addr, void* pBuffer, uint16_t len);
SPI_SERIAL_FLASH_RESULT_T SPI_SERIAL_FLASH_EraseSectionStart(uint32_t start_addr, uint32_t end_addr);
SPI_SERIAL_FLASH_RESULT_T SPI_SERIAL_FLASH_EraseSectionTick(void);
SPI_SERIAL_FLASH_RESULT_T SPI_SERIAL_FLASH_EraseSector64k(uint32_t addr);
SPI_SERIAL_FLASH_RESULT_T SPI_SERIAL_FLASH_EraseBulk(void);
SPI_SERIAL_FLASH_RESULT_T SPI_SERIAL_FLASH_WriteByte(uint32_t addr, void* pBuffer);
SPI_SERIAL_FLASH_RESULT_T SPI_SERIAL_FLASH_WriteBuffer(uint32_t addr, void* pBuffer, uint16_t len);

#endif /* SPI_SERIAL_FLASH_H_ */
