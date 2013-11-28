/*
 * heater.c
 *
 *  Created on: 08.04.2013
 *      Author: cba
 */
#include "stdheader.h"
#include "hw_configuration.h"
#include "settings.h"
#include "sensors.h"
#include "heater.h"

/******* Local Globals *******/
static HEATER_STATE_T heaterState;
static bool bHeaterOff;

/******* Local Functions ******/
static void heater_set(HEATER_STATE_T state);

/********* Implementation **********/
static void heater_set(HEATER_STATE_T state)
{
    if (state == HEATER_ON)
        GPIO_WriteBit(HEATER_PORT, HEATER_PIN, Bit_SET);
    else if (state == HEATER_OFF)
        GPIO_WriteBit(HEATER_PORT, HEATER_PIN, Bit_RESET);
}

HEATER_STATE_T HEATER_GetState(void)
{
    return heaterState;
}

void HEATER_Off(void)
{
    if (_settings_list_ram[SETTINGS_HEATER_MODE] == SETTINGS_HEATER_MODE_LED_INTERLEAVE)
    {
        bHeaterOff = TRUE;
        heater_set(HEATER_OFF);
    }
}

void HEATER_CouldBeSwitchedOn(void)
{
    bHeaterOff = FALSE;
}

void HEATER_Tick(void)
{
    // check if heater is switched on or off by settings
    // or in LED interleaved mode, the heater has to be switched off by dim if LEDs are on
    if ((_settings_list_ram[SETTINGS_HEATER_USE_HEATER] == FALSE)
            || (bHeaterOff && (_settings_list_ram[SETTINGS_HEATER_MODE] == SETTINGS_HEATER_MODE_LED_INTERLEAVE)))
    {
        // just in case: always set heater off
        heater_set(HEATER_OFF);
        return;
    }

    TEMPERATURE_SENSOR_T temperature;
    SENSORS_GetAvgLedTemperature(&temperature);

    // Check if we have a valid temperature
    if (temperature.state >= SENSOR_OK)
    {
        // check if heater has to be switched on or off
        if (temperature.value <= (int32_t) _settings_list_ram[SETTINGS_HEATER_ON_TEMPERATURE])
        {
            // switch heater on
            heaterState = HEATER_ON;
        }
        else if (temperature.value > (int32_t) _settings_list_ram[SETTINGS_HEATER_ON_TEMPERATURE] + _settings_list_ram[SETTINGS_HEATER_HYSTERESIS])
        {
            // switch heater off
            heaterState = HEATER_OFF;
        }
    }
    else
    {
        // Sensor failed and we do not know the current temperature: Switch heater off!
        heaterState = HEATER_OFF;
    }

    heater_set(heaterState);
}

void HEATER_Init(void)
{
    heater_set(HEATER_OFF);
    heaterState = HEATER_OFF;
    bHeaterOff = FALSE;
}
