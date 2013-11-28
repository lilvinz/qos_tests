/*
 * sensors.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 29.04.2010
 *      Author: DHA
 *
 * Purpose:
 * Collect sensor values
 *
 */

#include "stdheader.h"
#include "sensors.h"

#include "config.h"

#include "modbus_rtu.h"
#include "modbus_master.h"
#include "gloom_manager.h"
#include "clock.h"
#include "i2c.h"
#include "adc.h"
#include "log.h"
#include "settings.h"
#include "visibility.h"
#include "hw_configuration.h"
#include "watchdog.h"
#include "statistics.h"
#include "voltcraft_lx_1108.h"
#include "measureport.h"
#include "primestruct.h"

// terminal
static bool bRS485PORT2TerminalMode = FALSE;

// gloom
static bool b_gloom_measurement_enabled = FALSE;
static bool b_gloom_measurement_state_changed = FALSE;
static GLOOM_SENSOR_T      gloom;
static SENSORS_GLOOM_EMA_T gloom_ema;

// temperature
static struct
{
    TEMPERATURE_SENSOR_T temperature[TEMPERATURE_COUNT];
    uint8_t err_count[TEMPERATURE_COUNT];
    STATE_T lm75_sensor_state;
    LOG_DUPVAR_T log_dupvar[TEMPERATURE_COUNT];
} temperature_ds;

static TEMPERATURE_SENSOR_T    avg_led_temperature;

// visibility
static bool b_visibility_measurement_enabled;
static uint32_t timestamp_visibility_start_next_measurement;

void SENSORS_Set_Temperature(TEMPERATURE_T sensor, const TEMPERATURE_SENSOR_T *p_data);
void SENSORS_InitRS485Port2(SETTINGS_RS485_PORT2_FUNCTION_T function);


/**
  * @brief  Initializes gloom sensor.
  * @param  None
  * @retval None
  */
void SENSORS_Init_Gloom_Sensor(GLOOM_DATA_STRUCT_T *p_gloom_ds)
{
    p_gloom_ds->sensor_state            = GLOOM_INIT;
    b_gloom_measurement_enabled         = FALSE;  // disable gloom measurement
    b_gloom_measurement_state_changed   = FALSE;  // gloom measurement state changed
    p_gloom_ds->b_start                 = TRUE;
    p_gloom_ds->last                    = 0;
    p_gloom_ds->acq_err_count           = 0;
    p_gloom_ds->err_retry_count         = 0;
    p_gloom_ds->acq_value_stuck_count   = 0;

    gloom_ema.value_ema             = 0;
    gloom_ema.ema_last_update       = 0;
    gloom_ema.i_accepted_value_ctr  = 0;

    gloom.state                = SENSOR_WAIT_FOR_INIT;
    gloom.value                = 0;
}

/**
  * @brief  Initializes temperature sensor i2c.
  * @param  None
  * @retval None
  */
void SENSORS_Init_Tempsensor_I2C(void)
{
    state_init(&temperature_ds.lm75_sensor_state);
    temperature_ds.err_count[TEMPERATURE_MAIN] = 0;

    TEMPERATURE_SENSOR_T        temperature_temp;
    temperature_temp.value     = TEMPERATURE_INITIAL_VALUE;
    temperature_temp.state     = SENSOR_WAIT_FOR_INIT;

    SENSORS_Set_Temperature(TEMPERATURE_MAIN,  &temperature_temp);
}

/**
  * @brief  Calls init function of I2C driver.
  * @param  None
  * @retval None
  */
void SENSORS_Init_I2C(void)
{
    // I2C1 configuration
    I2C_InitTypeDef  I2C_InitStructure;
    I2C_StructInit(&I2C_InitStructure);

    I2C_FlushBus(I2C_MASTER_BUS_1);

    I2C_InitStructure.I2C_Mode                  = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle             = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1           = I2C1_SLAVE_ADDRESS7;
    I2C_InitStructure.I2C_Ack                   = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress   = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed            = I2C_BUS_ClockSpeed;

    I2C_InitSystem(I2C_MASTER_BUS_1, &I2C_InitStructure, DISABLE);  // initializes i2c. This is needed for gloom and temperature sensors.
}

