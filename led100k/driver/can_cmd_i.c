/*
 * can_cmd_i.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 30.07.2012
 *      Author: DHA
 */


#include "stdheader.h"
#include "can_cmd_i.h"
#include "hw_configuration.h"
#include "status_led.h"
#ifdef HWP_CAN_SLAVE
#include "system.h"
#endif

#ifdef HWP_CAN_MASTER
#include "detector.h"
#endif

#include "debug.h"

//variables from our linker script
extern uint8_t _image_ram_begin[];
extern uint8_t _image_ram_end[];

uint8_t u_my_can_id = 0xff;
volatile xQueueHandle xQueueCANRX;

#ifdef HWP_CAN_MASTER
uint8_t read_slave_memory_slave_id = 0;
uint8_t read_slave_memory_num_slaves = 0;
uint32_t read_slave_memory_address = 0;
uint32_t *p_read_slave_memory_data = 0;
CAN_CMD_I_RESULT_T *p_read_slave_memory_notifier = NULL;
CLOCK_MS_T read_slave_memory_timestamp = 0;

xSemaphoreHandle can_cmd_i_read_memory_master_mutex;
xSemaphoreHandle can_cmd_i_read_memory_semaphore;

//SLAVE_DATA_LIST_ITEM_T slave_data_list[CAN_CMD_I_HIGHEST_SLAVE_ID_ALLOWED];
SLAVE_DATA_LIST_ITEM_T *slave_data_list;
#endif

/**
 * @brief  Read ID of this device (switches on board).
 * @param  None
 * @retval None
 */
void CAN_CMD_I_ReadID(void)
{
#ifdef HWP_CAN_MASTER
    u_my_can_id = 0;  // id off master is always 0 and master has no switches to select id
#elif defined(HWP_CAN_SLAVE)
    uint8_t id;
    id = ((GPIO_ReadInputDataBit(ID_SWITCH_PORT, ID_SWITCH_SW1_PIN) |
                    (GPIO_ReadInputDataBit(ID_SWITCH_PORT, ID_SWITCH_SW2_PIN) << 1) |
                    (GPIO_ReadInputDataBit(ID_SWITCH_PORT, ID_SWITCH_SW3_PIN) << 2)));
    id = (~id) & 0x07;
    u_my_can_id = id;
#endif
}

/**
 * @brief  Read memory of this device and send data via CAN.
 * @param  address: address to read data from
 * @retval None
 */
CAN_CMD_I_RESULT_T CAN_CMD_I_ReadMemory (uint32_t address)
{
    uint32_t id;
    CAN_CMD_I_MEMACC_T data;

    id = (u_my_can_id << 8) | CAN_CMD_I_COMMAND_QN_READ_MEMORY_RESPONSE;
    data.addr = address;
    data.value = *(uint32_t*)address;  // todo[high] check if this can cause problems with mpu

    return CAN_STM32_F2XX_Transmit(id, sizeof(CAN_CMD_I_MEMACC_T), (uint8_t*)&data);  // send data to master
}

/**
 * @brief  Write data from CAN to memory of this device.
 * @param  address: address to write data to
 * @param  pData: data which should be written to the given address in the memory of this device
 * @retval None
 */
CAN_CMD_I_RESULT_T CAN_CMD_I_WriteMemory (uint32_t address, uint32_t data)
{
    if ((address < (uint32_t)_image_ram_begin) || (address > ((uint32_t)_image_ram_begin - (sizeof(uint32_t)))))
        return CAN_CMD_I_RESULT_ERROR;

    *(uint32_t*)address = data;  // todo[high] check if this can cause problems with mpu
    return CAN_CMD_I_RESULT_OK;
}

#ifdef HWP_CAN_MASTER
/**
 * @brief  Read memory of a slave via CAN (blocking).
 * @param  address: address to read data from
 * @param  num_slaves: number of slaves
 * @param  *pData: pointer to the address where the data from the slave should be stored
 * @retval None
 */
