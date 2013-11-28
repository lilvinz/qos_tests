#include "stdheader.h"
#include "debug_freertos.h"

#include "detector.h"

#include "fram_mirror.h"
#include "indication.h"
#include "settings.h"
#include "clock.h"
#include "sensors.h"
#include "gpssync.h"
#include "systemconfig.h"
#include "visibility.h"
#include "mesh.h"
#include "aguard.h"
#include "gloom_manager.h"
#include "hw_configuration.h"
#include "psu_monitoring.h"

#include "log.h"
#include "watchdog.h"



DETECTOR_STATUS_T _detector_status;
volatile uint8_t _detector_fault_bm;

static uint8_t number_of_failed_led_stages = 0;
static uint16_t led_stage_fail_bm[PS_LED_OVERVOLTAGE_BM_COUNT];  // fail bitmask
static uint16_t led_stage_fail_bm_fram[PS_LED_OVERVOLTAGE_BM_COUNT];  // fail bitmask

// Mutex to protect data of this module
static xSemaphoreHandle xDETECTOR_LedFailDataMutex;

bool DETECTOR_LedFailDataLockTimeout(portTickType xTicksToWait);
void DETECTOR_LedFailDataUnlock(void);
void DETECTOR_LedFailDataLock(void);

/**
  * @brief  Get led stage fail bitmask.
  * (Decide if mutex should be used)
  * @param  bm_index: Index of bitmask which should be returned
  * @retval led stage fail bitmask
  */
uint16_t DETECTOR_GetLedStageFailBitmask(PS_LED_OVERVOLTAGE_BM_E bm_index)
{
    if (bm_index > PS_LED_OVERVOLTAGE_BM_COUNT - 1)
        return 0;

    return led_stage_fail_bm[bm_index];
}

/**
  * @brief  Get the number of failed led stages.
  * (Decide if mutex should be used)
  * @param  None
  * @retval number of failed led stages
  */
uint8_t DETECTOR_GetNumFailedLedStages(void)
{
    return number_of_failed_led_stages;
}

/**
  * @brief  GPS VCC fault irq handler.
  * @param  None
  * @retval None
  */
void DETECTOR_GPS_VCC_Fault_IRQHandler(void)
{
    _detector_fault_bm |= DETECTOR_FAULT_BM_GPS_VCC;
}

/**
  * @brief  GPS RF fault irq handler.
  * @param  None
  * @retval None
  */
void DETECTOR_GPS_RF_Fault_IRQHandler(void)
{
    _detector_fault_bm |= DETECTOR_FAULT_BM_GPS_RF;
}

/**
  * @brief  LUX 2,8V fault irq handler.
  * @param  None
  * @retval None
  */
void DETECTOR_LUX_2_8_V_Fault_IRQHandler(void)
{
    _detector_fault_bm |= DETECTOR_FAULT_BM_LUX_2_8_V;
}

/**
  * @brief  Set error flag for can error.
  * @param  None
  * @retval None
  */
void DETECTOR_SetCANErrorFlag(void)
{
    //  todo[medium] implement a reset algorithm
    _detector_fault_bm |= FAIL_FLAG_CAN;
};

void DETECTOR_Powermanager_CheckFault(void)
{
    if (GPIO_ReadInputDataBit(DETECTOR_FAULT_SENSE_PORT, DETECTOR_GPS_VCC_FAULT_SENSE_PIN))
        _detector_fault_bm &= ~DETECTOR_FAULT_BM_GPS_VCC;

    if (GPIO_ReadInputDataBit(DETECTOR_FAULT_SENSE_PORT, DETECTOR_GPS_RF_FAULT_SENSE_PIN))
        _detector_fault_bm &= ~DETECTOR_FAULT_BM_GPS_RF;

    if (GPIO_ReadInputDataBit(DETECTOR_FAULT_SENSE_PORT, DETECTOR_LUX_2_8_V_FAULT_SENSE_PIN))
        _detector_fault_bm &= ~DETECTOR_FAULT_BM_LUX_2_8_V;
}

