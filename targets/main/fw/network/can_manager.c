/*
 * can_manager.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 07.08.2012
 *      Author: DHA
 *
 * Purpose:
 * Handle can network communication
 *
 */

#include "stdheader.h"

#include "rtc.h"
#include "clock.h"
#include "log.h"
#include "watchdog.h"

#include "can_manager.h"
#include "can_cmd_i.h"
#include "primestruct.h"
#include "detector.h"

#include "debug.h"

typedef enum
{
    CAN_MANAGER_READ_SLAVE_STATE_READ_STATUS,
    CAN_MANAGER_READ_SLAVE_STATE_WAIT_STATUS,
    CAN_MANAGER_READ_SLAVE_STATE_READ_VALUE,
    CAN_MANAGER_READ_SLAVE_STATE_WAIT_VALUE,
    CAN_MANAGER_READ_SLAVE_STATE_COUNT
} CAN_MANAGER_READ_SLAVE_STATE_E;

static const uint32_t ps_base_address_list[CAN_MANAGER_ACCESS_GROUP_COUNT] =
{
    CAN_MANAGER_PS_BASE_ADDRESS_ANALOG_LED,
    CAN_MANAGER_PS_BASE_ADDRESS_ANALOG_VOLTAGE,
    CAN_MANAGER_PS_BASE_ADDRESS_ANALOG_LED_TEMPERATURE,
    CAN_MANAGER_PS_BASE_ADDRESS_ANALOG_BOARD_TEMPERATURE,
};

static const uint32_t ui_max_item_index[CAN_MANAGER_ACCESS_GROUP_COUNT] =
{
    CAN_MANAGER_NUM_LED_VOLTAGES_PER_SLAVE,
    CAN_MANAGER_NUM_SUPPLY_VOLTAGES_PER_SLAVE,
    CAN_MANAGER_NUM_LED_TEMPERATURE_SENSORS_PER_SLAVE,
    CAN_MANAGER_NUM_BOARD_TEMPERATURE_SENSORS_PER_SLAVE,
};

#ifdef HWP_CAN_MASTER
static uint32_t ui_can_error_ctr;  // todo[medium] use or remove
static uint32_t ui_slave_error_ctr[CAN_MANAGER_NUM_SUPPORTED_SLAVES];
static uint64_t ui_system_statistics_slave_error_ctr[CAN_MANAGER_NUM_SUPPORTED_SLAVES];
static CLOCK_S_T last_slave_rx_ts[CAN_MANAGER_NUM_SUPPORTED_SLAVES];
#endif

/**
  * @brief  Send new pwm to slaves.
  * @param  pwm: new value for pwm
  * @retval None
  */
void CAN_MANAGER_SetSlavePWM(uint32_t pwm)
{
#ifdef HWP_CAN_MASTER
    CAN_CMD_I_WriteSlaveMemory (0x00, CAN_MANAGER_SLAVE_ADDRESS_PWM, pwm);  // set pwm
#endif
}

/**
  * @brief  Start sequence on slaves.
  * @param  time_ms: time in ms for which the leds should be activated
  * @retval None
  */
void CAN_MANAGER_StartSlaveSequence(uint32_t time_ms)
{
#ifdef HWP_CAN_MASTER
    CAN_CMD_I_WriteSlaveMemory (0x00, CAN_MANAGER_SLAVE_ADDRESS_SEQUENCE_TIME, time_ms);  // set time
#endif
}

/**
  * @brief  Start bootloader of all or selected slave.
  * @param  slave: id of slave
  * @retval None
  */
void CAN_MANAGER_StartSlaveBootloader(CAN_MANAGER_SLAVE_ID_E slave)
{
#ifdef HWP_CAN_MASTER
    CAN_CMD_I_SendCommand(slave, CAN_CMD_I_COMMAND_QN_RESTART_SLAVE);
#endif
}

/**
  * @brief  Copies the data into the struct.
  * @param  None
  * @retval None
  */
