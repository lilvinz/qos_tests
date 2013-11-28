/*
 * bootloaderupdater.c
 *
 *  Created on: 20.03.2013
 *      Author: cba
 */

#include <string.h>
#include "board.h"
#include "flashd.h"
#include "wdtCtrl.h"
#include "resetReason.h"


#define BOOTLOADER_ORG 0x100000
#define BOOTLOADER_SIZE 0x4000

#define BOOTLOADERUPDATER_MAX_RETRY 10


extern uint8_t _binary_start;
extern uint8_t _binary_end;

static const uint8_t * embedded_image_start = (uint8_t *) &(_binary_start);
static const uint8_t * embedded_image_end = (uint8_t *) &(_binary_end);
#define embedded_image_size (embedded_image_end - embedded_image_start)


void BootloaderUpdater(void)
{
    uint8_t retrycount = BOOTLOADERUPDATER_MAX_RETRY;

    HAL_ResetWdt();

    // Make sure not to break boundaries.
    if (embedded_image_size > BOOTLOADER_SIZE)
        return;

    // Check if embedded file differs from installed bootloader.
    if (memcmp((void*)embedded_image_start, (void*)BOOTLOADER_ORG, embedded_image_size) == 0)
        return;

    do
    {
        HAL_ResetWdt();

        FLASHD_Initialize(BOARD_MCK);

        // Remove flash protection
        FLASHD_Unlock(BOOTLOADER_ORG, BOOTLOADER_ORG + BOOTLOADER_SIZE, 0, 0);

        // Update
        FLASHD_Write(BOOTLOADER_ORG, embedded_image_start, embedded_image_size);

        // Enable flash protection
        FLASHD_Lock(BOOTLOADER_ORG, BOOTLOADER_ORG + BOOTLOADER_SIZE, 0, 0);

        if (memcmp((void*)embedded_image_start, (void*)BOOTLOADER_ORG, embedded_image_size) == 0)
            break;

        --retrycount;

    } while (retrycount > 0);

    // Reset to restart with new bootloader
    HAL_WarmReset();

}

