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

/**
  * @brief  Restore the selected section of the fram.
  * @param  section: selected section.
  * @retval bool:   - TRUE:     section restored
  *                 - FALSE:    an error occurred
  */
bool FRAM_MIRROR_RestoreSection(FRAM_MIRROR_SECTION_T section)
{
    bool dirty_bit;
    uint8_t     buffer[FRAM_MIRROR_RESTORE_BUFFER_SIZE];
    uint16_t    ReadAddress;
    uint16_t    WriteAddress;
    uint16_t    EndAddress;
    uint16_t    DirtyBitAddress;
    uint16_t    WriteCtrAddressRead;
    uint16_t    WriteCtrAddressWrite;

    switch (section)
    {
    case FRAM_MIRROR_SECTION_1:
        ReadAddress = FRAM_ADR_MIRROR_BASE + FRAM_MIRROR_SECTION_OFFSET;
        WriteAddress = FRAM_MIRROR_SECTION_OFFSET;
        EndAddress = FRAM_ADR_MIRROR_BASE;
        DirtyBitAddress = FRAM_DIRTY_BIT;
        WriteCtrAddressRead = FRAM_ADR_MIRROR_BASE + FRAM_WRITE_CTR;
        WriteCtrAddressWrite = FRAM_WRITE_CTR;
        break;
    case FRAM_MIRROR_SECTION_2:
        ReadAddress = FRAM_MIRROR_SECTION_OFFSET;
        WriteAddress = FRAM_ADR_MIRROR_BASE + FRAM_MIRROR_SECTION_OFFSET;
        EndAddress = 2 * FRAM_ADR_MIRROR_BASE;
        DirtyBitAddress = FRAM_ADR_MIRROR_BASE + FRAM_DIRTY_BIT;
        WriteCtrAddressRead = FRAM_WRITE_CTR;
        WriteCtrAddressWrite = FRAM_ADR_MIRROR_BASE + FRAM_WRITE_CTR;
        break;
    default:
        return FALSE;
    }

    // 1. set dirty bit in target section
    dirty_bit = TRUE;
    if (!FRAM_Write(DirtyBitAddress, &dirty_bit, FRAM_MIRROR_SIZE_OF_DIRTY_BIT))
        goto stop;

    // 2. copy section
    while (WriteAddress < EndAddress)
    {
        if (!FRAM_Read(ReadAddress, buffer, FRAM_MIRROR_RESTORE_BUFFER_SIZE))
            goto stop;
        if (!FRAM_Write(WriteAddress, buffer, FRAM_MIRROR_RESTORE_BUFFER_SIZE))
            goto stop;
        ReadAddress += FRAM_MIRROR_SECTION_OFFSET;
        WriteAddress += FRAM_MIRROR_SECTION_OFFSET;
    }

    // 3. copy write counter
    if (!FRAM_Read(WriteCtrAddressRead, buffer, FRAM_MIRROR_SIZE_OF_WRITE_CTR))
        goto stop;
    if (!FRAM_Write(WriteCtrAddressWrite, buffer, FRAM_MIRROR_SIZE_OF_WRITE_CTR))
        goto stop;

    // 4. reset dirty bit in target section
    dirty_bit = FALSE;
    if (!FRAM_Write(DirtyBitAddress, &dirty_bit, FRAM_MIRROR_SIZE_OF_DIRTY_BIT))
        goto stop;

    return TRUE;

stop:
    return FALSE;
}

/**
  * @brief  Initialize fram mirror.
  * @param  None
  * @retval bool:   - TRUE:     command executed
  *                 - FALSE:    an error occurred
  */
