
#include "stdheader.h"
#include "debug_freertos.h"
#include "meshcom_conn.h"

#include "mesh.h"
#include "meshcom_io.h"
#include "meshcom_cmd.h"
#include "meshcom_bc.h"
#include "timeout.h"

#include "state.h"
#include "debug.h"
#include "system.h"
#include "uart.h"
#include "log.h"
#include "sound.h"
#include "clock.h"
#include "version.h"
#include "debug.h"
#include "gpssync.h"
//#include "gpsgeometry.h"
#include "gloom.h"
#include "gloom_manager.h"
#include "visibility.h"
#include "detector.h"
#include "bmm.h"
#include "sequence.h"

#include "syndication.h"
#include "systemconfig.h"
#include "firmware_store.h"

#include "watchdog.h"




// **** Local Variables ****
static TIMEOUT_T MessageParseTimeout = 0;
static TIMEOUT_T NetworkDataParseTimeout = 0;
//static TIMEOUT_T NetworkTimeout = 0;
static TIMEOUT_T MessageSendTimeout = 0;

static uint8_t MessageParseErrorCount = 0;
static uint8_t NetworkDataParseErrorCount = 0;
static uint8_t NetworkErrorCount = 0;


static uint8_t old_meshcom_state;

static bool bMeshComEnabled = FALSE;

typedef struct
{
    uint8_t payload[MESHCOM_MESSAGESIZE_MAX];
    uint8_t size;
    uint8_t seq_id;
    MESHCOM_CB_HANDLER_SEQ *callback;
} MeshcomMessage_t;

typedef struct
{
    uint8_t seq_id;
    MESHCOM_CB_HANDLER_SEQ *callback;
} MeshcomMessageCallback_t;


xQueueHandle xQueueMeshComMessageOutput = NULL;
xQueueHandle xQueueMeshComMessageOutputCallback = NULL;


#define MESHCOM_MESSAGE_PARSER_COUNT     5
static MESHCOM_MESSAGE_PARSER *externalRxMessageParser[MESHCOM_MESSAGE_PARSER_COUNT] = {NULL, NULL, NULL, NULL, NULL};
static MESHCOM_MESSAGE_PARSER *externalTxMessageParser[MESHCOM_MESSAGE_PARSER_COUNT] = {NULL, NULL, NULL, NULL, NULL};

// **** Exported local Variabels ****

MESHCOM_MSG_STATUS_S _meshcom_node_status;
MESHCOM_STATUS_S _meshcom_status;

// **** Local Functions ****
void meshcom_putbyte(uint8_t c);
void meshcom_packetdump(uint8_t *pData, uint16_t length);

static void meshcomSendNextMessage(void);
static void meshcomSendConfirm(bool response);
static void meshcomSendConfirmCheckTimeout(void);

bool meshcomEnqueueMessage(uint8_t * pMessage, uint8_t uSize, MESHCOM_CB_HANDLER_SEQ *callback, uint8_t seq_id);
bool meshcomSendLocalMessage(uint8_t *pMessage, uint8_t uSize);

void meshcom_parse_local_message(MESHCOM_MSG_S *pMessage);
void meshcom_parse_broadcast_message(MESHCOM_MSG_S *pMessage);
void meshcom_parse_unicast_message(MESHCOM_MSG_S *pMessage);

void meshcom_external_rx_parser(MESHCOM_MSG_S *pMsg);
void meshcom_external_tx_parser(MESHCOM_MSG_S *pMsg);
void meshcom_register_message_parser(MESHCOM_MESSAGE_PARSER *parser, MESHCOM_MESSAGE_PARSER **library);
void meshcom_unregister_message_parser(MESHCOM_MESSAGE_PARSER *parser, MESHCOM_MESSAGE_PARSER **library);




void meshcom_putbyte(uint8_t c)
{
    MESHCOM_UART_SEND(c);
}


void meshcom_packetdump(uint8_t *pData, uint16_t length)
{
    DPRINT1(DEBUG_MESH, "\nPacket Payload length %u\n", length);
    for (uint16_t i = 1; i <= length; i++)
    {
        DPRINT1(DEBUG_MESH, "%02X ", *pData++);
        if (i % 8 == 0)
        {
            DPRINT(DEBUG_MESH, "   ");
        }
        if (i % 16 == 0)
        {
            DPRINT(DEBUG_MESH, "\n");
        }
    }
    DPRINT(DEBUG_MESH, "\n\n");
}


static void meshcomSendNextMessage(void)
{
    MeshcomMessage_t msg;
    MeshcomMessageCallback_t msg_cb;

    // check if no other send action is pending
    if (uxQueueMessagesWaiting(xQueueMeshComMessageOutputCallback) > 0)
        return;

    if (xQueueReceive(xQueueMeshComMessageOutput, (void *) &msg, 0) == pdPASS)
    {
        meshcom_external_tx_parser((MESHCOM_MSG_S *) msg.payload);
        MESHCOM_IO_Dispatch(msg.payload, msg.size);
        msg_cb.callback = msg.callback;
        msg_cb.seq_id = msg.seq_id;
        xQueueSend(xQueueMeshComMessageOutputCallback, (void*) &msg_cb, 0);
        TIMEOUT_Reset(MessageSendTimeout);
    }
}