/**
  * @brief  Calls all needed init functions to initialize the complete sensor system.
  * @param  None
  * @retval None
  */
void SENSORS_Init(GLOOM_DATA_STRUCT_T *p_gloom_ds, uint8_t *p_i2c1_error_ctr)
{
    // ATTENTION: LUXVCC is used for I2C!
    // LUXVCC must be enabled to use I2C!
    GLOOM_SENSOR_POWER_ON();

    vTaskDelay(100);

    *p_i2c1_error_ctr = 0;

    SENSORS_Init_Gloom_Sensor(p_gloom_ds);
    SENSORS_Init_Tempsensor_I2C();

    TEMPERATURE_SENSOR_T        temperature_temp;
    temperature_temp.value     = TEMPERATURE_INITIAL_VALUE;
    temperature_temp.state     = SENSOR_WAIT_FOR_INIT;

    for (TEMPERATURE_T idx = TEMPERATURE_MAIN; idx < TEMPERATURE_COUNT; idx++)
    {
        SENSORS_Set_Temperature(idx, &temperature_temp);
    }

    avg_led_temperature.value = TEMPERATURE_INITIAL_VALUE;
    avg_led_temperature.state = SENSOR_WAIT_FOR_INIT;

    b_visibility_measurement_enabled = FALSE;

    SENSORS_Init_I2C();  // initializes i2c. This is needed for gloom and temperature sensors.
    VISIBILITY_Init();
    timestamp_visibility_start_next_measurement = CLOCK_GetTimestamp() + VISI_STATE_TIMEOUT_INIT;
}


// ****************************************
// ******** GETTERS and SETTERS ***********
// ****************************************

// GLOOM

/**
  * @brief  Function to enable gloom measurement.
  * @param  None
  * @retval None
  */
void SENSORS_EnableGloomMeasurement(void)
{
    // gloom measurement should only be enabled if leds are off!
    if (!b_gloom_measurement_enabled)
        b_gloom_measurement_state_changed = TRUE;  // set flag only if state is changed
    b_gloom_measurement_enabled = TRUE;  // enable gloom measurement
}


/**
  * @brief  Function to disable gloom measurement.
  * @param  None
  * @retval None
  */
void SENSORS_DisableGloomMeasurement(void)
{
    if (b_gloom_measurement_enabled)
        b_gloom_measurement_state_changed = TRUE;  // set flag only if state is changed
    b_gloom_measurement_enabled = FALSE;  // disable gloom measurement
}

void SENSORS_GetGloom(GLOOM_SENSOR_T *p_data)
{
    p_data->state = gloom.state;
    p_data->value = gloom.value;
}

float SENSORS_GetGloomEMA(void)
{
    return gloom_ema.value_ema;
}

// TEMPERATURE
void SENSORS_GetTemperature(TEMPERATURE_T sensor, TEMPERATURE_SENSOR_T *p_data)
{
    if (sensor < TEMPERATURE_COUNT)
    {
        p_data->state = temperature_ds.temperature[sensor].state;
        p_data->value = temperature_ds.temperature[sensor].value;
    }
    else
    {
        // error
        p_data->state = SENSOR_NOT_AVAILABLE;
        p_data->value = 0;
    }
}

void SENSORS_Set_Temperature(TEMPERATURE_T sensor, const TEMPERATURE_SENSOR_T *p_data)
{
    if (sensor < TEMPERATURE_COUNT)
    {
        temperature_ds.temperature[sensor].value = p_data->value;
        temperature_ds.temperature[sensor].state = p_data->state;
    }
}

void SENSORS_GetAvgLedTemperature(TEMPERATURE_SENSOR_T *p_data)
{
    p_data->state = avg_led_temperature.state;
    p_data->value = avg_led_temperature.value;
}

