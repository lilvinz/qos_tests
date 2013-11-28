/*
 * bmm_profiles.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 08.07.2010
 *      Author: DHA
 *
 * Purpose:
 * Profiles for BMM (Beacon Mode Manager)
 * This file contains default profiles for the bmm.
 *
 */

#ifndef BMM_PROFILES_H_
#define BMM_PROFILES_H_

#include "bmm.h"
#include "config.h"
#include "stdheader.h"


#ifdef HWT_MULTI
#define POWER_LEVEL(a, b, c, d, e, f) (a),(b),(c),(d),(e),(f)
#define BMM_NUM_LED_PROFILES 12
#endif
#ifdef HWT_MEDIUM_WHITE
#define POWER_LEVEL(a, b, c, d, e, f) (a)
#define BMM_NUM_LED_PROFILES 3
#endif
#ifdef HWT_MEDIUM_RED
#define POWER_LEVEL(a, b, c, d, e, f) (d)
#define BMM_NUM_LED_PROFILES 3
#endif
#ifdef HWT_HIGH
#define POWER_LEVEL(a, b, c, d, e, f) (a)
#define BMM_NUM_LED_PROFILES 2
#endif

extern const BMM_LED_PROFILE bmm_led_profiles_flash[BMM_NUM_LED_PROFILES];
extern const BMM_LED_SETTINGS bmm_blank_led_profile;


/*{
 *    Name, // max 39char
 *    { // table
 *        table length,
 *        {
 *            { time span, dim time, dim value },    // << 10x
 *        },
 *        delay
 *    },
 *{// dim
 *    { type, gamma },
 *        { white, red 1, red 2, red center, red 3, red 4 }  // power level
 *    },
 *    { use visibility, reduction 1, reduction 2, distance 1, distance 2 },
 *    { use on demand, switch off delay }
 *}
 */


#define PROFILE_OFF \
{\
    "off",\
    {\
        0,\
        {\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0}\
        },\
        0\
    },\
    {\
        {0,33},\
        {POWER_LEVEL(0,0,0,0,0,0)}\
    },\
    {FALSE,30,10,5000,10000},\
    {0,0}\
}


#if (defined HWT_MULTI)
#define PROFILE_FEUERWROT \
{\
    "FeuerW, red",\
    {\
        4,\
        {\
            {1000,300,1000},\
            {500,300,0},\
            {1000,300,1000},\
            {1500,300,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0}\
        },\
        0\
    },\
    {\
        {0,33},\
        {POWER_LEVEL(0,5050,3700,1700,4050,2600)}\
    },\
    {TRUE,30,10,5000,10000},\
    {0,0}\
}
#endif

#if (defined HWT_MULTI)
#define PROFILE_LOW_INTENSITY_TYPE_A \
{\
    "Low-intensity Type A, red",\
    {\
        1,\
        {\
            {0,0,1000},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0}\
        },\
        0\
    },\
    {\
        {0,33},\
        {POWER_LEVEL(0,0,250,135,180,730)}\
    },\
    {FALSE,0,0,0,0},\
    {0,0}\
}
#endif

#if (defined HWT_MULTI)
#define PROFILE_LOW_INTENSITY_TYPE_B \
{\
    "Low-intensity Type B, red",\
    {\
        1,\
        {\
            {0,0,1000},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0}\
        },\
        0\
    },\
    {\
        {0,33},\
        {POWER_LEVEL(0,0,550,405,540,2450)}\
    },\
    {FALSE,0,0,0,0},\
    {0,0}\
}
#endif

#if (defined HWT_MULTI) || (defined HWT_MEDIUM_WHITE)
#define PROFILE_MEDIUM_INTENSITY_TYPE_A \
{\
    "Medium-intensity Type A, white flashing",\
    {\
        2,\
        {\
            {800,0,1000},\
            {2200,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0}\
        },\
        0\
    },\
    {\
        {0,33},\
        {POWER_LEVEL(32000,0,0,0,0,0)}\
    },\
    {TRUE,30,10,5000,10000},\
    {0,0}\
}
#endif

#if (defined HWT_MULTI) || (defined HWT_MEDIUM_WHITE)
#define PROFILE_MEDIUM_INTENSITY_TYPE_A_NIGHT \
{\
    "Medium-intensity Type A, white flashing",\
    {\
        2,\
        {\
            {800,0,1000},\
            {2200,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0}\
        },\
        0\
    },\
    {\
        {0,33},\
        {POWER_LEVEL(3200,0,0,0,0,0)}\
    },\
    {TRUE,30,10,5000,10000},\
    {0,0}\
}
#endif

#if (defined HWT_MULTI) || (defined HWT_MEDIUM_WHITE)
#define PROFILE_MEDIUM_INTENSITY_TYPE_B \
{\
    "Medium-intensity Type B, red flashing",\
    {\
        2,\
        {\
            {800,0,1000},\
            {2200,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0}\
        },\
        0\
    },\
    {\
        {0,33},\
        {POWER_LEVEL(0,0,0,32000,0,0)}\
    },\
    {TRUE,30,10,5000,10000},\
    {0,0}\
}
#endif