bool detector_check_sensors(void)
{
    if (DETECTOR_WARN_IF_SENSOR_FAILED)
    {
        GLOOM_SENSOR_T gloom;
        SENSORS_GetGloom(&gloom);
        if (gloom.state < SENSOR_OK)
            return FALSE;

        TEMPERATURE_SENSOR_T temperature;
        for (TEMPERATURE_T idx = TEMPERATURE_MAIN; idx < TEMPERATURE_COUNT; idx++)
        {
            SENSORS_GetTemperature(idx, &temperature);
            if (temperature.state < SENSOR_OK)
                return FALSE;
        }
    }
    return TRUE;
}

bool detector_check_visibility(void)
{
    bool RetVal;
    RetVal = TRUE;
    VISIBILITY_SENSOR_T visibility;

    if (!DETECTOR_WARN_IF_VISIBILITY_FAILED)
        return TRUE;
    else
    {
        switch (SYSTEMCONFIG_STORED_SETTING)
        {
        case SYSTYPE_SINGLE_VISI:
        case SYSTYPE_MESHNODE_VISI:
            VISIBILITY_GetVisibility(&visibility);
            if (visibility.state != SENSOR_OK)
                RetVal = FALSE;
            break;
        default:
            break;
        }
        return RetVal;
    }
}

bool detector_check_gps(void)
{
    static bool b_gps_failed = FALSE;

    if (DETECTOR_WARN_IF_GPS_FAILED)
    {
        if (_gpssync.status >= FAILED)
            b_gps_failed = TRUE;
        else if (_gpssync.status == SYNCED)
            b_gps_failed = FALSE;  // reset flag only if state is "SYNCED"

        if (b_gps_failed)
            return FALSE;
    }
    return TRUE;
}

bool detector_check_mesh(void)
{
    if (!DETECTOR_WARN_IF_MESH_FAILED)
        return TRUE;
    else
    {
        bool RetVal;
        RetVal = TRUE;

        switch (SYSTEMCONFIG_STORED_SETTING)
        {
        case SYSTYPE_MESHNODE:
        case SYSTYPE_MESHNODE_VISI:
            if (FALSE == MESH_NetworkingIsAvailable())
                RetVal = FALSE;
            break;
        default:
            break;
        }
        return RetVal;
    }
}

bool detector_check_psu_collective_warnings(void)
{
    if (!DETECTOR_WARN_IF_PSU_COLLECTIVE)
        return TRUE;
    else
    {
        if (_psu_monitoring_status == PSU_MONITORING_STATUS_MISSING)
            return FALSE;
        else
        {
            // no locking (only one read access)
            if (PSU_MONITORING_Get_WarningFlags() == 0)
                return TRUE;
            else
                return FALSE;

        }
    }
}

bool detector_check_psu_buffering(void)
{
    if (!DETECTOR_WARN_IF_PSU_BUFFERING)
        return TRUE;
    else
    {
        if (_b_psu_is_buffering == TRUE)
            return FALSE;
        else
            return TRUE;
    }
}

