#include "image_information.h"
#include "version.h"

IMAGE_INFORMATION_T __attribute__ ((section (".image_information"))) image_information =
{
    IMAGE_INFO_CHECKSUM,
    IMAGE_INFO_SIZE,
    VERSION_HARDWARE_ID,
    VERSION_HARDWARE_REVISION,
    VERSION_MAJOR,
    VERSION_MINOR,
    0xFF,
    0xFF
};
