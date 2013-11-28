/*
 * spi_memory.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 16.11.2010
 *      Author: DHA
 *
 * Purpose:
 *
 */

#ifndef SPI_H_MEMORY
#define SPI_H_MEMORY

#define SPI_DRIVER_MEMORY_CMD_WRITE_STATUS_REGISTER  0x01  // Write-Status-Register instruction
#define SPI_DRIVER_MEMORY_CMD_WRITE                  0x02  // Write to Memory instruction
#define SPI_DRIVER_MEMORY_CMD_READ                   0x03  // Read from Memory instruction up to 25 MHz
#define SPI_DRIVER_MEMORY_CMD_WRDI                   0x04  // Write Disable instruction
#define SPI_DRIVER_MEMORY_CMD_READ_STATUS_REGISTER   0x05  // Read status register
#define SPI_DRIVER_MEMORY_CMD_READ_HIGH_SPEED        0x0B  // Read from Memory instruction up to 80 MHz
#define SPI_DRIVER_MEMORY_CMD_WREN                   0x06  // Write enable instruction
#define SPI_DRIVER_MEMORY_CMD_SE_4K                  0x20  // 4-KByte sector erase instruction
#define SPI_DRIVER_MEMORY_CMD_BE_32K                 0x52  // 32-KByte block erase instruction
#define SPI_DRIVER_MEMORY_CMD_WRITE_BUFFER           0xAD  // Auto Address Increment Word-Program
#define SPI_DRIVER_MEMORY_CMD_BE_64K                 0xD8  // 64-KByte block erase instruction
#define SPI_DRIVER_MEMORY_CMD_CMD_BE                 0xC7  // Bulk Erase instruction

#define SPI_DRIVER_CMD_ADR_MAX_LENGTH    4

#define SPI_SERIAL_FLASH_BUSY_BIT_MASK   0x01

#define SPI_SERIAL_MEMORY_UNLOCK_MEMORY_COMMAND     0
#define SPI_SERIAL_MEMORY_NUM_BYTES_UNLOCK_MEM_CMD  2
#endif /* SPI_H_MEMORY */

