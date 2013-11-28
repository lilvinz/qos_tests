/*
 * sst_90_w.c
 *
 *  Created on: 14.03.2011
 *      Author: DHA
 */

#include "stdheader.h"
#include "sst_90_w.h"

#include "sensors.h"
#include "adc.h"
#include "dim.h"
#include "aguard.h"
#include "time_meter.h"
#include "primestruct.h"

/**
  * @brief  Returns calculated junction temperature.
  * @param  NJunction temperature
  * @retval - true : ok - false: an error occurred
  */
bool SST_90_W_GetJunctionTemperature(int32_t *junction_temperature)
{
#if 0
    // calculate junction temperature
    uint8_t led_voltage_mV_cnt = 0;
    uint32_t led_voltage_mV = 0;
    uint32_t led_power;
    int32_t temperature;
    TEMPERATURE_SENSOR_T T;

    if (junction_temperature == NULL)
        return FALSE;  // error

    SENSORS_GetAvgLedTemperature(&T);
    if (T.state < SENSOR_OK)
    {
        // temperature sensors for case temperature failed, try sensor on main board
        SENSORS_GetTemperature(TEMPERATURE_MAIN, &T);
        if (T.state < SENSOR_OK)
            return FALSE;  // sensor on main board failed too --> no compensation
    }

    if ((T.value > SENSORS_MAX_HEAT_SINK_TEMPERATURE) || (T.value < SENSORS_MIN_HEAT_SINK_TEMPERATURE))
        return FALSE;  // error, value out of range

    // get led voltage to calculate power
    for (uint8_t idx = 0; idx < SST_90_W_NUM_WHITE_LEDS; idx++)
    {
        if (_bm_adc_led_available & (1 << SST_90_W_NUM_WHITE_LEDS))
        {
            led_voltage_mV = (uint32_t) _primestruct.sensors.analog.led[idx].value;
            led_voltage_mV_cnt++;
        }
    }
    switch (led_voltage_mV_cnt)
    {
    case 0:
        led_voltage_mV = SST_90_W_LED_AVG_VOLTAGE;
        break;
    case 1:
        break;  // only one voltage value available -> use it
    default:
        led_voltage_mV /= led_voltage_mV_cnt;
    }

    led_power = led_voltage_mV * SST_90_W_LED_CURRENT / SST_90_W_NUMBER_LEDS_PER_CIRCUIT;  // power in uW (mV * mA)
    led_power /= 1000;  // uW --> mW
    led_power *= SST_90_W_LED_POWER_LOSS_FACTOR;  // consider only thermal power loss
    led_power /= 1000;
    // todo [medium] consider pwm and maybe on / off time of from beacon profile

    temperature = led_power * SST_90_W_RTHJB;
    temperature /= 1000;  // mW --> W / m°C --> °C
    temperature += (int32_t) T.value;  // add case temperature

    *junction_temperature = temperature;
    return TRUE;
#endif
    return FALSE;
}

/**
  * @brief  Returns a factor rectify the influence of junction temperature on aging.
  * @param  Type of led
  * @retval Compensation factor in percent
  */
uint16_t SST_90_W_GetTimemeterCompensation(void)
{
#if 0
    int32_t compensation;
    int32_t junction_temperature;

    compensation = SST_90_W_NO_COMPENSATION_FACTOR;  // set 100 % as default compensation (no compensation)

    if (!SST_90_W_GetJunctionTemperature(&junction_temperature))
        junction_temperature = SST_90_W_DEFAULT_JUNCTION_TEMPERATURE;

    if (junction_temperature < SST_90_W_TIMEMETER_COMPENSATION_MIN_TEMPERATURE)
        return compensation;  // no compensation for values below minimum

    // Temperature compensation:
    uint32_t factor;
    factor = SST_90_W_AGING_SLOPE_120 * 10000L / SST_90_W_AGING_SLOPE_80;  // avoid decimals
    factor -= 10000L;  // 1.XX --> 0.XX
    factor /= SST_90_W_TEMPERATURE_DIFFERENCE;
    factor *= (junction_temperature - SST_90_W_TIMEMETER_REF_TEMPERATURE);
    factor /= 100;  // get value in %

    compensation += factor;

    if (compensation > UINT16_T_MAX)
        compensation = UINT16_T_MAX;

    if (compensation < UINT16_T_MIN)
        compensation = UINT16_T_MIN;

    return (uint16_t)compensation;
#endif
    return 100;
}

/**
  * @brief  Returns relative light output in percent according to temperature.
  * @param  Type of led
  * @retval Compensation factor in percent
  */
uint16_t SST_90_W_GetCompensation(void)
{
#if 0
    int32_t compensation;
    int32_t junction_temperature;

    compensation = 100;

    if (!SST_90_W_GetJunctionTemperature(&junction_temperature))
    {
        junction_temperature = SST_90_W_DEFAULT_JUNCTION_TEMPERATURE;
    }

    // Temperature compensation:
    //
    //             1
    // p% = ------------------ * 100
    //      -0,00184 * T + 1,046
    int32_t divisor;
    divisor = (104600L - (184 * junction_temperature));
    if (divisor == 0)
        divisor = 1;  // avoid div 0, should not happen
    compensation = 10000000L / divisor;

    // Aging compensation:
    //
    //             1
    // p% = ------------------ * 100
    //      -0,00000149 * (time in hours) + 1,00002
    uint32_t time;
    time = TIME_METER_GetLedCompensatedOverallOperatingTime(DIM_LED_WHITE);
    time /= 3600;  // s --> h
    divisor = 100000200L - (149 * time);
    if (divisor == 0)
        divisor = 1;  // avoid div 0, should not happen

    uint32_t aging;
    aging = 10000000000L / divisor;
    if (aging < 100)
        aging = 100;

    compensation *= aging;  // add factor for aging compensation
    compensation /= 100;

    if (compensation > UINT16_T_MAX)
        compensation = UINT16_T_MAX;

    if (compensation < UINT16_T_MIN)
        compensation = UINT16_T_MIN;

    return (uint16_t)compensation;
#endif
    return 100;
}
