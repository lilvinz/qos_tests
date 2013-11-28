
#include "stdheader.h"
#include "firmware_store.h"

#include "version.h"
#include "state.h"
#include "system.h"
#include "flash_memory_map.h"
#include "spi_serial_flash.h"
#include "debug.h"
#include "log.h"
#include "external_firmware.h"
#include "mesh_fw_manager.h"
#include "mesh.h"
#include "fw_transfer.h"
#include "fram_mirror.h"

static FWT_SELECTED_FW_T currentFwType;
static uint32_t uBaseAddress;

static bool     bDownloadActive;

static uint32_t    uFileSize;

#if (VERSION_IS_DEBUG > 0)
static volatile uint32_t uLastAddress;
static volatile uint32_t uLastSize;
#endif

static IMAGE_INFORMATION_T  sFileInfo;
static bool bFileInfoValid = FALSE;

bool getBaseAddress(FWT_SELECTED_FW_T FwType, uint32_t *dst)
{
    switch (FwType)
    {
    case FWT_SELECTED_FW_MAIN:
        *dst = FLASH_MAIN_FW_BASE;
        return TRUE;
    case FWT_SELECTED_FW_MESH:
        *dst = FLASH_MESH_FW_BASE;
        return TRUE;
    default:
        break;
    }
    return FALSE;
}

FISTO_RESULT_T FISTO_Write(uint32_t uAddress, uint8_t uLen, uint8_t *pData)
{
    if (bDownloadActive != TRUE)
        return FISTO_ABORT;

#if (VERSION_IS_DEBUG > 0)

    if (uLastAddress)
    {
        if ((uLastAddress + uLastSize != uAddress))
        {
            DPRINT3(DEBUG_GENERAL, "Last A:%X L:%u S:%u\n", uLastAddress, uLastSize, uLastAddress + uLastSize);
            DPRINT3(DEBUG_GENERAL, "Recv A:%X L:%u S:%u\n", uAddress, uLen, uAddress + uLen);
            if (uLastAddress == uAddress)
            {
                DPRINT(DEBUG_GENERAL, "DUP!\n");
                //duplicate received.
                return FISTO_OK;
            }
            else
            {
                DPRINT(DEBUG_GENERAL, "ERROR: Paket Order\n");
                bDownloadActive = FALSE;
                return FISTO_ABORT;
            }
        }
    }

    uLastAddress = uAddress;
    uLastSize = uLen;
#endif


//the following is a test function. send a file with ASCII 01234567890123...
//#define FWSTORE_CHECK_FWTRANSFER 1
#if (FWSTORE_CHECK_FWTRANSFER == 1)
    {
        uint8_t idx;
        static uint8_t last_number = '9';

        for (idx = 0; idx < uLen; idx++)
        {
            if (pData[idx] >= '1' && pData[idx] <= '9')
            {
                if (pData[idx] != last_number + 1)
                {
                    DPRINT3(DEBUG_GENERAL, "ERROR: Continuosity at index %u. Curr: %u Last: %u\n", idx, pData[idx] - '0', last_number - '0');
                }
            }
            else if (pData[idx] == '0')
            {
                if (last_number != '9')
                {
                    DPRINT3(DEBUG_GENERAL, "ERROR: Continuosity at index %u. Curr: %u Last: %u\n", idx, pData[idx] - '0', last_number - '0');
                }
            }
            else
            {
                DPRINT1(DEBUG_GENERAL, "ERROR: Not A Digit: ASCII %u\n", pData[idx]);
            }

            last_number = pData[idx];
        }

    }
#endif


    SPI_SERIAL_FLASH_RESULT_T result = SPI_SERIAL_FLASH_RESULT_BUSY;
    while (result == SPI_SERIAL_FLASH_RESULT_BUSY)
    {
        result = SPI_SERIAL_FLASH_WriteBuffer(uBaseAddress + uAddress, pData, uLen);
    }
    if (result == SPI_SERIAL_FLASH_RESULT_ERROR)
    {
        DPRINT(DEBUG_GENERAL, "Flash write error!\n");
        return FISTO_ERROR;
    }


    uFileSize = uAddress + uLen;

#if (FWSTORE_CHECK_FWTRANSFER != 1)
    if (uFileSize >= sFileInfo.size)
    {
        DPRINT1(DEBUG_GENERAL, "%u Bytes received!\n", uFileSize);

        bDownloadActive = FALSE;

        if (EXT_FW_CheckValidFirmwareAvailable(currentFwType))
        {
            DPRINT(DEBUG_GENERAL, "Firmware valid!\n");
            LOG(LOG_ID_MESH_FIRMWARE_RECEIVE_SUCCESS, currentFwType, sFileInfo.sw_version_major, sFileInfo.sw_version_minor, NULL);

            switch (currentFwType)
            {
            case FWT_SELECTED_FW_MAIN:
            {
                uint8_t data = FW_AUTO_UPDATE_MAX_RETRIES;
                FRAM_MIRROR_WriteByte(FRAM_FW_UPDATE_RETRIES, &data);  // reset retry counter
                if (sFileInfo.b_manual_update != TRUE)
                {
                    DPRINT(DEBUG_GENERAL, "Reboot to bootloader...\n");
                    LOG(LOG_ID_MAIN_FIRMWARE_UPDATE, sFileInfo.sw_version_major, sFileInfo.sw_version_minor, 0, NULL);
                    SYSTEM_InvokeBootloader();
                }
                return FISTO_OK;
            }
            break;
            case FWT_SELECTED_FW_MESH:
                if (sFileInfo.b_manual_update != TRUE)
                {
                    DPRINT(DEBUG_GENERAL, "Starting Mesh Firmware update...\n");
                    LOG(LOG_ID_MESH_FIRMWARE_UPDATE, sFileInfo.sw_version_major, sFileInfo.sw_version_minor, 0, NULL);
                    MESH_FirmwareUpdateStart();
                }
                return FISTO_OK;
                break;
            default:
                break;
            }
        }
        else
        {
            DPRINT(DEBUG_GENERAL, "Firmware INVALID!\n");
            LOG(LOG_ID_MESH_FIRMWARE_RECEIVE_FAILED, currentFwType, sFileInfo.sw_version_major, sFileInfo.sw_version_minor, NULL);
        }
    }
#endif

    return FISTO_OK;
}

