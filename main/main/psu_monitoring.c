/*
 * psu_monitoring.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 15.02.2013
 *      Author: DHA
 *
 * Purpose:
 * Provide system information of external power supply unit and monitoring functions.
 *
 */

#include "stdheader.h"
#include "psu_monitoring.h"
#include "debug_freertos.h"

#include "clock.h"
#include "fram_mirror.h"
#include "log.h"

// variables used to store and manage data from psu
static bool b_data_valid = FALSE;  // flag to indicate that data from psu is valid
static uint8_t data_available_bm = 0;  // bit mask used to detect timeouts of received data (one bit for each message)
static uint32_t psu_serial_fram = 0;  // serial read from fram

// data from psu
static PSU_MONITORING_PSU_INFO_S psu_info;
static PSU_MONITORING_PSU_STATUS_S psu_status;
static PSU_MONITORING_PSU_SENSORS_S psu_sensors;
static PSU_MONITORING_PSU_FAN_INFO_S psu_fan_info;
static PSU_MONITORING_BATTERY_INFO_S psu_battery_info[PSU_MONITORING_NUM_SUPPORTED_BATTERIES];
// Mutex to protect data of this module
static xSemaphoreHandle xPSU_MonitoringDataMutex;

// variables used for error handling within this module
static uint8_t psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_COUNT];
static uint8_t psu_monitor_battery_status_error_counter[PSU_MONITORING_BAT_STATUS_IDX_COUNT];
static uint32_t psu_monitoring_warning_flags = 0;  // warning flags

// external variables
bool _b_psu_is_buffering = FALSE;  // flag to indicate battery mode
PSU_MONITORING_STATUS_T _psu_monitoring_status = PSU_MONITORING_STATUS_INIT;  // status of this module

// ****************************************
// ******** GETTERS and SETTERS ***********
// ****************************************

/**
  * @brief  Set psu_info.
  * @param  p_data: Pointer to data which should be set.
  * @retval None
  */
void PSU_MONITORING_Set_PSU_Info(PSU_MONITORING_PSU_INFO_S *p_data)
{
    memcpy(&psu_info, p_data, sizeof(PSU_MONITORING_PSU_INFO_S));
    data_available_bm |= PSU_MONITORING_PSU_INFO_AVAILABLE_FLAG;
}

/**
  * @brief  Set psu_status.
  * @param  p_data: Pointer to data which should be set.
  * @retval None
  */
void PSU_MONITORING_Set_PSU_Status(PSU_MONITORING_PSU_STATUS_S *p_data)
{
    memcpy(&psu_status, p_data, sizeof(PSU_MONITORING_PSU_STATUS_S));
    data_available_bm |= PSU_MONITORING_PSU_STATUS_AVAILABLE_FLAG;
}

/**
  * @brief  Set psu_sensors.
  * @param  p_data: Pointer to data which should be set.
  * @retval None
  */
void PSU_MONITORING_Set_PSU_Sensors(PSU_MONITORING_PSU_SENSORS_S *p_data)
{
    memcpy(&psu_sensors, p_data, sizeof(PSU_MONITORING_PSU_SENSORS_S));
    data_available_bm |= PSU_MONITORING_PSU_SENSORS_AVAILABLE_FLAG;
}

/**
  * @brief  Set psu_fan_info.
  * @param  p_data: Pointer to data which should be set.
  * @retval None
  */
void PSU_MONITORING_Set_PSU_FanInfo(PSU_MONITORING_PSU_FAN_INFO_S *p_data)
{
    memcpy(&psu_fan_info, p_data, sizeof(PSU_MONITORING_PSU_FAN_INFO_S));
    data_available_bm |= PSU_MONITORING_PSU_FANINFO_AVAILABLE_FLAG;
}

/**
  * @brief  Set psu_battery_info.
  * @param  i_battery: Index of battery.
  * @param  p_data: Pointer to data which should be set.
  * @retval None
  */
