/*
 * fram_mirror.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 21.01.2011
 *      Author: DHA
 *
 * Purpose: Mirroring of data in fram.
 *
 */

#include "stdheader.h"
#include "fram_mirror.h"

#include "spi_serial_fram.h"
#include "spi_memory.h"
#include "fram_memory_map.h"

/**
  * @brief  Initialize fram mirror.
  * @param  None
  * @retval None
  */
void FRAM_MIRROR_Init(void)
{

}

/**
  * @brief  Reads a block of data from the FRAM.
  * @param  addr: FRAM's internal address to read from.
  * @param  pBuffer: pointer to the buffer that receives the data read from the FRAM.
  * @param  len: number of bytes to read from the FRAM.
  * @retval None
  */
bool FRAM_MIRROR_Read(uint16_t addr, void* pBuffer, uint16_t len)
{
    return FRAM_Read(addr, pBuffer, len);
}

/**
  * @brief  Writes a block of data to the fram with mirroring.
  * @param  addr: FRAM's internal address to write to.
  * @param  pBuffer: pointer to the buffer containing the data to be written to the FRAM.
  * @param  len: number of bytes to write to the FRAM.
  * @retval None
  */
bool FRAM_MIRROR_Write(uint16_t addr, const void* pBuffer, uint16_t len)
{
    if (len == 0)
        return TRUE;

    // write bytes with mirroring
    // 1. set dirty bit in part 1 (original)
    // 2. increment write counter in part 1
    // 3. write data to part 1
    // 4. delete dirty bit in part 1
    // 5. set dirty bit in part 2 (mirror)
    // 6. increment write counter in part 2
    // 7. write data to part 2
    // 8. delete dirty bit in part 2
    bool dirty_bit;
    uint64_t write_ctr;

    // 1. set dirty bit in part 1 (original)
    dirty_bit = TRUE;
    if (!FRAM_Write(FRAM_DIRTY_BIT, &dirty_bit, FRAM_MIRROR_SIZE_OF_DIRTY_BIT))
        goto stop;

    // 2. increment write counter in part 1
    if (!FRAM_Read(FRAM_WRITE_CTR, &write_ctr, FRAM_MIRROR_SIZE_OF_WRITE_CTR))
        goto stop;
    write_ctr++;
    if (!FRAM_Write(FRAM_WRITE_CTR, &write_ctr, FRAM_MIRROR_SIZE_OF_WRITE_CTR))
        goto stop;

    // 3. write data to part 1
    if (!FRAM_Write(addr, pBuffer, len))
        goto stop;

    // 4. reset dirty bit bit in part 1
    dirty_bit = FALSE;
    if (!FRAM_Write(FRAM_DIRTY_BIT, &dirty_bit, FRAM_MIRROR_SIZE_OF_DIRTY_BIT))
        goto stop;

    return TRUE;

    // 5. set dirty bit in part 2 (mirror)
    dirty_bit = TRUE;
    if (!FRAM_Write(FRAM_DIRTY_BIT + FRAM_ADR_MIRROR_BASE, &dirty_bit, FRAM_MIRROR_SIZE_OF_DIRTY_BIT))
        goto stop;

    // 6. increment write counter in part 2
    if (!FRAM_Read(FRAM_WRITE_CTR + FRAM_ADR_MIRROR_BASE, &write_ctr, FRAM_MIRROR_SIZE_OF_WRITE_CTR))
        goto stop;
    write_ctr++;
    if (!FRAM_Write(FRAM_WRITE_CTR + FRAM_ADR_MIRROR_BASE, &write_ctr, FRAM_MIRROR_SIZE_OF_WRITE_CTR))
        goto stop;

    // 7. write data to part 2
    if (!FRAM_Write(addr + FRAM_ADR_MIRROR_BASE, pBuffer, len))
        goto stop;

    // 8. delete dirty bit in part 2
    dirty_bit = FALSE;
    if (!FRAM_Write(FRAM_DIRTY_BIT + FRAM_ADR_MIRROR_BASE, &dirty_bit, FRAM_MIRROR_SIZE_OF_DIRTY_BIT))
        goto stop;

    return TRUE;

stop:
return FALSE;
}
