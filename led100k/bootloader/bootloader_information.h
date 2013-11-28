/*
 * bootloader_information.h
 *
 *  Created on: 04.10.2010
 *      Author: DHA
 */

#ifndef BOOTLOADER_INFORMATION_H_
#define BOOTLOADER_INFORMATION_H_


#define BOOTLOADER_VERSION_MAJOR                1
#define BOOTLOADER_VERSION_MINOR                6
#define BOOTLOADER_VERSION_HARDWARE_ID          0x2200
#define BOOTLOADER_VERSION_HARDWARE_REVISION    0x0200

#define VERSION_IS_DEBUG        (BOOTLOADER_VERSION_MINOR % 2)

typedef struct
{
    uint8_t     sw_version_major;
    uint8_t     sw_version_minor;
    uint16_t    hw_id;
    uint16_t    hw_revision;
} BOOTLOADER_INFORMATION_T;

extern BOOTLOADER_INFORMATION_T bootloader_information;

#endif /* BOOTLOADER_INFORMATION_H_ */
