/*
 * bmm.c
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

#include "stdheader.h"
#include "bmm.h"

#include "bmm_profiles.h"
#include "settings.h"
#include "syndication.h"
#include "spi_serial_fram.h"
#include "fram_memory_map.h"
#include "fram_mirror.h"
#include "clock.h"
#include "log.h"
#include "sensor.h"
#include "sound.h"
#include "timer.h"
#include "sequence.h"
#include "detector.h"
#include "gloom_manager.h"

//******************** Local Variables *********************

static uint16_t actual_reduction;
static BMM_VISI_T actual_visibility_range = BMM_VISI_DISTANCE_0;
static BMM_VISI_T visibility_manual_range = BMM_VISI_DISTANCE_0;

BMM_LED_PROFILE _bmm_active_led_profile;
static const BMM_LED_SETTINGS * actual_led_setting = &bmm_blank_led_profile;
static bool b_led_stage_off = FALSE;
static bool b_led_demand_on = FALSE;

static LOG_DUPVAR_T bmm_logimmediateondupvar;

static TIMER_T demand_on_timer;

//******************** Local Functions *********************
static void bmmDemandOntimeoutCallback(uint32_t parameter);
static bool bmmIsItSafeToStart(void);
static BMM_VISI_T bmmGetVisibilityRange(BMM_VISI_T current_range, uint16_t* visibility_value);



/**
  * @brief  Callback function of demand on request timer.
  * @param  Switch off delay
  * @retval None
  */
static void bmmDemandOntimeoutCallback(uint32_t parameter)
{
    b_led_demand_on = FALSE;

    LOG(LOG_ID_BMM_OPERATING_MODE_OFF, 0, parameter, 0, &bmm_logimmediateondupvar);

    if (actual_led_setting->on_demand.use_switch_on_demand > 0)
        b_led_stage_off = TRUE;
}


/**
  * @brief  Determine if it is allowed to turn LEDs on.
  * @param  None
  * @retval None
  */
static bool bmmIsItSafeToStart(void)
{
    if (_settings_list_ram[SETTINGS_INDICATOR_BAD_LED_STOP] == TRUE)
    {
        for (uint8_t ov_bm = 0; ov_bm < PS_LED_OVERVOLTAGE_BM_COUNT; ov_bm++)
        {
            // Check all available led overvoltage bitmasks for failures.
            // We do not distinguish if failed leds are needed for the actual operating mode.
            // We do not count how many leds failed and how many are allowed to fail until all leds have to be turned off.
            // Only one simple rule: If bad led stop is activated every led fault will cause the light to switch off!
            // Background: This feature is intended to fulfill the FAA requirement to turn the light off if more than 25% off all leds failed.
            // The 2k cd light has 2 led stages with 24 leds each. So if one stage fails, more than 25% failed.
            // Todo [medium]: Implement functions to check if failed leds are needed and to check (periodically) if failed leds are working again.
            if (DETECTOR_GetLedStageFailBitmask(ov_bm) != 0)
                return FALSE;
        }
    }

    return TRUE;
}


/**
  * @brief  Determine if it is dark.
  * @param  Value from brightness sensor
  * @retval TRUE if dark
  */
bool BMM_CheckDarkness(uint16_t gloom_value)
{
    static bool bgloomy = TRUE;
    static LOG_DUPVAR_T bmm_logdupvar;

    //return state with hysteresis
    if (gloom_value > _bmm_active_led_profile.conditions.gloom_threshold_dawn)
        bgloomy = FALSE;
    if (gloom_value < _bmm_active_led_profile.conditions.gloom_threshold_dusk)
        bgloomy = TRUE;

    if (bgloomy)
    {
        if (LOG(LOG_ID_BMM_GLOOM_MEAS_LEVEL_LOWER_LIMIT, gloom_value, (uint16_t) SENSORS_GetGloomEMA(), 0, &bmm_logdupvar))
            SOUND_PlayMelody(NIGHT_MELODY);
    }
    else
    {
        if (LOG(LOG_ID_BMM_GLOOM_MEAS_LEVEL_UPPER_LIMIT, gloom_value, (uint16_t) SENSORS_GetGloomEMA(), 0, &bmm_logdupvar))
            SOUND_PlayMelody(DAY_MELODY);
    }
    return bgloomy;
}


