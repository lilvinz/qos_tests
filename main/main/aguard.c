#include "stdheader.h"
#include "aguard.h"

#include "debug_freertos.h"
#include "adc.h"
#include "log.h"
#include "clock.h"
#include "sensors.h"
#include "statistics.h"
#include "fram_mirror.h"
#include "settings.h"
#include "debug.h"

// Analog Guard
// Requests ADC measurements and validates values

static LOG_DUPVAR_T aguard_temperature_logged_state[TEMPERATURE_COUNT];  // error logged state

static uint16_t aguard_led_stage_ov_fail_bm[PS_LED_OVERVOLTAGE_BM_COUNT];  // fail bitmask
static uint16_t aguard_temperature_fail_bm;  // fail bitmask

static AGUARD_EXTREMES_T aguard_extremes;
static AGUARD_LED_CALIBRATION_T aguard_led_calibration[ADC_LED_CH_INDEX_COUNT];

static CLOCK_S_T local_led_voltages_ema_last_update[ADC_LED_CH_INDEX_COUNT];  // used to store the timestamp of last ema update (used for led fail detection)
static uint16_t led_avg_value_counter[ADC_LED_CH_INDEX_COUNT];  // counter for number of values considered for average (used for led fail detection)
static uint16_t aguard_led_fail_counter[ADC_LED_CH_INDEX_COUNT];  // error counter (used for led fail detection)
static uint16_t led_fail_detection_fail_ctr[ADC_LED_CH_INDEX_COUNT];  // counter to count number of measurements with a voltage below minimum, used for led fail detection
static uint16_t led_fail_detection_total_ctr[ADC_LED_CH_INDEX_COUNT];  // counter to count total number of measured values, used for led fail detection

const uint8_t led_ov_num_flags_list[PS_LED_OVERVOLTAGE_BM_COUNT] =
{
#if defined(HWP_LED_WHITE_20K) || defined(HWP_LED_RED_UNI) || defined(HWP_LED_RED_2K)
    ADC_LED_NUM_CHANNELS,
#endif
#ifdef HWP_LED_WHITE_100K
    CONFIG_NUM_LEDS_PER_100K_SLAVE,
    CONFIG_NUM_LEDS_PER_100K_SLAVE,
#endif

};

const float aguard_max_led_voltage[ADC_LED_CH_INDEX_COUNT] =
{
#ifdef HWP_LED_WHITE_20K
    AGUARD_MAX_LED_VOLTAGE_WHITE_V,
    AGUARD_MAX_LED_VOLTAGE_WHITE_V,
    AGUARD_MAX_LED_VOLTAGE_WHITE_V,
    AGUARD_MAX_LED_VOLTAGE_WHITE_V,
#endif
#ifdef HWP_LED_RED_UNI
    AGUARD_MAX_LED_VOLTAGE_RED_V,
    AGUARD_MAX_LED_VOLTAGE_RED_V,
    AGUARD_MAX_LED_VOLTAGE_RED_V,
    AGUARD_MAX_LED_VOLTAGE_RED_V,
#endif
#ifdef HWP_LED_RED_2K
    AGUARD_MAX_LED_VOLTAGE_RED_V,
    AGUARD_MAX_LED_VOLTAGE_RED_V
#endif
};

static xQueueHandle xQueueAguard_LedVoltages;

/**
  * @brief  Get led stage overvoltage fail bitmask.
  * @param  bm_index: Index of bitmask which should be returned
  * @retval led stage fail bitmask
  */
uint16_t AGUARD_GetLedStageOvFailBitmask(PS_LED_OVERVOLTAGE_BM_E bm_index)
{
    if (bm_index > PS_LED_OVERVOLTAGE_BM_COUNT - 1)
        return 0;

    return aguard_led_stage_ov_fail_bm[bm_index];
}

/**
  * @brief  Get temperature fail bitmask.
  * @param  None
  * @retval temperature fail bitmask
  */
