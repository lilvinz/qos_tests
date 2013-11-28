
#ifndef VERSION_H
#define VERSION_H

#include "config.h"

#if defined HWT_MULTI
#define VERSION_HARDWARE_ID         0x2200  // hardware id sirius
#define VERSION_HARDWARE_REVISION   0x0200
#define HARDWARE_NAME               "multi"
#elif defined HWT_MEDIUM_WHITE
#define VERSION_HARDWARE_ID         0x2200  // hardware id sirius
#define VERSION_HARDWARE_REVISION   0x0200
#define HARDWARE_NAME               "medium white"
#elif defined HWT_MEDIUM_RED
#define VERSION_HARDWARE_ID         0x2204  // hardware id 2000 cd
#define VERSION_HARDWARE_REVISION   0x0101
#define HARDWARE_NAME               "medium red"
#elif defined HWT_HIGH_MAIN
#define VERSION_HARDWARE_ID          0x2201  // hardware id mainboard supernova
#define VERSION_HARDWARE_REVISION    0x0100
#define HARDWARE_NAME               "high white"
#elif defined HWT_HIGH_SV
#define VERSION_HARDWARE_ID          0x2202  // hardware id led driver board supernova
#define VERSION_HARDWARE_REVISION    0x0100
#define HARDWARE_NAME               "high white sv"
#error "Hardware type not supported!"
#else
#error "Hardware type not defined or unknown!"
#define HARDWARE_NAME "unknown"
#endif

#define VERSION_MAJOR   1
#define VERSION_MINOR   30

#if (VERSION_MINOR < 10)
#define VERSION_MINOR_STR       "0" STR(VERSION_MINOR)
#else
#define VERSION_MINOR_STR       STR(VERSION_MINOR)
#endif
#define VERSION_MAJOR_STR       STR(VERSION_MAJOR)

#ifdef DEBUG_BUILD
#define VERSION_IS_DEBUG 1
#endif

#ifdef VCS_REVISION
#define VCS_REVISION_STR        STR(VCS_REVISION)
#else
#define VCS_REVISION_STR        "n/a"
#endif

#ifdef VCS_PATH
#define VCS_PATH_STR            STR(VCS_PATH)
#else
#define VCS_PATH_STR            "n/a"
#endif

typedef enum
{
    HW_REV_0,
    HW_REV_1_0,
    HW_REV_2_3,
    HW_REV_2_4,
} HW_REV_T;




typedef struct device_info_s
{
    uint16_t hardware_id;
    uint16_t hardware_revision;
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t reserved[9];
} DEVICE_INFO_T;

HW_REV_T VERSION_GetHWRev(void);
void VERSION_Init(void);

#endif
