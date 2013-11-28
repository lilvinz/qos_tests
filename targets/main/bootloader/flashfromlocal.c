
#include "stdheader.h"
#include "flashfromlocal.h"

#include "bootloader.h"
#include "../main/image_information.h"
#include "firmware_types.h"
#include "spi_serial_flash.h"
#include "spi_serial_fram.h"
#include "fram_mirror.h"
#include "../system/flash_memory_map.h"
#include "../system/fram_memory_map.h"
#include "fram_mirror.h"
#include "external_firmware.h"
#include "lfsr.h"
#include "uart.h"
#include "delay.h"
#include "bootloader_information.h"

static const char FFL_ERASING_FLASH[] = "\r\nErasing flash.\r\n";
static const char FFL_INSTALLING_FIRMWARE[] = "\r\nInstalling firmware.\r\n";
static const char FFL_PLEASE_WAIT[] = "Please wait!\r\n";
static const char FFL_WRONG_HW_ID[] = "\nError! Wrong Hardware ID!\r\n";
static const char FFL_WRONG_HW_REVISION[] = "\nError! Wrong Hardware revision!\r\n";

bool FFL_ResetRetryCounter(void)
{
    uint8_t data = FFL_MAX_RETRIES;
    if (!FRAM_MIRROR_WriteByte(FRAM_FW_UPDATE_RETRIES, &data))
        return FALSE;
    return TRUE;
}

bool FFL_GetRetryCounter(uint8_t *pdata)
{
    if (!FRAM_MIRROR_ReadByte(FRAM_FW_UPDATE_RETRIES, pdata))
        return FALSE;
    return TRUE;
}

bool FFL_DecrementRetryCounter(void)
{
    uint8_t data;
    if (!FRAM_MIRROR_ReadByte(FRAM_FW_UPDATE_RETRIES, &data))
        return FALSE;
    data--;
    if (!FRAM_MIRROR_WriteByte(FRAM_FW_UPDATE_RETRIES, &data))
        return FALSE;
    return TRUE;
}

bool FFL_Start(void)
{
    uint8_t buffer[BLOCK_SIZE];
    uint8_t uErrorCount;
    uint8_t uErrorCtr;
    uint32_t extMainFWAddress;
    uint32_t uFlashAddress;
    uint32_t iByte = 0;
    IMAGE_INFORMATION_T sFileInfo;

    IWDG_ReloadCounter();  // Reload IWDG counter
    if (!EXT_FW_CheckNotEmpty(FWT_SELECTED_FW_MAIN))
        return FALSE; // return if firmware is not available
    if (!EXT_FW_GetImageInformation(FWT_SELECTED_FW_MAIN, &sFileInfo))
        return FALSE; // return if image information can not be read

#if (IMAGE_INFO_BASE != 0x08008000)
#error "Adjust flash sector here!"
#endif

    if (sFileInfo.hw_id != bootloader_information.hw_id)
    {
        UART5_SendString(FFL_WRONG_HW_ID);
        return FALSE;
    }

    if ((sFileInfo.hw_revision & 0xFF00) < (bootloader_information.hw_revision & 0xFF00))
    {
        UART5_SendString(FFL_WRONG_HW_REVISION);
        return FALSE;
    }

    uFlashAddress = FLASH_Sector_2; //0x08008000
    extMainFWAddress = FLASH_MAIN_FW_BASE;

    FLASH_Unlock(); // Unlock the Flash Program Erase controller
    uErrorCtr = 0;

    /* Clear All pending flags */
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    UART5_SendString(FFL_ERASING_FLASH);
    UART5_SendString(FFL_PLEASE_WAIT);

    IWDG_ReloadCounter();  // Reload IWDG counter

    while (uFlashAddress <= FLASH_Sector_11)
    {
        if (FLASH_EraseSector(uFlashAddress, VoltageRange_3) == FLASH_COMPLETE)     //<Device operating range: 2.7V to 3.6V as we don't have external Vpp (7V - 9V)
        {
            uFlashAddress += 0x08;
            uErrorCtr = 0;
        }
        else
        {
            uErrorCtr++;
            if (uErrorCtr > FFL_MAX_ERROR_COUNT)
                return FALSE;
        }
        UART5_PutChar('-');
    }

    uErrorCtr = 0;
    while (FLASH_WaitForLastOperation() != FLASH_COMPLETE)
    {
        uErrorCtr++;
        if (uErrorCtr > FFL_MAX_ERROR_COUNT)
            return FALSE;
    }

    LFSR_Init();
    uFlashAddress = IMAGE_INFO_BASE;
    uErrorCount = 0;
    uErrorCtr = 0;

    UART5_SendString(FFL_INSTALLING_FIRMWARE);
    UART5_SendString(FFL_PLEASE_WAIT);

    IWDG_ReloadCounter();  // Reload IWDG counter
    while (iByte < sFileInfo.size)
    {
        // read data from external flash
        if (SPI_SERIAL_FLASH_ReadBuffer(extMainFWAddress, (void*) buffer, BLOCK_SIZE))
        {
            LFSR_Decrypt(buffer, BLOCK_SIZE); // decrypt firmware

            for (uint16_t i_ctr = 0; i_ctr < BLOCK_SIZE;)
            {
                static volatile uint32_t u_data;
                u_data = (buffer[i_ctr + 3] << 24) | (buffer[i_ctr + 2] << 16) | (buffer[i_ctr + 1] << 8) | (buffer[i_ctr]);

                while (FLASH_ProgramWord((uFlashAddress + i_ctr), u_data) != FLASH_COMPLETE) // write firmware
                {
                    uErrorCtr++;
                    if (uErrorCtr > FFL_MAX_ERROR_COUNT)
                        return FALSE;
                }
                iByte += 4;
                i_ctr += 4;
                uErrorCtr = 0;
                if (iByte >= sFileInfo.size)
                    break;
            }
            uFlashAddress += BLOCK_SIZE;
            extMainFWAddress += BLOCK_SIZE;
            uErrorCount = 0;
        }
        else
        {
            uErrorCount++;
            if (uErrorCount > FFL_MAX_ERROR_COUNT)
                return FALSE;
            DELAY_MS(FFL_ERROR_WAIT_TIME_MS);
        }
        UART5_PutChar('.');
    }

    return TRUE;
}