uint16_t AGUARD_GetTemperatureFailBitmask(void)
{
    return aguard_temperature_fail_bm;
}

/**
  * @brief  Get aguard extremes.
  * @param  None
  * @retval extreme values
  */
void AGUARD_GetExtremes(AGUARD_EXTREMES_T *p)
{
    memcpy(p, &aguard_extremes, sizeof(AGUARD_EXTREMES_T));
}

/**
  * @brief  Set LED calibration
  * @param  LedId, LedCalibration Pointer
  * @retval -
  */
void AGUARD_SetLEDCalibration(uint8_t LedId, AGUARD_LED_CALIBRATION_T *p)
{
    memcpy(&aguard_led_calibration[LedId], p, sizeof(AGUARD_LED_CALIBRATION_T));
}

/**
  * @brief  Get LED calibration
  * @param  LedId, LedCalibration Pointer
  * @retval -
  */
void AGUARD_GetLEDCalibration(uint8_t LedId, AGUARD_LED_CALIBRATION_T *p)
{
    memcpy(p, &aguard_led_calibration[LedId], sizeof(AGUARD_LED_CALIBRATION_T));
}

/**
  * @brief  Add new voltage to queue.
  * @param  None
  * @retval None
  */
void AGUARD_AddLedVoltage(ADC_LED_CH_INDEX_T led_channel, float led_voltage)
{
    AGUARD_LED_VOLTAGE_QUEUE_ITEM_T queue_item;

    queue_item.led_channel = led_channel;
    queue_item.led_voltage = led_voltage;

    xQueueSendToBack(xQueueAguard_LedVoltages, &queue_item, 0);  // no timeout
}

/**
  * @brief  Save LED calibration to FRAM
  * @param  -
  * @retval -
  */
void AGUARD_SaveLEDCalibration(void)
{
#if ((FRAM_CALIBRATION_LED_BASE + (AGUARD_LED_CALIBRATION_T_SIZE * ADC_LED_CH_INDEX_COUNT)) > FRAM_BMM_LED_PROFILES_BASE)
#error "AGUARD fram section size violation!"
#endif
    FRAM_MIRROR_Write(FRAM_CALIBRATION_LED_BASE, aguard_led_calibration, sizeof(AGUARD_LED_CALIBRATION_T) * ADC_LED_CH_INDEX_COUNT);
}

/**
  * @brief  Load LED calibration from FRAM
  * @param  -
  * @retval -
  */
void AGUARD_LoadLEDCalibration(void)
{
    FRAM_MIRROR_Read(FRAM_CALIBRATION_LED_BASE, aguard_led_calibration, sizeof(AGUARD_LED_CALIBRATION_T) * ADC_LED_CH_INDEX_COUNT);
}

/**
  * @brief  Create queue for aguard.
  * @param  None
  * @retval None
  */
void AGUARD_CreateQueue(void)
{
    xQueueAguard_LedVoltages = xQueueCreate(AGUARD_LED_VOLTAGE_QUEUE_SIZE, sizeof(AGUARD_LED_VOLTAGE_QUEUE_ITEM_T));
    DQueueAddToRegistry(xQueueAguard_LedVoltages, "xQueueAguard_LedVoltages");
    assert_param(xQueueAguard_LedVoltages != NULL);
}

/**
  * @brief  Initializes the AGUARD module.
  * @param  None
  * @retval None
  */