static void meshcomSendConfirm(bool response)
{
    MeshcomMessageCallback_t msg_cb;

    if (xQueueReceive(xQueueMeshComMessageOutputCallback, (void *) &msg_cb, 0) == pdPASS)
    {
        if (msg_cb.callback != NULL)
        {
            //DPRINT1(DEBUG_MESH, "MESHCOM MsgTxCallback ACK=%u\n", response);
            if (response == TRUE)
                (msg_cb.callback)(MESHCOM_OK, msg_cb.seq_id);
            else
                (msg_cb.callback)(MESHCOM_ERROR, msg_cb.seq_id);
        }
    }
}


static void meshcomSendConfirmCheckTimeout(void)
{
    if (TIMEOUT_Check(MessageSendTimeout, MESHCOM_MESSAGE_QUEUE_CONFIRM_TIMEOUT_SEC))
    {
        if (uxQueueMessagesWaiting(xQueueMeshComMessageOutputCallback) > 0)
            meshcomSendConfirm(FALSE);
    }
}


bool meshcomEnqueueMessage(uint8_t * pMessage, uint8_t uSize, MESHCOM_CB_HANDLER_SEQ *callback, uint8_t seq_id)
{
    MeshcomMessage_t msg;
    uint8_t *p;

    // ToDo optimize this stuff here by creating MeshcomMessage_t in caller directly. saves a lot of stupid copying

    if (uSize > MESHCOM_MESSAGESIZE_MAX)
        return FALSE;

    msg.callback = callback;
    msg.seq_id = seq_id;
    msg.size = uSize;
    p = (uint8_t *)&msg.payload;

    // Add Frame Payload
    while (uSize-- > 0)
    {
        *p++ = *pMessage++;
    }

    if (xQueueMeshComMessageOutput != NULL)
        xQueueSendToBack(xQueueMeshComMessageOutput,(void *) &msg, (portTickType) 100);

    return TRUE;
}


bool meshcomSendLocalMessage(uint8_t *pMessage, uint8_t uSize)
{
    meshcom_external_tx_parser((MESHCOM_MSG_S *)pMessage);
    MESHCOM_IO_Dispatch(pMessage, uSize);
    return TRUE;
}





MESHCOM_RESULT_T MESHCOM_SendBroadcastShutup(uint16_t timeout, MESHCOM_CB_HANDLER_SEQ *callback)
{
    MESHCOM_MSG_S meshcom_send_message;

    memset(&meshcom_send_message, 0, sizeof(MESHCOM_MSG_S));

    meshcom_send_message.header.version.major = MESHCOM_PROTOCOL_VERSION_MAJOR;
    meshcom_send_message.header.version.minor = MESHCOM_PROTOCOL_VERSION_MINOR;
    meshcom_send_message.header.port = MESHCOM_PORT_BROADCAST;
    meshcom_send_message.header.id.major = MESHCOM_MSG_ID_BC_COMMAND;
    meshcom_send_message.header.id.minor = 0;
    meshcom_send_message.header.length = MESHCOM_MSG_BC_COMMAND_SIZE;

    MESHCOM_MSG_BC_COMMAND_S *bc_command = &meshcom_send_message.message.bc_command;
    bc_command->command = MESHCOM_BC_COMMAND_BROADCAST_SHUTUP;
    bc_command->parameter16 = timeout;
    bc_command->parameter32 = 0UL;

    bc_command->serial = SERIAL_NUMBER;

    meshcomEnqueueMessage((uint8_t*) &meshcom_send_message, MESHCOM_MSG_HEADER_SIZE + meshcom_send_message.header.length, callback, 0);

    return MESHCOM_PENDING;
}


MESHCOM_RESULT_T MESHCOM_SendBroadcast(MESHCOM_MSG_S *meshcom_send_message, MESHCOM_CB_HANDLER_SEQ *callback, uint8_t seq_id)
{
    DPRINT2(DEBUG_MESH, "MESHCOM_SendBroadcast: ID 0x%02X.0x%02X\n", meshcom_send_message->header.id.major, meshcom_send_message->header.id.minor);

    meshcom_send_message->header.port = MESHCOM_PORT_BROADCAST;
    // meshcom_send_message.header.id.major already set by calling func
    // meshcom_send_message.header.id.minor already set by calling func
    meshcom_send_message->header.version.major = MESHCOM_PROTOCOL_VERSION_MAJOR;
    meshcom_send_message->header.version.minor = MESHCOM_PROTOCOL_VERSION_MINOR;
    // meshcom_send_message.header.length already set by calling func

    meshcomEnqueueMessage((uint8_t*) meshcom_send_message, MESHCOM_MSG_HEADER_SIZE + meshcom_send_message->header.length, callback, seq_id);

    return MESHCOM_PENDING;
}


MESHCOM_RESULT_T MESHCOM_SendBroadcastCommand(MESHCOM_MSG_S *meshcom_send_message, MESHCOM_CB_HANDLER_SEQ *callback, uint8_t seq_id)
{
    DPRINT3(DEBUG_MESH, "MESHCOM_SendBroadcastCommand: CmdID 0x%02X, Para16 %u, Para32 %u\n", meshcom_send_message->message.bc_command.command, meshcom_send_message->message.bc_command.parameter16, meshcom_send_message->message.bc_command.parameter32);

    meshcom_send_message->header.version.major = MESHCOM_PROTOCOL_VERSION_MAJOR;
    meshcom_send_message->header.version.minor = MESHCOM_PROTOCOL_VERSION_MINOR;
    meshcom_send_message->header.port = MESHCOM_PORT_BROADCAST;
    meshcom_send_message->header.id.major = MESHCOM_MSG_ID_BC_COMMAND;
    meshcom_send_message->header.id.minor = 0;
    meshcom_send_message->header.length = MESHCOM_MSG_BC_COMMAND_SIZE;

    meshcomEnqueueMessage((uint8_t*) meshcom_send_message, MESHCOM_MSG_HEADER_SIZE + meshcom_send_message->header.length, callback, seq_id);

    return MESHCOM_PENDING;
}