/**
  * @brief  Get actual visibility range.
  * @param  None
  * @retval Visibility range
  */
BMM_VISI_T BMM_GetVisibilityRange(void)
{
    return actual_visibility_range;
}


/**
  * @brief  Set visibility range.
  * @param  Visibility range
  * @retval None
  */
void BMM_SetVisibilityRange(BMM_VISI_T range)
{
    visibility_manual_range = range;
}

/**
  * @brief  Get actual reduction.
  * @param  None
  * @retval Reduction in percent.
  */
uint16_t BMM_GetActualReduction(void)
{
    return actual_reduction;
}


/**
  * @brief  Force Led Stage on or off
  */

void BMM_SetLedStageOff(bool new_state)
{
    if (b_led_demand_on == FALSE)
        b_led_stage_off = new_state;
}

bool BMM_GetLedStageOff(void)
{
    return b_led_stage_off;
}

void BMM_RequestImmediateDemandOn(void)
{
    if (actual_led_setting->on_demand.use_switch_on_demand == 0)
        return;

    b_led_stage_off = FALSE;
    b_led_demand_on = TRUE;

    LOG(LOG_ID_BMM_IMMEDIATE_ON_REQUEST, 0, 0, 0, &bmm_logimmediateondupvar);

    if (actual_led_setting->on_demand.switch_off_delay > 0)
    {
        demand_on_timer.callback = bmmDemandOntimeoutCallback;
        demand_on_timer.callback_parameter = actual_led_setting->on_demand.switch_off_delay;
        demand_on_timer.interval = 1000UL * actual_led_setting->on_demand.switch_off_delay ;
        demand_on_timer.mode = TIMER_ONE_SHOT_MODE;
        TIMER_Stop(&demand_on_timer);
        TIMER_Start(&demand_on_timer);
    }
    else
    {
        b_led_stage_off = TRUE;
        b_led_demand_on = FALSE;
    }

    // if synchronization not used start sequence immediately
    if (actual_led_setting->on_demand.use_switch_on_demand == 1)
        SEQUENCE_KickStart();
}


/**
  * @brief  Get current active profile ID.
  * @param  None
  * @retval Profile ID
  */
uint32_t BMM_GetCurrentProfileID(void)
{
    return _bmm_active_led_profile.descriptor.profile_id;
}


/**
  * @brief  Return profile index of given profile ID.
  * @param  Profile ID
  * @retval Profile Index
  */
int32_t BMM_GetProfileIndex(uint32_t profileId)
{
    for (uint8_t idx = 0; idx < BMM_NUM_LED_PROFILES; idx++)
    {
        if (profileId == bmm_led_profiles_flash[idx].descriptor.profile_id)
            return idx;
    }
    return -1;
}


static BMM_VISI_T bmmGetVisibilityRange(BMM_VISI_T current_range, uint16_t* visibility_value)
{
    if (visibility_value != NULL)
        *visibility_value = 0;

    // Check if output power reduction is desired
    if (actual_led_setting->visibility_factor.use_visibility == 0)
        return BMM_VISI_DISTANCE_0;

    // return manual range if enabled
    if (_settings_list_ram[SETTINGS_TERMINAL_CONTROL_MODBUS_REDUCTION] > 0)
        return visibility_manual_range;

    SDC_A_VISIBILITY_T visibility;
    // Get visibility sensor measurements.
    if (SYNDICATION_GetAssimilatedVisibility(&visibility) == FALSE)
        return BMM_VISI_DISTANCE_0;

    // Make sure that at least one measurement source is present
    if (visibility.count == 0)
        return BMM_VISI_DISTANCE_0;  // no measurement reading available

    // Make sure measurement data is not outdated.
    if (CLOCK_GetTimestamp() > (visibility.timestamp + SDC_CLIENT_VISIBILITY_TRUST_AGE_MAX))
        return BMM_VISI_DISTANCE_0;

    if (visibility_value != NULL)
        *visibility_value = visibility.minimum;

    // Calculate current range based on visibility measurements
    uint16_t range1 = actual_led_setting->visibility_factor.range_1_distance;
    uint16_t range2 = actual_led_setting->visibility_factor.range_2_distance;
    uint16_t visi = visibility.minimum;

    if ((visi > range2 + BMM_VISI_HYSTERESIS)
        || (current_range == BMM_VISI_DISTANCE_2 && visi >= range2))
        return BMM_VISI_DISTANCE_2;

    if ((visi > range1 + BMM_VISI_HYSTERESIS)
        || (current_range == BMM_VISI_DISTANCE_1 && visi >= range1))
        return BMM_VISI_DISTANCE_1;

    return BMM_VISI_DISTANCE_0;
}