int8_t FISTO_Read(uint32_t uAddress, uint8_t uLen, uint8_t *pDst)
{
    if (uAddress > sFileInfo.size)
        return -1;

    if (uLen > 127)
        return -2;

    if (uAddress + uLen > sFileInfo.size)
        uLen = sFileInfo.size - uAddress;

    if (uLen == 0)
        return 0;

    if (SPI_SERIAL_FLASH_ReadBuffer(uBaseAddress + uAddress, pDst, uLen) == SPI_SERIAL_FLASH_RESULT_FINISHED)
        return (int8_t)uLen;

    return -3;
}

bool FISTO_Open(FWT_SELECTED_FW_T FwType)
{
    if (bDownloadActive)
        return FALSE;

    if (EXT_FW_CheckValidFirmwareAvailable(FwType))
    {
        EXT_FW_GetImageInformation(FwType, &sFileInfo);
        currentFwType = FwType;
        getBaseAddress(FwType, &uBaseAddress);
        bFileInfoValid = TRUE;
        return TRUE;
    }

    bFileInfoValid = FALSE;
    return FALSE;
}

bool FISTO_GetFileInfo(IMAGE_INFORMATION_T **ppsFileInfo)
{
    if (TRUE == bFileInfoValid)
    {
        *ppsFileInfo = &sFileInfo;
        return TRUE;
    }
    return FALSE;
}

bool FISTO_StartReceiver(IMAGE_INFORMATION_T * pFileInfo, FWT_SELECTED_FW_T FwType)
{
    DPRINT(DEBUG_GENERAL, "Start Receiver!\n");

    DPRINT(DEBUG_GENERAL, "File info received\n");

    sFileInfo.hw_id             = pFileInfo->hw_id;
    sFileInfo.hw_revision       = pFileInfo->hw_revision;
    sFileInfo.sw_version_major  = pFileInfo->sw_version_major;
    sFileInfo.sw_version_minor  = pFileInfo->sw_version_minor;
    sFileInfo.checksum          = pFileInfo->checksum;
    sFileInfo.size              = pFileInfo->size;

    DPRINT1(DEBUG_GENERAL, "HW ID:      0x%04X\n", sFileInfo.hw_id);
    DPRINT1(DEBUG_GENERAL, "HW Rev:     0x%04X\n", sFileInfo.hw_revision);
    DPRINT2(DEBUG_GENERAL, "SW Version: %u.%02u\n", sFileInfo.sw_version_major, sFileInfo.sw_version_minor);
    DPRINT1(DEBUG_GENERAL, "Checksum:   0x%08X\n", sFileInfo.checksum);
    DPRINT1(DEBUG_GENERAL, "Size:       0x%04X\n", sFileInfo.size);
    DPRINT1(DEBUG_GENERAL, "HWID:       0x%04X\n", sFileInfo.hw_id);

#if (VERSION_IS_DEBUG > 0)
    uLastAddress = 0;
#endif

    uFileSize = 0;


    xSemaphoreHandle wait_mutex;
    if (!FW_TRANSFER_EraseFw(FwType, &wait_mutex))
        return FALSE;
    else
        while (xSemaphoreTake(wait_mutex, portMAX_DELAY) != pdTRUE) {} // wait until erase process has finished

    currentFwType = FwType;
    getBaseAddress(FwType, &uBaseAddress);

    uFileSize = 0;
    bDownloadActive = TRUE;
    bFileInfoValid = TRUE;

    LOG(LOG_ID_MESH_FIRMWARE_RECEIVE_START, FwType, sFileInfo.sw_version_major, sFileInfo.sw_version_minor, NULL);

    return TRUE;
}

bool FISTO_IsActive(void)
{
    return bDownloadActive;
}
