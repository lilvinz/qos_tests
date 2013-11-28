/*
 * spi_master.h
 *
 *  Created on: 28.09.2010
 *      Author: vinz
 */

#ifndef SPI_MASTER_H_
#define SPI_MASTER_H_


/* Simple SPI Master Driver for FreeRTOS
 *
 * This driver only supports the most basic SPI setup:
 * - two wire
 * - full duplex
 * - no checksum
 * - only supports 8bit datasize
 * - no dma support
 *
 * Possible enhancements for the future could be
 * -
 * -
 *
 */

#define SPI_MASTER_TIMEOUT      100

typedef enum
{
    SPI_MASTER_BUS_1,
#if defined(STM32F10X_MD) || defined(STM32F10X_MD_VL) || defined(STM32F10X_HD) || defined(STM32F10X_XL) || defined(STM32F2XX)
    SPI_MASTER_BUS_2,
#endif
#if defined(STM32F10X_HD) || defined(STM32F10X_XL) || defined(STM32F2XX)
    SPI_MASTER_BUS_3,
#endif
    SPI_MASTER_BUS_COUNT
} SPI_MASTER_BUS_E;


void SPI_MASTER_Init(SPI_MASTER_BUS_E SpiBus, SPI_InitTypeDef* SPI_InitStruct);

bool SPI_MASTER_Transceive(SPI_MASTER_BUS_E SpiBus, const uint8_t *pDataMOSI, uint8_t *pDataMISO, uint16_t Length);
#define SPI_MASTER_Transmit(SpiBus, pDataMOSI, Length) SPI_MASTER_Transceive(SpiBus, pDataMOSI, 0, Length)
#define SPI_MASTER_Receive(SpiBus, pDataMISO, Length) SPI_MASTER_Transceive(SpiBus, 0, pDataMISO, Length)

#define SPI_MASTER_TransceiveByte(SpiBus, pDataMOSI, pDataMISO) SPI_MASTER_Transceive(SpiBus, pDataMOSI, pDataMISO, 1)
#define SPI_MASTER_TransmitByte(SpiBus, pDataMOSI) SPI_MASTER_Transmit(SpiBus, pDataMOSI, 1)
#define SPI_MASTER_ReceiveByte(SpiBus, pDataMISO) SPI_MASTER_Receive(SpiBus, pDataMISO, 1)

#endif /* SPI_MASTER_H_ */