void PSU_MONITORING_Set_PSU_BatteryInfo(uint8_t i_battery, PSU_MONITORING_BATTERY_INFO_S *p_data)
{
    if (i_battery >= PSU_MONITORING_NUM_SUPPORTED_BATTERIES)
        return;  // avoid exceeding array borders

    memcpy(&psu_battery_info[i_battery], p_data, sizeof(PSU_MONITORING_BATTERY_INFO_S));
    data_available_bm |= PSU_MONITORING_PSU_BATTERYINFO_AVAILABLE_FLAG;
}

/**
  * @brief  Get psu_info.
  * @param  p_data: Pointer where the returned data should be stored.
  * @retval boolean: - true: Ok, data available
  *                  - false: Error, data not available
  */
bool PSU_MONITORING_Get_PSU_Info(PSU_MONITORING_PSU_INFO_S *p_data)
{
    if (!b_data_valid)
        return FALSE;  // no information received yet or timeout

    memcpy(p_data, &psu_info, sizeof(PSU_MONITORING_PSU_INFO_S));
    return TRUE;
}

/**
  * @brief  Get psu_status.
  * @param  p_data: Pointer where the returned data should be stored.
  * @retval boolean: - true: Ok, data available
  *                  - false: Error, data not available
  */
bool PSU_MONITORING_Get_PSU_Status(PSU_MONITORING_PSU_STATUS_S *p_data)
{
    if (!b_data_valid)
        return FALSE;  // no information received yet or timeout

    memcpy(p_data, &psu_status, sizeof(PSU_MONITORING_PSU_STATUS_S));
    return TRUE;
}

/**
  * @brief  Get psu_sensors.
  * @param  p_data: Pointer where the returned data should be stored.
  * @retval boolean: - true: Ok, data available
  *                  - false: Error, data not available
  */
bool PSU_MONITORING_Get_PSU_Sensors(PSU_MONITORING_PSU_SENSORS_S *p_data)
{
    if (!b_data_valid)
        return FALSE;  // no information received yet or timeout
    memcpy(p_data, &psu_sensors, sizeof(PSU_MONITORING_PSU_SENSORS_S));
    return TRUE;
}

/**
  * @brief  Get psu_fan_info.
  * @param  p_data: Pointer where the returned data should be stored.
  * @retval boolean: - true: Ok, data available
  *                  - false: Error, data not available
  */
bool PSU_MONITORING_Get_PSU_FanInfo(PSU_MONITORING_PSU_FAN_INFO_S *p_data)
{
    if (!b_data_valid)
        return FALSE;  // no information received yet or timeout
    memcpy(p_data, &psu_fan_info, sizeof(PSU_MONITORING_PSU_FAN_INFO_S));
    return TRUE;
}

/**
  * @brief  Get psu_battery_info.
  * @param  i_battery: Index of battery.
  * @param  p_data: Pointer to data which should be set.
  * @param  p_data: Pointer where the returned data should be stored.
  * @retval boolean: - true: Ok, data available
  *                  - false: Error, data not available
  */
bool PSU_MONITORING_Get_PSU_BatteryInfo(uint8_t i_battery, PSU_MONITORING_BATTERY_INFO_S *p_data)
{
    if (!b_data_valid)
        return FALSE;  // no information received yet or timeout
    if (i_battery >= PSU_MONITORING_NUM_SUPPORTED_BATTERIES)
        return FALSE;  // avoid exceeding array borders

    memcpy(p_data, &psu_battery_info[i_battery], sizeof(PSU_MONITORING_BATTERY_INFO_S));
    return TRUE;
}

/**
  * @brief  Get warning flags.
  * @param  None
  * @retval warning flags
  */
uint32_t PSU_MONITORING_Get_WarningFlags(void)
{
    return psu_monitoring_warning_flags;
}

// ****************************************
// ******** internal functions ************
// ****************************************

/**
  * @brief  Check timeout.
  * @param  None
  * @retval None
  */
void PSU_MONITORING_CheckTimeout(void)
{
    static CLOCK_S_T last_valid = 0;

    if ((CLOCK_GetTimestamp() > last_valid + PSU_MONITORING_DATA_TIMEOUT_S) || (b_data_valid == FALSE))
    {
        if (data_available_bm ==  PSU_MONITORING_ALL_DATA_AVAILABLE_BM)
        {
            data_available_bm = 0;
            b_data_valid = TRUE;
            last_valid = CLOCK_GetTimestamp();
        }
        else
            b_data_valid = FALSE;
    }
}