CAN_CMD_I_RESULT_T CAN_CMD_I_ReadAllSlavesMemory (uint32_t address, uint8_t num_slaves, SLAVE_DATA_LIST_ITEM_T *pData)
{
    uint32_t id;
    CAN_CMD_I_RESULT_T result = CAN_CMD_I_RESULT_ERROR;

    if (num_slaves == 0)
        return CAN_CMD_I_RESULT_OK;

    if ((pData == NULL) || (num_slaves > CAN_CMD_I_HIGHEST_SLAVE_ID_ALLOWED))
        return CAN_CMD_I_RESULT_ERROR;

    xSemaphoreTake(can_cmd_i_read_memory_master_mutex, portMAX_DELAY);

    slave_data_list = pData;
    read_slave_memory_num_slaves = num_slaves;
    for (uint8_t i = 0; i < num_slaves; i++)
    {
        slave_data_list[i].b_received = false;
    }

    read_slave_memory_slave_id = 0;  // 0 to read all slaves
    read_slave_memory_address = address;

    id = CAN_CMD_I_COMMAND_QN_READ_MEMORY;

    // send request to slave
    result = CAN_STM32_F2XX_Transmit(id, COMMAND_QN_READ_MEMORY_MSG_SIZE, (uint8_t*)&address);

    if (result == CAN_CMD_I_RESULT_OK)
    {
        // wait for answer
        if (xSemaphoreTake(can_cmd_i_read_memory_semaphore, CAN_CMD_I_READ_SLAVE_MEMORY_TIMEOUT_MS) != pdTRUE)  // todo[medium] check timeout
            result = CAN_CMD_I_RESULT_TIMEOUT;
    }

    // clean up
    read_slave_memory_slave_id = 0;
    read_slave_memory_address = 0;
    read_slave_memory_num_slaves = 0;
    slave_data_list = 0;

    xSemaphoreGive(can_cmd_i_read_memory_master_mutex);
    return result;
}

/**
 * @brief  Read memory of a slave via CAN (blocking).
 * @param  slave_id: id of the slave
 * @param  address: address to read data from
 * @param  *pData: pointer to the address where the data from the slave should be stored
 * @retval None
 */
CAN_CMD_I_RESULT_T CAN_CMD_I_ReadSlaveMemoryBlocking (uint8_t slave_id, uint32_t address, uint32_t *pData)
{
    uint32_t id;
    CAN_CMD_I_RESULT_T result = CAN_CMD_I_RESULT_ERROR;

    if (xSemaphoreTake(can_cmd_i_read_memory_master_mutex, 0) == pdFALSE)
        return CAN_CMD_I_RESULT_BLOCKED;

    read_slave_memory_slave_id = slave_id;
    read_slave_memory_address = address;
    p_read_slave_memory_data = pData;

    id = (slave_id << 8) | CAN_CMD_I_COMMAND_QN_READ_MEMORY;

    result = CAN_STM32_F2XX_Transmit(id, COMMAND_QN_READ_MEMORY_MSG_SIZE, (uint8_t*)&address);  // send request to slave

    if (result == CAN_CMD_I_RESULT_OK)
    {
        // wait for answer
        if (xSemaphoreTake(can_cmd_i_read_memory_semaphore, CAN_CMD_I_READ_SLAVE_MEMORY_TIMEOUT_MS) != pdTRUE)
            result = CAN_CMD_I_RESULT_TIMEOUT;
    }

    // clean up
    read_slave_memory_slave_id = 0;
    read_slave_memory_address = 0;
    p_read_slave_memory_data = 0;

    xSemaphoreGive(can_cmd_i_read_memory_master_mutex);
    return result;
}

/**
 * @brief  Read memory of a slave via CAN (non blocking).
 * @param  slave_id: id of the slave
 * @param  address: address to read data from
 * @param  *pData: pointer to the address where the data from the slave should be stored
 * @retval None
 */
