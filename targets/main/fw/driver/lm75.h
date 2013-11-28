/*
 * tempsensor.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 06.05.2010
 *      Author: DHA
 */

#ifndef LM75_H
#define LM75_H

#include "sensor.h"

#define LM75_I2C_ADDRESS_SENSOR_1       0x90

#define LM75_I2C_REG_TEMP       0x00
#define LM75_I2C_REG_CONF       0x01
#define LM75_I2C_REG_THYST  0x02
#define LM75_I2C_REG_TOS        0x03

#define LM75_FAILED_STATE_TIMEOUT (10000)
#define LM75_ERR_COUNT_MAX  10

typedef enum tempsensor_states
{
    LM75_INIT,
    LM75_ACQUIRE,
    LM75_FAILED,
} LM75_STATES_T;

typedef enum tempsensor_acuire_result_e
{
    LM75_ACQUIRE_SUCCESS,
    LM75_ACQUIRE_ERROR,
    LM75_ACQUIRE_FAILED,
} LM75_ACQUIRE_RESULT_T;

bool LM75_InitSensor(void);
bool LM75_Acquire(float *p_value);

#endif