#if 0  // disabled because we have not enough bits to handle each warning separately
void detector_check_psu_warnings(uint32_t * flags)
{
    uint32_t psu_flags = PSU_MONITORING_Get_WarningFlags();  // no locking (only one read access)

    // flag to warn if psu is running on battery or if this information could not be obtained (error)
    if (DETECTOR_WARN_IF_PSU_BUFF_OR_ERR && (psu_flags & (1 << PSU_MONITORING_FLAG_BIT_UPS)))
        *flags |= (1 << DETECTOR_WARN_IF_PSU_BUFF_OR_ERR);
    else
        *flags &= ~(1 << DETECTOR_WARN_IF_PSU_BUFF_OR_ERR);

    // flag to warn if ups detected a battery error  or if this information could not be obtained (error)
    if (DETECTOR_WARN_IF_PSU_UPS_BAT_ERR && (psu_flags & (1 << PSU_MONITORING_FLAG_BIT_UPS_BATTERY_STATUS)))
        *flags |= (1 << DETECTOR_WARN_IF_PSU_UPS_BAT_ERR);
    else
        *flags &= ~(1 << DETECTOR_WARN_IF_PSU_UPS_BAT_ERR);

    // flag to warn if voltage is out of range or if this information could not be obtained (error)
    if (DETECTOR_WARN_IF_PSU_VOLTAGE_ERR && (psu_flags & (1 << PSU_MONITORING_FLAG_BIT_VOLTAGE)))
        *flags |= (1 << DETECTOR_WARN_IF_PSU_VOLTAGE_ERR);
    else
        *flags &= ~(1 << DETECTOR_WARN_IF_PSU_VOLTAGE_ERR);

    // flag to warn if thermal fuses of psu are broken or if this information could not be obtained (error)
    if (DETECTOR_WARN_IF_PSU_THERMAL_FUSE_ERR && (psu_flags & (1 << PSU_MONITORING_FLAG_BIT_THERMAL_FUSE)))
        *flags |= (1 << DETECTOR_WARN_IF_PSU_THERMAL_FUSE_ERR);
    else
        *flags &= ~(1 << DETECTOR_WARN_IF_PSU_THERMAL_FUSE_ERR);

    // flag to warn if temperature is out of specified range or if this information could not be obtained (error)
    if (DETECTOR_WARN_IF_PSU_TEMPERATURE_ERR && (psu_flags & (1 << PSU_MONITORING_FLAG_BIT_SYSTEM_TEMPERATURE)))
        *flags |= (1 << DETECTOR_WARN_IF_PSU_TEMPERATURE_ERR);
    else
        *flags &= ~(1 << DETECTOR_WARN_IF_PSU_TEMPERATURE_ERR);

    // flag to warn if fan of psu has reached end of live or if this information could not be obtained (error)
    if (DETECTOR_WARN_IF_PSU_FAN_EOL && (psu_flags & (1 << PSU_MONITORING_FLAG_BIT_FAN)))
        *flags |= (1 << DETECTOR_WARN_IF_PSU_FAN_EOL);
    else
        *flags &= ~(1 << DETECTOR_WARN_IF_PSU_FAN_EOL);

    // flag to warn if fan of psu has reached end of live or if this information could not be obtained (error)
    if (DETECTOR_WARN_IF_PSU_FAN_FILTER_EOL && (psu_flags & (1 << PSU_MONITORING_FLAG_BIT_FAN_FILTER)))
        *flags |= (1 << DETECTOR_WARN_IF_PSU_FAN_FILTER_EOL);
    else
        *flags &= ~(1 << DETECTOR_WARN_IF_PSU_FAN_FILTER_EOL);

    // flag to warn if battery / batteries of psu has / have reached end of live or if this information could not be obtained (error)
    if (DETECTOR_WARN_IF_PSU_BATTERY_EOL && (psu_flags & (1 << PSU_MONITORING_FLAG_BIT_BATTERY_AGE)))
        *flags |= (1 << DETECTOR_WARN_IF_PSU_BATTERY_EOL);
    else
        *flags &= ~(1 << DETECTOR_WARN_IF_PSU_BATTERY_EOL);
}
#endif

/**
  * @brief  Load fail flags of led stages fram FRAM.
  * @param  None
  * @retval None
  */