// VISIBILITY
/**
  * @brief  Function to enable visibility measurement.
  * @param  None
  * @retval None
  */
void SENSORS_Enable_Visibility_Measurement(void)
{
    VISIBILITY_Init();
    timestamp_visibility_start_next_measurement = CLOCK_GetTimestamp() + VISI_STATE_TIMEOUT_INIT;
    b_visibility_measurement_enabled = TRUE;  // enable visibility measurement
}


/**
  * @brief  Function to disable visibility measurement.
  * @param  None
  * @retval None
  */
void SENSORS_Disable_Visibility_Measurement(void)
{
    b_visibility_measurement_enabled = FALSE;  // disable visibility measurement
    VISIBILITY_DeInit();
}

void SENSORS_Calculate_Local_Gloom_EMA(GLOOM_DATA_STRUCT_T *p_gloom_ds)
{
    static uint8_t      i_value_out_of_tolerance_ctr = 0;
    static uint16_t     i_act_lowest_gloom_value = 0;
    //if we are just powered up and get the first value, use it directly
    if (p_gloom_ds->b_start)
    {
        p_gloom_ds->b_start       = FALSE;
        gloom_ema.value_ema = gloom.value;
    }
    else
    {
        // three different ways to calculate the ema (at least for development)
        // the first way is to accept all values
        // the second way is to accept only values in the near of the actual average
        // if a value is to far away the value is not considered for the average
        // to avoid errors caused by ignoring too many values an error counter is implemented
        // and the ignored values are used to calculate a second average value.
        // If the error counter reaches a limit, the average value will be set to the actual value
        // and the error counter will be cleared
        // the third way is to use the lowest value in a given time period

        switch (GLOOM_GLOOM_AVG_CALC_METHOD)
        {
        case GLOOM_SIMPLE_EMA:
        {
            // calculate the ema by this formula
            // x = a*y + (1-a)*x   x=avrg, a=factor, y=newval

            float difference = gloom.value - gloom_ema.value_ema;
            // check if the absolute value of difference is not to small for following operations
            if (((difference > 0) && (difference < GLOOM_EMA_MIN_DIFFERENCE)) || ((difference < 0) && (difference > (-GLOOM_EMA_MIN_DIFFERENCE))))
            {
                // the absolute value is to small for normal calculation -> add or subtract 1
                if (difference > 0)
                    gloom_ema.value_ema++;  // add 1
                else
                    gloom_ema.value_ema--;  // subtract 1
            }
            else
            {
                // difference is big enough -> normal calculation
                difference *= GLOOM_EMA_MULTIPLIER;
                difference /= GLOOM_EMA_DIVISOR;
                gloom_ema.value_ema += difference;
            }
            gloom_ema.ema_last_update  = CLOCK_GetTimestamp();
            break;
        }
        case GLOOM_EMA_WITHOUT_RUNAWAY_VALUES:
        {
            // ignore runaway values
            uint32_t tolerance;
            uint16_t min_value;
            uint16_t max_value;

            tolerance = (uint32_t)gloom_ema.value_ema * GLOOM_EMA_TOLERANCE_FACTOR;
            tolerance /= 100;

            if (tolerance < GLOOM_MIN_GLOOM_TOLERANCE)
            {
                // avoid too small values for tolerance
                min_value = 0;
                if ((UINT16_T_MAX - GLOOM_MIN_GLOOM_TOLERANCE) >= gloom_ema.value_ema)
                    max_value = (uint16_t)gloom_ema.value_ema + GLOOM_MIN_GLOOM_TOLERANCE;
                else
                    max_value = UINT16_T_MAX;  // avoid overflow
            }
            else
            {
                if (gloom_ema.value_ema > tolerance)
                    min_value = (uint16_t)gloom_ema.value_ema - tolerance;
                else
                    min_value = 0;  // avoid underflow
                if ((UINT16_T_MAX - tolerance) >= gloom_ema.value_ema)
                    max_value = (uint16_t)gloom_ema.value_ema + tolerance;
                else
                    max_value = UINT16_T_MAX;
            }
            if (((gloom.value >= min_value) && (gloom.value <= max_value)) ||
                    (gloom_ema.i_accepted_value_ctr < GLOOM_MIN_NUM_VALUES_FOR_FILTERING))
            {
                // value is within tolerance --> calculate ema
                float difference;
                difference = gloom.value - gloom_ema.value_ema;
                // check if the absolute value of difference is not to small for following operations
                if (((difference > 0) && (difference < GLOOM_EMA_MIN_DIFFERENCE)) || ((difference < 0) && (difference > (-GLOOM_EMA_MIN_DIFFERENCE))))
                {
                    // the absolute value is to small for normal calculation -> add or subtract 1
                    if (difference > 0)
                        gloom_ema.value_ema++;  // add 1
                    else
                        gloom_ema.value_ema--;  // subtract 1
                }
                else
                {
                    // difference is big enough -> normal calculation
                    difference *= GLOOM_EMA_MULTIPLIER;
                    difference /= GLOOM_EMA_DIVISOR;
                    gloom_ema.value_ema += difference;
                }
                gloom_ema.ema_last_update  = CLOCK_GetTimestamp();

                if (gloom_ema.i_accepted_value_ctr < UINT16_T_MAX)
                    gloom_ema.i_accepted_value_ctr++;

                if (i_value_out_of_tolerance_ctr > 0)
                    i_value_out_of_tolerance_ctr--;
            }
            else
            {
                // value is out of tolerance --> ignore value, increment error counter
                i_value_out_of_tolerance_ctr++;
                if (i_value_out_of_tolerance_ctr > GLOOM_TOLERANCE_COUNTER_MAX)
                {
                    // too many values out of tolerance --> ema doesn't represent measured values any more
                    // set the ema to 0
                    gloom_ema.value_ema = gloom.value;
                    gloom_ema.ema_last_update  = CLOCK_GetTimestamp();
                    gloom_ema.i_accepted_value_ctr = 0;
                    GLOOM_MANAGER_SetReliability(SENSOR_RUNAWAY_VALUE);
                    i_value_out_of_tolerance_ctr = 0;
                }
            }
            break;
        }
        case GLOOM_LOWEST_VALUE_IN_PERIOD:
        {
            if ((gloom.value < i_act_lowest_gloom_value) || (i_act_lowest_gloom_value == 0))
                i_act_lowest_gloom_value = (uint16_t)gloom.value;
            if (CLOCK_GetTimestamp() > gloom_ema.ema_last_update + GLOOM_LENGTH_OF_PERIOD_FOR_LOWEST_VALUE_SEC)
            {
                // update "ema" value
                gloom_ema.value_ema       = i_act_lowest_gloom_value;
                gloom_ema.ema_last_update = CLOCK_GetTimestamp();
                i_act_lowest_gloom_value  = 0;
            }
            break;
        }
        }
    }
}

