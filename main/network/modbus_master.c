/*
 * modbus_master.c
 *
 *  Created on: 02.08.2010
 *      Author: Carsten Bartels
 */

#include "stdheader.h"
#include "modbus_master.h"
#include "clock.h"
#include "settings.h"
#include "modbus_control.h"
#include "debug_freertos.h"

// ****************** Local Defines ********************

#define MODBUS_RESPONSE_TIMEOUT 2000 // Modbus response message timeout in ms
#define MODBUS_REQUEST_RETRIES 10 // Modbus request retries till fail state

enum
{
    REQUEST_INPUTS,
    SET_OUTPUTS,
    MESSAGE_COUNT
};

// ***************** Local Variables *******************
static xQueueHandle xQueueModbusInbox;
static xSemaphoreHandle xSemaphoreReinit;

static MODBUS_RTU_PACKET_T ModbusReceivedRTUPacket;
static MODBUS_RTU_PACKET_T ModbusRequestRTUPacket;

static CLOCK_MS_T ResponseTimer;
static uint8_t uRetryCounter[MESSAGE_COUNT];


// ***************** Global Variables *******************
MODBUS_MASTER_STATS_T modbusMasterStats;


void MODBUS_MASTER_Response_Receiver(MODBUS_RTU_PACKET_T *packet)
{
    if (xQueueModbusInbox != NULL)
    {
        xQueueSendToBack(xQueueModbusInbox, ( void * ) packet, ( portTickType ) 10 );
        modbusMasterStats.io.responses++;
        ResponseTimer = 0;
    }
}

void MODBUS_MASTER_Response_Handler(void)
{
    MODBUS_PDU_T *pModbusPDUFrame;

    pModbusPDUFrame = (MODBUS_PDU_T*) &ModbusReceivedRTUPacket.pdu;

    if ((pModbusPDUFrame->function_code & 0x80) > 0)
    {
        // modbus exception message received
        modbusMasterStats.error.exceptions++;
        return;
    }

    switch(pModbusPDUFrame->function_code)
    {
    case MODBUS_FC_READ_WRITE_MODULE_SETTINGS:
        if (pModbusPDUFrame->message.responses.read_write_module_settings.sub_function_code == MODBUS_SUBFC_READ_THE_MODULE_NAME)
        {
            asm("nop");
        }
        break;
    case MODBUS_FC_READ_DISCRETE_INPUTS:
        uRetryCounter[REQUEST_INPUTS] = MODBUS_REQUEST_RETRIES;
        MODBUS_CONTROL_DigitalInput((uint16_t)pModbusPDUFrame->message.responses.read_discrete_inputs.input_status[0]);
        break;
    case MODBUS_FC_WRITE_MULTIPLE_COILS:
        uRetryCounter[SET_OUTPUTS] = MODBUS_REQUEST_RETRIES; // got response. reset counter.
        break;
    default:
        break;
    }
}

void MODBUS_MASTER_Request(void)
{
    MODBUS_RTU_Transmit(&ModbusRequestRTUPacket);
    ResponseTimer = CLOCK_GetMillitimestamp();
    modbusMasterStats.io.requests++;
}

void MODBUS_MASTER_Worker(void)
{
    static uint8_t reqid = REQUEST_INPUTS;

    switch (reqid)
    {
    case REQUEST_INPUTS:
        ModbusRequestRTUPacket.slave = MODBUS_MASTER_SLAVE_ADDRESS;
        ModbusRequestRTUPacket.pdu.function_code = MODBUS_FC_READ_DISCRETE_INPUTS;
        ModbusRequestRTUPacket.pdu.message.requests.read_discrete_inputs.starting_address = HTOMODBUS16(0x0000);
        ModbusRequestRTUPacket.pdu.message.requests.read_discrete_inputs.quantity_of_inputs = HTOMODBUS16(0x0008);
        ModbusRequestRTUPacket.pdulen = 5;
        MODBUS_MASTER_Request();
        break;

    case SET_OUTPUTS:
        ModbusRequestRTUPacket.slave = MODBUS_MASTER_SLAVE_ADDRESS;
        ModbusRequestRTUPacket.pdu.function_code = MODBUS_FC_WRITE_MULTIPLE_COILS;
        ModbusRequestRTUPacket.pdu.message.requests.write_multiple_coils.starting_channel_number = HTOMODBUS16(0x0000);
        ModbusRequestRTUPacket.pdu.message.requests.write_multiple_coils.output_channel_number = HTOMODBUS16(0x0008);
        ModbusRequestRTUPacket.pdu.message.requests.write_multiple_coils.bytecount = 0x01;
        ModbusRequestRTUPacket.pdu.message.requests.write_multiple_coils.output_value = (uint8_t)MODBUS_CONTROL_DigitalOutput();
        ModbusRequestRTUPacket.pdulen = 7;
        MODBUS_MASTER_Request();
        break;

    default:
        reqid = REQUEST_INPUTS;
        break;
    }

    // retry counter handling.
    if (uRetryCounter[reqid] == 0)
    {
        switch (reqid)
        {
        case REQUEST_INPUTS:
            MODBUS_CONTROL_DigitalInputFailed();
            break;
        case SET_OUTPUTS:
            break;
        default:
            break;
        }
    }
    else
    {
        uRetryCounter[reqid]--;
    }

    if (++reqid == MESSAGE_COUNT)
        reqid = REQUEST_INPUTS;
}

void MODBUS_MASTER_SettingsUpdated(SETTINGS_GROUP_T group_id)
{
    // reinit control module to reset all controlled targets to default state on settings alteration
    xSemaphoreGive(xSemaphoreReinit);
}

void MODBUS_MASTER_Init()
{
    xQueueModbusInbox = xQueueCreate(4, sizeof(MODBUS_RTU_PACKET_T));
    DQueueAddToRegistry(xQueueModbusInbox, "xQueueModbusInbox");
    assert_param(xQueueModbusInbox != NULL);

    vSemaphoreCreateBinary(xSemaphoreReinit);

    MODBUS_CONTROL_Init();

    SETTINGS_GROUP_CHANGE_CALLBACK_T cbStruct;
    cbStruct.pCallbackFunc = MODBUS_MASTER_SettingsUpdated;
    cbStruct.uGroupMask = SETTINGS_GROUP_TERMINAL;
    SETTINGS_RegisterGroupChangeCallback(&cbStruct);
}

void MODBUS_MASTER_Tick()
{
    assert_param(xQueueModbusInbox != NULL);

    if (xSemaphoreTake(xSemaphoreReinit, 0) == pdTRUE)
        MODBUS_CONTROL_Init();

    if ((ResponseTimer > 0) && (ResponseTimer + MODBUS_RESPONSE_TIMEOUT < CLOCK_GetMillitimestamp()))
    {
        // Error Response Timeout
        modbusMasterStats.error.timeouts++;
        ResponseTimer = 0;
    }

    if (pdTRUE == xQueueReceive(xQueueModbusInbox, ( void * ) &ModbusReceivedRTUPacket, 0))
        MODBUS_MASTER_Response_Handler();

    if (ResponseTimer == 0)
    {
        // No ongoing request. Start a new one.
        MODBUS_MASTER_Worker();
    }
}