/**
  * @brief  Get actual sequence, dim_parameter and light reduction factor from profile.
  * @param  - sequence: pointer to actual sequence depends on active profile and gloom
  *         - dim_parameter: pointer to actual dim_parameter depends on actual profile and gloom
  *         - factor: factor for led output power reduction depends on active profile, mode (day or night) and visibility
  * @retval None
  */
void BMM_GetActualProfile(const SEQUENCE_T **sequence, const DIM_BMM_DIM_PARAMETER_T **dim_parameter, const uint16_t **reduction)
{
    BMM_OPERATING_MODE_T new_operating_mode = BMM_OPERATING_MODE_INITIALIZED;
    static BMM_OPERATING_MODE_T old_operating_mode = BMM_OPERATING_MODE_INITIALIZED;
    static BMM_VISI_T old_visibility_range = BMM_VISI_DISTANCE_0;

    if ((actual_led_setting->on_demand.use_switch_on_demand > 0) && (b_led_demand_on == FALSE))
        b_led_stage_off = TRUE;

    // Determine if it is allowed to turn LEDs on.
    // Attention: This is a bugfix for FAA-Test!
    // Todo[high]: add support to distinguish if broken leds are needed or not

    // decide if leds are disabled or not and if it is day or night
    if (b_led_stage_off | (bmmIsItSafeToStart() != TRUE))
    {
        actual_led_setting = &bmm_blank_led_profile;  // switch leds off
        new_operating_mode = BMM_OPERATING_MODE_OFF;
    }
    else if (GLOOM_MANAGER_GetLocalLedState() == GLOOM_MANAGER_LED_STATE_NIGHT)
    {
        // night
        // check if night profile should be used
        if (BMM_ACTIVE_MODE == BMM_GLOOM_ACTIVE_MODE_DAY_ONLY)
        {
            actual_led_setting = &bmm_blank_led_profile;  // do not use night profile
            new_operating_mode = BMM_OPERATING_MODE_NIGHT_OFF;
        }
        else
        {
            actual_led_setting = &_bmm_active_led_profile.night;
            new_operating_mode = BMM_OPERATING_MODE_NIGHT;
        }
    }
    else // todo [low] if (gloom_manager_get_local_led_state() == GLOOM_MANAGER_LED_STATE_DAY)
    {
        // day
        // check if day profile should be used
        if (BMM_ACTIVE_MODE == BMM_GLOOM_ACTIVE_MODE_NIGHT_ONLY)
        {
            actual_led_setting = &bmm_blank_led_profile;  // do not use day profile
            new_operating_mode = BMM_OPERATING_MODE_DAY_OFF;
        }
        else
        {
            actual_led_setting = &_bmm_active_led_profile.day;
            new_operating_mode = BMM_OPERATING_MODE_DAY;
        }
    }
    // todo [low] if (gloom_manager_get_local_led_state() == GLOOM_MANAGER_LED_STATE_TWILIGHT)

    if (new_operating_mode != old_operating_mode)
    {
        // operating mode has changed -> create log entry
        switch(new_operating_mode)
        {
        case BMM_OPERATING_MODE_DAY:
            LOG(LOG_ID_BMM_OPERATING_MODE_DAY, 0, _bmm_active_led_profile.descriptor.profile_id, 0, NULL);
            break;
        case BMM_OPERATING_MODE_NIGHT:
            LOG(LOG_ID_BMM_OPERATING_MODE_NIGHT, 0, _bmm_active_led_profile.descriptor.profile_id, 0, NULL);
            break;
        case BMM_OPERATING_MODE_DAY_OFF:
            LOG(LOG_ID_BMM_OPERATING_MODE_DAY_OFF, 0, _bmm_active_led_profile.descriptor.profile_id, 0, NULL);
            break;
        case BMM_OPERATING_MODE_NIGHT_OFF:
            LOG(LOG_ID_BMM_OPERATING_MODE_NIGHT_OFF, 0, _bmm_active_led_profile.descriptor.profile_id, 0, NULL);
            break;
        case BMM_OPERATING_MODE_OFF:
            LOG(LOG_ID_BMM_OPERATING_MODE_OFF, 0, _bmm_active_led_profile.descriptor.profile_id, 0, NULL);
            break;

        default:
            break;
        }

        old_operating_mode = new_operating_mode;
    }

    uint16_t visibility;
    actual_visibility_range = bmmGetVisibilityRange(old_visibility_range, &visibility);

    switch(actual_visibility_range)
    {
    case BMM_VISI_DISTANCE_1:
    {
        actual_reduction = actual_led_setting->visibility_factor.level_reduction_1;
        break;
    }
    case BMM_VISI_DISTANCE_2:
    {
        actual_reduction = actual_led_setting->visibility_factor.level_reduction_2;
        break;
    }
    default:
        actual_reduction = 100;  // 100%
        break;
    }

    if (actual_visibility_range != old_visibility_range)
    {
        LOG(LOG_ID_BMM_VISIBILITY_REDUCTION, visibility, actual_visibility_range, actual_reduction, NULL);
        old_visibility_range = actual_visibility_range;
    }

    *sequence   = &actual_led_setting->sequence;
    *dim_parameter = &actual_led_setting->dim;
    *reduction = &actual_reduction;
}