/**
  * @brief  Function to use gloom sensor.
  * @param  None
  * @retval None
  */
SENSORS_GLOOM_RESULT_E SENSORS_Gloom(GLOOM_DATA_STRUCT_T *p_gloom_ds, volatile uint8_t *p_i2c1_error_ctr)
{
    static LOG_DUPVAR_T log_igndup = 0;
    static uint8_t waitcnt = 0;         // todo: SENSORS_Gloom call interval is currently 1s thus waitcnt represents seconds. keep this in mind if call changes.

    // todo[medium] fix error handling, change waiting (do not use vTaskDelay)
    GLOOM_SENSOR_T local_gloom;

    switch(p_gloom_ds->sensor_state)
    {
    case GLOOM_FAILED:
    {
        GLOOM_SENSOR_POWER_OFF();

        *p_i2c1_error_ctr = *p_i2c1_error_ctr + 1;  // increment error counter to detect i2c bus error
        gloom.state = SENSOR_FAILED;
        p_gloom_ds->acq_err_count = 0;

        LOG(LOG_ID_GLOOMSENSOR_STATUS_FAILED, 0, 0, 0, &log_igndup);

        if (++waitcnt >= 5)
        {
            waitcnt = 0;
            p_gloom_ds->sensor_state = GLOOM_INIT;
        }

    }
    break;
    case GLOOM_INIT:
    {
        GLOOM_SENSOR_POWER_ON();
        vTaskDelay(500);
        if (!GLOOM_InitSensor())
        {
            p_gloom_ds->acq_err_count++;
            break;
        }
        vTaskDelay(1000);       // simply wait some time
        p_gloom_ds->sensor_state = GLOOM_ACQUIRE;
    }
    break;
    case GLOOM_ACQUIRE:
    {
        GLOOM_RESULT_T temp_result;
        temp_result = GLOOM_Acquire(&local_gloom);
        if (temp_result != GLOOM_RESULT_SUCCESS)
        {
            if (temp_result != GLOOM_RESULT_SWITCHED_RANGE)
                p_gloom_ds->acq_err_count++;
        }
        else
        {
            // new value received
            *p_i2c1_error_ctr = 0;  // reset error counter if a new value was received
            // if gloom state on/off changed drop value cause it is not reliable
            if (b_gloom_measurement_state_changed == TRUE)
            {
                b_gloom_measurement_state_changed = FALSE;
                return SENSORS_GLOOM_RESULT_CANCELED;
            }
            // test if leds are still off
            if (!b_gloom_measurement_enabled)
                return SENSORS_GLOOM_RESULT_CANCELED;

            if (p_gloom_ds->last == local_gloom.value)
                p_gloom_ds->acq_value_stuck_count++;
            else
            {
                p_gloom_ds->acq_value_stuck_count = 0;
                p_gloom_ds->err_retry_count = 0;
            }

            p_gloom_ds->last = (uint16_t)local_gloom.value;

            if (local_gloom.value < UINT16_T_MAX)
            {
                local_gloom.state = SENSOR_OK;
                LOG(LOG_ID_GLOOMSENSOR_STATUS_OK, 0, 0, 0, &log_igndup);
            }

            gloom.value = local_gloom.value;
            gloom.state = local_gloom.state;

            SENSORS_Calculate_Local_Gloom_EMA(p_gloom_ds);
        }
    }
    break;
    default:
    {
        p_gloom_ds->sensor_state = GLOOM_FAILED;
        return SENSORS_GLOOM_RESULT_OK;  // todo[medium] change to support error handling
    }
    }

    if (p_gloom_ds->acq_value_stuck_count >= SENSOR_STUCK_COUNT_MAX)
    {
        p_gloom_ds->err_retry_count++;
        p_gloom_ds->acq_value_stuck_count = 0;
        p_gloom_ds->sensor_state = GLOOM_INIT;
    }
    if (p_gloom_ds->acq_err_count > 0)
    {
        p_gloom_ds->err_retry_count++;
        p_gloom_ds->acq_err_count = 0;
        p_gloom_ds->sensor_state = GLOOM_INIT;
    }
    if (p_gloom_ds->err_retry_count >= GLOOM_ERR_COUNT_MAX)
    {
        p_gloom_ds->err_retry_count = 0;
        p_gloom_ds->sensor_state = GLOOM_FAILED;
    }

    return SENSORS_GLOOM_RESULT_OK;
}