MESHCOM_RESULT_T MESHCOM_SendLocalCommand(uint8_t command, uint16_t parameter16, uint32_t parameter32)
{
    MESHCOM_MSG_S meshcom_send_message;

    DPRINT3(DEBUG_MESH, "MESHCOM_SendLocalCommand: CmdID 0x%02X, Para16 %u, Para32 %u\n", command, parameter16, parameter32);

    memset (&meshcom_send_message, 0, sizeof(MESHCOM_MSG_S));

    meshcom_send_message.header.id.major = MESHCOM_MSG_ID_COMMAND;
    meshcom_send_message.header.id.minor = 0;
    meshcom_send_message.header.port = MESHCOM_PORT_LOCAL;
    meshcom_send_message.header.version.major = MESHCOM_PROTOCOL_VERSION_MAJOR;
    meshcom_send_message.header.version.minor = MESHCOM_PROTOCOL_VERSION_MINOR;
    meshcom_send_message.header.length = MESHCOM_MSG_COMMAND_SIZE;

    MESHCOM_MSG_COMMAND_S *msg_command = &meshcom_send_message.message.command;
    msg_command->cmdid          = command;
    msg_command->parameter16    = parameter16;
    msg_command->parameter32    = parameter32;

    meshcomSendLocalMessage((uint8_t*) &meshcom_send_message, MESHCOM_MSG_HEADER_SIZE + meshcom_send_message.header.length);

    return MESHCOM_OK;
}


MESHCOM_RESULT_T MESHCOM_SendCommandstream(MESHCOM_MSG_S *meshcom_send_message, MESHCOM_CB_HANDLER_SEQ *callback, uint8_t seq_id)
{
    meshcom_send_message->header.port = MESHCOM_PORT_UNICAST;
    meshcom_send_message->header.id.major = MESHCOM_MSG_ID_COMMANDSTREAM;
    // meshcom_send_message.header.id.minor already set by calling func
    meshcom_send_message->header.version.major = MESHCOM_PROTOCOL_VERSION_MAJOR;
    meshcom_send_message->header.version.minor = MESHCOM_PROTOCOL_VERSION_MINOR;
    // meshcom_send_message.header.length already set by calling func

    if (meshcomEnqueueMessage((uint8_t*) meshcom_send_message, MESHCOM_MSG_HEADER_SIZE + meshcom_send_message->header.length, callback, seq_id))
        return MESHCOM_PENDING;

    return MESHCOM_BUSY;
}


MESHCOM_RESULT_T MESHCOM_SendDatastream(MESHCOM_MSG_S *meshcom_send_message, uint8_t port, MESHCOM_CB_HANDLER_SEQ *callback, uint8_t seq_id)
{
    //        if (!meshcom_callback_set(&sMeshComCallbacks.unicast, callback))
    //            return MESHCOM_BUSY;

    meshcom_send_message->header.port = port;
    meshcom_send_message->header.id.major = MESHCOM_MSG_ID_DATASTREAM;
    // meshcom_send_message.header.id.minor already set by calling func
    meshcom_send_message->header.version.major = MESHCOM_PROTOCOL_VERSION_MAJOR;
    meshcom_send_message->header.version.minor = MESHCOM_PROTOCOL_VERSION_MINOR;
    // meshcom_send_message.header.length already set by calling func

    if (meshcomEnqueueMessage((uint8_t*) meshcom_send_message, MESHCOM_MSG_HEADER_SIZE + meshcom_send_message->header.length, callback, seq_id))
        return MESHCOM_PENDING;

    return MESHCOM_BUSY;
}