/**
  * @brief  Set a parameter in active led profile (only in ram).
  * @param  param1: parameter to select between (day, night and conditions)
  * @param  param2: sub parameter
  * @param  param3: sub parameter
  * @param  param4: sub parameter
  * @param  param5: sub parameter
  * @param  value: new value
  * @retval BMM_SET_PP_RESULT
  */
BMM_SET_PP_RESULT_T BMM_SetProfileParameter(uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4, uint8_t param5, uint32_t value)
{
    BMM_LED_SETTINGS * pLEDSetting;

    if (param1 == 1 || param1 == 2)
    {
        if (param1 == 1)
            pLEDSetting = &_bmm_active_led_profile.day;
        else
            pLEDSetting = &_bmm_active_led_profile.night;

        switch(param2)
        {
        case 1:  // sequence
        {
            switch(param3)
            {
            case 1:  // length
                if (value > SEQUENCE_LENGTH)
                    return BMM_SET_PP_RESULT_VALUE_OUT_OF_RANGE;
                pLEDSetting->sequence.length = value;
                break;
            case 2:  // table
                if (param4 > SEQUENCE_LENGTH - 1)
                    return BMM_SET_PP_RESULT_VALUE_PARAMETER_NOT_FOUND_4;

                switch(param5)
                {
                case 1:
                    pLEDSetting->sequence.table[param4].timespan = value;
                    break;
                case 2:
                    pLEDSetting->sequence.table[param4].dim_time = value;
                    break;
                case 3:
                    pLEDSetting->sequence.table[param4].dim_value = value;
                    break;
                default:
                    return BMM_SET_PP_RESULT_VALUE_PARAMETER_NOT_FOUND_5;
                    break;
                }
                break;
            case 3:  // delay
                pLEDSetting->sequence.delay = value;
                break;
            default:
                return BMM_SET_PP_RESULT_VALUE_PARAMETER_NOT_FOUND_3;
            }
            break;
        }
        case 2:  // dim
        {
            switch(param3)
            {
            case 1:  // settings
                switch(param4)
                {
                case 1:
                    pLEDSetting->dim.settings.type = value;
                    break;
                case 2:
                    pLEDSetting->dim.settings.gamma = value;
                    break;
                default:
                    return BMM_SET_PP_RESULT_VALUE_PARAMETER_NOT_FOUND_4;
                    break;
                }
            case 2:  // led power level
                if (param4 > DIM_LED_COUNT || param4 < 1)
                    return BMM_SET_PP_RESULT_VALUE_PARAMETER_NOT_FOUND_4;// break;

                pLEDSetting->dim.led_powerlevel[param4 - 1] = value;
                break;
            default:
                return BMM_SET_PP_RESULT_VALUE_PARAMETER_NOT_FOUND_3;
            }
            break;
        }
        case 3:  // visibility factor
            switch(param3)
            {
            case 1:
                pLEDSetting->visibility_factor.use_visibility = value;
                break;
            case 2:
                pLEDSetting->visibility_factor.range_1_distance = value;
                break;
            case 3:
                pLEDSetting->visibility_factor.level_reduction_1 = value;
                break;
            case 4:
                pLEDSetting->visibility_factor.range_2_distance = value;
                break;
            case 5:
                pLEDSetting->visibility_factor.level_reduction_2 = value;
                break;
            default:
                return BMM_SET_PP_RESULT_VALUE_PARAMETER_NOT_FOUND_3;
                break;
            }
            break;
        case 4:  // on demand
            switch(param3)
            {
            case 1:
                pLEDSetting->on_demand.use_switch_on_demand = value;
                break;
            case 2:
                pLEDSetting->on_demand.switch_off_delay = value;
                break;
            default:
                return BMM_SET_PP_RESULT_VALUE_PARAMETER_NOT_FOUND_3;
                break;
            }
            break;
        default:
            return BMM_SET_PP_RESULT_VALUE_PARAMETER_NOT_FOUND_2;
        }
    }
    else if (param1 == 3)
    {
        switch(param2)
        {
        case 1:
            _bmm_active_led_profile.conditions.gloom_threshold_dusk = value;
            break;
        case 2:
            _bmm_active_led_profile.conditions.gloom_threshold_dawn = value;
            break;
        default:
            return BMM_SET_PP_RESULT_VALUE_PARAMETER_NOT_FOUND_2;
        }
    }
    else
    {
        return BMM_SET_PP_RESULT_VALUE_PARAMETER_NOT_FOUND_1;
    }

    LOG(LOG_ID_BMM_PROFILE_PARAMETER_CHANGED, param1, (uint32_t)(param2<<24) + (param3<<16) + (param4<<8) + param5, value, NULL);

    return BMM_SET_PP_RESULT_OK;
}