void AGUARD_Init(void)
{
#ifdef HWP_LOCAL_LEDS
    _primestruct.flags.led_overvoltage_bm[PS_LED_OVERVOLTAGE_BM_MAIN] = 0;
    _primestruct.flags.led_voltage_ok_bm[PS_LED_OVERVOLTAGE_BM_MAIN] = 0;
#endif

    aguard_extremes.pwr24v.max = UINT16_T_MIN;
    aguard_extremes.pwr24v.min = UINT16_T_MAX;
    aguard_extremes.pwr24v.average = 0;

    for (uint8_t i = 0; i < PS_LED_OVERVOLTAGE_BM_COUNT; i++)
    {
        aguard_led_stage_ov_fail_bm[i] = 0;
    }
    for (uint8_t i = 0; i < ADC_LED_CH_INDEX_COUNT; i++)
    {
        local_led_voltages_ema_last_update[i] = 0;
        led_avg_value_counter[i] = 0;
        aguard_led_fail_counter[i] = 0;
        led_fail_detection_fail_ctr[i] = 0;
        led_fail_detection_total_ctr[i] = 0;
    }
#if 0  // disabled, because implementation is not finished
    AGUARD_LoadLEDCalibration();
#endif
}

/**
 * \brief calculate minimum, maximum and average of 24 V
 *
 * \param none
 * \return none
*/
void AGUARD_Calc24VMinMax(void)
{
    static uint32_t aguard_minmax_hold_timestamp;
    // ADC conversion is running automatically in background. see adc.c for more info on how its done.

    if (CLOCK_GetTimestamp() > aguard_minmax_hold_timestamp)
    {
        aguard_minmax_hold_timestamp = CLOCK_GetTimestamp() + AGUARD_POWER_MINMAX_HOLD_TIME_S;
        aguard_extremes.pwr24v.max = UINT16_T_MIN;
        aguard_extremes.pwr24v.min = UINT16_T_MAX;
    }

    if (_primestruct.sensors.analog.supply[PS_ANALOG_VOLTAGE_MAIN_24V].value > aguard_extremes.pwr24v.max)
        aguard_extremes.pwr24v.max = _primestruct.sensors.analog.supply[PS_ANALOG_VOLTAGE_MAIN_24V].value;

    if (_primestruct.sensors.analog.supply[PS_ANALOG_VOLTAGE_MAIN_24V].value < aguard_extremes.pwr24v.min)
        aguard_extremes.pwr24v.min = _primestruct.sensors.analog.supply[PS_ANALOG_VOLTAGE_MAIN_24V].value;
    {
        float avrg;
        avrg = aguard_extremes.pwr24v.average;
        avrg *= 2;
        avrg += _primestruct.sensors.analog.supply[PS_ANALOG_VOLTAGE_MAIN_24V].value;
        aguard_extremes.pwr24v.average = avrg / 3;
    }
}