CAN_CMD_I_RESULT_T CAN_CMD_I_ReadSlaveMemoryNonBlocking (uint8_t slave_id, uint32_t address, uint32_t *pData, CAN_CMD_I_RESULT_T *p_notifier)
{
    uint32_t id;
    CAN_CMD_I_RESULT_T result = CAN_CMD_I_RESULT_ERROR;

    if (xSemaphoreTake(can_cmd_i_read_memory_master_mutex, 0) == pdFALSE)
        return CAN_CMD_I_RESULT_BLOCKED;

    read_slave_memory_slave_id = slave_id;
    read_slave_memory_address = address;
    p_read_slave_memory_data = pData;
    p_read_slave_memory_notifier = p_notifier;
    read_slave_memory_timestamp = CLOCK_GetMillitimestamp();

    id = (slave_id << 8) | CAN_CMD_I_COMMAND_QN_READ_MEMORY;

    result = CAN_STM32_F2XX_Transmit(id, COMMAND_QN_READ_MEMORY_MSG_SIZE, (uint8_t*)&address);  // send request to slave

    if (result == CAN_CMD_I_RESULT_OK)
    {
        *p_notifier = CAN_CMD_I_RESULT_WAITING;
        return CAN_CMD_I_RESULT_OK;
    }

    // clean up
    read_slave_memory_slave_id = 0;
    read_slave_memory_address = 0;
    p_read_slave_memory_data = 0;
    p_read_slave_memory_notifier = NULL;

    xSemaphoreGive(can_cmd_i_read_memory_master_mutex);
    return result;
}

/**
 * @brief  Write data via CAN to memory of slave .
 * @param  address: address to write data to
 * @param  pData: data which should be written to the given address in the memory of the slave
 * @param  slave_id: id of the slave
 * @retval None
 */
CAN_CMD_I_RESULT_T CAN_CMD_I_WriteSlaveMemory (uint8_t slave_id, uint32_t address, uint32_t data)
{
    uint32_t id;
    CAN_CMD_I_MEMACC_T message;

    id = (slave_id << 8) | CAN_CMD_I_COMMAND_QN_WRITE_MEMORY;

    message.addr = address;
    message.value = data;

    return CAN_STM32_F2XX_Transmit(id, sizeof(CAN_CMD_I_MEMACC_T), (uint8_t*)&message);  // send data to slave
}
#endif

/**
 * @brief  Send a command to the selected slave / all slaves.
 * @param  slave_id: id of the slave
 * @param  command: command to send
 * @retval None
 */
CAN_CMD_I_RESULT_T CAN_CMD_I_SendCommand(uint8_t slave_id, uint8_t command)
{
    uint32_t id;

    id = (slave_id << 8) | command;

    return CAN_STM32_F2XX_Transmit(id, 0, 0);  // send data to slave
}

#ifdef HWP_CAN_MASTER
/**
 * @brief  Function to process rx messages / execute command.
 * @param  RxMessage: received message
 * @retval None
 */