void CAN_MANAGER_PutSensorDataIntoPrimestruct(uint8_t slave_id, CAN_MANAGER_ACCESS_GROUP_E group, uint32_t item_index, PS_SENSOR_T data)
{
#ifdef HWP_CAN_MASTER
    uint32_t idx;
    switch (group)
    {
    case CAN_MANAGER_ACCESS_GROUP_PS_ANALOG_LED:
        idx = PS_ANALOG_LED_WHITE_1;  // start offset
        idx += (slave_id - 1) * CAN_MANAGER_NUM_LED_VOLTAGES_PER_SLAVE;  // slave offset
        idx += item_index;  // led index
        _primestruct.sensors.analog.led[idx].value = data.value;
        _primestruct.sensors.analog.led[idx].status = data.status;
        break;
    case CAN_MANAGER_ACCESS_GROUP_PS_ANALOG_VOLTAGE:
        idx = PS_ANALOG_VOLTAGE_SLAVE_1_24V;  // start offset
        idx += (slave_id - 1) * CAN_MANAGER_NUM_SUPPLY_VOLTAGES_PER_SLAVE;  // slave offset
        idx += item_index;  // led index
        _primestruct.sensors.analog.supply[idx].value = data.value;
        _primestruct.sensors.analog.supply[idx].status = data.status;
        break;
    case CAN_MANAGER_ACCESS_GROUP_PS_ANALOG_LED_TEMPERATURE:
        idx = PS_LED_TEMPERATURE_WHITE_1;  // start offset
        idx += (slave_id - 1) * CAN_MANAGER_NUM_LED_TEMPERATURE_SENSORS_PER_SLAVE;  // slave offset
        idx += item_index;  // led index
        _primestruct.sensors.led_temperature[idx].value = data.value;
        _primestruct.sensors.led_temperature[idx].status = data.status;
        break;
    case CAN_MANAGER_ACCESS_GROUP_PS_ANALOG_BOARD_TEMPERATURE:
        idx = PS_BOARD_TEMPERATURE_SLAVE_1;  // start offset
        idx += (slave_id - 1) * CAN_MANAGER_NUM_BOARD_TEMPERATURE_SENSORS_PER_SLAVE;  // slave offset
        idx += item_index;  // led index
        _primestruct.sensors.board_temperature[idx].value = data.value;
        _primestruct.sensors.board_temperature[idx].status = data.status;
        break;
    default:
        break;
    }
#endif
}

/**
  * @brief  Read data from slaves.
  * @param  None
  * @retval bool:   - true  : received value from slave
  *                 - false : waiting or an error occurred
  */
void CAN_MANAGER_CollectSlaveDataTick(void)
{
#ifdef HWP_CAN_MASTER
    static CAN_MANAGER_ACCESS_GROUP_E group_id = 0;
    static CAN_MANAGER_READ_SLAVE_STATE_E read_state = CAN_MANAGER_READ_SLAVE_STATE_READ_STATUS;  // read status first
    static uint8_t slave_id = CAN_MANAGER_FIRST_SLAVE_ID;
    static uint32_t item_index = 0;
    static CAN_CMD_I_RESULT_T notifier;
    static PS_SENSOR_T data;

    if ((read_state == CAN_MANAGER_READ_SLAVE_STATE_READ_STATUS) || (read_state == CAN_MANAGER_READ_SLAVE_STATE_READ_VALUE))
    {
        uint32_t read_address;
        uint32_t *p_data;
        read_address = ps_base_address_list[group_id];
        read_address += item_index * CAN_MANAGER_SLAVE_ADC_SENSORS_ENTRY_SIZE;  // add item offset

        if (read_state == CAN_MANAGER_READ_SLAVE_STATE_READ_STATUS)
        {
            read_address += CAN_MANAGER_SLAVE_ADC_SENSORS_ENTRY_ELEMENT_SIZE;  // add element offset
            p_data = (uint32_t*)&data.status;
        }
        else
            p_data = (uint32_t*)&data.value;

        CAN_CMD_I_RESULT_T can_result;
        can_result = CAN_CMD_I_ReadSlaveMemoryNonBlocking (slave_id, read_address, p_data, &notifier);
        if (can_result == CAN_CMD_I_RESULT_OK)
        {
            read_state++;
            if (ui_can_error_ctr > 0)
                ui_can_error_ctr--;  // decrement error counter
        }
        else if (can_result == CAN_CMD_I_RESULT_ERROR)
        {
            if (ui_can_error_ctr < CAN_MANAGER_MAX_CAN_ERRORS)
                ui_can_error_ctr++;  // increment error counter
        }
    }
    else
    {
        switch (notifier)
        {
        case CAN_CMD_I_RESULT_OK:
            if (read_state == CAN_MANAGER_READ_SLAVE_STATE_WAIT_STATUS)
                read_state++;  // read value after status
            else
            {
                // copy data to prime_struct
                CAN_MANAGER_PutSensorDataIntoPrimestruct(slave_id, group_id, item_index, data);

                last_slave_rx_ts[slave_id - 1] = CLOCK_GetTimestamp();

                if (ui_slave_error_ctr[slave_id - 1] > 0)
                    ui_slave_error_ctr[slave_id - 1]--;

                read_state = CAN_MANAGER_READ_SLAVE_STATE_READ_STATUS;  // read status first
                item_index++;
                if (item_index >= ui_max_item_index[group_id])
                {
                    item_index = 0;
                    group_id++;
                    if (group_id >= CAN_MANAGER_ACCESS_GROUP_COUNT)
                    {
                        group_id = 0;
                        slave_id++;
                        if (slave_id > CAN_MANAGER_NUM_SUPPORTED_SLAVES)
                            slave_id = CAN_MANAGER_FIRST_SLAVE_ID;
                    }
                }
            }
            break;
        case CAN_CMD_I_RESULT_WAITING:
            break;  // do nothing
        default:
            read_state = CAN_MANAGER_READ_SLAVE_STATE_READ_STATUS;  // read status first

            if (ui_system_statistics_slave_error_ctr[slave_id - 1] < UINT64_T_MAX)
                ui_system_statistics_slave_error_ctr[slave_id - 1]++;
            if (ui_slave_error_ctr[slave_id - 1] < CAN_MANAGER_MAX_CAN_SLAVE_ERRORS)
                ui_slave_error_ctr[slave_id - 1]++;
            else
            {
                // error ==> set sensor status of all sensors to SENSOR_FAILED
                data.value = 0;
                data.status = SENSOR_FAILED;
                for (uint8_t ui_slave = 1; ui_slave <= CAN_MANAGER_NUM_SUPPORTED_SLAVES; ui_slave++)
                {
                    for (CAN_MANAGER_ACCESS_GROUP_E group = 0; group < CAN_MANAGER_ACCESS_GROUP_COUNT; group++)
                    {
                        for (uint32_t item_idx = 0; item_idx < ui_max_item_index[group_id]; item_idx++)
                        {
                            CAN_MANAGER_PutSensorDataIntoPrimestruct(slave_id, group_id, item_index, data);
                        }
                    }
                }

                // try next slave
                item_index = 0;
                group_id = 0;
                slave_id++;
                if (slave_id > CAN_MANAGER_NUM_SUPPORTED_SLAVES)
                    slave_id = CAN_MANAGER_FIRST_SLAVE_ID;
            }
            break;
        }
    }
#endif
}

