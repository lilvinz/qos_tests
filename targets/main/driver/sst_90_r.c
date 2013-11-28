/*
 * sst_90_r.c
 *
 *  Created on: 14.03.2011
 *      Author: DHA
 */

#include "stdheader.h"
#include "sst_90_r.h"

#include "sensors.h"
#include "adc.h"
#include "dim.h"
#include "aguard.h"
#include "time_meter.h"

/**
  * @brief  Returns calculated junction temperature.
  * @param  NJunction temperature
  * @retval - true : ok - false: an error occurred
  */
bool SST_90_R_GetJunctionTemperature(int32_t *junction_temperature)
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

#if 0  // ToDo Prüfen, ob das so richtig ist und sinnvoll. Dann portieren auf neues primestruct
    // get led voltage to calculate power
    if (_bm_adc_led_available & (1 << ADC_LED_CH_INDEX_RED_C_A))
    {
        led_voltage_mV = (uint32_t)adc_values.values.led.array[ADC_LED_CH_INDEX_RED_C_A];
        led_voltage_mV_cnt++;
    }
    if (_bm_adc_led_available & (1 << ADC_LED_CH_INDEX_RED_C_B))
    {
        led_voltage_mV += (uint32_t)adc_values.values.led.array[ADC_LED_CH_INDEX_RED_C_B];
        led_voltage_mV_cnt++;
    }
#endif
    switch (led_voltage_mV_cnt)
    {
    case 1:
        break;  // only one voltage value available -> use it
    case 2:
        led_voltage_mV /= 2;
        break;  // two voltage values available
    default:
        led_voltage_mV = SST_90_R_LED_AVG_VOLTAGE;
    }

    led_power = led_voltage_mV * SST_90_R_LED_CURRENT / SST_90_R_NUMBER_LEDS_PER_CIRCUIT;  // power in uW (mV * mA)
    led_power /= 1000;  // uW --> mW
    led_power *= SST_90_R_LED_POWER_LOSS_FACTOR;  // consider only thermal power loss
    led_power /= 1000;
    // todo [medium] consider pwm and maybe on / off time of from beacon profile

    temperature = led_power * SST_90_R_RTHJB;
    temperature /= 100;  // 215 --> 2.15 K/W
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
uint16_t SST_90_R_GetTimemeterCompensation(void)
{
#if 0
    int32_t junction_temperature;

    if (!SST_90_R_GetJunctionTemperature(&junction_temperature))
        junction_temperature = SST_90_R_DEFAULT_JUNCTION_TEMPERATURE;

    if (junction_temperature < SST_90_R_TIMEMETER_COMPENSATION_MIN_TEMPERATURE)
        return SST_90_R_NO_COMPENSATION_FACTOR;  // no compensation for values below minimum, set 100 % as default compensation

    if (junction_temperature <= 70)
        return 100;
    else if (junction_temperature <= 75)
        return 150;
    else if (junction_temperature <= 80)
        return 200;
    else if (junction_temperature <= 85)
        return 250;
    else if (junction_temperature <= 90)
        return 400;
    else if (junction_temperature <= 95)
        return 560;
    else if (junction_temperature <= 100)
        return 760;
    else if (junction_temperature <= 105)
        return 1080;
    else if (junction_temperature <= 110)
        return 1460;
    else
        return 10000;  // todo [medium] think about values for high temperature maybe the factor should be limited to a value between 5 and 10
#endif
    return 100;
}

/**
  * @brief  Returns relative light output in percent according to temperature.
  * @param  Type of led
  * @retval Compensation factor in percent
  */
uint16_t SST_90_R_GetCompensation(void)
{
#if 0
    int32_t compensation;
    TEMPERATURE_SENSOR_T T;

    SENSORS_GetAvgLedTemperature(&T);
    if (T.state < SENSOR_OK)
    {
        // temperature sensors for case temperature failed, try sensor on main board
        SENSORS_GetTemperature(TEMPERATURE_MAIN, &T);
        if (T.state < SENSOR_OK)
            return SST_90_R_NO_COMPENSATION_FACTOR;  // sensor on main board failed too --> no compensation
    }

    if ((T.value > SENSORS_MAX_HEAT_SINK_TEMPERATURE) || (T.value < SENSORS_MIN_HEAT_SINK_TEMPERATURE))
        return SST_90_R_NO_COMPENSATION_FACTOR;  // error, value out of range

    compensation = SST_90_R_NO_COMPENSATION_FACTOR;

    // Temperature compensation:
    //
    //             1
    // p% = ------------------ * 100
    //      -0,008798 * T + 1,34962
    int32_t divisor;
    divisor = (int32_t)(1349620L - (8798 * T.value));
    if (divisor == 0)
        divisor = 1;  // avoid div 0, should not happen
    compensation = 100000000L / divisor;

    // Aging compensation:
    //
    //             1
    // p% = ------------------ * 100
    //      -0,00000324857 * (time in hours) + 0,9992
    uint32_t time;
    time = TIME_METER_GetLedCompensatedOverallOperatingTime(DIM_LED_WHITE);
    time /= 3600;  // s --> h
    divisor = 99920000L - (325 * time);
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