/**
  * @brief  Check if a psu is available.
  * @param  None
  * @retval None
  */
void PSU_MONITORING_CheckPSUAvailable(void)
{
    static LOG_DUPVAR_T psu_logdupvar = 0;

    if (b_data_valid)
    {
        // data from psu available
        if (psu_info.serial != psu_serial_fram)
        {
            // write new serial to fram
            psu_serial_fram = psu_info.serial;
            FRAM_MIRROR_Write32(FRAM_PSU_SERIAL, &psu_serial_fram);
        }
        _psu_monitoring_status = PSU_MONITORING_STATUS_AVAILABLE;
        LOG(LOG_ID_PSU_MONITORING_PSU_DETECTED, psu_info.hw_revision, (((uint32_t) psu_info.sw_version_major << 16) | psu_info.sw_version_minor), psu_info.serial, &psu_logdupvar);
    }
    else
    {
        // data from psu not available
        if (psu_serial_fram != 0)
        {
            if (CLOCK_GetTimestamp() > PSU_MONITORING_DATA_TIMEOUT_S)
            {
                _psu_monitoring_status = PSU_MONITORING_STATUS_MISSING;  // psu is not available but it should be ==> error
                LOG(LOG_ID_PSU_MONITORING_PSU_MISSING, 0, 0, psu_serial_fram, &psu_logdupvar);
            }
        }
        else
        {
            _psu_monitoring_status = PSU_MONITORING_STATUS_NOT_AVAILABLE;
            LOG(LOG_ID_PSU_MONITORING_PSU_UNKNOWN, 0, 0, 0, &psu_logdupvar);
        }
    }
}

/**
  * @brief  Check service status.
  * @param  status: status to check
  * @param  err_ctr_idx: index of error counter
  * @param  warn_flag_bit: bit of warn flag
  * @retval None
  */
void PSU_MONITORING_CheckServiceStatus(SVWIRE_DETECTOR_SERVICE_STATUS_T status, PSU_MONITORING_STATUS_ERROR_CTR_T err_ctr_idx, PSU_MONITORING_FLAG_BIT_T warn_flag_bit)
{
    switch (status)
    {

    case SVWIRE_DETECTOR_SERVICE_STATUS_ERROR:
    case SVWIRE_DETECTOR_SERVICE_STATUS_REPLACE:
    {
        psu_monitoring_warning_flags |= (1 << warn_flag_bit);
        if (psu_monitor_status_error_counter[err_ctr_idx] > 0)
            psu_monitor_status_error_counter[err_ctr_idx]--;  // decrement error counter
        break;
    }
    case SVWIRE_DETECTOR_SERVICE_STATUS_OK:
    {
        psu_monitoring_warning_flags &= ~(1 << warn_flag_bit);
        if (psu_monitor_status_error_counter[err_ctr_idx] > 0)
            psu_monitor_status_error_counter[err_ctr_idx]--;  // decrement error counter
        break;
        break;
    }
    case SVWIRE_DETECTOR_SERVICE_STATUS_INIT: // same as default
    default:
    {
        // increment error counter
        if (psu_monitor_status_error_counter[err_ctr_idx] < PSU_MONITORING_INTERNAL_ERROR_COUNT_MAX)
            psu_monitor_status_error_counter[err_ctr_idx]++;
        break;
    }
    }

}

/**
  * @brief  Check battery service status.
  * @param  status: status to check
  * @param  warn_flags: pointer to warn flags
  * @param  warn_flag_bit: bit of warn flag and index of error counter
  * @retval None
  */
