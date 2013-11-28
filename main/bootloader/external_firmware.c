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
#include "bootloader.h"
#include "../system/flash_memory_map.h"
#include "spi_serial_flash.h"

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

/** @brief  This function tests if the external flash contains data within the selected area.
 * @param  selected_fw: firmware to erase (main or mesh)
 * @retval bool:   - TRUE:     memory not empty
 *                 - FALSE:    memory empty or error occurred
 */
bool EXT_FW_CheckNotEmpty(FWT_SELECTED_FW_T selected_fw)
{
    uint8_t buffer[BLOCK_SIZE];
    uint32_t FlashAddress;

    switch (selected_fw)
    {
    case FWT_SELECTED_FW_MAIN:
        FlashAddress = FLASH_MAIN_FW_BASE;
        break;
    case FWT_SELECTED_FW_MESH:
        FlashAddress = FLASH_MESH_FW_BASE;
        break;
    default:
        return FALSE;
    }

    // read data from external flash
    if (!SPI_SERIAL_FLASH_ReadBuffer(FlashAddress, (void*) buffer, BLOCK_SIZE))
        return FALSE;

    for (uint16_t i_ctr = 0; i_ctr < BLOCK_SIZE; i_ctr++)
    {
        if (buffer[i_ctr] != 0xFF)
            return TRUE;
    }
    return FALSE;
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
        if (!SPI_SERIAL_FLASH_ReadBuffer(FlashAddress, (void*) buffer, BLOCK_SIZE))
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
        if (!SPI_SERIAL_FLASH_ReadBuffer(FlashAddress, (void*) buffer, BLOCK_SIZE))
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
    if (ImageInformation.size > (uint32_t)(_image_flash_end - (uint8_t*)IMAGE_INFO_BASE))
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