void CAN_CMD_PutMessageIntoRXQueue(CanRxMsg RxMessage)
{
    if (xQueueCANRX == NULL)
        return;  // error

    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    CAN_CMD_I_CAN_MSG_DATA_STRUCT_T data;

    memcpy(&data.message, &RxMessage, sizeof(CanRxMsg));
//    data.timestamp = CLOCK_GetMillitimestamp();
    xQueueSendFromISR(xQueueCANRX, &data, &xHigherPriorityTaskWoken);
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief  Tick function for can master. Used to execute commands.
 * @param  None
 * @retval None
 */
void CAN_CMD_Tick(void)
{
    if (xQueueCANRX == NULL)
        return;  // error

    CAN_CMD_I_CAN_MSG_DATA_STRUCT_T data;
    if (xQueueReceive(xQueueCANRX, &data, 0) == pdTRUE)
    {
        CanRxMsg RxMessage;
        memcpy(&RxMessage, &data.message, sizeof(CanRxMsg));
        CAN_CMD_ExecuteCommand(RxMessage);
    }

    if (p_read_slave_memory_notifier != NULL)
    {
        // non blocking read ==> check timeout
        if (CLOCK_GetMillitimestamp() > (read_slave_memory_timestamp + CAN_CMD_I_READ_SLAVE_MEMORY_TIMEOUT_MS))
        {
            // timeout
            // clean up
            read_slave_memory_slave_id = 0;
            read_slave_memory_address = 0;
            p_read_slave_memory_data = 0;
            xSemaphoreGive(can_cmd_i_read_memory_master_mutex);
            // notify
            *p_read_slave_memory_notifier = CAN_CMD_I_RESULT_TIMEOUT;
        }
    }
}
#endif

/**
 * @brief  Function to process rx messages / execute command.
 * @param  RxMessage: received message
 * @retval None
 */
void CAN_CMD_ExecuteCommand(CanRxMsg RxMessage)
{
    CAN_CMD_I_MEMACC_T data;
    uint8_t u_cmd;
    CAN_CMD_I_RESULT_T result = CAN_CMD_I_RESULT_ERROR;
#ifdef HWP_CAN_MASTER
    uint8_t u_id;
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    u_id = (RxMessage.StdId >> 8) & 0x3;
#endif

    u_cmd = RxMessage.StdId & 0xFF;
    data.addr = *((uint32_t*)(&RxMessage.Data[0]));
    data.value = *((uint32_t*)(&RxMessage.Data[4]));

    switch (u_cmd)
    {
    case CAN_CMD_I_COMMAND_QN_WRITE_MEMORY:
        if (RxMessage.DLC != COMMAND_QN_WRITE_MEMORY_MSG_SIZE)
            break;  // error
        result = CAN_CMD_I_WriteMemory(data.addr, data.value);
        break;
    case CAN_CMD_I_COMMAND_QN_WRITE_MEMORY_RESPONSE:
        break;  // nothing to do
    case CAN_CMD_I_COMMAND_QN_READ_MEMORY:
        if (RxMessage.DLC != COMMAND_QN_READ_MEMORY_MSG_SIZE)
            break;  // error
        result = CAN_CMD_I_ReadMemory(data.addr);
        break;
    case CAN_CMD_I_COMMAND_QN_READ_MEMORY_RESPONSE:
#ifdef HWP_CAN_MASTER
        if ((read_slave_memory_address  != 0))
        {
            if ((read_slave_memory_address  == data.addr) && (RxMessage.DLC == COMMAND_QN_READ_MEMORY_RESPONSE_MSG_SIZE))
            {
                if ((read_slave_memory_slave_id == u_id)  && (p_read_slave_memory_data != 0))
                {
                    // read only one slave
                    *p_read_slave_memory_data = data.value;
                    if (p_read_slave_memory_notifier == NULL)
                    {
                        // blocking read
                        xSemaphoreGiveFromISR(can_cmd_i_read_memory_semaphore, &xHigherPriorityTaskWoken);
                        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
                    }
                    else
                    {
                        // non blocking read
                        // clean up
                        read_slave_memory_slave_id = 0;
                        read_slave_memory_address = 0;
                        p_read_slave_memory_data = 0;
                        xSemaphoreGive(can_cmd_i_read_memory_master_mutex);
                        // notify
                        *p_read_slave_memory_notifier = CAN_CMD_I_RESULT_OK;
                    }
                }
                else if ((read_slave_memory_slave_id == 0) && (u_id <= read_slave_memory_num_slaves) && (slave_data_list != 0))
                {
                    // read more than one slave
                    *(slave_data_list[u_id - 1].p_data) = data.value;
                    slave_data_list[u_id - 1].b_received = true;

                    bool b_finished = true;
                    for (uint8_t i = 0; i < read_slave_memory_num_slaves; i++)
                    {
                        if (slave_data_list[i].b_received == false)
                        {
                            b_finished = false;
                            break;
                        }
                    }
                    if (b_finished)
                    {
                        xSemaphoreGiveFromISR(can_cmd_i_read_memory_semaphore, &xHigherPriorityTaskWoken);
                        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
                    }
                }
            }
        }
#else
        // do nothing
#endif
        break;
    case CAN_CMD_I_COMMAND_QN_RESTART_SLAVE:
        #ifdef HWP_CAN_SLAVE
        SYSTEM_InvokeBootloader();
        #else
        // do nothing
        #endif
        break;
    default: break;  // do nothing
    }

    result = result;  // no transmit error handling on slaves, increment error counter on master
}

/**
 * @brief  Configures the CAN.
 * @param  None
 * @retval None
 */
void CAN_CMD_I_ConfigCANFilter(void)
{
    // CAN filter init
    CAN_FilterInitTypeDef CAN_FilterInitStructure;

    CAN_FilterInitStructure.CAN_FilterNumber = 0;
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_16bit;
    CAN_FilterInitStructure.CAN_FilterIdHigh = (u_my_can_id << 8) << 5;  // second filter, all messages to this slave
    CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;  // first filter, all messages from master to all slaves
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = CAN_CMD_I_ID_FILTER_MASK << 5;
#ifdef HWP_CAN_MASTER
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0;  // master should receive all messages
#else
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = CAN_CMD_I_ID_FILTER_MASK << 5;
#endif
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&CAN_FilterInitStructure);
}

