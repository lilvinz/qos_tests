/*
 * bootloader_information.h
 *
 *  Created on: 04.10.2010
 *      Author: DHA
 */

#ifndef BOOTLOADER_INFORMATION_H_
#define BOOTLOADER_INFORMATION_H_

#include "../main/config.h"

#define BOOTLOADER_VERSION_MAJOR                1
#define BOOTLOADER_VERSION_MINOR                10

#if (defined HWT_MULTI) || (defined HWT_MEDIUM_WHITE)
#define BOOTLOADER_VERSION_HARDWARE_ID          0x2200  // hardware id sirius
#define BOOTLOADER_VERSION_HARDWARE_REVISION    0x0200
#elif defined HWT_MEDIUM_RED
#define BOOTLOADER_VERSION_HARDWARE_ID          0x2204  // hardware id 2000 cd
#define BOOTLOADER_VERSION_HARDWARE_REVISION    0x0101
#elif defined HWT_HIGH_MAIN
#define BOOTLOADER_VERSION_HARDWARE_ID          0x2201  // hardware id mainboard supernova
#define BOOTLOADER_VERSION_HARDWARE_REVISION    0x0100
#elif defined HWT_HIGH_SV
#define BOOTLOADER_VERSION_HARDWARE_ID          0x2202  // hardware id led driver board supernova
#define BOOTLOADER_VERSION_HARDWARE_REVISION    0x0100
#error "Hardware type not supported!"
#else
#error "Hardware type not defined or unknown!"
#endif

#ifdef DEBUG_BUILD
#define VERSION_IS_DEBUG 1
#endif

typedef struct
{
    uint8_t     sw_version_major;
    uint8_t     sw_version_minor;
    uint16_t    hw_id;
    uint16_t    hw_revision;
} BOOTLOADER_INFORMATION_T;

extern BOOTLOADER_INFORMATION_T bootloader_information;

#endif /* BOOTLOADER_INFORMATION_H_ */