MESHCOM_RESULT_T MESHCOM_SendSetup(void)
{
    MESHCOM_MSG_S meshcom_send_message;

    DPRINT(DEBUG_MESH, "MESHCOM_SendSetup\n");

    memset(&meshcom_send_message, 0, sizeof(MESHCOM_MSG_S));

    meshcom_send_message.header.id.major = MESHCOM_MSG_ID_SETUP;
    meshcom_send_message.header.id.minor = 0;
    meshcom_send_message.header.version.major = MESHCOM_PROTOCOL_VERSION_MAJOR;
    meshcom_send_message.header.version.minor = MESHCOM_PROTOCOL_VERSION_MINOR;
    meshcom_send_message.header.port = MESHCOM_PORT_LOCAL;
    meshcom_send_message.header.length = MESHCOM_MSG_SETUP_SIZE;

    if (MESHCOM_SETTING_TYPE != 0) // 0 = auto, 1 = Coord, 2 = Router, 3 = EndDevice
        meshcom_send_message.message.setup.nodeType = (uint8_t) MESHCOM_SETTING_TYPE - 1;
    else
        meshcom_send_message.message.setup.nodeType = 1;
    DPRINT1(DEBUG_MESH, "  nodeType: %u\n", meshcom_send_message.message.setup.nodeType);

    meshcom_send_message.message.setup.channelPage = (uint8_t) MESHCOM_CHANNEL_PAGE;
    DPRINT1(DEBUG_MESH, "  channelPage: %u\n", meshcom_send_message.message.setup.channelPage);
    meshcom_send_message.message.setup.channelMask = MESHCOM_SETTING_CHANNEL_MASK;
    DPRINT1(DEBUG_MESH, "  channelMask: 0x%08X\n", meshcom_send_message.message.setup.channelMask);

    // the 64Bit Unified ID consists of a static 32Bit upper part and the beacons serial number as the lower part.
    // The Coordinator is the security trust center and must be known to all other nodes. Thus the Coordinators UID
    // is set to upper part only (it is serial number independend and always the same).
#if 0
    if (meshcom_send_message.message.setup.nodeType == 0)
        meshcom_send_message.message.setup.uid = SETTINGS_MESH_UID_BASE; // Coordinator == TrustCenter -> Static and well known UID.
    else
#endif
        meshcom_send_message.message.setup.uid = SETTINGS_MESH_UID_BASE + SERIAL_NUMBER;
    DPRINT2(DEBUG_MESH, "  uid: 0x%08X %08X\n", (uint32_t)((meshcom_send_message.message.setup.uid >> 32) & 0xFFFFFFFFUL), (uint32_t)(meshcom_send_message.message.setup.uid & 0xFFFFFFFFUL));

    // the 64Bit Personal Area Network ID consists like the UID of a static upper part and a configurable lower part.
    meshcom_send_message.message.setup.extPanId = SETTINGS_MESH_EXTPANID_BASE + MESHCOM_SETTING_PAN_ID;
    DPRINT2(DEBUG_MESH, "  extPanId: 0x%08X %08X\n", (uint32_t)((meshcom_send_message.message.setup.extPanId >> 32) & 0xFFFFFFFFUL), (uint32_t)(meshcom_send_message.message.setup.extPanId & 0xFFFFFFFFUL));

    // transmitter power to be set (dBm). Valid range is hardware-dependent:
    // For AT86RF230 RF part, it is from 0xEF to 0x03 (-17 to 3).
    // For AT86RF212 RF part, it is from 0xF5 to 0x0B (-11 to 11).
    meshcom_send_message.message.setup.txPower = (int8_t) MESHCOM_SETTING_TRANSMITTER_POWER;
    DPRINT1(DEBUG_MESH, "  txPower: %d\n", meshcom_send_message.message.setup.txPower);

    meshcom_send_message.message.setup.nwkKey[15] = (uint8_t) (MESHCOM_SETTING_NWK_KEY_1 >> 24);
    meshcom_send_message.message.setup.nwkKey[14] = (uint8_t) (MESHCOM_SETTING_NWK_KEY_1 >> 16);
    meshcom_send_message.message.setup.nwkKey[13] = (uint8_t) (MESHCOM_SETTING_NWK_KEY_1 >> 8);
    meshcom_send_message.message.setup.nwkKey[12] = (uint8_t) (MESHCOM_SETTING_NWK_KEY_1);
    meshcom_send_message.message.setup.nwkKey[11] = (uint8_t) (MESHCOM_SETTING_NWK_KEY_2 >> 24);
    meshcom_send_message.message.setup.nwkKey[10] = (uint8_t) (MESHCOM_SETTING_NWK_KEY_2 >> 16);
    meshcom_send_message.message.setup.nwkKey[9] = (uint8_t) (MESHCOM_SETTING_NWK_KEY_2 >> 8);
    meshcom_send_message.message.setup.nwkKey[8] = (uint8_t) (MESHCOM_SETTING_NWK_KEY_2);
    meshcom_send_message.message.setup.nwkKey[7] = (uint8_t) (MESHCOM_SETTING_NWK_KEY_3 >> 24);
    meshcom_send_message.message.setup.nwkKey[6] = (uint8_t) (MESHCOM_SETTING_NWK_KEY_3 >> 16);
    meshcom_send_message.message.setup.nwkKey[5] = (uint8_t) (MESHCOM_SETTING_NWK_KEY_3 >> 8);
    meshcom_send_message.message.setup.nwkKey[4] = (uint8_t) (MESHCOM_SETTING_NWK_KEY_3);
    meshcom_send_message.message.setup.nwkKey[3] = (uint8_t) (MESHCOM_SETTING_NWK_KEY_4 >> 24);
    meshcom_send_message.message.setup.nwkKey[2] = (uint8_t) (MESHCOM_SETTING_NWK_KEY_4 >> 16);
    meshcom_send_message.message.setup.nwkKey[1] = (uint8_t) (MESHCOM_SETTING_NWK_KEY_4 >> 8);
    meshcom_send_message.message.setup.nwkKey[0] = (uint8_t) (MESHCOM_SETTING_NWK_KEY_4);

    DPRINT(DEBUG_MESH, "  nwkKey: 0x");
    for (int i = 15; i >= 0; i--)
        DPRINT1N(DEBUG_MESH, "%02X ", meshcom_send_message.message.setup.nwkKey[i]);
    DPRINTN(DEBUG_MESH, "\n");

    // Trust Center settings.
    meshcom_send_message.message.setup.trustCenterAddr = meshcom_send_message.message.setup.uid;  // set own address to not to use trust center at all //SETTINGS_MESH_UID_BASE;
    DPRINT2(DEBUG_MESH, "  TrustCenterAddr: 0x%08X %08X\n", (uint32_t)((meshcom_send_message.message.setup.trustCenterAddr >> 32) & 0xFFFFFFFFUL), (uint32_t)(meshcom_send_message.message.setup.trustCenterAddr & 0xFFFFFFFFUL));

    meshcomSendLocalMessage((uint8_t*) &meshcom_send_message, MESHCOM_MSG_HEADER_SIZE + meshcom_send_message.header.length);

    return MESHCOM_OK;
}