#if (defined HWT_MULTI)
#define PROFILE_MEDIUM_INTENSITY_TYPE_C \
{\
    "Medium-intensity Type C, red steady",\
    {\
        1,\
        {\
            {0,0,1000},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0}\
        },\
        0\
    },\
    {\
        {0,33},\
        {POWER_LEVEL(0,0,0,32000,0,0)}\
    },\
    {TRUE,30,10,5000,10000},\
    {0,0}\
}
#endif

#if (defined HWT_MULTI)
#define PROFILE_MEDIUM_INTENSITY_TYPE_A_FRANCE \
{\
    "FR Med-int, Type A, white flashing",\
    {\
        4,\
        {\
            {200,0,1000},\
            {1300,0,0},\
            {200,0,1000},\
            {1300,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0}\
        },\
        0\
    },\
    {\
        {0,33},\
        {POWER_LEVEL(34000,0,0,0,0,0)}\
    },\
    {FALSE,30,10,5000,10000},\
    {0,0}\
}
#endif

#if (defined HWT_MULTI)
#define PROFILE_MEDIUM_INTENSITY_TYPE_B_FRANCE \
{\
    "FR Med-int, Type B, red flashing",\
    {\
        4,\
        {\
            {200,0,1000},\
            {1300,0,0},\
            {200,0,1000},\
            {1300,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0}\
        },\
        0\
    },\
    {\
        {0,33},\
        {POWER_LEVEL(0,0,0,30000,0,0)}\
    },\
    {FALSE,30,10,5000,10000},\
    {0,0}\
}
#endif

#if (defined HWT_MULTI) || (defined HWT_MEDIUM_WHITE)
#define PROFILE_ICAO_MEDIUM_INTENSITY_TYPE_A_DAY_40FPM \
{\
    /*************************************** max. 39 chars */ \
    "ICAO Med-int, Type A day, 40 FPM",\
    {\
        4,\
        {\
            {300,0,1000},\
            {1200,0,0},\
            {300,0,1000},\
            {1200,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0}\
        },\
        0\
    },\
    {\
        {0,33},\
        {POWER_LEVEL(42000,0,0,0,0,0)}\
    },\
    {FALSE,30,10,5000,10000},\
    {0,0}\
}
#endif

#if (defined HWT_MULTI) || (defined HWT_MEDIUM_WHITE)
#define PROFILE_ICAO_MEDIUM_INTENSITY_TYPE_A_NIGHT_40FPM \
{\
    /*************************************** max. 39 chars */ \
    "ICAO Med-int, Type A night, 40 FPM",\
    {\
        4,\
        {\
            {300,0,1000},\
            {1200,0,0},\
            {300,0,1000},\
            {1200,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0}\
        },\
        0\
    },\
    {\
        {0,33},\
        {POWER_LEVEL(3900,0,0,0,0,0)}\
    },\
    {FALSE,30,10,5000,10000},\
    {0,0}\
}
#endif


// Profile for small 2000cd ICAO Medium Intensity Type B
#if (defined HWT_MEDIUM_RED)
#define PROFILE_MEDIUM_INTENSITY_TYPE_B \
{\
    "Medium-intensity Type B, red flashing",\
    {\
        2,\
        {\
            {800,0,1000},\
            {2200,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0}\
        },\
        0\
    },\
    {\
        {0,33},\
        {POWER_LEVEL(0,0,0,21500,0,0)}\
    },\
    {TRUE,30,10,5000,10000},\
    {0,0}\
}

#define PROFILE_FAA_L864 \
{\
    "FAA L-864, night, red flashing",\
    {\
        2,\
        {\
            {800,0,1000},\
            {2200,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0}\
        },\
        0\
    },\
    {\
        {0,33},\
        {POWER_LEVEL(0,0,0,21500,0,0)}\
    },\
    {FALSE,30,10,5000,10000},\
    {0,0}\
}
#endif


// Profile for huge white 100.000cd High Intensity Type B
#if (defined HWT_HIGH_WHITE)
#define PROFILE_HIGH_INTENSITY_TYPE_B \
{\
    "High-intensity Type B, white flashing",\
    {\
        2,\
        {\
            {100,0,1000},\
            {900,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0}\
        },\
        0\
    },\
    {\
        {0,33},\
        {POWER_LEVEL(50000,0,0,0,0,0)}\
    },\
    {FALSE,30,10,5000,10000},\
    {0,0}\
}
#endif

#if (defined HWT_HIGH_WHITE)
#define PROFILE_HIGH_INTENSITY_TYPE_B_NIGHT \
{\
    "High-intensity Type B, white flashing",\
    {\
        2,\
        {\
            {100,0,1000},\
            {900,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0},\
            {0,0,0}\
        },\
        0\
    },\
    {\
        {0,33},\
        {POWER_LEVEL(1000,0,0,0,0,0)}\
    },\
    {FALSE,30,10,5000,10000},\
    {0,0}\
}
#endif

#endif /* BMM_PROFILES_H_ */