#if defined (HWP_LOCAL_LEDS) || defined (HWP_REMOTE_LEDS)
/**
 * \brief check if leds are broken
 *
 * \param none
 * \return none
*/
void AGUARD_CheckLeds(void)
{
    static uint32_t aguard_led_error_logged_bm[PS_LED_OVERVOLTAGE_BM_COUNT] =
    {
#if defined(HWP_LED_WHITE_20K) || defined(HWP_LED_RED_UNI) || defined(HWP_LED_RED_2K)
        0,
#endif
#ifdef HWP_LED_WHITE_100K
        0,
        0,
#endif
    };  // error logged bitmask

    static CLOCK_S_T local_led_fail_timestamp[PS_LED_OVERVOLTAGE_BM_COUNT] =
    {
#if defined(HWP_LED_WHITE_20K) || defined(HWP_LED_RED_UNI) || defined(HWP_LED_RED_2K)
        0,
#endif
#ifdef HWP_LED_WHITE_100K
        0,
        0,
#endif
    };  // error timestamp

    uint16_t temp_led_overvoltage_bm = 0;
    uint16_t temp_led_voltage_ok_bm = 0;

    for (uint8_t i_bitmask = 0; i_bitmask < PS_LED_OVERVOLTAGE_BM_COUNT; i_bitmask++)
    {
        temp_led_overvoltage_bm = _primestruct.flags.led_overvoltage_bm[i_bitmask];
        _primestruct.flags.led_overvoltage_bm[i_bitmask] = 0;  // reset bitmask
        temp_led_voltage_ok_bm = _primestruct.flags.led_voltage_ok_bm[i_bitmask];
        _primestruct.flags.led_voltage_ok_bm[i_bitmask] = 0;  // reset bitmask

        if (temp_led_overvoltage_bm != 0)
        {
            // error, led broken
            for (uint8_t idx = 0; idx < led_ov_num_flags_list[i_bitmask]; idx++)
            {
                if (temp_led_overvoltage_bm & (1 << idx))
                {
                    aguard_led_stage_ov_fail_bm[i_bitmask] |= (1 << idx);  // set fail flags
                    if ((aguard_led_error_logged_bm[i_bitmask] & (1 << idx)) == 0)
                    {
                        LOG(LOG_ID_AGUARD_LED_STAGE_FAILED, i_bitmask, idx, _primestruct.sensors.analog.led[idx].value, NULL);
                        aguard_led_error_logged_bm[i_bitmask] |= (1 << idx);
                    }
                    local_led_fail_timestamp[i_bitmask] = CLOCK_GetTimestamp();
                }
            }
        }
        else
        {
            if ((aguard_led_stage_ov_fail_bm[i_bitmask] != 0) && (CLOCK_GetTimestamp() > (local_led_fail_timestamp[i_bitmask] + AGUARD_LED_OVERVOLTAGE_FAIL_TIMEOUT_S)))
            {
                for (uint8_t idx = 0; idx < led_ov_num_flags_list[i_bitmask]; idx++)
                {
                    if ((aguard_led_stage_ov_fail_bm[i_bitmask] & (1 << idx)) && (temp_led_voltage_ok_bm & (1 << idx)))
                    {
                        // leds are working properly again ==> reset fail flags
                        // note: To reset fail flags all stages of one group have to be OK, we do not store timestamps for each single stage!
                        aguard_led_stage_ov_fail_bm[i_bitmask] &= ~(1 << idx);  // reset fail flag, this will not result in self-healing because the fail flags are stored in next level (detector)
#if (VERSION_IS_DEBUG > 0)  // no self-healing
                        LOG(LOG_ID_AGUARD_LED_STAGE_OK_AGAIN, i_bitmask, idx, _primestruct.sensors.analog.led[idx].value, NULL);
                        aguard_led_error_logged_bm[i_bitmask] &= ~(1 << idx);  // reset flag
#endif
                    }
                }
            }
        }
    }
}
#endif

/**
  * @brief  This function decides if the detection of broken leds is available or not.
  * @param  None
  * @retval None
  */
bool AGUARD_CheckLedFailDetectionAvailable(void)
{
    bool b_result = FALSE;

#ifdef HWP_LOCAL_LEDS
#if (defined HWT_MULTI) || (defined HWT_MEDIUM_WHITE)
    HW_REV_T hw_rev = VERSION_GetHWRev();
    // hardware id sirius
    if (hw_rev >= HW_REV_2_4)
    {
        b_result = TRUE;  // supported on revision 2.4
    }
    else
    {
        b_result = FALSE;  // not supported because of insufficient capabilities of hardware
    }

#elif defined HWT_MEDIUM_RED
    // hardware id 2000 cd
    b_result = TRUE;  // supported on revision 1
#else
#error "Hardware type not supported!"
#endif

    return b_result;
#endif

}

#if defined (HWP_LOCAL_LEDS) || defined (HWP_REMOTE_LEDS)
/**
 * \brief reset counter for voltage average
 *
 * \param none
 * \return none
*/
void AGUARD_ResetAVGCounter(void)
{
    for (uint8_t i = 0; i < ADC_LED_CH_INDEX_COUNT; i++)
    {
        if (CLOCK_GetTimestamp() > (local_led_voltages_ema_last_update[i] + AGUARD_LED_AVG_VOLTAGE_TIMEOUT_S))
            led_avg_value_counter[i] = 0;
    }
}
#endif

