/*
 * external_firmware.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 02.09.2010
 *      Author: DHA
 *
 * Purpose:
 *
 */
#include "stdheader.h"
#include "external_firmware.h"

#include "lfsr.h"
#include "flash_memory_map.h"
#include "spi_serial_flash.h"

/** @brief  This function is used to calculate the checksum.
 * @param   p_checksum: pointer to checksum
 *          byte:       actual byte
 * @retval none
 */
void EXT_FW_CalcCrc(uint32_t *p_checksum, uint8_t byte)
{
    *p_checksum ^= byte;
    for (uint8_t iBit = 0; iBit < 8; ++iBit)
    {
        if (*p_checksum & 1)
            *p_checksum = (*p_checksum >> 1) ^ 0xEDB88320;
        else
            *p_checksum >>= 1;
    }
}

/** @brief  This function can be used to determine if a valid firmware is available in external memory.
 * @param  selected_fw: firmware to check (main or mesh)
 * @retval bool:   - TRUE:     valid firmware available
 *                 - FALSE:    no valid firmware available
 */
bool EXT_FW_CheckValidFirmwareAvailable(FWT_SELECTED_FW_T selected_fw)
{
    if (EXT_FW_CheckEmpty(selected_fw) != EXT_FW_CE_RESULT_NOT_EMPTY)
        return FALSE;
    if (!EXT_FW_ExtImageIsValid(selected_fw))
        return FALSE;
    return TRUE;
}

/** @brief  This function tests if the external flash contains data within the selected area.
 * @param  selected_fw: firmware to erase (main or mesh)
 * @retval EXT_FW_RESULT_T: - EXT_FW_RESULT_ERROR       : error
 *                          - EXT_FW_RESULT_NOT_EMPTY   : memory not empty
 *                          - EXT_FW_RESULT_EMPTY       : memory empty
 */
EXT_FW_CE_RESULT_T EXT_FW_CheckEmpty(FWT_SELECTED_FW_T selected_fw)
{
    uint8_t     buffer[BLOCK_SIZE];
    uint32_t    act_address;
    uint32_t    end_address;

    switch(selected_fw)
    {
    case FWT_SELECTED_FW_MAIN:
        act_address = FLASH_MAIN_FW_BASE;
        end_address = FLASH_MAIN_FW_END;
        break;
    case FWT_SELECTED_FW_MESH:
        act_address = FLASH_MESH_FW_BASE;
        end_address = FLASH_MESH_FW_END;
        break;
    default:
        return EXT_FW_CE_RESULT_ERROR;
    }

    while (act_address < end_address)
    {
        // read data from external flash
        if (SPI_SERIAL_FLASH_ReadBuffer(act_address, (void*) buffer, BLOCK_SIZE) != SPI_SERIAL_FLASH_RESULT_FINISHED)
            return EXT_FW_CE_RESULT_ERROR;

        for (uint16_t i_ctr = 0; i_ctr < BLOCK_SIZE; i_ctr++)
        {
            if (buffer[i_ctr] != 0xFF)
                return EXT_FW_CE_RESULT_NOT_EMPTY;
        }
        act_address += BLOCK_SIZE;
    }
    return EXT_FW_CE_RESULT_EMPTY;
}

/** @brief  This function extracts the image information of the selected firmware.
 * @param  selected_fw: firmware to erase (main or mesh)
 * @retval bool:   - TRUE:     image information determined
 *                 - FALSE:    an error occurred
 */