/**
  * @brief  Function to use temperature sensor (LM75).
  * @param  *p_i2c1_error_ctr: pointer to error counter
  * @retval None
  */
void SENSORS_Tempsensor(volatile uint8_t *p_i2c1_error_ctr)
{
    switch(temperature_ds.lm75_sensor_state.current)
    {
    case LM75_FAILED:
    {
        if (state_is_new(&temperature_ds.lm75_sensor_state))
        {
            *p_i2c1_error_ctr = *p_i2c1_error_ctr + 1;  // increment error counter to detect i2c bus error
            TEMPERATURE_SENSOR_T temperature_temp;
            temperature_temp.value = INT16_T_MAX;
            temperature_temp.state = SENSOR_FAILED;
            SENSORS_Set_Temperature(TEMPERATURE_MAIN, &temperature_temp);
            LOG(LOG_ID_TEMPSENSOR_STATUS_FAILED, TEMPERATURE_MAIN, 0, 0, &temperature_ds.log_dupvar[TEMPERATURE_MAIN]);
            temperature_ds.err_count[TEMPERATURE_MAIN] = 0;
        }
        if (!state_timeout(&temperature_ds.lm75_sensor_state, LM75_FAILED_STATE_TIMEOUT))
            return;
        temperature_ds.lm75_sensor_state.current = LM75_INIT;
    }
    break;
    case LM75_INIT:
    {
        if (!LM75_InitSensor())
        {
            temperature_ds.err_count[TEMPERATURE_MAIN]++;
            break;
        }
        temperature_ds.temperature[TEMPERATURE_MAIN].value = INT16_T_MAX;
        temperature_ds.err_count[TEMPERATURE_MAIN] = 0;
        state_set(&temperature_ds.lm75_sensor_state, LM75_ACQUIRE);
    }
    break;
    case LM75_ACQUIRE:
    {
        float value;
        if (!LM75_Acquire(&value))
        {
            temperature_ds.err_count[TEMPERATURE_MAIN]++;
            break;
        }
        TEMPERATURE_SENSOR_T temperature_temp;
        temperature_temp.value = value;
        temperature_temp.state = SENSOR_OK;
        SENSORS_Set_Temperature(TEMPERATURE_MAIN, &temperature_temp);
        STATISTICS_AutoSetValue(STATISTICS_SENSORS_TEMPERATURE_MAIN, value);
        LOG(LOG_ID_TEMPSENSOR_STATUS_OK, TEMPERATURE_MAIN, (uint32_t)temperature_temp.value, 0, &temperature_ds.log_dupvar[TEMPERATURE_MAIN]);
        *p_i2c1_error_ctr = 0;  // reset error counter if a new value was received
    }
    break;
    default:
    {
        temperature_ds.lm75_sensor_state.current = LM75_FAILED;
        break;
    }
    }

    if (temperature_ds.err_count[TEMPERATURE_MAIN] > LM75_ERR_COUNT_MAX)
        state_set(&temperature_ds.lm75_sensor_state, LM75_FAILED);
}


