/*
 * tempsensor.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 06.05.2010
 *      Author: DHA
 */

#include "stdheader.h"
#include "lm75.h"

#include "i2c.h"

// This is a driver for the LM75A Digital temperature sensor and thermal watchdog

bool lm75_i2c_set_wreg(uint8_t address, uint8_t reg, uint16_t word)
{
    uint8_t i2c_send_buffer[I2C_BUFFER_SIZE];

    i2c_send_buffer[0] = reg;
    i2c_send_buffer[1] = (uint8_t)(word >> 8);
    i2c_send_buffer[2] = (uint8_t)(word & 0xFF);

    if (I2C_MasterBufferWrite(I2C_MASTER_BUS_1, &i2c_send_buffer[0],  3, (address + TW_WRITE)) != I2C_RESULT_SUCCESS)
        return FALSE;

    return TRUE;
}

bool lm75_i2c_get_wreg(uint8_t address, uint16_t * dst, uint8_t reg)
{
    uint8_t i2c_send_buffer[I2C_BUFFER_SIZE];
    uint8_t i2c_receive_buffer[I2C_BUFFER_SIZE];

    i2c_send_buffer[0] = reg;

    if (I2C_MasterBufferWrite(I2C_MASTER_BUS_1, &i2c_send_buffer[0],  1, (address + TW_WRITE)) != I2C_RESULT_SUCCESS)
        return FALSE;

    if (I2C_MasterBufferRead(I2C_MASTER_BUS_1, &i2c_receive_buffer[0],  2, (address + TW_READ)) != I2C_RESULT_SUCCESS)
        return FALSE;

    *dst = (uint16_t)(((uint16_t)i2c_receive_buffer[0] << 8) & 0xFF00);
    *dst += (uint16_t)i2c_receive_buffer[1];
    return TRUE;
}


/**
  * @brief  Initializes the temperature sensor.
  * @param  None
  * @retval None
  */
bool LM75_InitSensor(void)
{
    if (!lm75_i2c_set_wreg(LM75_I2C_ADDRESS_SENSOR_1, LM75_I2C_REG_THYST, (0xFA << 7)))
    {
        return FALSE;
    }
    if (!lm75_i2c_set_wreg(LM75_I2C_ADDRESS_SENSOR_1, LM75_I2C_REG_TOS, (0xFA << 7)))
    {
        return FALSE;
    }
    return TRUE;
}


/**
  * @brief  Acquires value from sensor.
  * @param  None
  * @retval None
  */
bool LM75_Acquire(float *p_value)
{
    uint16_t    temp;

    if (!lm75_i2c_get_wreg(LM75_I2C_ADDRESS_SENSOR_1, &temp, LM75_I2C_REG_TEMP))
    {
        return FALSE;
    }
    else
    {
        int16_t temp_raw = 0;
        // received temperature value is 0.125degC per count, signed
        temp_raw = (int16_t)(temp & 0xFFE0);
        temp_raw >>= 5;
        *p_value = ((float) temp_raw) / 8;
        return TRUE;
    }
}
