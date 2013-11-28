
#include "stdheader.h"
#include "bootloader_information.h"

BOOTLOADER_INFORMATION_T __attribute__ ((section (".image_information"))) bootloader_information =
{
    BOOTLOADER_VERSION_MAJOR,
    BOOTLOADER_VERSION_MINOR,
    BOOTLOADER_VERSION_HARDWARE_ID,
    BOOTLOADER_VERSION_HARDWARE_REVISION,
};