void meshcom_parse(MESHCOM_MSG_S *pMessage)
{
    //  static volatile MESHCOM_MSG_S * pMsg;

    //save timestamp for timeout detection
    TIMEOUT_Reset(MessageParseTimeout);

    //  pMsg = pMessage;

    //If major version does not match, return immediately to prevent errors.
    if (pMessage->header.version.major != MESHCOM_PROTOCOL_VERSION_MAJOR)
        return;

    switch (pMessage->header.port)
    {
    case MESHCOM_PORT_LOCAL:
        meshcom_parse_local_message(pMessage);
        break;
    case MESHCOM_PORT_BROADCAST:
        if (pMessage->header.id.major != MESHCOM_MSG_ID_RESPONSE)
            TIMEOUT_Reset(NetworkDataParseTimeout);
        meshcom_parse_broadcast_message(pMessage);
        break;
    case MESHCOM_PORT_UNICAST:
    case MESHCOM_PORT_UNICAST_NOACK:
        TIMEOUT_Reset(NetworkDataParseTimeout);
        meshcom_parse_unicast_message(pMessage);
        break;
    default:
        break;
    }

    meshcom_external_rx_parser(pMessage);
}


void meshcom_parse_local_message(MESHCOM_MSG_S *pMessage)
{
    switch (pMessage->header.id.major)
    {
    case MESHCOM_MSG_ID_COMMAND:
    {
        switch (pMessage->message.command.cmdid)
        {
        case MESHCOM_CMD_ACK:
            break;
        default:
            break;
        }
    }
    break;

    case MESHCOM_MSG_ID_SETUP:
    {

    }
    break;

    case MESHCOM_MSG_ID_STATUS:
    {
        _meshcom_node_status.softVersionMajor = pMessage->message.status.softVersionMajor;
        _meshcom_node_status.softVersionMinor = pMessage->message.status.softVersionMinor;
        _meshcom_node_status.nodeStatus       = pMessage->message.status.nodeStatus;
        _meshcom_node_status.nodeType         = pMessage->message.status.nodeType;
        _meshcom_node_status.workingChannel   = pMessage->message.status.workingChannel;
        _meshcom_node_status.shortPanId       = pMessage->message.status.shortPanId;
        _meshcom_node_status.extAddr          = pMessage->message.status.extAddr;
        _meshcom_node_status.shortAddr        = pMessage->message.status.shortAddr;
        _meshcom_node_status.parentShortAddr  = pMessage->message.status.parentShortAddr;
        _meshcom_node_status.lqi              = pMessage->message.status.lqi;
        _meshcom_node_status.rssi             = pMessage->message.status.rssi;
        _meshcom_node_status.status           = pMessage->message.status.status;

        DPRINT2(DEBUG_MESH, "MESHRADIO: Status: 0x%02X - 0x%02X\n",  _meshcom_node_status.nodeStatus, _meshcom_node_status.status);

        //Check Protocol Version
        if (pMessage->message.status.softVersionMajor != MESHCOM_PROTOCOL_VERSION_MAJOR)
            _meshcom_status.bProtocolVersionMismatch = TRUE;
    }
    break;
    case MESHCOM_MSG_ID_RESPONSE:
    {
        DPRINT3(DEBUG_MESH, "MESHCOM <RESPONSE Port: 0x%02x Response: %u Code: 0x%02x >\n",  pMessage->message.response.message_id, pMessage->message.response.response, pMessage->message.response.code);

        if (pMessage->message.response.response == MESHCOM_RESPONSE_MSG_ACK)
            _meshcom_status.uMessageNackCount = 0;
        else
            _meshcom_status.uMessageNackCount++;
    }
    break;
    default:
        break;
    }
}