/**
 * @brief  Initializes this module.
 * @param  None
 * @retval bool:   - true  : finished write operation
 *                 - false : an error occurred
 */
bool CAN_CMD_I_Init(void)
{
#ifdef HWP_CAN_MASTER
    xQueueCANRX = xQueueCreate(64, sizeof(CAN_CMD_I_CAN_MSG_DATA_STRUCT_T));
    vQueueAddToRegistry(xQueueCANRX, (signed char*)"CAN RX Queue");

    if (can_cmd_i_read_memory_master_mutex == NULL)
    {
        can_cmd_i_read_memory_master_mutex = xSemaphoreCreateMutex();
        assert_param(can_cmd_i_read_memory_master_mutex != NULL);
        vQueueAddToRegistry(can_cmd_i_read_memory_master_mutex, (signed char*)"can_cmd_i_read_memory_master_mutex");
    }

    if (can_cmd_i_read_memory_semaphore == NULL)
    {
        vSemaphoreCreateBinary(can_cmd_i_read_memory_semaphore);
        assert_param(can_cmd_i_read_memory_semaphore != NULL);
        vQueueAddToRegistry(can_cmd_i_read_memory_semaphore, (signed char*)"can_cmd_i_read_memory_semaphore");
    }
    xSemaphoreTake(can_cmd_i_read_memory_semaphore, 0);
#endif

    CAN_CMD_I_ReadID();

#ifdef HWP_CAN_MASTER
    if (!CAN_STM32_F2XX_Init())
        return FALSE;
#else
    if (!CAN_STM32_F2XX_Init())
        NVIC_SystemReset();  // reset system, slave can not work without CAN
#endif

    CAN_CMD_I_ConfigCANFilter();

#ifdef HWP_CAN_MASTER
    CAN_STM32_F2XX_RX_ChannelInit(CAN_CHANNEL_1_RX_0, CAN_CMD_PutMessageIntoRXQueue);
#else
    CAN_STM32_F2XX_RX_ChannelInit(CAN_CHANNEL_1_RX_0, CAN_CMD_ExecuteCommand);
#endif

    CAN_ITConfig(CAN1, CAN_IT_EPV, ENABLE);
    CAN_ITConfig(CAN1, CAN_IT_BOF, ENABLE);
    CAN_ITConfig(CAN1, CAN_IT_ERR, ENABLE);
    CAN_ITConfig(CAN1, CAN_IT_TME, ENABLE);

    return TRUE;
}