/**
  * @brief  Function to calculate temperature of all led temperature sensors.
  * @param  None
  * @retval None
  */
void SENSORS_LED_Tempsensor(void)
{
    float temp = 0.0f;
    float max = -1000.0f;
    float min = +1000.0f;
    float sum = 0.0f;
    uint8_t num = 0;

    for (TEMPERATURE_T idx = TEMPERATURE_LED_FIRST; idx <= TEMPERATURE_LED_LAST; idx++)
    {
        temp = _primestruct.sensors.led_temperature[idx - 1].value;  // todo[medium] change, we are using two different enums here!
        if (temp > TEMPERATURE_SENSOR_MIN_VALID_VALUE && TEMPERATURE_SENSOR_MAX_VALID_VALUE > temp)
        {
            // calculate the ema by this formula
            // x = a*y + (1-a)*x   x=avrg, a=factor, y=newval

#define TEMP_EMA_FACTOR 0.10f
            if (temperature_ds.temperature[idx].state == SENSOR_OK)
                temperature_ds.temperature[idx].value = TEMP_EMA_FACTOR * temp + temperature_ds.temperature[idx].value - TEMP_EMA_FACTOR * temperature_ds.temperature[idx].value;
            else
                temperature_ds.temperature[idx].value = temp;

            temperature_ds.temperature[idx].state = SENSOR_OK;
            if (temperature_ds.temperature[idx].value < min)
                min = temperature_ds.temperature[idx].value;
            if (temperature_ds.temperature[idx].value > max)
                max = temperature_ds.temperature[idx].value;
            sum += temperature_ds.temperature[idx].value;
            num++;

            if (temperature_ds.err_count[idx] > 0)
                temperature_ds.err_count[idx]--;
            else
                LOG(LOG_ID_TEMPSENSOR_STATUS_OK, idx, temp, 0, &temperature_ds.log_dupvar[idx]);
        }
        else
        {
            if (temperature_ds.err_count[idx] < SENSORS_TEMPSENSOR_MAX_ERRORS)
                temperature_ds.err_count[idx]++;
            else
            {
                temperature_ds.temperature[idx].state = SENSOR_FAILED;
                LOG(LOG_ID_TEMPSENSOR_STATUS_FAILED, idx, 0, 0, &temperature_ds.log_dupvar[idx]);
            }
        }
    }

    if (num > 0)
    {
        // avoid div 0
        sum = sum / num;

        avg_led_temperature.value = sum;
        avg_led_temperature.state = SENSOR_OK;
        STATISTICS_SetValue(STATISTICS_SENSORS_TEMPERATURE_LED, min, max, sum);
    }
    else
        avg_led_temperature.state = SENSOR_FAILED;
}

