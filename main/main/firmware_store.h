#ifndef FIRMWARE_STORE_H
#define FIRMWARE_STORE_H

#include "image_information.h"
#include "firmware_types.h"


#define FISTO_BLOCK_SIZE                    128U
#define FISTO_BLOCK_COUNT                   (FISTO_FILE_SIZE_MAX / FISTO_BLOCK_SIZE)        // Firmware size / block size

#define FISTO_STATE_INITIALIZE_TIMEOUT      10000UL //erasing 256kBytes of FlashEEP should not take that long
#define FISTO_STATE_DOWNLOAD_TIMEOUT        (20UL*60UL*1000UL)
#define FISTO_STATE_CHECK_CHECKSUM_TIMEOUT  20000UL

#define FISTO_CRC32_POLY                    0xEDB88320UL


typedef enum
{
    FISTO_OK,
    FISTO_PENDING,
    FISTO_BUSY,
    FISTO_ERROR,
    FISTO_ERROR_PARAM,
    FISTO_ERROR_STATE,
    FISTO_ABORT
} FISTO_RESULT_T;

typedef enum
{
    FISTO_CHK_COMPUTING,
    FISTO_CHK_OK,
    FISTO_CHK_ERROR,
} FISTO_CHECKSUM_STATE_T;

enum
{
    FISTO_STATE_IDLE,
    FISTO_STATE_INITIALIZE,
    FISTO_STATE_DOWNLOADING,
    FISTO_STATE_CHECK_CHECKSUM,
    FISTO_STATE_READY,
    FISTO_STATE_INITIALIZE_ERROR,
    FISTO_STATE_CHECKSUM_ERROR
};


FISTO_RESULT_T FISTO_Write(uint32_t uAddress, uint8_t uLen, uint8_t *pData);
int8_t FISTO_Read(uint32_t uAddress, uint8_t uLen, uint8_t *pData);
bool FISTO_StartReceiver(IMAGE_INFORMATION_T * pFileInfo, FWT_SELECTED_FW_T FwType);
bool FISTO_Finish(IMAGE_INFORMATION_T * pFileInfo);
bool FISTO_Open(FWT_SELECTED_FW_T FwType);
bool FISTO_GetFileInfo(IMAGE_INFORMATION_T **ppsFileInfo);
bool FISTO_Erase(uint32_t uStartAddr, uint32_t uLength);
bool FISTO_IsActive(void);

void FISTO_Tick(void);
void FISTO_Init(void);


#endif
