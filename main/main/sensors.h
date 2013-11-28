/*
 * sensors.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 29.04.2010
 *      Author: DHA
 */

#ifndef SENSORS_H
#define SENSORS_H

#include "lm75.h"
#include "gloom.h"
#include "log.h"
#include "state.h"
#include "config.h"

#define SENSORS_I2C1_ERROR_CTR_MAX 10  // number of errors before i2c1 will be restarted
#define GLOOM_ACQUIRE_INTERVAL  1000
#define TEMPERATURE_ACQUIRE_INTERVAL 1000

// exponential moving average
#define GLOOM_EMA_UPDATE_TIME       1000
#define GLOOM_EMA_MULTIPLIER        (_settings_list_ram[SETTINGS_GLOOM_LOCAL_EMA_FACTOR])  //5       // multiplier / divisor 5 / 100 means 0.05
#define GLOOM_EMA_DIVISOR           100     //
#define GLOOM_EMA_MIN_DIFFERENCE    (GLOOM_EMA_DIVISOR / GLOOM_EMA_MULTIPLIER)  // attention: divisor % multiplier has to be 0 otherwise GLOOM_EMA_MIN_DIFFERENCE has to be incremented by one
#define SENSOR_VISIBILITY_SENSOR_TYPE _settings_list_ram[SETTINGS_VISIBILITY_SENSOR_TYPE]

#define GLOOM_EMA_TOLERANCE_FACTOR  50  // factor/100 -> 30 means 0.30
#define GLOOM_TOLERANCE_COUNTER_MAX 50
#define GLOOM_MIN_NUM_VALUES_FOR_FILTERING  20

// Settings
#define GLOOM_MIN_GLOOM_TOLERANCE           (_settings_list_ram[SETTINGS_ADVANCED_MIN_GLOOM_TOLERANCE])

// Measurement and calculation method selectors
#define GLOOM_GLOOM_AVG_CALC_METHOD (_settings_list_ram[SETTINGS_GLOOM_AVG_CALC_METHOD])

// gloom
typedef struct
{
    bool b_start;
    uint16_t last;
    uint8_t acq_err_count;
    uint8_t err_retry_count;
    uint8_t acq_value_stuck_count;
    GLOOM_STATE_T sensor_state;
} GLOOM_DATA_STRUCT_T;

// temperature
typedef enum
{
    TEMPERATURE_MAIN = 0,
#if defined(HWT_MULTI)
    TEMPERATURE_LED1,
    TEMPERATURE_LED2,
    TEMPERATURE_LED3,
    TEMPERATURE_LED4,
#endif
#if defined(HWT_MEDIUM_RED)
    TEMPERATURE_LED1,
#endif
#if defined(HWT_MEDIUM_WHITE)
    TEMPERATURE_LED1,
    TEMPERATURE_LED2,
#endif
    TEMPERATURE_COUNT
} TEMPERATURE_T;

#define TEMPERATURE_LED_FIRST   TEMPERATURE_LED1
#define TEMPERATURE_LED_LAST    (TEMPERATURE_COUNT - 1)
#define TEMPERATURE_LED_COUNT   (TEMPERATURE_COUNT - TEMPERATURE_LED1)

#define TEMPERATURE_SENSOR_MIN_VALID_VALUE                  (-55.0f)
#define TEMPERATURE_SENSOR_MAX_VALID_VALUE                  (150.0f)

#define TEMPERATURE_INITIAL_VALUE                           20.0f  // 20 degree C
#define SENSORS_TEMPSENSOR_MAX_ERRORS   40

#define SENSORS_TASK_DELAY_TIME 100

typedef struct sensors_gloom_ema_s
{
    float value_ema;  // average value
    uint32_t ema_last_update;
    uint32_t i_accepted_value_ctr;
} SENSORS_GLOOM_EMA_T;

typedef enum gloom_ema_calculation_modes_e
{
    GLOOM_SIMPLE_EMA,  // Every value is used to calculate the ema
    GLOOM_EMA_WITHOUT_RUNAWAY_VALUES,  // runaway values are ignored for the ema
    GLOOM_LOWEST_VALUE_IN_PERIOD // only the lowest value in a given period is used
} GLOOM_EMA_CALCULATION_MODES_T;

typedef enum
{
    SENSORS_GLOOM_RESULT_OK, // obtained value or error which is not passed to calling function (quick fix)
    SENSORS_GLOOM_RESULT_CANCELED  // measurement was canceled
} SENSORS_GLOOM_RESULT_E;

#define GLOOM_LENGTH_OF_PERIOD_FOR_LOWEST_VALUE_SEC 60 // period in s
#define SENSORS_MAX_HEAT_SINK_TEMPERATURE   100  // maximum temperature, used to verify values todo [medium] check value
#define SENSORS_MIN_HEAT_SINK_TEMPERATURE   (-50)  // minimum temperature, used to verify values todo [medium] check value
extern xSemaphoreHandle xGloomValueMutex;

void SENSORS_Task(void *pvParameters);
void SENSORS_EnableGloomMeasurement(void);
void SENSORS_DisableGloomMeasurement(void);
void SENSORS_GetGloom(GLOOM_SENSOR_T *p_data);
float SENSORS_GetGloomEMA(void);
void SENSORS_GetTemperature(TEMPERATURE_T sensor, TEMPERATURE_SENSOR_T *p_data);
void SENSORS_GetAvgLedTemperature(TEMPERATURE_SENSOR_T *p_data);
void SENSORS_RS485Port2EnterTerminalMode(void);
void SENSORS_RS485Port2LeaveTerminalMode(void);

#endif