void PSU_MONITORING_CheckBatteryServiceStatus(SVWIRE_DETECTOR_SERVICE_STATUS_T status, uint32_t *warn_flags, PSU_MONITORING_BAT_STATUS_IDX_T warn_flag_bit)
{
    switch (status)
    {

    case SVWIRE_DETECTOR_SERVICE_STATUS_ERROR:
    case SVWIRE_DETECTOR_SERVICE_STATUS_REPLACE:
    {
        *warn_flags |= (1 << warn_flag_bit);
        if (psu_monitor_battery_status_error_counter[warn_flag_bit] > 0)
            psu_monitor_battery_status_error_counter[warn_flag_bit]--;  // decrement error counter
        break;
    }
    case SVWIRE_DETECTOR_SERVICE_STATUS_OK:
    {
        *warn_flags &= ~(1 << warn_flag_bit);
        if (psu_monitor_battery_status_error_counter[warn_flag_bit] > 0)
            psu_monitor_battery_status_error_counter[warn_flag_bit]--;  // decrement error counter
        break;
        break;
    }
    case SVWIRE_DETECTOR_SERVICE_STATUS_INIT: // same as default
    default:
    {
        // increment error counter
        if (psu_monitor_battery_status_error_counter[warn_flag_bit] < PSU_MONITORING_INTERNAL_ERROR_COUNT_MAX)
            psu_monitor_battery_status_error_counter[warn_flag_bit]++;
        break;
    }
    }
}

/**
  * @brief  Check operation mode and status information of psu.
  * @param  None
  * @retval None
  */
