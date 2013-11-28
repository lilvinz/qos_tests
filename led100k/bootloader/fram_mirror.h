/*
 * fram_mirror.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 15.06.2010
 *      Author: DHA
 *
 * Purpose:
 *
 */

#ifndef FRAM_MIRROR_H_
#define FRAM_MIRROR_H_

#define FRAM_MIRROR_SIZE_OF_DIRTY_BIT   1
#define FRAM_MIRROR_SIZE_OF_WRITE_CTR   8

void FRAM_MIRROR_Init(void);
bool FRAM_MIRROR_Read(uint16_t addr, void* pBuffer, uint16_t len);
#define FRAM_MIRROR_ReadByte(addr, pBuffer) FRAM_MIRROR_Read(addr, pBuffer, 1)
#define FRAM_MIRROR_Read16(addr, pBuffer) FRAM_MIRROR_Read(addr, pBuffer, 2)
#define FRAM_MIRROR_Read32(addr, pBuffer) FRAM_MIRROR_Read(addr, pBuffer, 4)
#define FRAM_MIRROR_Read64(addr, pBuffer) FRAM_MIRROR_Read(addr, pBuffer, 8)
bool FRAM_MIRROR_Write(uint16_t addr, const void* pBuffer, uint16_t len);
#define FRAM_MIRROR_WriteByte(addr, pBuffer) FRAM_MIRROR_Write(addr, pBuffer, 1)
#define FRAM_MIRROR_Write16(addr, pBuffer) FRAM_MIRROR_Write(addr, pBuffer, 2)
#define FRAM_MIRROR_Write32(addr, pBuffer) FRAM_MIRROR_Write(addr, pBuffer, 4)
#define FRAM_MIRROR_Write64(addr, pBuffer) FRAM_MIRROR_Write(addr, pBuffer, 8)
#endif
/* FRAM_MIRROR_H_ */
