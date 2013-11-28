/*
 * bmm_profiles.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 08.07.2010
 *      Author:DHA
 *
 * Purpose:
 * Profiles for BMM (Beacon Mode Manager)
 * This file contains default profiles for the bmm.
 *
 */

#include "stdheader.h"
#include "bmm_profiles.h"
#include "config.h"

#include "version.h"

#if (defined HWT_MULTI)
const BMM_LED_PROFILE bmm_led_profiles_flash[BMM_NUM_LED_PROFILES] =
{
    {   // profile 0: always off
        {
            0, // profile id
            {0,0} // reserved
        },
        PROFILE_OFF,
        PROFILE_OFF,
        PROFILE_OFF,
        // conditions
        {50, 60}
    },
    {   // profile 1: Feuer W rot
        {
            1, // profile id
            {0,0} // reserved
        },
        PROFILE_OFF,
        PROFILE_FEUERWROT,
        PROFILE_OFF,
        // conditions
        {50, 60}
    },
    {   // profile 2: Low-intensity Type A, red always on
        {
            2, // profile id
            {0,0} // reserved
        },
        PROFILE_OFF,
        PROFILE_LOW_INTENSITY_TYPE_A,
        PROFILE_LOW_INTENSITY_TYPE_A,
        // conditions
        {50, 60}
    },
    {   // profile 3: Low-intensity Type B, red always on
        {
            3, // profile id
            {0,0} // reserved
        },
        PROFILE_OFF,
        PROFILE_LOW_INTENSITY_TYPE_B,
        PROFILE_LOW_INTENSITY_TYPE_B,
        // conditions
        {50, 60}
    },
    {   // profile 4: Medium-intensity Type A, white flashing
        {
            4, // profile id
            {0,0} // reserved
        },
        PROFILE_MEDIUM_INTENSITY_TYPE_A,
        PROFILE_MEDIUM_INTENSITY_TYPE_A_NIGHT,
        PROFILE_MEDIUM_INTENSITY_TYPE_A,
        // conditions
        {50, 60}
    },
    {   // profile 5: Medium-intensity Type B, red flashing
        {
            5, // profile id
            {0,0} // reserved
        },
        PROFILE_OFF,
        PROFILE_MEDIUM_INTENSITY_TYPE_B,
        PROFILE_OFF,
        // conditions
        {50, 60}
    },
    {   // profile 6: Medium-intensity Type C, red always on
        {
            6, // profile id
            {0,0} // reserved
        },
        PROFILE_OFF,
        PROFILE_MEDIUM_INTENSITY_TYPE_C,
        PROFILE_OFF,
        // conditions
        {50, 60}
    },
    {   // profile 7:  Day: Medium-intensity Type A (white flashing) Night: Feuer W rot
        {
            7, // profile id
            {0,0} // reserved
        },
        PROFILE_MEDIUM_INTENSITY_TYPE_A,
        PROFILE_FEUERWROT,
        PROFILE_FEUERWROT,
        // conditions
        {50, 60}
    },
    {   // profile 8: Day: Medium-intensity Type A (white flashing) Night: Medium-intensity Type B (red flashing)
        {
            8, // profile id
            {0,0} // reserved
        },
        PROFILE_MEDIUM_INTENSITY_TYPE_A,
        PROFILE_MEDIUM_INTENSITY_TYPE_B,
        PROFILE_MEDIUM_INTENSITY_TYPE_A,
        // conditions
        {50, 60}
    },
    {   // profile 9: Day: Medium-intensity Type A (white flashing) Night: Medium-intensity Type C (red always on)
        {
            9, // profile id
            {0,0} // reserved
        },
        PROFILE_MEDIUM_INTENSITY_TYPE_A,
        PROFILE_MEDIUM_INTENSITY_TYPE_C,
        PROFILE_MEDIUM_INTENSITY_TYPE_A,
        // conditions
        {50, 60}
    },
    {   // profile 10: FRANCE: Day Medium-intensity Type A, white flashing, Night: Medium-intensity Type B, red flashing
        {
            10, // profile id
            {0,0} // reserved
        },
        PROFILE_MEDIUM_INTENSITY_TYPE_A_FRANCE,
        PROFILE_MEDIUM_INTENSITY_TYPE_B_FRANCE,
        PROFILE_MEDIUM_INTENSITY_TYPE_A_FRANCE,

        // conditions
        {50, 60}
    },
    {   // profile 12:  ICAO Medium Intensity Type A, 40 FPM
        {
            12, // profile id
            {0,0} // reserved
        },
        PROFILE_ICAO_MEDIUM_INTENSITY_TYPE_A_DAY_40FPM,
        PROFILE_ICAO_MEDIUM_INTENSITY_TYPE_A_NIGHT_40FPM,
        PROFILE_ICAO_MEDIUM_INTENSITY_TYPE_A_DAY_40FPM,
        // conditions
        {50, 60}
    },
};
#endif

