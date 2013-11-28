/*
 * external_firmware.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 02.09.2010
 *      Author: DHA
 *
 * Purpose:
 *
 */
#ifndef EXTERNAL_FIRMWARE_H_
#define EXTERNAL_FIRMWARE_H_

#include "image_information.h"
#include "firmware_types.h"

#define BLOCK_SIZE  256

typedef enum
{
    EXT_FW_CE_RESULT_ERROR = -1,
    EXT_FW_CE_RESULT_NOT_EMPTY = 0,
    EXT_FW_CE_RESULT_EMPTY = 1
} EXT_FW_CE_RESULT_T;

void EXT_FW_CalcCrc(uint32_t *p_checksum, uint8_t byte);
EXT_FW_CE_RESULT_T EXT_FW_CheckEmpty(FWT_SELECTED_FW_T selected_fw);
bool EXT_FW_CheckValidFirmwareAvailable(FWT_SELECTED_FW_T selected_fw);
bool EXT_FW_GetImageInformation(FWT_SELECTED_FW_T selected_fw, IMAGE_INFORMATION_T *ImageInformation);
bool EXT_FW_ExtImageIsValid(FWT_SELECTED_FW_T selected_fw);

#endif /* EXTERNAL_FIRMWARE_H_ */