void meshcom_parse_broadcast_message(MESHCOM_MSG_S *pMessage)
{
    switch (pMessage->header.id.major)
    {
    case MESHCOM_MSG_ID_BC_BEACON:
    {
        SDC_CLIENT_DATA_T new_data;

        _meshcom_status.bBroadcastReceived = TRUE;

        new_data.serial_number              = pMessage->message.bc_beacon.host.Serial;
        new_data.gloom.value                = pMessage->message.bc_beacon.sensors.gloom;
        new_data.gloom.sensor_state         = pMessage->message.bc_beacon.sensors.gloom_sensor_status;
        new_data.temperature.value          = pMessage->message.bc_beacon.sensors.temperature;
        new_data.temperature.sensor_state   = pMessage->message.bc_beacon.sensors.temperature_sensor_status;
        new_data.visibility.value           = pMessage->message.bc_beacon.sensors.visibility;
        new_data.visibility.sensor_status   = pMessage->message.bc_beacon.sensors.visibility_sensor_status;
        new_data.visibility.cluster_id      = pMessage->message.bc_beacon.sensors.visibility_cluster_id;

        SYNDICATION_Submit(&new_data);
        GLOOM_MANAGER_SetNetworkLedOnOffOpinion(pMessage->message.bc_beacon.opinions.led_on_off_opinion);

        _meshcom_status.uBroadcastMessagesReceivedCount++;
        _meshcom_status.uBroadcastMessageReceiveTimestamp = CLOCK_GetTimestamp();

        break;
    }

    case MESHCOM_MSG_ID_BC_NODEINFO:
    {
        //check if new firmware from coordinator is ready for distribution
        //only a coordinator is allowed to be the distribution source of new firmware
        if (pMessage->message.bc_nodeinfo.nodeinfo.nodeType == MESHCOM_NODE_TYPE_COORDINATOR)
        {
            // Make sure not to change Major Version by automatic update
            if ((pMessage->message.bc_nodeinfo.hostinfo.VersionMajor == VERSION_MAJOR)
                    && (pMessage->message.bc_nodeinfo.hostinfo.VersionMinor  > VERSION_MINOR))
            {
                MESHCOM_CMD_RequestFirmware(pMessage->message.bc_nodeinfo.nodeinfo.shortAddr, FWT_SELECTED_FW_MAIN);
            }
            else if ((pMessage->message.bc_nodeinfo.nodeinfo.softVersionMajor == _meshcom_node_status.softVersionMajor)
                     &&pMessage->message.bc_nodeinfo.nodeinfo.softVersionMinor  > _meshcom_node_status.softVersionMinor)
            {
                MESHCOM_CMD_RequestFirmware(pMessage->message.bc_nodeinfo.nodeinfo.shortAddr, FWT_SELECTED_FW_MESH);
            }
        }
        break;
    }

    case MESHCOM_MSG_ID_BC_COMMAND:
    {
        switch (pMessage->message.bc_command.command)
        {
        case MESHCOM_BC_COMMAND_BROADCAST_SHUTUP:
        {
            DPRINT1(DEBUG_MESH, "MESHCOM Received Broadcast ShutUp Message! Timeout = %u", pMessage->message.bc_command.parameter16);
            MESH_BroadcastShutUp(pMessage->message.bc_command.parameter16);
        }
        break;
        case MESHCOM_BC_COMMAND_LIGHTUP:
        {
            if (pMessage->message.bc_command.parameter16 == 0)
                break;
            if (pMessage->message.bc_command.parameter16 > MESHCOM_COMMAND_LED_LIGHTUP_INTERVAL_MAX)
                pMessage->message.bc_command.parameter16 = MESHCOM_COMMAND_LED_LIGHTUP_INTERVAL_MAX;
            SEQUENCE_TestDriveStart(pMessage->message.bc_command.parameter16);
        }
        break;

        case MESHCOM_BC_COMMAND_SWITCH_ON_OFF:
        {
            GLOOM_MANAGER_SetOnOffCommandReceivedState(pMessage->message.bc_command.parameter16,
                    pMessage->message.bc_command.parameter32);
        }
        break;
        default:
            break;
        }
    }
    break;

    case MESHCOM_MSG_ID_RESPONSE:
    {
        //DPRINT3(DEBUG_MESH, "MESHCOM <RESPONSE Port: 0x%02x Response: %u Code: 0x%02x >\n",  pMessage->message.response.message_id, pMessage->message.response.response, pMessage->message.response.code);

        if (pMessage->message.response.response == MESHCOM_RESPONSE_MSG_ACK)
            _meshcom_status.uMessageNackCount = 0;
        else
            _meshcom_status.uMessageNackCount++;

        if (pMessage->message.response.response == MESHCOM_RESPONSE_MSG_ACK)
            meshcomSendConfirm(TRUE);
        else
            meshcomSendConfirm(FALSE);
        break;
    }

    default:
        break;

    }
}


void meshcom_parse_unicast_message(MESHCOM_MSG_S *pMessage)
{
    // meshcom_packetdump((uint8_t*) pMessage, pMessage->header.length + MESHCOM_MSG_HEADER_SIZE);

    switch (pMessage->header.id.major)
    {
    case MESHCOM_MSG_ID_COMMANDSTREAM:
    {
        MESHCOM_CMD_CommandstreamReceive(pMessage);
    }
    break;

    case MESHCOM_MSG_ID_DATASTREAM:
    {
        MESHCOM_CMD_DatastreamReceive(pMessage);
    }
    break;

    case MESHCOM_MSG_ID_RESPONSE:
    {
        //DPRINT3(DEBUG_MESH, "MESHCOM <RESPONSE Port: 0x%02x Response: %u Code: 0x%02x >\n",  pMessage->message.response.message_id, pMessage->message.response.response, pMessage->message.response.code);

        if (pMessage->message.response.response == MESHCOM_RESPONSE_MSG_ACK)
            _meshcom_status.uMessageNackCount = 0;
        else
            _meshcom_status.uMessageNackCount++;

        if (pMessage->message.response.response == MESHCOM_RESPONSE_MSG_ACK)
            meshcomSendConfirm(TRUE);
        else
            meshcomSendConfirm(FALSE);
    }
    break;

    default:
        asm("nop");
        break;
    }
}

void meshcom_external_rx_parser(MESHCOM_MSG_S *pMsg)
{
    for (uint8_t mpidx = 0; mpidx < MESHCOM_MESSAGE_PARSER_COUNT; mpidx++)
    {
        if (externalRxMessageParser[mpidx] != NULL)
            (externalRxMessageParser[mpidx])(pMsg);
    }
}

void meshcom_external_tx_parser(MESHCOM_MSG_S *pMsg)
{
    for (uint8_t mpidx = 0; mpidx < MESHCOM_MESSAGE_PARSER_COUNT; mpidx++)
    {
        if (externalTxMessageParser[mpidx] != NULL)
            (externalTxMessageParser[mpidx])(pMsg);
    }
}