#if defined (HWP_LOCAL_LEDS) || defined (HWP_REMOTE_LEDS)
/**
 * \brief check if leds are broken
 *
 * \param none
 * \return none
*/
void AGUARD_CheckRedLeds(void)
{
    static CLOCK_S_T last_flag_check_ts = 0;

    if (CLOCK_GetTimestamp() > (last_flag_check_ts + AGUARD_LED_VOLTAGE_FAIL_FLAG_CHECK_INTERVAL_S))
    {
        last_flag_check_ts = CLOCK_GetTimestamp();

#if (VERSION_IS_DEBUG > 0)
        TEMPERATURE_SENSOR_T temperature;
        SENSORS_GetTemperature(0, &temperature);
        if (temperature.state >= SENSOR_OK)
            DPRINT2(DEBUG_AGUARD, "S: %4d.%03d", (int)temperature.value, abs((int)(temperature.value * 1000.0f) % 1000));
        else
            DPRINT(DEBUG_AGUARD, "S: FAIL ");

        for (uint8_t i_channel = 0; i_channel < ADC_LED_CH_INDEX_COUNT; i_channel++)
        {
            DPRINT4(DEBUG_AGUARD, " %2u.%03u %2u.%03u", (uint16_t)_primestruct.sensors.analog.local_led_voltages_min_max[i_channel].min, (uint32_t)(_primestruct.sensors.analog.local_led_voltages_min_max[i_channel].min * 1000) % 1000, (uint16_t)_primestruct.sensors.analog.local_led_voltages_min_max[i_channel].max, (uint32_t)(_primestruct.sensors.analog.local_led_voltages_min_max[i_channel].max * 1000) % 1000);
        }

        DPRINT(DEBUG_AGUARD, "\n");
#endif

#ifdef HWT_MEDIUM_RED  // fail detection using voltage at dimming FET (measure during pwm low time)
        // todo[high] add fail detection for other hardware types, this is only for 2kcd
#if (VERSION_IS_DEBUG > 0)  // debug error counter (beginning)
        if (temperature.state >= SENSOR_OK)
            DPRINT2(DEBUG_AGUARD, "C: %4d.%03d", (int)temperature.value, abs((int)(temperature.value * 1000.0f) % 1000));
        else
            DPRINT(DEBUG_AGUARD, "C: FAIL ");
#endif

        for (uint8_t i_channel = 0; i_channel < ADC_LED_CH_INDEX_COUNT; i_channel++)
        {
            if (led_fail_detection_total_ctr[i_channel] > AGUARD_MINIMUM_NUMBER_MEASUREMENTS_FOR_FAIL_DETECTION)
            {
                // enough values for fail detection
                if (led_fail_detection_fail_ctr[i_channel] > AGUARD_LED_FAIL_DETECTION_FAIL_SAMPLE_COUNTER_MAX)  // for the first test, we use 100 as maximum
                {
                    // error detected ==> increment error counter
                    if (aguard_led_fail_counter[i_channel] < AGUARD_LED_FAIL_COUNTER_MAX)
                        aguard_led_fail_counter[i_channel]++;
                }
                else
                {
                    // no error detected ==> reset error counter
                    aguard_led_fail_counter[i_channel] = 0;
                }
            }

#if (VERSION_IS_DEBUG > 0)  // debug error counter (data output)
            DPRINT2(DEBUG_AGUARD, " %u %u", led_fail_detection_fail_ctr[i_channel], led_fail_detection_total_ctr[i_channel]);
#endif

            // reset counter
            led_fail_detection_fail_ctr[i_channel] = 0;
            led_fail_detection_total_ctr[i_channel] = 0;

            _primestruct.sensors.analog.local_led_voltages_min_max[i_channel].min = PS_MINIMUM_RESET_VALUE_1000_VOLT;
            _primestruct.sensors.analog.local_led_voltages_min_max[i_channel].max = 0;
        }
#if (VERSION_IS_DEBUG > 0)  // debug error counter (end)
        DPRINT(DEBUG_AGUARD, "\n");
#endif

#if (VERSION_IS_DEBUG > 0)
        if (temperature.state >= SENSOR_OK)
            DPRINT2(DEBUG_AGUARD, "B: %4d.%03d", (int)temperature.value, abs((int)(temperature.value * 1000.0f) % 1000));
        else
            DPRINT(DEBUG_AGUARD, "B: FAIL ");

        for (uint8_t i_channel = 0; i_channel < ADC_LED_CH_INDEX_COUNT; i_channel++)
        {
            DPRINT1(DEBUG_AGUARD, " %u", aguard_led_fail_counter[i_channel]);
        }

        DPRINT(DEBUG_AGUARD, "\n");
#endif
        // check error counters
        for (uint8_t i_channel = 0; i_channel < ADC_LED_CH_INDEX_COUNT; i_channel++)
        {
            if (aguard_led_fail_counter[i_channel] >= AGUARD_LED_FAIL_COUNTER_MAX)
            {
                // too many errors detected ==> set led stage into fail state
                // Attention: this is dirty
                // todo[HIGH] replace this
                // The overvoltage bitmask is used as error bitmask
                // The bitmask is used for all channels. Only the red channels shoud be handled here.
                _primestruct.flags.led_overvoltage_bm[PS_LED_OVERVOLTAGE_BM_MAIN] |= 1 << i_channel;
                _primestruct.flags.led_voltage_ok_bm[PS_LED_OVERVOLTAGE_BM_MAIN] &= ~(1 << i_channel);
            }
            else
            {
                // Attention: this is dirty
                // todo[HIGH] replace this
                // The overvoltage bitmask is used as error bitmask
                // The bitmask is used for all channels. Only the red channels shoud be handled here.
                _primestruct.flags.led_voltage_ok_bm[PS_LED_OVERVOLTAGE_BM_MAIN] |= 1 << i_channel;
            }
        }
#endif
    }
}
#endif

