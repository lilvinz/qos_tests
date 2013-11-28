
#include "stdheader.h"
#include "image_information.h"

#include "version.h"

//This structure may not be altered
IMAGE_INFORMATION_T __attribute__ ((section (".image_information"))) image_information =
{
    0xefbeadde,
    0xeddaebfe,
    VERSION_HARDWARE_ID,
    VERSION_HARDWARE_REVISION,
    VERSION_MAJOR,
    VERSION_MINOR,
    false,                      // b_manual_update;
    0x00,
    0x00
};