void meshcom_register_message_parser(MESHCOM_MESSAGE_PARSER *parser, MESHCOM_MESSAGE_PARSER **library)
{
    //check if parser is already registered. return if it is so.
    for (uint8_t mpidx = 0; mpidx < MESHCOM_MESSAGE_PARSER_COUNT; mpidx++)
    {
        if (library[mpidx] == parser)
            return;
    }

    // ..otherwise find a free spot
    for (uint8_t mpidx = 0; mpidx < MESHCOM_MESSAGE_PARSER_COUNT; mpidx++)
    {
        if (library[mpidx] == NULL)
        {
            library[mpidx] = parser;
            return;
        }
    }
}

void meshcom_unregister_message_parser(MESHCOM_MESSAGE_PARSER *parser, MESHCOM_MESSAGE_PARSER **library)
{
    //if parser is registered remove it.
    for (uint8_t mpidx = 0; mpidx < MESHCOM_MESSAGE_PARSER_COUNT; mpidx++)
    {
        if (library[mpidx] == parser)
            library[mpidx] = NULL;
    }
}

void MESHCOM_RegisterRxMessageParser(MESHCOM_MESSAGE_PARSER *parser)
{
    meshcom_register_message_parser(parser, externalRxMessageParser);
}

void MESHCOM_RegisterTxMessageParser(MESHCOM_MESSAGE_PARSER *parser)
{
    meshcom_register_message_parser(parser, externalTxMessageParser);
}

void MESHCOM_UnregisterRxMessageParser(MESHCOM_MESSAGE_PARSER *parser)
{
    meshcom_unregister_message_parser(parser, externalRxMessageParser);
}

void MESHCOM_UnregisterTxMessageParser(MESHCOM_MESSAGE_PARSER *parser)
{
    meshcom_unregister_message_parser(parser, externalTxMessageParser);
}

void MESHCOM_NetworkStateActions(void)
{
    static LOG_DUPVAR_T node_status_logdupvar = 0;

    // actions on node status change
    if (old_meshcom_state != _meshcom_node_status.nodeStatus)
    {
        DPRINT2(DEBUG_MESH, "MESHCOM_NetworkStateActions switching from state %u to %u\n", old_meshcom_state, _meshcom_node_status.nodeStatus);
        // manage actions on leaving of old state
        switch (old_meshcom_state)
        {
        case MESHCOM_NODE_STATUS_IN_NETWORK_STATE:
            // We have left the network. Stop all Broadcasts.
            MESHCOM_BC_Stop();
            break;
        default:
            break;
        }

        // manage actions on entering new state
        switch (_meshcom_node_status.nodeStatus)
        {
        case MESHCOM_NODE_STATUS_INITING_STATE:
            // state changed to initing state
            LOG(LOG_ID_MESHCOM_MESH_INIT_STATUS, 0, 0, 0, &node_status_logdupvar);
            break;
        case MESHCOM_NODE_STATUS_CONFIG_STATE:
            MESHCOM_SendSetup();
            break;
        case MESHCOM_NODE_STATUS_STARTING_NETWORK_STATE:
            break;
        case MESHCOM_NODE_STATUS_IN_NETWORK_STATE:
            LOG(LOG_ID_MESHCOM_MESH_IN_NETWORK_STATUS, 0, 0, 0, &node_status_logdupvar);
            systemconfig_update_meshnode();
            SOUND_PlayMelody(MESH_INNWK_MELODY);
            _network.type = NWK_TYPE_MESH;
            _network.id = (NWK_NETWORK_ID_T)_meshcom_node_status.shortAddr;
            _meshcom_status.bNetworkFailed = FALSE;  // reset fail flag
            MESHCOM_BC_Start();
            break;
        case MESHCOM_NODE_STATUS_LEAVING_NETWORK_STATE:
            break;
        case MESHCOM_NODE_STATUS_STOP_STATE:
            break;
        case MESHCOM_NODE_STATUS_UNKNOWN:
            break;
        default:
            break;
        }

        old_meshcom_state = _meshcom_node_status.nodeStatus;
    }
}


void meshcomCheckTimeouts(void)
{
    if (TIMEOUT_Check(MessageParseTimeout, MESHCOM_MESSAGE_PARSE_TIMEOUT_SEC))
    {
        TIMEOUT_Reset(MessageParseTimeout);
        _meshcom_status.bNetworkFailed = TRUE;
#if !(VERSION_IS_DEBUG > 0)
        MESH_ResetModule();
#endif
        if (MessageParseErrorCount < UINT8_T_MAX)
            MessageParseErrorCount++;
    }

    if (TIMEOUT_Check(NetworkDataParseTimeout, MESHCOM_NETWORKDATA_PARSE_TIMEOUT_SEC))
    {
        TIMEOUT_Reset(NetworkDataParseTimeout);
        _meshcom_status.bNetworkFailed = TRUE;
#if !(VERSION_IS_DEBUG > 0)
        MESH_ResetModule();
#endif
        if (NetworkDataParseErrorCount < UINT8_T_MAX)
            NetworkDataParseErrorCount++;
    }

    /* todo[high] check, increment error counter if device is in network seems to be wrong!
        if (_meshcom_node_status.nodeStatus == MESHCOM_NODE_STATUS_IN_NETWORK_STATE)
        {
            TIMEOUT_Reset(NetworkTimeout);
            if (NetworkErrorCount < UINT8_T_MAX)
                NetworkErrorCount++;
        }
        else if (TIMEOUT_Check(NetworkTimeout, MESHCOM_NETWORK_TIMEOUT_S))
        {
            TIMEOUT_Reset(NetworkTimeout);
            MESH_ResetModule();
        }
    */
}