#if defined(HWP_LOCAL_LEDS)
/**
 * \brief add new value to led average voltage
 *
 * \param new_value: new value to add to average
 * \param led_channel: index of led channel
 * \return none
*/
void AGUARD_LedAvgVoltageAddValue(float new_value, ADC_LED_CH_INDEX_T led_channel)
{
    // check array index
    if (led_channel >= ADC_LED_CH_INDEX_COUNT)
        return;

    if (led_avg_value_counter[led_channel] < AGUARD_LED_AVG_VALUE_COUNTER_STARTUP_LIMIT)
    {
        // startup ==> calculate the ema by this formula
        // x = x + (y-x)*0.5   x=avrg, y=newval

        float temp;
        temp = (new_value - _primestruct.sensors.analog.local_led_voltages_min_max[led_channel].average) * 0.5f;
        _primestruct.sensors.analog.local_led_voltages_min_max[led_channel].average += temp;
    }
    else
    {
        // calculate the ema by this formula
        // x = a*y + (1-a)*x   x=avrg, a=factor, y=newval

        float avrg;
        avrg = new_value - _primestruct.sensors.analog.local_led_voltages_min_max[led_channel].average;
        avrg *= AGUARD_LED_VOLTAGE_EMA_MULTIPLIER;
        avrg /= AGUARD_LED_VOLTAGE_EMA_DIVISOR;
        _primestruct.sensors.analog.local_led_voltages_min_max[led_channel].average += avrg;
    }

    local_led_voltages_ema_last_update[led_channel] = CLOCK_GetTimestamp();
    if (led_avg_value_counter[led_channel] < AGUARD_LED_AVG_VALUE_COUNTER_MAX)
        led_avg_value_counter[led_channel]++;
}
#endif