void DETECTOR_LoadLedFailBitmasks(void)
{
    uint8_t failed_led_stages = 0;
    uint16_t temp_led_stage_fail_bm[PS_LED_OVERVOLTAGE_BM_COUNT];

    for (uint8_t i_bm = 0; i_bm < PS_LED_OVERVOLTAGE_BM_COUNT; i_bm++)
    {
        if (FRAM_MIRROR_Read16(FRAM_LED_FAIL_FLAGS_START + (i_bm * DETECTOR_LED_FAIL_FLAG_FRAM_ENTRY_SIZE), &temp_led_stage_fail_bm[i_bm]) != TRUE)
        {
            // an error occurred ==> set fail flags to 0 (do not cause fault positives)
            temp_led_stage_fail_bm[i_bm] = 0;
        }

        for (uint8_t i_bit = 0; i_bit < 16; i_bit++)
        {
            // count number of bits set in this bit mask
            if ((temp_led_stage_fail_bm[i_bm] & (1 << i_bit)) != 0)
                failed_led_stages++;
        }
    }

    // get mutex
    if (DETECTOR_LedFailDataLockTimeout(DETECTOR_GET_MUTEX_TIMEOUT_MS) == pdTRUE)
    {
        memcpy(&led_stage_fail_bm[0], &temp_led_stage_fail_bm[0], sizeof(temp_led_stage_fail_bm));
        memcpy(&led_stage_fail_bm_fram[0], &temp_led_stage_fail_bm[0], sizeof(temp_led_stage_fail_bm));
        number_of_failed_led_stages = failed_led_stages;
        DETECTOR_LedFailDataUnlock();
    }

    if (failed_led_stages > 0)
    {
        LOG(LOG_ID_DETECTOR_LED_FAIL_FLAGS_LOADED, failed_led_stages, temp_led_stage_fail_bm[0], 0, NULL);
    }
}

/**
  * @brief  Add new flags to bitmask in FRAM.
  * @param  id: index of bitmask
  * @param  new_flags: flags to add
  * @retval None
  */
void DETECTOR_FRAM_LedFailBitmasksAddFlags(PS_LED_OVERVOLTAGE_BM_E id, uint16_t new_flags)
{
    uint16_t addr;
    uint16_t temp_flags;  // fail bitmask

    if (id >= PS_LED_OVERVOLTAGE_BM_COUNT)
        return;  // avoid wrong access

    // calculate address
    addr = FRAM_LED_FAIL_FLAGS_START + (id * DETECTOR_LED_FAIL_FLAG_FRAM_ENTRY_SIZE);
    if (addr >= FRAM_LED_FAIL_FLAGS_NEXT_FREE)
        return;  // avoid wrong access

    if (FRAM_MIRROR_Read16(addr, &temp_flags) != TRUE)
        return;  // error during fram access

    temp_flags |= new_flags;  // add new flags to existing flags

    if (FRAM_MIRROR_Write16(addr, &temp_flags) != TRUE)
        return;  // error during fram access

    // everything ok ==> write new value to variable
    led_stage_fail_bm_fram[id] = temp_flags;

    LOG(LOG_ID_DETECTOR_LED_FAIL_FLAGS_STORED, id, new_flags, temp_flags, NULL);
}

/**
  * @brief  Reset fail flags of led stages in FRAM.
  * @param  None
  * @retval bool: - TRUE: data stored
  *               - FALSE: an error occurred
  */
bool DETECTOR_ResetFRAMLedFailBitmasks(void)
{
    bool b_result = TRUE;
    uint16_t led_fail_bm = 0;

    for (uint8_t i_bm = 0; i_bm < PS_LED_OVERVOLTAGE_BM_COUNT; i_bm++)
    {
        if (FRAM_MIRROR_Write16(FRAM_LED_FAIL_FLAGS_START + (i_bm * DETECTOR_LED_FAIL_FLAG_FRAM_ENTRY_SIZE), &led_fail_bm) != TRUE)
        {
            b_result = FALSE;  // write failed
        }
    }

    return b_result;
}

/**
  * @brief  Clear led fail flags.
  * @param  None
  * @retval None
  */
DETECTOR_CLEAR_LED_FAIL_FLAGS_RESULT_T DETECTOR_ClearLedFailFlags(portTickType xTicksToWait)
{
    if (DETECTOR_ResetFRAMLedFailBitmasks() != TRUE)
        return DETECTOR_CLEAR_LED_FAIL_FLAGS_RESULT_ERROR_WRITING_FRAM;

    // FRAM cleared, try to get mutex to clear variables
    if (DETECTOR_LedFailDataLockTimeout(xTicksToWait) == pdTRUE)
    {
        // Mutex obtained ==> clear variables
        for (uint8_t i_bm = 0; i_bm < PS_LED_OVERVOLTAGE_BM_COUNT; i_bm++)
        {
            led_stage_fail_bm_fram[i_bm] = 0;
            led_stage_fail_bm[i_bm] = 0;
        }
        number_of_failed_led_stages = 0;

        DETECTOR_LedFailDataUnlock();

        return DETECTOR_CLEAR_LED_FAIL_FLAGS_RESULT_OK;
    }
    else
    {
        // Timeout
        return DETECTOR_CLEAR_LED_FAIL_FLAGS_RESULT_TIMEOUT;
    }
}

