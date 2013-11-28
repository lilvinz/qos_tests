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

#include "firmware_types.h"
#include "../main/image_information.h"

#define BLOCK_SIZE  256

void EXT_FW_CalcCrc(uint32_t *p_checksum, uint8_t byte);
bool EXT_FW_CheckNotEmpty(FWT_SELECTED_FW_T selected_fw);
bool EXT_FW_GetImageInformation(FWT_SELECTED_FW_T selected_fw, IMAGE_INFORMATION_T *ImageInformation);
bool EXT_FW_ExtImageIsValid(FWT_SELECTED_FW_T selected_fw);

#endif /* EXTERNAL_FIRMWARE_H_ */