#if defined(HWP_LOCAL_LEDS)
/**
 * \brief process led voltages from adc
 *
 * \param none
 * \return none
*/
void AGUARD_ProcessLedVoltages(void)
{
    while (uxQueueMessagesWaiting(xQueueAguard_LedVoltages) > 0)
    {
        AGUARD_LED_VOLTAGE_QUEUE_ITEM_T aguard_queue_item;

        if (xQueueReceive(xQueueAguard_LedVoltages, &aguard_queue_item, 0) == pdTRUE)
        {
#if defined(HWP_LED_RED_UNI) || defined(HWP_LED_RED_2K)
            // check for broken red LEDs (voltage at gate of dimming FET)
            // This has to be done here to ensure that all values are used and the average is not affected by new measurement
            // detection of broken led stages is not supported on some hw revisions
#ifdef HWT_MEDIUM_RED  // fail detection using voltage at gate dimming FET (measure during pwm high time)
            // todo[high] add fail detection for other hardware types, this is only for 2kcd
            if (aguard_queue_item.led_voltage < AGUARD_RED_LED_MIN_VOLTAGE_AT_DIMMING_FET)
            {
                // in case of a broken led stage, this voltage is near 0
                // increment counter
                if (led_fail_detection_fail_ctr[aguard_queue_item.led_channel] < UINT16_T_MAX)
                    led_fail_detection_fail_ctr[aguard_queue_item.led_channel]++;  // avoid overflow
            }
            if (led_fail_detection_total_ctr[aguard_queue_item.led_channel] < UINT16_T_MAX)
                led_fail_detection_total_ctr[aguard_queue_item.led_channel]++;  // avoid overflow

            _primestruct.sensors.analog.local_led_voltages_min_max[aguard_queue_item.led_channel].max = aguard_queue_item.led_voltage;

            // calc minimum and maximum, with new fail detection this can be used for debugging only
            if (aguard_queue_item.led_voltage > _primestruct.sensors.analog.local_led_voltages_min_max[aguard_queue_item.led_channel].max)
                _primestruct.sensors.analog.local_led_voltages_min_max[aguard_queue_item.led_channel].max = aguard_queue_item.led_voltage;
            if (aguard_queue_item.led_voltage < _primestruct.sensors.analog.local_led_voltages_min_max[aguard_queue_item.led_channel].min)
                _primestruct.sensors.analog.local_led_voltages_min_max[aguard_queue_item.led_channel].min = aguard_queue_item.led_voltage;

            AGUARD_LedAvgVoltageAddValue(aguard_queue_item.led_voltage, aguard_queue_item.led_channel);
#endif
#endif
#if (VERSION_IS_DEBUG > 0)
            TEMPERATURE_SENSOR_T temperature;
            SENSORS_GetTemperature(0, &temperature);
            if (temperature.state >= SENSOR_OK)
                DPRINT5(DEBUG_AGUARD, "A: %u: %4d.%03d %2u.%03u\n", aguard_queue_item.led_channel, (int)temperature.value, abs((int)(temperature.value * 1000.0f) % 1000), (uint16_t)aguard_queue_item.led_voltage, (uint32_t)(aguard_queue_item.led_voltage * 1000) % 1000);
            else
                DPRINT3(DEBUG_AGUARD, "A: %u: FAIL %2u.%03u\n", aguard_queue_item.led_channel, (uint16_t)aguard_queue_item.led_voltage, (uint32_t)(aguard_queue_item.led_voltage * 1000) % 1000);
#endif

#if defined (HWP_LED_WHITE_20K)
            // Old implementation, todo[high] change this code, this code should be used to check white leds only, we do this after the 2kcd release!
            // check for broken LEDs (open circuit, overvoltage)
            // This has to be done here to ensure that all values are used
            if (AGUARD_CheckLedFailDetectionAvailable())
            {
                // detection of broken led stages is not supported on hw revisions < 2.4
                if (aguard_queue_item.led_voltage > aguard_max_led_voltage[aguard_queue_item.led_channel])
                {
                    _primestruct.flags.led_overvoltage_bm[PS_LED_OVERVOLTAGE_BM_MAIN] |= 1 << aguard_queue_item.led_channel;
                    _primestruct.flags.led_voltage_ok_bm[PS_LED_OVERVOLTAGE_BM_MAIN] &= ~(1 << aguard_queue_item.led_channel);
                }
                else
                {
                    _primestruct.flags.led_voltage_ok_bm[PS_LED_OVERVOLTAGE_BM_MAIN] |= 1 << aguard_queue_item.led_channel;
                }
            }
#endif
        }
    }
}
#endif

