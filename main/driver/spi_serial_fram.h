/*
 * spi_serial_fram.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 15.06.2010
 *      Author: DHA
 *
 * Purpose:
 *
 */

#ifndef SPI_SERIAL_FRAM_H_
#define SPI_SERIAL_FRAM_H_

#include "hw_configuration.h"

#define SELECT_FRAM()              GPIO_WriteBit(FRAM_CS_PORT, FRAM_CS_PIN , Bit_RESET)
#define DESELECT_FRAM()            GPIO_WriteBit(FRAM_CS_PORT, FRAM_CS_PIN , Bit_SET)

#define SPI_SERIAL_FRAM_SPI_BUS                SPI_MASTER_BUS_2
#define SPI_SERIAL_FRAM_NUM_CMD_ADR_BYTES      3
#define SPI_SERIAL_FRAM_WRITE_BUFFER_SIZE      2

#define SPI_SERIAL_FRAM_Deselect_Fram_Wait() DESELECT_FRAM()  // this needs about 2.5us and must be at least 40ns

void SPI_SERIAL_FRAM_Init(void);
bool SPI_SERIAL_FRAM_UnlockFram(void);
bool FRAM_Read(uint16_t addr, void* pBuffer, uint16_t len);
#define FRAM_ReadByte(addr, pBuffer) FRAM_Read(addr, pBuffer, 1)
#define FRAM_Read16(addr, pBuffer) FRAM_Read(addr, pBuffer, 2)
#define FRAM_Read32(addr, pBuffer) FRAM_Read(addr, pBuffer, 4)
#define FRAM_Read64(addr, pBuffer) FRAM_Read(addr, pBuffer, 8)
bool FRAM_Write(uint16_t addr, const void* pBuffer, uint16_t len);
#define FRAM_WriteByte(addr, pBuffer) FRAM_Write(addr, pBuffer, 1)
#define FRAM_Write16(addr, pBuffer) FRAM_Write(addr, pBuffer, 2)
#define FRAM_Write32(addr, pBuffer) FRAM_Write(addr, pBuffer, 4)
#define FRAM_Write64(addr, pBuffer) FRAM_Write(addr, pBuffer, 8)

#endif /* SPI_SERIAL_FRAM_H_ */