void meshcomMeshErrorhandling(void)
{
    static TIMEOUT_T mesh_last_power_cycle_time_stamp = 0;

//#define MESH_NETWORK_TIMEOUT_S            180 todo[medium] use or remove
#define MESH_NETWORK_FAIL_COUNT_MAX       20
//#define MESH_NETWORK_NACK_COUNT_MAX       30 todo[medium] use or remove
//#define MESH_FW_UPGRADE_RETRY_COUNT_MAX   10 todo[medium] use or remove
#define MESHCOM_MESH_POWER_CYCLE_TIMEOUT_SEC (24 * 60 * 60)  // max one power cycle per day

    if ((MessageParseErrorCount > MESH_NETWORK_FAIL_COUNT_MAX)
            || (NetworkDataParseErrorCount > MESH_NETWORK_FAIL_COUNT_MAX)
            || (NetworkErrorCount > MESH_NETWORK_FAIL_COUNT_MAX))
    {
        if ((mesh_last_power_cycle_time_stamp == 0) || TIMEOUT_Check(mesh_last_power_cycle_time_stamp, MESHCOM_MESH_POWER_CYCLE_TIMEOUT_SEC))
        {
            TIMEOUT_Reset(mesh_last_power_cycle_time_stamp);
            // reset all error counter
            MessageParseErrorCount = 0;
            NetworkDataParseErrorCount = 0;
            NetworkErrorCount = 0;
#if !(VERSION_IS_DEBUG > 0)
            MESH_PowerToggleModule();
#endif
        }
    }
}


void MESHCOM_Task(void *pvParameters)
{
    MESHCOM_MSG_S *pRecvMessage;

    xQueueMeshComMessageOutput = xQueueCreate(10, sizeof(MeshcomMessage_t));
    DQueueAddToRegistry(xQueueMeshComMessageOutput, "xQueueMeshComMessageOutput");
    xQueueMeshComMessageOutputCallback = xQueueCreate(1, sizeof(MeshcomMessageCallback_t));
    DQueueAddToRegistry(xQueueMeshComMessageOutputCallback, "xQueueMeshComMessageOutputCallback");

    MESHCOM_Init();

    while (1)
    {
        WATCHDOG_Heartbeat(WATCHDOG_TASK_ID_MESH);

        if (bMeshComEnabled)
        {
            while(MESHCOM_UART_CHAR_AVAIL())
                if ((pRecvMessage = MESHCOM_IO_Receive(MESHCOM_UART_RECV())) != NULL)
                    meshcom_parse(pRecvMessage);

            MESHCOM_CMD_Tick();
            MESHCOM_BC_Tick();

            MESHCOM_NetworkStateActions();

            meshcomCheckTimeouts();
            meshcomMeshErrorhandling();

            meshcomSendConfirmCheckTimeout();
            meshcomSendNextMessage();
        }

        vTaskDelay(1);
    }
}


bool MESH_NetworkingIsAvailable(void)
{
    if (MESH_SETTING_USE > 0)
    {
        if (_meshcom_status.bNetworkFailed == TRUE)
            return FALSE;
        else
            return TRUE;
    }

    return FALSE;
}


void MESHCOM_UnInit(void)
{
    bMeshComEnabled = FALSE;
}

void meshcom_init(void)
{
    _meshcom_status.bProtocolVersionMismatch = FALSE;
    _meshcom_status.uMessageNackCount = 0;
    _meshcom_status.uBroadcastMessagesReceivedCount = 0;

    _meshcom_node_status.nodeStatus = MESHCOM_NODE_STATUS_UNKNOWN;
    _meshcom_node_status.nodeType = MESHCOM_NODE_TYPE_UNKNOWN;
    _meshcom_node_status.workingChannel = 0;
    _meshcom_node_status.shortPanId = 0;
    _meshcom_node_status.extAddr = 0;
    _meshcom_node_status.shortAddr = 0;
    _meshcom_node_status.parentShortAddr = 0;
    _meshcom_node_status.lqi = 0;
    _meshcom_node_status.rssi = 0;

    MESHCOM_CMD_Init();
    MESHCOM_BC_Init();

    TIMEOUT_Reset(MessageParseTimeout);
    TIMEOUT_Reset(NetworkDataParseTimeout);

    if (MESH_SETTING_USE == 1)
        bMeshComEnabled = TRUE;
    else
        bMeshComEnabled = FALSE;
}

void MESHCOM_SettingsUpdated(SETTINGS_GROUP_T group_id)
{
    //settings have been altered - reinit mesh
    MESH_Init();
    meshcom_init();
}


void MESHCOM_Init(void)
{
    SETTINGS_GROUP_CHANGE_CALLBACK_T cbStruct;
    cbStruct.pCallbackFunc = MESHCOM_SettingsUpdated;
    cbStruct.uGroupMask = SETTINGS_GROUP_MESH | SETTINGS_GROUP_SYSTEM;
    SETTINGS_RegisterGroupChangeCallback(&cbStruct);

    UART_ChannelInitTypeDef UART_ChannelInitStruct;

    UART_ChannelInitStruct.baudrate = 115200;
    UART_ChannelInitStruct.ctrl_flags = UART_CTRL_8N1;
    UART_ChannelInitStruct.rx_callback_func = NULL;

    UART_ChannelInit(MESHCOM_UART_CHANNEL, &UART_ChannelInitStruct);

    meshcom_init();
}