/**
 * \brief tick function (has to be called every 10 ms).
 *
 * \param none
 * \return none
*/
void AGUARD_Tick_10_MS(void)
{
#if defined (HWP_LOCAL_LEDS)
    // process new values (this should be done every 10 ms to avoid queue-overflow)
    AGUARD_ProcessLedVoltages();
#endif
}

/**
 * \brief tick function
 *
 * \param none
 * \return none
*/
void AGUARD_Tick(void)
{
    AGUARD_Calc24VMinMax();

#if defined (HWP_LOCAL_LEDS)  // todo[medium] add support for remote leds || defined (HWP_REMOTE_LEDS)
    AGUARD_ResetAVGCounter();
    // check leds
    if (AGUARD_CheckLedFailDetectionAvailable())
    {
        // detection of broken led stages is not supported on all hw types and hw revisions
        AGUARD_CheckRedLeds();
        AGUARD_CheckLeds();
    }
#endif

    // check temperature
    for (TEMPERATURE_T idx = TEMPERATURE_MAIN; idx < TEMPERATURE_COUNT; idx++)
    {
        TEMPERATURE_SENSOR_T tempsensor_data;

        SENSORS_GetTemperature(idx, &tempsensor_data);

        if (tempsensor_data.state == SENSOR_OK)
        {
            if (tempsensor_data.value > AGUARD_TEMPERATURE_MAX_VALUE + AGUARD_TEMPERATURE_HYSTERESIS)
            {
                // temperature above maximum
                LOG(LOG_ID_AGUARD_TEMPERATURE_OUT_OF_RANGE, idx, (uint32_t)tempsensor_data.value, 0, &aguard_temperature_logged_state[idx]);
                aguard_temperature_fail_bm |= (1 << idx);
            }
            else if (tempsensor_data.value < AGUARD_TEMPERATURE_MIN_VALUE - AGUARD_TEMPERATURE_HYSTERESIS)
            {
                // temperature below minimum
                //ToDo If transition to new Log output with inline values occures, remove abs() here and below for int (+/-) representation
                LOG(LOG_ID_AGUARD_TEMPERATURE_OUT_OF_RANGE, idx, (uint32_t)abs((int)tempsensor_data.value), 0, &aguard_temperature_logged_state[idx]);
                aguard_temperature_fail_bm |= (1 << idx);
            }
            else
            {
                // temperature within range
                if ((tempsensor_data.value > AGUARD_TEMPERATURE_MIN_VALUE) || (tempsensor_data.value < AGUARD_TEMPERATURE_MAX_VALUE))
                {
                    // write log and reset flag to prevent flooding the log file
                    STATISTICS_SENSORS_T sensor_stats;
                    sensor_stats = STATISTICS_GetValue(idx);
                    if (tempsensor_data.value > 0.0f)
                        LOG(LOG_ID_AGUARD_TEMPERATURE_WITHIN_RANGE, idx, (uint32_t)tempsensor_data.value, (uint32_t)abs((int)sensor_stats.max), &aguard_temperature_logged_state[idx]);
                    else
                        LOG(LOG_ID_AGUARD_TEMPERATURE_WITHIN_RANGE, idx, (uint32_t)abs((int)tempsensor_data.value), (uint32_t)abs((int)sensor_stats.min), &aguard_temperature_logged_state[idx]);

                    aguard_temperature_fail_bm   &= ~(1 << idx);
                }
            }
        }
    }
}