/**
  * @brief  Check if leds are broken and set fail flags.
  * @param  None
  * @retval None
  */

void detector_check_led(void)
{
    uint8_t failed_led_stages = 0;
    uint16_t led_fail_bm[PS_LED_OVERVOLTAGE_BM_COUNT];  // fail bitmask

    // collect fail flags from aguard
    for (uint8_t i_bm = 0; i_bm < PS_LED_OVERVOLTAGE_BM_COUNT; i_bm++)
    {
        led_fail_bm[i_bm] = AGUARD_GetLedStageOvFailBitmask(i_bm);
    }

    // get mutex to avoid conflicts with the clear function
    if (DETECTOR_LedFailDataLockTimeout(DETECTOR_GET_MUTEX_TIMEOUT_MS) == pdTRUE)
    {
        // set fail flags, avoid self-healing
        for (uint8_t i_bm = 0; i_bm < PS_LED_OVERVOLTAGE_BM_COUNT; i_bm++)
        {
            led_stage_fail_bm[i_bm] |= led_fail_bm[i_bm];

            for (uint8_t i_bit = 0; i_bit < 16; i_bit++)
            {
                // count number of bits set in this bit mask
                if ((led_stage_fail_bm[i_bm] & (1 << i_bit)) != 0)
                    failed_led_stages++;
            }
        }

        number_of_failed_led_stages = failed_led_stages;

        // store all error flags to fram to avoid self-healing by restart
        for (uint8_t i_bm = 0; i_bm < PS_LED_OVERVOLTAGE_BM_COUNT; i_bm++)
        {
            // check if bitmask has to be updated
            uint16_t delta_bm;

            delta_bm = (led_stage_fail_bm[i_bm] | led_stage_fail_bm_fram[i_bm]) ^ led_stage_fail_bm_fram[i_bm];

            if (delta_bm != 0)
            {
                // actual fail bitmask contains flags which are not stored in FRAM, add them to FRAM
                DETECTOR_FRAM_LedFailBitmasksAddFlags(i_bm, led_stage_fail_bm[i_bm]);
            }
        }

        DETECTOR_LedFailDataUnlock();
    }
}

void DETECTOR_check_all(void)
{
    uint32_t flags = 0;

    if (!detector_check_sensors())
        flags += FAIL_FLAG_SENSOR;

    if (!detector_check_visibility())
        flags += FAIL_FLAG_VISIBILITY;

    if (!detector_check_gps())
        flags += FAIL_FLAG_GPS;

    if (!detector_check_mesh())
        flags += FAIL_FLAG_MESH;

#if (SVINFOCOM_IMPLEMENTATION_FINISHED == 1)
    if (!detector_check_psu_collective_warnings())
        flags += FAIL_FLAG_PSU_MISSING;

    if (!detector_check_psu_buffering())
        flags += FAIL_FLAG_PSU_BUFFERING;
#endif
//    detector_check_psu_warnings(&flags);

    _detector_status.uWarningFlags = flags;

    detector_check_led();

}