void SENSORS_I2C1_Reset(GLOOM_DATA_STRUCT_T *p_gloom_ds)
{
    bool b_gloom_measurement_enabled_old;
    b_gloom_measurement_enabled_old = b_gloom_measurement_enabled;
    SENSORS_Init_Gloom_Sensor(p_gloom_ds);
    SENSORS_Init_Tempsensor_I2C();
    SENSORS_Init_I2C();
    b_gloom_measurement_enabled = b_gloom_measurement_enabled_old;
    LOG(LOG_ID_SENSORS_RESET_I2C, 0, 0, 0, NULL);
}

void SENSORS_RS485Port2EnterTerminalMode(void)
{
    bRS485PORT2TerminalMode = TRUE;
    SENSORS_InitRS485Port2(SETTINGS_RS485_PORT2_FUNCTION_NONE);
}

void SENSORS_RS485Port2LeaveTerminalMode(void)
{
    bRS485PORT2TerminalMode = FALSE;
    SENSORS_InitRS485Port2(_settings_list_ram[SETTINGS_TERMINAL_RS485_PORT2_FUNCTION]);
}


void SENSORS_InitRS485Port2(SETTINGS_RS485_PORT2_FUNCTION_T function)
{
    VISIBILITY_DeInit();
    VOLTCRAFT_LX_1108_DeInit();
    MODBUS_RTU_DeInit();

    switch (function)
    {
    case SETTINGS_RS485_PORT2_FUNCTION_NONE:
        break;  // do nothing
    case SETTINGS_RS485_PORT2_FUNCTION_VISIBILITY:
        SENSORS_Enable_Visibility_Measurement();
        break;
    case SETTINGS_RS485_PORT2_FUNCTION_MODBUS:
        MODBUS_RTU_Init();  // modbus enabled
        break;
    case SETTINGS_RS485_PORT2_FUNCTION_LIGHT_METER_VOLTCRAFT_LX_1108:
        VOLTCRAFT_LX_1108_Init();
        break;
    case SETTINGS_RS485_PORT2_FUNCTION_GPS:
        break;  // todo [medium] implement
    case SETTINGS_RS485_PORT2_FUNCTION_MEASUREMENTS:
    case SETTINGS_RS485_PORT2_FUNCTION_SYNC:
        MEASUREPORT_Init();  // measureport enabled
        break;

    default:
        break;  // do nothing
    }
}

