/*
 * gloom.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 29.04.2010
 *      Author: DHA
 */

#ifndef GLOOM_H
#define GLOOM_H

#include "sensor.h"
#include "hw_configuration.h"

#define GLOOM_SENSOR_POWER_ON()  GPIO_WriteBit(LUXVCC_PORT, LUXVCC_PIN, Bit_SET)  // activate LUXVCC
#define GLOOM_SENSOR_POWER_OFF() GPIO_WriteBit(LUXVCC_PORT, LUXVCC_PIN, Bit_RESET)  // deactivate LUXVCC

#define GLOOM_64K_TO_LUX(x)     (((float)x * 62272.0f) / 65535)  // k = 4, R_ext = 100k => Range 0..62272
#define GLOOM_1K_TO_LUX(x)      (((float)x * 973.0f) / 65535)    // k = 1, R_ext = 100k => Range 0..973

#define GLOOM_MAX_RAW_OUTPUT_VALUE   65500
#define GLOOM_HR2LR_SWITCH_THRESHOLD ((800 * 65535) / 62272)

#define GLOOM_STATE_INIT_WAIT       (2000)
#define GLOOM_STATE_RESET_WAIT      (1000)
#define GLOOM_FAILED_STATE_TIMEOUT  (10000)
#define GLOOM_ERR_COUNT_LOG         5           // log error if count reached
#define GLOOM_ERR_COUNT_MAX         10
#define SENSOR_STUCK_COUNT_MAX      60
#define GLOOM_ERR_REBOOT_COUNT_MAX  10

#define ISL29003_INTEGRATION_DURATION (250)  // old 450, minimum for worst-case is 226

#define ISL29003_I2C_ADDRESS        0x88

#define ISL29003_REG_COMMAND        0x00
#define ISL29003_REG_CONTROL        0x01
#define ISL29003_REG_IRQTH_HI       0x02
#define ISL29003_REG_IRGTH_LO       0x03
#define ISL29003_REG_SENS_LSB       0x04
#define ISL29003_REG_SENS_MSB       0x05
#define ISL29003_REG_TIMER_LSB      0x06
#define ISL29003_REG_TIMER_MSB      0x07
// special registers which are triggered just by accessing them without payload
#define ISL29003_REG_SYNC_IIC       0x80
#define ISL29003_REG_CLEAR_INT      0x40

// register bit values
#define ISL29003_BIT_CMD_ADC_EN     0x80
#define ISL29003_BIT_CMD_ADC_PWRD   0x40
#define ISL29003_BIT_CMD_TIMING     0x20
#define ISL29003_BIT_CMD_MODE1      0x08
#define ISL29003_BIT_CMD_MODE0      0x04
#define ISL29003_BIT_CMD_WIDTH1     0x02
#define ISL29003_BIT_CMD_WIDTH0     0x01

#define ISL29003_CMD_MODE_D1        (0)
#define ISL29003_CMD_MODE_D2        (ISL29003_BIT_CMD_MODE0)
#define ISL29003_CMD_MODE_D1D2      (ISL29003_BIT_CMD_MODE1)

#define ISL29003_CMD_WIDTH_16BIT    (0)
#define ISL29003_CMD_WIDTH_12BIT    (ISL29003_BIT_CMD_WIDTH0)
#define ISL29003_CMD_WIDTH_8BIT     (ISL29003_BIT_CMD_WIDTH1)
#define ISL29003_CMD_WIDTH_4BIT     (ISL29003_BIT_CMD_WIDTH0 + ISL29003_BIT_CMD_WIDTH1)


//Command register value during setup sequence
#define ISL29003_COMMAND_SETUP      (ISL29003_CMD_WIDTH_16BIT + ISL29003_CMD_MODE_D1)
#define ISL29003_COMMAND_ENABLE     (ISL29003_COMMAND_SETUP + ISL29003_BIT_CMD_ADC_EN)
#define ISL29003_COMMAND_DISABLE    (ISL29003_COMMAND_SETUP)
#define ISL29003_COMMAND_DISABLE_ADC    (ISL29003_COMMAND_SETUP)
#define ISL29003_COMMAND_SHUTDOWN       (ISL29003_BIT_CMD_ADC_PWRD)


#define ISL29003_BIT_CTRL_INTFLAG   0x20
#define ISL29003_BIT_CTRL_GAIN1     0x08
#define ISL29003_BIT_CTRL_GAIN0     0x04
#define ISL29003_BIT_CTRL_INTPER1   0x02
#define ISL29003_BIT_CTRL_INTPER0   0x01

#define ISL29003_CTRL_RANGE_1K      (0)
#define ISL29003_CTRL_RANGE_4K      (ISL29003_BIT_CTRL_GAIN0)
#define ISL29003_CTRL_RANGE_16K     (ISL29003_BIT_CTRL_GAIN1)
#define ISL29003_CTRL_RANGE_64K     (ISL29003_BIT_CTRL_GAIN0 + ISL29003_BIT_CTRL_GAIN1)

//Control register value during setup sequence
#define ISL29003_CONTROL_SETUP      (ISL29003_CTRL_RANGE_1K)    // Range 1: 0..1000 lux
#define ISL29003_CONTROL_HR         (ISL29003_CTRL_RANGE_64K)   // Range 4: 0..64000 lux
#define ISL29003_CONTROL_LR         (ISL29003_CTRL_RANGE_1K)    // Range 1: 0..1000 lux

typedef enum gloom_states
{
    GLOOM_INIT,
    GLOOM_ACQUIRE,
    GLOOM_IDLE,
    GLOOM_FAILED,
} GLOOM_STATE_T;

typedef enum gloom_result_e
{
    GLOOM_RESULT_SUCCESS,
    GLOOM_RESULT_SWITCHED_RANGE,
    GLOOM_RESULT_ERROR,
    GLOOM_RESULT_FAILED,
} GLOOM_RESULT_T;


typedef struct gloom_sensor_s
{
    float        value;      // last valid value
    SENSOR_STATUS_T  state;
} GLOOM_SENSOR_T;


void GLOOM_Init(void);
void GLOOM_ResetSensor(void);
bool GLOOM_InitSensor(void);
GLOOM_RESULT_T GLOOM_Acquire(GLOOM_SENSOR_T *p_gloom);
#endif