void DETECTOR_control_relais(void)
{
    static LOG_DUPVAR_T detector_logdupvar;

    // ***** RELAIS 1 *****
    // Used for warnings or Day/Night switch
    if (DETECTOR_USE_DAYNIGHT_SWITCH == TRUE)
    {
        // switch indicator relais regarding our led state
        if (GLOOM_MANAGER_GetLocalLedState())
            INDICATION_Reset(IND_A);  // night
        else
            INDICATION_Set(IND_A, DETECTOR_INDICATION_DAYNIGHT_TIMEOUT);  // day
    }
    else
    {
        // use relais for indicating warnings
        if (_detector_status.uWarningFlags != 0)
        {
            if (_detector_status.failure_timestamp + DETECTOR_TIMEOUT_TILL_WARNING_SEC < CLOCK_GetTimestamp())
                INDICATION_Set(IND_A, DETECTOR_INDICATION_WARNING_TIMEOUT);
        }
        else
            _detector_status.failure_timestamp = CLOCK_GetTimestamp();
        LOG(LOG_ID_DETECTOR_WARNINGS, 0, _detector_status.uWarningFlags, 0, &detector_logdupvar);
    }

    // ***** RELAIS 2 *****
    // Used for fatal errors like broken LEDs and/or Main Power failure
    if (DETECTOR_GetNumFailedLedStages() != 0)
    {
        INDICATION_Set(IND_B, DETECTOR_LED_FAILED_INDICATION_HOLD_TIME);
        _detector_status.uErrorFlags |= FAIL_FLAG2_LED;
    }
    else
        _detector_status.uErrorFlags &= ~FAIL_FLAG2_LED;
}

/**
  * @brief  Get lock for detector data.
  * @param  None
  * @retval None
  */
void DETECTOR_LedFailDataLock(void)
{
    xSemaphoreTake(xDETECTOR_LedFailDataMutex, portMAX_DELAY);
}

/**
  * @brief  Get lock for detector data with timeout.
  * @param  xTicksToWait: Timeout in ms
  * @retval boolean: - true: success
  *                  - false: an error occurred
  */
bool DETECTOR_LedFailDataLockTimeout(portTickType xTicksToWait)
{
    return (xSemaphoreTake(xDETECTOR_LedFailDataMutex, xTicksToWait) == pdTRUE);
}

/**
  * @brief  Unlock detector data.
  * @param  None
  * @retval None
  */
void DETECTOR_LedFailDataUnlock(void)
{
    xSemaphoreGive(xDETECTOR_LedFailDataMutex);
}

/**
  * @brief  Create.
  * @param  None
  * @retval None
  */
void DETECTOR_Create(void)
{
    if (xDETECTOR_LedFailDataMutex == NULL)
    {
        xDETECTOR_LedFailDataMutex = xSemaphoreCreateMutex();
        assert_param(xDETECTOR_LedFailDataMutex != NULL);
        DQueueAddToRegistry(xDETECTOR_LedFailDataMutex, (const signed char*)"xDETECTOR_LedFailDataMutex");
    }
    else
        assert_param(FALSE);
}

void DETECTOR_Init(void)
{
    _detector_status.failure_timestamp = CLOCK_GetTimestamp();
    _detector_status.uWarningFlags = 0;
    _detector_status.uErrorFlags = 0;

    INDICATION_Reset(IND_A);
    INDICATION_Reset(IND_B);

    // load led stage fail flags from fram
    // we use these flag to avoid self-healing by system reset
    DETECTOR_LoadLedFailBitmasks();
}

void DETECTOR_Task(void *pvParameters)
{
    CLOCK_MS_T detector_milli_timestamp = 0;

    DETECTOR_Init();
    AGUARD_Init();
    PSU_MONITORING_Init();  // init psu monitoring

    while(1)
    {

        AGUARD_Tick_10_MS();

        if (CLOCK_GetMillitimestamp() >= detector_milli_timestamp + DETECTOR_WAIT_TIME)
        {
            // 100 ms tick, attention: do not block longer than 10 ms
            detector_milli_timestamp = CLOCK_GetMillitimestamp();
            WATCHDOG_Heartbeat(WATCHDOG_TASK_ID_DETECTOR);
            AGUARD_Tick();  // call aguard
#if (SVINFOCOM_IMPLEMENTATION_FINISHED == 1)
            PSU_MONITORING_Tick();  // manage psu
#endif

            DETECTOR_check_all();  // check all components
            DETECTOR_control_relais();  // set and reset relais
            DETECTOR_Powermanager_CheckFault();  // check if an error was cleared
        }

        vTaskDelay(DETECTOR_TASK_DELAY_MS);
    }
}