void PSU_MONITORING_MaintainStatus(void)
{
    if (_psu_monitoring_status != PSU_MONITORING_STATUS_AVAILABLE)
        return;  // this function needs psu to work

    // ups status
    switch (psu_status.ups_status)
    {
    case SVWIRE_UPS_CONTROL_UPS_STATUS_BUFFERING:
        // mains power failure ==> running on battery!
        _b_psu_is_buffering = TRUE;
        psu_monitoring_warning_flags |= (1 << PSU_MONITORING_FLAG_BIT_UPS);
        if (psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_UPS] > 0)
            psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_UPS]--;  // decrement error counter
        break;
    case SVWIRE_UPS_CONTROL_UPS_STATUS_MAINS:
        _b_psu_is_buffering = FALSE;
        psu_monitoring_warning_flags &= ~(1 << PSU_MONITORING_FLAG_BIT_UPS);
        if (psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_UPS] > 0)
            psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_UPS]--;  // decrement error counter
        break;
    default:
        // increment error counter
        if (psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_UPS] < PSU_MONITORING_INTERNAL_ERROR_COUNT_MAX)
            psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_UPS]++;
        break;
    }

    // ups battery status (using data from ups)
    switch (psu_status.ups_battery_status)
    {
    case SVWIRE_UPS_CONTROL_BATTERY_STATUS_NOK:
        psu_monitoring_warning_flags |= (1 << PSU_MONITORING_FLAG_BIT_UPS_BATTERY_STATUS);
        if (psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_UPS_BATTERY_STATUS] > 0)
            psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_UPS_BATTERY_STATUS]--;  // decrement error counter
        break;
    case SVWIRE_UPS_CONTROL_BATTERY_STATUS_OK:
        psu_monitoring_warning_flags &= ~(1 << PSU_MONITORING_FLAG_BIT_UPS_BATTERY_STATUS);
        if (psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_UPS_BATTERY_STATUS] > 0)
            psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_UPS_BATTERY_STATUS]--;  // decrement error counter
        break;
    case SVWIRE_UPS_CONTROL_BATTERY_STATUS_UNKNOWN:  // same as default
    default:
        // increment error counter
        if (psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_UPS_BATTERY_STATUS] < PSU_MONITORING_INTERNAL_ERROR_COUNT_MAX)
            psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_UPS_BATTERY_STATUS]++;
        break;
    }

    // voltage
    switch (psu_status.voltage_rating)
    {
    case SVWIRE_DETECTOR_VOLTAGE_RATING_UNDER_VOLTAGE:
    case SVWIRE_DETECTOR_VOLTAGE_RATING_OVER_VOLTAGE:
    {
        psu_monitoring_warning_flags |= (1 << PSU_MONITORING_FLAG_BIT_VOLTAGE);
        if (psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_VOLTAGE] > 0)
            psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_VOLTAGE]--;  // decrement error counter
        break;
    }
    case SVWIRE_DETECTOR_VOLTAGE_RATING_OK:
    {
        psu_monitoring_warning_flags &= ~(1 << PSU_MONITORING_FLAG_BIT_VOLTAGE);
        if (psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_VOLTAGE] > 0)
            psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_VOLTAGE]--;  // decrement error counter
        break;
    }
    case SVWIRE_DETECTOR_VOLTAGE_RATING_INIT:  //same as default
    default:
    {
        // increment error counter
        if (psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_VOLTAGE] < PSU_MONITORING_INTERNAL_ERROR_COUNT_MAX)
            psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_VOLTAGE]++;
        break;
    }
    }

    // temperature fuse
    switch (psu_status.temperature_fuse_rating)
    {
    case SVWIRE_DETECTOR_TEMPERATURE_FUSE_RATING_BROKEN:
    {
        psu_monitoring_warning_flags |= (1 << PSU_MONITORING_FLAG_BIT_THERMAL_FUSE);
        if (psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_THERMAL_FUSE] > 0)
            psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_THERMAL_FUSE]--;  // decrement error counter
        break;
    }
    case SVWIRE_DETECTOR_TEMPERATURE_FUSE_RATING_OK:
    {
        psu_monitoring_warning_flags &= ~(1 << PSU_MONITORING_FLAG_BIT_THERMAL_FUSE);
        if (psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_THERMAL_FUSE] > 0)
            psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_THERMAL_FUSE]--;  // decrement error counter
        break;
    }
    case SVWIRE_DETECTOR_TEMPERATURE_FUSE_RATING_INIT:  // same as default
    default:
    {
        // increment error counter
        if (psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_THERMAL_FUSE] < PSU_MONITORING_INTERNAL_ERROR_COUNT_MAX)
            psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_THERMAL_FUSE]++;
        break;
    }
    }

    // psu temperature
    switch (psu_status.temperature_rating_system)
    {
    case SVWIRE_DETECTOR_TEMPERATURE_RATING_UNDER_TEMPERATURE:
    case SVWIRE_DETECTOR_TEMPERATURE_RATING_OVER_TEMPERATURE:
    case SVWIRE_DETECTOR_TEMPERATURE_RATING_ERROR:
    {
        psu_monitoring_warning_flags |= (1 << PSU_MONITORING_FLAG_BIT_SYSTEM_TEMPERATURE);
        if (psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_LAG_SYSTEM_TEMPERATURE] > 0)
            psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_LAG_SYSTEM_TEMPERATURE]--;  // decrement error counter
        break;
    }
    case SVWIRE_DETECTOR_TEMPERATURE_RATING_OK:
    {
        psu_monitoring_warning_flags &= ~(1 << PSU_MONITORING_FLAG_BIT_SYSTEM_TEMPERATURE);
        if (psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_LAG_SYSTEM_TEMPERATURE] > 0)
            psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_LAG_SYSTEM_TEMPERATURE]--;  // decrement error counter
        break;
        break;
    }
    case SVWIRE_DETECTOR_TEMPERATURE_RATING_INIT: // same as default
    default:
    {
        // increment error counter
        if (psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_LAG_SYSTEM_TEMPERATURE] < PSU_MONITORING_INTERNAL_ERROR_COUNT_MAX)
            psu_monitor_status_error_counter[PSU_MONITORING_STATUS_ERROR_CTR_LAG_SYSTEM_TEMPERATURE]++;
        break;
    }
    }

    // fan
    PSU_MONITORING_CheckServiceStatus(psu_status.service_fan, PSU_MONITORING_STATUS_ERROR_CTR_FAN, PSU_MONITORING_FLAG_BIT_FAN);
    // fan filter
    PSU_MONITORING_CheckServiceStatus(psu_status.service_fan_filter, PSU_MONITORING_STATUS_ERROR_CTR_FAN_FILTER, PSU_MONITORING_FLAG_BIT_FAN_FILTER);

    static uint32_t status_error_flags = 0;  // used to determine error status for each battery (overwrite warning flag if there are too many errors)
    for (uint8_t i = 0; i < PSU_MONITORING_STATUS_ERROR_CTR_COUNT; i++)
    {
        if (psu_monitor_status_error_counter[i] >= PSU_MONITORING_INTERNAL_ERROR_COUNT_MAX)
            status_error_flags |= (1 << i);
        else if (psu_monitor_status_error_counter[i] >= PSU_MONITORING_INTERNAL_ERROR_COUNT_MAX)
            status_error_flags &= ~(1 << i);
    }

    // check if warning flags should be set because of too many errors
    for (uint8_t i = 0; i < PSU_MONITORING_STATUS_ERROR_CTR_COUNT; i++)
    {
        if (status_error_flags & (1 << i))
            psu_monitoring_warning_flags |= (1 << i);  // set flag / overwrite flag if there are too many errors, reset is done within check function
    }


    // BATTERIES
    // Batteries are treated separately because the number of batteries may increase in the future.
    // Using data from dongles!
    static uint32_t battery_warn_flags = 0;  // used to determine flag status for each battery
    static uint32_t battery_status_error_flags = 0;  // used to determine error status for each battery (overwrite warning flag if there are too many errors)

    PSU_MONITORING_CheckBatteryServiceStatus(psu_status.service_battery_1, &battery_warn_flags, PSU_MONITORING_BAT_STATUS_IDX_BAT_1_AGE);  // battery 1
    PSU_MONITORING_CheckBatteryServiceStatus(psu_status.service_battery_2, &battery_warn_flags, PSU_MONITORING_BAT_STATUS_IDX_BAT_2_AGE);  // battery 2

    // combine warn flags and error flags of all batteries to one warn flag and one error flag
    if (battery_warn_flags != 0)
        psu_monitoring_warning_flags |= (1 << PSU_MONITORING_FLAG_BIT_BATTERY_AGE);
    else
        psu_monitoring_warning_flags &= ~(1 << PSU_MONITORING_FLAG_BIT_BATTERY_AGE);

    for (uint8_t i = 0; i < PSU_MONITORING_BAT_STATUS_IDX_COUNT; i++)
    {
        if (psu_monitor_battery_status_error_counter[i] >= PSU_MONITORING_INTERNAL_ERROR_COUNT_MAX)
            battery_status_error_flags |= (1 << i);  // set the error flag if too many errors occurred
        else if (psu_monitor_battery_status_error_counter[i] <= PSU_MONITORING_INTERNAL_ERROR_COUNT_MAX / 2)
            battery_status_error_flags &= ~(1 << i);  // reset the error flag if the number of errors felt below the limit
    }

    // set warning flag if too many errors have been detected
    if (battery_status_error_flags != 0)
        psu_monitoring_warning_flags |= (1 << PSU_MONITORING_FLAG_BIT_BATTERY_AGE);
}