void SENSORS_Task(void *pvParameters)
{
    SETTINGS_RS485_PORT2_FUNCTION_T old_rs485_port2_function;

    CLOCK_MS_T gloom_acquire_timestamp = 0;
    CLOCK_MS_T temperature_acquire_timestamp = 0;

    GLOOM_DATA_STRUCT_T gloom_ds;
    uint8_t      i2c1_error_ctr;

    // init sensors
    SENSORS_Init(&gloom_ds, &i2c1_error_ctr);
    MODBUS_MASTER_Init();

    SENSORS_InitRS485Port2(_settings_list_ram[SETTINGS_TERMINAL_RS485_PORT2_FUNCTION]);
    old_rs485_port2_function = _settings_list_ram[SETTINGS_TERMINAL_RS485_PORT2_FUNCTION];

    while(1)
    {
        WATCHDOG_Heartbeat(WATCHDOG_TASK_ID_SENSORS);

        if (old_rs485_port2_function != _settings_list_ram[SETTINGS_TERMINAL_RS485_PORT2_FUNCTION])
        {
            // setting changed --> reinitialize RS485 port 2 function
            SENSORS_InitRS485Port2(_settings_list_ram[SETTINGS_TERMINAL_RS485_PORT2_FUNCTION]);
            old_rs485_port2_function = _settings_list_ram[SETTINGS_TERMINAL_RS485_PORT2_FUNCTION];
        }

        // check for i2c errors
        if (i2c1_error_ctr > SENSORS_I2C1_ERROR_CTR_MAX)
        {
            SENSORS_I2C1_Reset(&gloom_ds);  // to many errors -> reset i2c
            i2c1_error_ctr = 0;
        }

        // do not measure gloom if leds are on!
        if (b_gloom_measurement_enabled && (gloom_acquire_timestamp + GLOOM_ACQUIRE_INTERVAL) < CLOCK_GetMillitimestamp())
        {
            // todo [medium] change acquisition of gloom values, do not cause sensors task to wait within gloom module
            if (SENSORS_Gloom(&gloom_ds, &i2c1_error_ctr) == SENSORS_GLOOM_RESULT_OK)
                gloom_acquire_timestamp = CLOCK_GetMillitimestamp();  // reset timestamp only if measurement was not canceled
        }

        // temperature sensor
        if ((temperature_acquire_timestamp + TEMPERATURE_ACQUIRE_INTERVAL) < CLOCK_GetMillitimestamp())
        {
            temperature_acquire_timestamp = CLOCK_GetMillitimestamp();
            SENSORS_Tempsensor(&i2c1_error_ctr);
        }

        SENSORS_LED_Tempsensor();

        if (!bRS485PORT2TerminalMode)
        {
            switch (old_rs485_port2_function)
            {
            case SETTINGS_RS485_PORT2_FUNCTION_NONE:
                break;  // do nothing
            case SETTINGS_RS485_PORT2_FUNCTION_VISIBILITY:
            {
                if (CLOCK_GetTimestamp() > timestamp_visibility_start_next_measurement)
                {
                    if (SENSOR_VISIBILITY_SENSOR_TYPE != SETTINGS_VISIBILITY_SENSOR_TYPE_NONE)
                    {
                        // get new value
                        if (VISIBILITY_Acquire())
                            timestamp_visibility_start_next_measurement = CLOCK_GetTimestamp() + VISI_STATE_INTERVAL_ACQUIRE;
                        else
                            // error
                            timestamp_visibility_start_next_measurement = CLOCK_GetTimestamp() + VISI_STATE_TIMEOUT_FAILED;  // wait for 6 seconds
                    }
                }
                break;
            }
            case SETTINGS_RS485_PORT2_FUNCTION_MODBUS:
                MODBUS_MASTER_Tick();
                break;
            case SETTINGS_RS485_PORT2_FUNCTION_MEASUREMENTS:
            case SETTINGS_RS485_PORT2_FUNCTION_SYNC:
                MEASUREPORT_Tick();
                break;
            case SETTINGS_RS485_PORT2_FUNCTION_LIGHT_METER_VOLTCRAFT_LX_1108:
                VOLTCRAFT_LX_1108_Tick();
                break;  // do nothing
            case SETTINGS_RS485_PORT2_FUNCTION_GPS:
                break;  // todo [medium] implement
            default:
                break;  // do nothing
            }
        }

        vTaskDelay(SENSORS_TASK_DELAY_TIME);
    }
}