/**
  * @brief  Initializes can manager.
  * @param  None
  * @retval bool:   - true  : finished write operation
  *                 - false : an error occurred
  */
bool CAN_MANAGER_Init(void)
{
#ifdef HWP_CAN_MASTER
    for (uint8_t i = 0; i < CAN_MANAGER_NUM_SUPPORTED_SLAVES; i++)
    {
        ui_slave_error_ctr[i] = 0;
        ui_system_statistics_slave_error_ctr[i] = 0;
        last_slave_rx_ts[CAN_MANAGER_NUM_SUPPORTED_SLAVES] = 0;
    }
    ui_can_error_ctr = 0;

    return CAN_CMD_I_Init();
#else
    return TRUE;
#endif
}

/**
  * @brief  CAN_MANAGER task.
  * @param  pvParameters
  * @retval None
  */
void CAN_MANAGER_Task(void *pvParameters)
{
#ifdef HWP_CAN_MASTER
    CLOCK_S_T last_slave_reset_ts = 0;

    while (!CAN_MANAGER_Init())
    {
        DETECTOR_SetCANErrorFlag();  // set error flag if CAN could not be initialized
        WATCHDOG_Heartbeat(WATCHDOG_TASK_ID_CAN_MANAGER);
        vTaskDelay(100);
    }

    // todo[medium] think about adding a function to check if all slaves are present
    while(1)
    {
        WATCHDOG_Heartbeat(WATCHDOG_TASK_ID_CAN_MANAGER);

        CAN_CMD_Tick();  // tick CAN driver

        //  try to collect data from slaves
        CAN_MANAGER_CollectSlaveDataTick();

        // error handling
        for (uint8_t ui_slave = 0; ui_slave < CAN_MANAGER_NUM_SUPPORTED_SLAVES; ui_slave++)
        {
            if ((CLOCK_GetTimestamp() > last_slave_rx_ts[ui_slave] + CAN_MANAGER_LAST_SLAVE_RX_RESET_TIMEOUT_S) && (CLOCK_GetTimestamp() > last_slave_reset_ts + CAN_MANAGER_LAST_SLAVE_RX_RESET_TIMEOUT_S))
            {
                last_slave_reset_ts = CLOCK_GetTimestamp();
                CAN_MANAGER_StartSlaveBootloader(CAN_MANAGER_SLAVE_ID_ALL_SLAVES);  // reset all slaves
                break;
            }
        }
        for (uint8_t ui_slave = 0; ui_slave < CAN_MANAGER_NUM_SUPPORTED_SLAVES; ui_slave++)
        {
            if (CLOCK_GetTimestamp() > last_slave_rx_ts[ui_slave] + CAN_MANAGER_LAST_SLAVE_RX_ERROR_FLAG_TIMEOUT_S)
                DETECTOR_SetCANErrorFlag();  // set error flag
        }

        vTaskDelay(10);
    }
#else
    while(1)
    {
        vTaskDelay(1000);
    }
#endif
}