/**
  * @brief  Generate log messages if necessary.
  * @param  None
  * @retval None
  */
void PSU_MONITORING_GenerateLogMessages(void)
{
    static LOG_DUPVAR_T ups_logdupvar = 0;

    // extra log message for supply mode
    if (_b_psu_is_buffering == TRUE)
        LOG(LOG_ID_PSU_MONITORING_PSU_BUFFERING, 0, 0, 0, &ups_logdupvar);
    else
        LOG(LOG_ID_PSU_MONITORING_PSU_MAINS, 0, 0, 0, &ups_logdupvar);

    // log messages for warnings which have been added or cleared
    static uint32_t warning_flags_old = 0;  // old warning flags
    uint32_t old_and_new = psu_monitoring_warning_flags & warning_flags_old;

    if (old_and_new != psu_monitoring_warning_flags)
    {
        // new warnings added
        LOG(LOG_ID_PSU_MONITORING_NEW_WARNINGS, 0, (old_and_new ^ psu_monitoring_warning_flags), 0, NULL);
    }

    if (old_and_new != warning_flags_old)
    {
        // warnings cleared
        LOG(LOG_ID_PSU_MONITORING_WARNINGS_CLEARED, 0, (old_and_new ^ warning_flags_old), 0, NULL);
    }

    warning_flags_old = psu_monitoring_warning_flags;
}

// ****************************************
// **************** Locking ***************
// ****************************************

/**
  * @brief  Lock psu data.
  * @param  None
  * @retval None
  */
void PSU_MONITORING_Lock(void)
{
    xSemaphoreTake(xPSU_MonitoringDataMutex, portMAX_DELAY);
}