/**
  * @brief  Save actual led profile to fram.
  * @param  None
  * @retval None
  */
void BMM_SaveProfile(void)
{
    // write selected profile into fram
    (void)FRAM_MIRROR_Write(FRAM_BMM_LED_PROFILES_BASE, &_bmm_active_led_profile, sizeof(BMM_LED_PROFILE));
    LOG(LOG_ID_BMM_PROFILE_SAVED, 0, 0, 0, NULL);
}


/**
  * @brief  Load flash led profile preset to ram.
  * @param  Profile ID
  * @retval TRUE on success
  */
bool BMM_LoadProfile(uint32_t profileId)
{
    int32_t idx = BMM_GetProfileIndex(profileId);
    if (idx >= 0)
    {
        // load selected profile preset from flash into ram
        LOG(LOG_ID_BMM_PROFILE_LOADED, idx, _bmm_active_led_profile.descriptor.profile_id, profileId, NULL);
        memcpy(&_bmm_active_led_profile, &bmm_led_profiles_flash[idx], sizeof(BMM_LED_PROFILE));
        return TRUE;
    }
    return FALSE;
}


/**
  * @brief  Initializes the BMM module (loads profile from fram into ram).
  * @param  None
  * @retval None
  */
void BMM_Init(void)
{
    // read selected profile into ram
    (void)FRAM_MIRROR_Read(FRAM_BMM_LED_PROFILES_BASE, &_bmm_active_led_profile, sizeof(BMM_LED_PROFILE));
}
