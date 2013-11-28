/*
 * coredump.h
 *
 *  Created on: 27.05.2011
 *      Author: cba
 */

#ifndef COREDUMP_H_
#define COREDUMP_H_


// SPI SERIAL FLASH
#define COREDUMP_FLASH_EEP_CS_PORT           GPIOF
#define COREDUMP_FLASH_EEP_CS_PIN            GPIO_Pin_15

#define COREDUMP_SELECT_FLASH()              GPIO_WriteBit(COREDUMP_FLASH_EEP_CS_PORT, COREDUMP_FLASH_EEP_CS_PIN , Bit_RESET)
#define COREDUMP_DESELECT_FLASH()            GPIO_WriteBit(COREDUMP_FLASH_EEP_CS_PORT, COREDUMP_FLASH_EEP_CS_PIN , Bit_SET)

#define COREDUMP_FLASH_SPI_BUS                SPI1
#define COREDUMP_FLASH_NUM_CMD_ADR_BYTES      4
#define COREDUMP_FLASH_NUM_BYTES_AAI_WORD     3
#define COREDUMP_FLASH_WRITE_BUFFER_SIZE      2

#define COREDUMP_SPI_CR1_SPE    ((uint16_t)0x0040)
#define COREDUMP_SPI_CR1_RXONLY ((uint16_t)0x0400)


#define COREDUMP_FLASH_SECTOR_ERASE_WAIT_TIME 25  // 25ms
#define COREDUMP_FLASH_BLOCK_ERASE_WAIT_TIME  25  // 25ms
#define COREDUMP_FLASH_CHIP_ERASE_WAIT_TIME   50  // 50ms

#define COREDUMP_FLASH_BLOCK_SIZE             65536UL
#define COREDUMP_FLASH_PAGE_SIZE              256

#define COREDUMP_FLASH_Deselect_Flash_Wait() COREDUMP_DESELECT_FLASH()  // this needs about 2us

#define COREDUMP_MEMORY_CMD_WREN                   0x06  // Write enable instruction
#define COREDUMP_MEMORY_CMD_WRITE                  0x02  // Write to Memory instruction
#define COREDUMP_MEMORY_CMD_WRDI                   0x04  // Write Disable instruction
#define COREDUMP_MEMORY_CMD_READ_STATUS_REGISTER   0x05  // Read status register
#define COREDUMP_MEMORY_CMD_BE_64K                 0xD8  // 64-KByte block erase instruction
#define COREDUMP_MEMORY_CMD_WRITE_BUFFER           0xAD  // Auto Address Increment Word-Program
#define COREDUMP_MEMORY_CMD_HSREAD                 0x0B  // High-Speed-Read (80MHz)
#define COREDUMP_CMD_ADR_MAX_LENGTH    4

#define COREDUMP_FLASH_WIP_BIT                0x01
#define COREDUMP_FLASH_POLL_WIP_BIT_DELAY     1  // 1ms

void COREDUMP_PerformCoreDump(void);
bool COREDUMP_IsEmpty(void);
void COREDUMP_Erase(void);


#endif /* COREDUMP_H_ */