#if (defined HWT_MEDIUM_RED)
const BMM_LED_PROFILE bmm_led_profiles_flash[BMM_NUM_LED_PROFILES] =
{
    {   // profile 0: always off
        {
            0, // profile id
            {0,0} // reserved
        },
        PROFILE_OFF,
        PROFILE_OFF,
        PROFILE_OFF,
        // conditions
        {50, 60}
    },
    {   // profile 5: Medium-intensity Type B, red flashing
        {
            5, // profile id
            {0,0} // reserved
        },
        PROFILE_OFF,
        PROFILE_MEDIUM_INTENSITY_TYPE_B,
        PROFILE_OFF,
        // conditions
        {50, 60}
    },
    {   // profile 13: FAA L-864, red flashing
        {
            13, // profile id
            {0,0} // reserved
        },
        PROFILE_OFF,
        PROFILE_FAA_L864,
        PROFILE_OFF,
        // conditions
        {170, 230}
    },
};
#endif

#if (defined HWT_MEDIUM_WHITE)
const BMM_LED_PROFILE bmm_led_profiles_flash[BMM_NUM_LED_PROFILES] =
{
    {   // profile 0: always off
        {
            0, // profile id
            {0,0} // reserved
        },
        PROFILE_OFF,
        PROFILE_OFF,
        PROFILE_OFF,
        // conditions
        {50, 60}
    },
    {   // profile 4: Medium-intensity Type A, white flashing
        {
            4, // profile id
            {0,0} // reserved
        },
        PROFILE_MEDIUM_INTENSITY_TYPE_A,
        PROFILE_MEDIUM_INTENSITY_TYPE_A_NIGHT,
        PROFILE_MEDIUM_INTENSITY_TYPE_A,
        // conditions
        {50, 60}
    },
    {   // profile 12:  ICAO Medium Intensity Type A, 40 FPM
        {
            12, // profile id
            {0,0} // reserved
        },
        PROFILE_ICAO_MEDIUM_INTENSITY_TYPE_A_DAY_40FPM,
        PROFILE_ICAO_MEDIUM_INTENSITY_TYPE_A_NIGHT_40FPM,
        PROFILE_ICAO_MEDIUM_INTENSITY_TYPE_A_DAY_40FPM,
        // conditions
        {50, 60}
    },
};
#endif


#if (defined HWT_HIGH_WHITE)
const BMM_LED_PROFILE bmm_led_profiles_flash[BMM_NUM_LED_PROFILES] =
{
    {   // profile 0: always off
        {
            0, // profile id
            {0,0} // reserved
        },
        PROFILE_OFF,
        PROFILE_OFF,
        PROFILE_OFF,
        // conditions
        {50, 60}
    },
    {   // profile 11:  100.000cd Day High-intensity Type B, white flashing, Night: High-intensity Type B, 2000cd white flashing
        {
            11, // profile id
            {0,0} // reserved
        },
        PROFILE_HIGH_INTENSITY_TYPE_B,
        PROFILE_HIGH_INTENSITY_TYPE_B_NIGHT,
        PROFILE_HIGH_INTENSITY_TYPE_B,
        // conditions
        {50, 60}
    }
};
#endif

// blank profile
const BMM_LED_SETTINGS bmm_blank_led_profile =
{
    "blank",
    {   // sequence
        0, // length
        {
            // table
            // time span, dim time, dim value
            { 0, 0, 0}, // table entry 0
            { 0, 0, 0}, // table entry 1
            { 0, 0, 0}, // table entry 2
            { 0, 0, 0}, // table entry 3
            { 0, 0, 0}, // table entry 4
            { 0, 0, 0}, // table entry 5
            { 0, 0, 0}, // table entry 6
            { 0, 0, 0}, // table entry 7
            { 0, 0, 0}, // table entry 8
            { 0, 0, 0}  // table entry 9
        },
        0 // delay
    },
    {   // dim
        // settings
        // type, gamma
        { 0, 0}, // settings
        // power level
        // white, red 1, red 2, red 3, red 4, red center
        {
#ifdef HWP_LED_WHITE_20K
            0,
#endif
#ifdef HWP_LED_RED_UNI
            0, 0, 0, 0,
#endif
#ifdef HWP_LED_RED_2K
            0
#endif
        } // power level
    },
    // visibility factor
    // use visibility, reduction 1, reduction 2, distance 1, distance 2
    {FALSE, 30, 10, 5000, 10000},
    // on_demand
    // use on demand, switch off delay
    { 0, 0}
};
