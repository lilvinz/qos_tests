/*
 * board_info.c
 *
 *  Created on: 09.01.2014
 *      Author: vke
 */

#include "board_info.h"

enum
{
    BOARD_INFO_MAGIC = 0xf712ba55,
};

/**
 * @brief   Reads board info data from non volatile memory and checks magic.
 *
 * @param[in] nvmp      pointer to a @p BaseNVMDevice object
 * @param[out] bip      pointer to a @p BOARD_INFOT_T structure
 *
 */
bool board_info_read(BaseNVMDevice* nvmp, BOARD_INFO_T* bip)
{
    if (nvmRead(nvmp, 0, sizeof(*bip), (uint8_t*)bip) != CH_SUCCESS)
        return false;
    if (bip->magic != BOARD_INFO_MAGIC)
        return false;
    return true;
}

/**
 * @brief   Sets magic and writes board info data to non volatile memory.
 *
 * @param[in] nvmp      pointer to a @p BaseNVMDevice object
 * @param[out] bip      pointer to a @p BOARD_INFOT_T structure
 *
 */
bool board_info_write(BaseNVMDevice* nvmp, BOARD_INFO_T* bip)
{
    bip->magic = BOARD_INFO_MAGIC;
    if (nvmErase(nvmp, 0, sizeof(*bip)) != CH_SUCCESS)
        return false;
    if (nvmWrite(nvmp, 0, sizeof(*bip), (uint8_t*)bip) != CH_SUCCESS)
        return false;
    return true;
}
