/*
 * gloom.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 29.04.2010
 *      Author: DHA
 */

#include "stdheader.h"
#include "gloom.h"

#include "i2c.h"
#include "state.h"
#include "debug.h"

static bool b_range_high;


bool Gloom_i2c_set_reg(uint8_t reg, uint8_t byte)
{
    uint8_t i2c_send_buffer[I2C_BUFFER_SIZE];

    i2c_send_buffer[0] = reg;
    i2c_send_buffer[1] = byte;

    if (I2C_MasterBufferWrite(I2C_MASTER_BUS_1, &i2c_send_buffer[0],  2, (ISL29003_I2C_ADDRESS + TW_WRITE)) != I2C_RESULT_SUCCESS)
        return FALSE;

    return TRUE;
}

bool Gloom_i2c_get_wreg(uint16_t * dst, uint8_t reg)
{
    uint8_t i2c_send_buffer[I2C_BUFFER_SIZE];
    uint8_t i2c_receive_buffer[I2C_BUFFER_SIZE];

    i2c_send_buffer[0] = reg;

    if (I2C_MasterBufferWrite(I2C_MASTER_BUS_1, &i2c_send_buffer[0],  1, (ISL29003_I2C_ADDRESS + TW_WRITE)) != I2C_RESULT_SUCCESS)
        return FALSE;

    if (I2C_MasterBufferRead(I2C_MASTER_BUS_1, &i2c_receive_buffer[0],  2, (ISL29003_I2C_ADDRESS + TW_READ)) != I2C_RESULT_SUCCESS)
        return FALSE;

    *dst = (uint16_t)i2c_receive_buffer[0];
    *dst += (uint16_t)(((uint16_t)i2c_receive_buffer[1] << 8) & 0xFF00);
    return TRUE;
}

void GLOOM_ResetSensor(void)
{
    Gloom_i2c_set_reg(ISL29003_REG_COMMAND, ISL29003_COMMAND_DISABLE);

    //GLOOM_EXTI2CVCC_OFF;
}


/**
  * @brief  Initializes the gloom sensor.
  * @param  None
  * @retval None
  */
bool GLOOM_InitSensor(void)
{
    b_range_high = FALSE;

    if (!Gloom_i2c_set_reg(ISL29003_REG_COMMAND, ISL29003_COMMAND_DISABLE_ADC))
        return FALSE;
    if (!Gloom_i2c_set_reg(ISL29003_REG_COMMAND, ISL29003_COMMAND_SHUTDOWN))
        return FALSE;
    vTaskDelay(GLOOM_STATE_RESET_WAIT);
    if (!Gloom_i2c_set_reg(ISL29003_REG_CONTROL, ISL29003_CONTROL_SETUP))
        return FALSE;
    // switch on ADC
    if (!Gloom_i2c_set_reg(ISL29003_REG_COMMAND, ISL29003_COMMAND_ENABLE))
        return FALSE;
    return TRUE;
}


/**
  * @brief  Acquires value from sensor.
  * @param  None
  * @retval None
  */
GLOOM_RESULT_T GLOOM_Acquire(GLOOM_SENSOR_T *p_gloom)
{
    uint16_t        gloom_raw;      // temporary gloom raw value

    vTaskDelay(ISL29003_INTEGRATION_DURATION);
    if (Gloom_i2c_get_wreg(&gloom_raw, ISL29003_REG_SENS_LSB))
    {
        if (gloom_raw >= GLOOM_MAX_RAW_OUTPUT_VALUE)
        {
            if (b_range_high == TRUE)
            {
                p_gloom->state = SENSOR_UPPER_LIMIT;
            }
            else
            {
                Gloom_i2c_set_reg(ISL29003_REG_CONTROL, ISL29003_CONTROL_HR);
                b_range_high = TRUE;
                return GLOOM_RESULT_SWITCHED_RANGE;
            }
        }
        else if (gloom_raw <= 1)
        {
            if (b_range_high == FALSE)
            {
                p_gloom->state = SENSOR_LOWER_LIMIT;
            }
        }

        if ((gloom_raw <= GLOOM_HR2LR_SWITCH_THRESHOLD) && (b_range_high == TRUE))
        {
            Gloom_i2c_set_reg(ISL29003_REG_CONTROL, ISL29003_CONTROL_LR);
            b_range_high = FALSE;
            return GLOOM_RESULT_SWITCHED_RANGE;
        }
    }
    else
    {
        return GLOOM_RESULT_ERROR;  // an error occurred
    }
    if (b_range_high == TRUE)
    {
        p_gloom->value = (float)GLOOM_64K_TO_LUX(gloom_raw);
        //DPRINT2(DEBUG_GENERAL, "GLOOM Range: High  RawValue: %u  Result: %u\n", gloom_raw, (unsigned int)p_gloom->value);
    }
    else
    {
        p_gloom->value = (float)GLOOM_1K_TO_LUX(gloom_raw);
        //DPRINT2(DEBUG_GENERAL, "GLOOM Range: Low  RawValue: %u  Result: %u\n", gloom_raw, (unsigned int)p_gloom->value);
    }
    return GLOOM_RESULT_SUCCESS;
}
