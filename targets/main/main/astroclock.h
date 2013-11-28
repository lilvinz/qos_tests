/*
 * astroclock.h
 *
 *  Created on: 04.11.2013
 *      Author: cba
 */

#ifndef ASTROCLOCK_H_
#define ASTROCLOCK_H_

#include "rtc.h"

#define ASTROCLOCK_SUN (0.0d)
#define ASTROCLOCK_CIVIL_TWILIGHT (-6.0d)
#define ASTROCLOCK_NAUTICAL_TWILIGHT (-12.0d)
#define ASTROCLOCK_ASTRONOMICAL_TWILIGHT (-18.0d)

typedef enum
{
    ASTROCLOCK_NORMAL,
    ASTROCLOCK_POLAR_DAY,
    ASTROCLOCK_POLAR_NIGHT,
} ASTROCLOCK_POLAR_T;

typedef struct
{
    double dLatitude;
    double dLongitude;
    RTCTIME uTimestamp;
    RTCTIME uSunRiseTimestamp;
    RTCTIME uSunSetTimestamp;
    ASTROCLOCK_POLAR_T ePolar;
} ASTROCLOCK_T;

typedef struct
{
    double dLatitude;
    double dLongitude;
    RTCTIME uTimestamp;
    float fAzimuth;
    float fElevation;
} ASTROCLOCK_POS_T;

typedef enum
{
    SUNSTATE_DAY,
    SUNSTATE_CIVIL_TWILIGHT,
    SUNSTATE_NAUTICAL_TWILIGHT,
    SUNSTATE_ASTRONOMICAL_TWILIGHT,
    SUNSTATE_NIGHT
} ASTROCLOCK_SUN_STATE_T;

void ASTROCLOCK_GetSunRiseAndSet(ASTROCLOCK_T* param);
void ASTROCLOCK_GetSunPosition(ASTROCLOCK_POS_T* param);
ASTROCLOCK_SUN_STATE_T ASTROCLOCK_GetSunState(const double dLatitude, const double dLongitude, const RTCTIME uTimestamp);

#endif /* ASTROCLOCK_H_ */
