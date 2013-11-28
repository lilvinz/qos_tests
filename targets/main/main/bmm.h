/*
 * bmm.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 09.06.2010
 *      Author:
 *
 * Purpose:
 * BMM - Beacon Mode Manager
 * Determine if LEDs should be turned on or off.
 * Determine which LEDs should be turned on or off.
 *
 */

#ifndef BMM_H
#define BMM_H

#include "sequence.h"
#include "dim.h"

extern bool bmm_profile_changed;

// ****** SETTINGS ******
// GLOOM is from DUSK till DAWN
#define BMM_ACTIVE_MODE                 (_settings_list_ram[SETTINGS_BMM_ACTIVE_MODE])

// VISIBILITY Range Settings
#define BMM_VISI_HYSTERESIS             (_settings_list_ram[SETTINGS_VISIBILITY_HYSTERESIS])

typedef struct
{
    uint8_t profile_id;
    uint32_t reserved[2];
} BMM_PROFILE_DESCRIPTOR_T;

typedef enum
{
    BMM_GLOOM_ACTIVE_MODE_DAY_NIGHT,
    BMM_GLOOM_ACTIVE_MODE_DAY_ONLY,
    BMM_GLOOM_ACTIVE_MODE_NIGHT_ONLY
} BMM_GLOOM_ACTIVE_MODE_T;

typedef enum
{
    BMM_OPERATING_MODE_INITIALIZED,
    BMM_OPERATING_MODE_DAY,
    BMM_OPERATING_MODE_NIGHT,
    BMM_OPERATING_MODE_DAY_OFF,
    BMM_OPERATING_MODE_NIGHT_OFF,
    BMM_OPERATING_MODE_OFF,
} BMM_OPERATING_MODE_T;

typedef enum
{
    BMM_VISI_DISTANCE_0,    // visibility bad --> led full power
    BMM_VISI_DISTANCE_1,    // visibility medium --> medium power
    BMM_VISI_DISTANCE_2     // visibility good --> low power
} BMM_VISI_T;

typedef enum
{
    BMM_SET_PP_RESULT_OK,
    BMM_SET_PP_RESULT_VALUE_OUT_OF_RANGE,
    BMM_SET_PP_RESULT_VALUE_PARAMETER_NOT_FOUND_1,
    BMM_SET_PP_RESULT_VALUE_PARAMETER_NOT_FOUND_2,
    BMM_SET_PP_RESULT_VALUE_PARAMETER_NOT_FOUND_3,
    BMM_SET_PP_RESULT_VALUE_PARAMETER_NOT_FOUND_4,
    BMM_SET_PP_RESULT_VALUE_PARAMETER_NOT_FOUND_5,
} BMM_SET_PP_RESULT_T;

typedef struct
{
    bool        use_visibility;  // power level depends on visibility (true) or not (false)
    uint16_t    level_reduction_1;  // in %
    uint16_t    level_reduction_2;  // in %
    uint16_t    range_1_distance;  // in m
    uint16_t    range_2_distance;  // in m
} BMM_VISIBILITY_FACTOR_T; // 20 (not packed)

typedef struct
{
    uint8_t     use_switch_on_demand;
    uint16_t    switch_off_delay;
} BMM_ON_DEMAND_T;

typedef struct
{
    uint16_t    gloom_threshold_dusk;
    uint16_t    gloom_threshold_dawn;
} BMM_USE_CONDITIONS_T; // 4 Bytes

typedef struct
{
    char                    name[40]; // 40
    SEQUENCE_T              sequence; // 128 (not packed)
    DIM_BMM_DIM_PARAMETER_T dim; // 32 (not packed)
    BMM_VISIBILITY_FACTOR_T visibility_factor; // 20 (not packed)
    BMM_ON_DEMAND_T         on_demand; // 8 (not packed)
} BMM_LED_SETTINGS; // 228 (not packed)

// ATTENTION!
// In the first step only two operation modes are supported: DAY and NIGHT, twilight is prepared
typedef struct
{
    BMM_PROFILE_DESCRIPTOR_T    descriptor; // 12 (unpacked)
    BMM_LED_SETTINGS            day;        // 228 (not packed)
    BMM_LED_SETTINGS            night;      // 228 (not packed)
    BMM_LED_SETTINGS            twilight;   // 228 (not packed)
    BMM_USE_CONDITIONS_T        conditions;  // 8 (not packed)
} BMM_LED_PROFILE; // 704 (not packed)


extern BMM_LED_PROFILE _bmm_active_led_profile;

void BMM_SetVisibilityRange(BMM_VISI_T range);
uint16_t BMM_GetActualReduction(void);
void BMM_SetLedStageOff(bool new_state);
bool BMM_GetLedStageOff(void);
void BMM_RequestImmediateDemandOn(void);
BMM_VISI_T BMM_GetVisibilityRange(void);
bool BMM_CheckDarkness(uint16_t gloom_value);
void BMM_GetActualProfile(const SEQUENCE_T **sequence, const DIM_BMM_DIM_PARAMETER_T **dim_parameter, const uint16_t **reduction);
uint32_t BMM_GetCurrentProfileID(void);
BMM_SET_PP_RESULT_T BMM_SetProfileParameter(uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint8_t param5, uint32_t value);
void BMM_SaveProfile(void);
bool BMM_LoadProfile(uint32_t profileId);
int32_t BMM_GetProfileIndex(uint32_t profileId);
void BMM_Init(void);
#endif