bool FRAM_MIRROR_Init(void)
{
    // check data in fram
    bool b_dirty_bit_section_1;
    bool b_dirty_bit_section_2;
    uint64_t write_ctr_section_1;
    uint64_t write_ctr_section_2;

    // read dirty bits
    if (!FRAM_Read(FRAM_DIRTY_BIT, &b_dirty_bit_section_1, FRAM_MIRROR_SIZE_OF_DIRTY_BIT))
        goto stop;
    if (!FRAM_Read(FRAM_DIRTY_BIT + FRAM_ADR_MIRROR_BASE, &b_dirty_bit_section_2, FRAM_MIRROR_SIZE_OF_DIRTY_BIT))
        goto stop;

    // read write counters
    if (!FRAM_Read(FRAM_WRITE_CTR, &write_ctr_section_1, FRAM_MIRROR_SIZE_OF_WRITE_CTR))
        goto stop;
    if (!FRAM_Read(FRAM_WRITE_CTR + FRAM_ADR_MIRROR_BASE, &write_ctr_section_2, FRAM_MIRROR_SIZE_OF_WRITE_CTR))
        goto stop;

    // check dirty bits
    if ((!b_dirty_bit_section_1) && (!b_dirty_bit_section_2))
    {
        return TRUE;  // everything ok
    }

    if (b_dirty_bit_section_1 && (!b_dirty_bit_section_2))
    {
        // copy data from section 2 to section 1
        return FRAM_MIRROR_RestoreSection(FRAM_MIRROR_SECTION_1);
    }

    if (b_dirty_bit_section_2 && (!b_dirty_bit_section_1))
    {
        // copy data from section 1 to section 2
        return FRAM_MIRROR_RestoreSection(FRAM_MIRROR_SECTION_2);
    }

    // if both dirty bits are set (this should not happen) -> check write counters
    if (write_ctr_section_1 > write_ctr_section_2)
    {
        // reset dirty bit in target section 2
        b_dirty_bit_section_2 = FALSE;
        if (!FRAM_Write(FRAM_DIRTY_BIT + FRAM_ADR_MIRROR_BASE, &b_dirty_bit_section_2, FRAM_MIRROR_SIZE_OF_DIRTY_BIT))
            goto stop;
        // copy data from section 2 to section 1
        return FRAM_MIRROR_RestoreSection(FRAM_MIRROR_SECTION_1);
    }
    else
    {
        // reset dirty bit in section 1
        b_dirty_bit_section_1 = FALSE;
        if (!FRAM_Write(FRAM_DIRTY_BIT, &b_dirty_bit_section_1, FRAM_MIRROR_SIZE_OF_DIRTY_BIT))
            goto stop;
        // copy data from section 1 to section 2
        return FRAM_MIRROR_RestoreSection(FRAM_MIRROR_SECTION_2);
    }

stop:
    return FALSE;
}

/**
  * @brief  Reads a block of data from the FRAM.
  * @param  addr: FRAM's internal address to read from.
  * @param  pBuffer: pointer to the buffer that receives the data read from the FRAM.
  * @param  len: number of bytes to read from the FRAM.
  * @retval bool:   - TRUE:     dara read
  *                 - FALSE:    an error occurred
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
  * @retval bool:   - TRUE:     data written
  *                 - FALSE:    an error occurred
  */
bool FRAM_MIRROR_Write(uint16_t addr, const void* pBuffer, uint16_t len)
{
    if (len == 0)
    {
        return TRUE;
    }

    // write bytes with mirroring
    // 1. set dirty bit in section 1 (original)
    // 2. increment write counter in section 1
    // 3. write data to section 1
    // 4. delete dirty bit in section 1
    // 5. set dirty bit in section 2 (mirror)
    // 6. increment write counter in section 2
    // 7. write data to section 2
    // 8. delete dirty bit in section 2
    bool dirty_bit;
    uint64_t write_ctr;

    // 1. set dirty bit in section 1 (original)
    dirty_bit = TRUE;
    if (!FRAM_Write(FRAM_DIRTY_BIT, &dirty_bit, FRAM_MIRROR_SIZE_OF_DIRTY_BIT))
        goto stop;

    // 2. increment write counter in section 1
    if (!FRAM_Read(FRAM_WRITE_CTR, &write_ctr, FRAM_MIRROR_SIZE_OF_WRITE_CTR))
        goto stop;
    write_ctr++;
    if (!FRAM_Write(FRAM_WRITE_CTR, &write_ctr, FRAM_MIRROR_SIZE_OF_WRITE_CTR))
        goto stop;

    // 3. write data to section 1
    if (!FRAM_Write(addr, pBuffer, len))
        goto stop;

    // 4. reset dirty bit bit in section 1
    dirty_bit = FALSE;
    if (!FRAM_Write(FRAM_DIRTY_BIT, &dirty_bit, FRAM_MIRROR_SIZE_OF_DIRTY_BIT))
        goto stop;

    // 5. set dirty bit in section 2 (mirror)
    dirty_bit = TRUE;
    if (!FRAM_Write(FRAM_DIRTY_BIT + FRAM_ADR_MIRROR_BASE, &dirty_bit, FRAM_MIRROR_SIZE_OF_DIRTY_BIT))
        goto stop;

    // 6. increment write counter in section 2
    if (!FRAM_Read(FRAM_WRITE_CTR + FRAM_ADR_MIRROR_BASE, &write_ctr, FRAM_MIRROR_SIZE_OF_WRITE_CTR))
        goto stop;
    write_ctr++;
    if (!FRAM_Write(FRAM_WRITE_CTR + FRAM_ADR_MIRROR_BASE, &write_ctr, FRAM_MIRROR_SIZE_OF_WRITE_CTR))
        goto stop;

    // 7. write data to section 2
    if (!FRAM_Write(addr + FRAM_ADR_MIRROR_BASE, pBuffer, len))
        goto stop;

    // 8. delete dirty bit in section 2
    dirty_bit = FALSE;
    if (!FRAM_Write(FRAM_DIRTY_BIT + FRAM_ADR_MIRROR_BASE, &dirty_bit, FRAM_MIRROR_SIZE_OF_DIRTY_BIT))
        goto stop;

    return TRUE;

stop:
    return FALSE;
}