bool EXT_FW_GetImageInformation(FWT_SELECTED_FW_T selected_fw, IMAGE_INFORMATION_T *ImageInformation)
{
    uint8_t buffer[BLOCK_SIZE];
    uint32_t ImageInformationAddress;
    uint32_t FlashAddress;
    uint32_t FlashEndAddress;
    uint32_t iByte = 0;

    switch (selected_fw)
    {
    case FWT_SELECTED_FW_MAIN:
        FlashAddress = FLASH_MAIN_FW_BASE;
        FlashEndAddress = FLASH_MAIN_FW_END;
        ImageInformationAddress = IMAGE_INFO_STRUCT_OFFSET;
        break;
    case FWT_SELECTED_FW_MESH:
        FlashAddress = FLASH_MESH_FW_BASE;
        FlashEndAddress = FLASH_MESH_FW_END;
        ImageInformationAddress = MESH_IMAGE_INFO_STRUCT_OFFSET;
        break;
    default:
        return FALSE;
    }

    LFSR_Init();

    while (FlashAddress < FlashEndAddress)
    {
        // read data from external flash
        if (SPI_SERIAL_FLASH_ReadBuffer(FlashAddress, (void*) buffer, BLOCK_SIZE) != SPI_SERIAL_FLASH_RESULT_FINISHED)
            return FALSE;
        FlashAddress += BLOCK_SIZE;

        LFSR_Decrypt(buffer, BLOCK_SIZE);

        for (uint16_t i_ctr = 0; i_ctr < BLOCK_SIZE; i_ctr++, iByte++)
        {
            if (iByte == ImageInformationAddress)
            {
                // copy image information
                if (i_ctr > (BLOCK_SIZE - sizeof(IMAGE_INFORMATION_T) - 1))
                    return FALSE; // avoid overflow
                memcpy(ImageInformation, &buffer[i_ctr], sizeof(IMAGE_INFORMATION_T));
                return TRUE;
            }
        }
    }
    return FALSE;
}

/**
 * @brief  This function tests if the external flash contains a valid firmware for the selected device.
 * @param  selected_fw: firmware to erase (main or mesh)
 * @retval bool:   - TRUE:     image is valid
 *                 - FALSE:    image is invalid or an error occurred
 */
bool EXT_FW_ExtImageIsValid(FWT_SELECTED_FW_T selected_fw)
{
    bool b_extracted_image_information = FALSE;
    bool b_finished = FALSE;
    uint8_t buffer[BLOCK_SIZE];
    uint32_t ImageInformationAddress;
    uint32_t FlashAddress;
    uint32_t FlashEndAddress;
    uint32_t checksum = 0xffffffff;
    uint32_t iByte = 0;
    IMAGE_INFORMATION_T ImageInformation;

    ImageInformation.checksum = 0;
    ImageInformation.size = 0;

    switch (selected_fw)
    {
    case FWT_SELECTED_FW_MAIN:
        FlashAddress = FLASH_MAIN_FW_BASE;
        FlashEndAddress = FLASH_MAIN_FW_END;
        ImageInformationAddress = IMAGE_INFO_STRUCT_OFFSET;
        break;
    case FWT_SELECTED_FW_MESH:
        FlashAddress = FLASH_MESH_FW_BASE;
        FlashEndAddress = FLASH_MESH_FW_END;
        ImageInformationAddress = MESH_IMAGE_INFO_STRUCT_OFFSET;
        break;
    default:
        return FALSE;
    }

    LFSR_Init();

    while ((FlashAddress < FlashEndAddress) && (!b_finished))
    {
        // read data from external flash
        if (SPI_SERIAL_FLASH_ReadBuffer(FlashAddress, (void*) buffer, BLOCK_SIZE) != SPI_SERIAL_FLASH_RESULT_FINISHED)
            return FALSE;
        FlashAddress += BLOCK_SIZE;

        LFSR_Decrypt(buffer, BLOCK_SIZE);

        for (uint16_t i_ctr = 0; i_ctr < BLOCK_SIZE; i_ctr++, iByte++)
        {
            if (iByte == ImageInformationAddress)
            {
                // copy image information
                if (i_ctr > (BLOCK_SIZE - sizeof(IMAGE_INFORMATION_T) - 1))
                    return FALSE; // avoid overflow
                memcpy(&ImageInformation, &buffer[i_ctr], sizeof(IMAGE_INFORMATION_T));
                b_extracted_image_information = TRUE;
            }
            uint8_t byte;
            if ((iByte >= ImageInformationAddress) && (iByte < ImageInformationAddress + sizeof(IMAGE_INFORMATION_T)))
                byte = 0;
            else
                byte = buffer[i_ctr];
            EXT_FW_CalcCrc(&checksum, byte);

            if (b_extracted_image_information && (iByte == ImageInformation.size - 1))
            {
                b_finished = TRUE;
                break; // reached end of firmware
            }
        }
    }

    if (!b_finished)
        return FALSE;

    // check the image size
    if (ImageInformation.size != iByte + 1)
        return FALSE;

    checksum ^= 0xffffffff;
    if (checksum != ImageInformation.checksum)
    {
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        return FALSE;
    }

    return TRUE;
}