/**
  * @brief  Get psu data lock with timeout.
  * @param  xTicksToWait: Timeout in ms
  * @retval boolean: - true: success
  *                  - false: an error occurred
  */
bool PSU_MONITORING_LockTimeout(portTickType xTicksToWait)
{
    return (xSemaphoreTake(xPSU_MonitoringDataMutex, xTicksToWait) == pdTRUE);
}

/**
  * @brief  Unlock psu data.
  * @param  None
  * @retval None
  */
void PSU_MONITORING_Unlock(void)
{
    xSemaphoreGive(xPSU_MonitoringDataMutex);
}

// ****************************************
// ******* CREATE, INIT and TICK **********
// ****************************************

/**
  * @brief  Create.
  * @param  None
  * @retval None
  */
void PSU_MONITORING_Create(void)
{
    if (xPSU_MonitoringDataMutex == NULL)
    {
        xPSU_MonitoringDataMutex = xSemaphoreCreateMutex();
        assert_param(xPSU_MonitoringDataMutex != NULL);
        DQueueAddToRegistry(xPSU_MonitoringDataMutex, (const signed char*)"xPSU_MonitoringDataMutex");
    }
    else
        assert_param(FALSE);
}

/**
  * @brief  Init.
  * @param  None
  * @retval None
  */
void PSU_MONITORING_Init(void)
{
    PSU_MONITORING_Lock();
    b_data_valid = FALSE;
    _b_psu_is_buffering = FALSE;
    _psu_monitoring_status = PSU_MONITORING_STATUS_INIT;

    psu_monitoring_warning_flags = 0;

    for (uint8_t i = 0; i < PSU_MONITORING_STATUS_ERROR_CTR_COUNT; i++)
    {
        psu_monitor_status_error_counter[i] = 0;
    }

    for (uint8_t i = 0; i < PSU_MONITORING_BAT_STATUS_IDX_COUNT; i++)
    {
        psu_monitor_battery_status_error_counter[i] = 0;
    }

    memset(&psu_info, 0, sizeof(PSU_MONITORING_PSU_INFO_S));
    memset(&psu_status, 0, sizeof(PSU_MONITORING_PSU_STATUS_S));
    memset(&psu_sensors, 0, sizeof(PSU_MONITORING_PSU_SENSORS_S));
    memset(&psu_fan_info, 0, sizeof(PSU_MONITORING_PSU_FAN_INFO_S));
    memset(&psu_battery_info, 0, PSU_MONITORING_NUM_SUPPORTED_BATTERIES * sizeof(PSU_MONITORING_BATTERY_INFO_S));

    // load psu serial from fram
    FRAM_MIRROR_Read32(FRAM_PSU_SERIAL, &psu_serial_fram);  // return value not evaluated because this function is not critical
    PSU_MONITORING_Unlock();
}

void PSU_MONITORING_Tick(void)
{
    uint32_t svwire_sw_version = 0;

    if (PSU_MONITORING_LockTimeout(PSU_MONITORING_GET_MUTEX_TIMEOUT_MS) != TRUE)
        return;  // fatal error todo[HIGH] decide what to do if the mutex could not be taken (reboot?)
    // check timeout
    PSU_MONITORING_CheckTimeout();
    // check psu (available/new/missing)
    PSU_MONITORING_CheckPSUAvailable();  // error handling is included in this function
    // check psu status (mains or buffering, health of system and batteries, ...)
    svwire_sw_version = ((uint32_t) psu_info.sw_version_major << 16) | psu_info.sw_version_minor;

    // The actual version of svwire has 2 bugs!
    // The first bug: There is no debounce of inputs from ups.
    // The second bug: The function to write data into battery dongles does not work.
    // To avoid wrong warnings (false positives) this function should not be used with current version (1.10).
    if (svwire_sw_version > PSU_MONITORING_MIN_SVWIRE_SW_VERSION)
        PSU_MONITORING_MaintainStatus();
    PSU_MONITORING_Unlock();
    if (svwire_sw_version > PSU_MONITORING_MIN_SVWIRE_SW_VERSION)  // see comment above
        PSU_MONITORING_GenerateLogMessages();
}
