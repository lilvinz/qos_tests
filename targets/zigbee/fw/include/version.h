
#ifndef VERSION_H
#define VERSION_H

#include <inttypes.h>

#define VERSION_HARDWARE_ID			0x4200  // hardware id zig2rs232 2.4GHz
#define VERSION_HARDWARE_REVISION	0x0001
#define VERSION_MAJOR				1
#define VERSION_MINOR				18

#if (VERSION_MINOR < 10)
#define VERSION_MINOR_STR		"0" STR(VERSION_MINOR)
#else
#define VERSION_MINOR_STR		STR(VERSION_MINOR)
#endif
#define VERSION_MAJOR_STR		STR(VERSION_MAJOR)

#define VERSION_IS_DEBUG		(VERSION_MINOR % 2)

/* *** Template ***
#if (VERSION_IS_DEBUG > 0)

#endif
 */


#endif
