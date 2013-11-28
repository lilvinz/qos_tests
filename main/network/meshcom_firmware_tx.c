
#include "stdheader.h"
#include "meshcom_firmware_tx.h"

#include "state.h"
#include "image_information.h"
#include "firmware_store.h"
#include "mesh.h"
#include "meshcom_cmd.h"
#include "debug.h"
#include "log.h"

static bool bBcSend = FALSE;

static uint16_t uFwTxShortAddr = 0;
static FWT_SELECTED_FW_T FwTxType = 0;
static uint32_t uFwTxSerial = 0;

static uint32_t uFwTxFileLength = 0;
static uint8_t  uTxLen = 0;
static uint32_t uFwTxAddress = 0;

static uint8_t  uFwTxRetryCounter = 0;

static STATE_T  sFwTxState;
static uint8_t  uFwTxResponse = 0;
static uint8_t  uFwTxResponseId = 0;

static IMAGE_INFORMATION_T *psFileInfo = NULL;

struct
{
    bool                        bUpdated;
    DATASTREAM_RESPONSE_ENUM_T  eResponse;
    uint32_t                    uSequenceId;
} sDataStreamResponse;


void MESHCOM_FIRMWARE_TX_MessageParser(MESHCOM_MSG_S *pMessage);


bool MESHCOM_FIRMWARE_TX_Request(uint16_t sAddr, FWT_SELECTED_FW_T FwType, uint32_t uSerial)
{
    DPRINT3(DEBUG_MESH, "FWTX: firmware_tx_request FwType %u from Serial %08u (sAddr 0x%04X) ", FwType, uSerial, sAddr);

    if (sFwTxState.current != FWTX_IDLE)
    {
        DPRINT(DEBUG_MESH, "REJECTED: !FWTX_IDLE\n");
        return FALSE;
    }

    DPRINT(DEBUG_MESH, "ACCEPTED\n");

    uFwTxShortAddr  = sAddr;
    FwTxType        = FwType;
    uFwTxSerial     = uSerial;

    sFwTxState.current = FWTX_START;
    return TRUE;
}


void MESHCOM_FIRMWARE_TX_Abort(void)
{
    DPRINT(DEBUG_MESH, "FWTX: firmware_tx_abort\n");
    state_init(&sFwTxState);
}


void MESHCOM_FIRMWARE_TX_Confirm(MESHCOM_RESULT_T result, uint8_t sequence_id)
{
    if (result == MESHCOM_OK)
    {
        DPRINT(DEBUG_MESH, "FWTX_ACK\n");
        uFwTxResponse = FWTX_PACKET_ACK;
    }
    else
    {
        DPRINT(DEBUG_MESH, "FWTX_NACK\n");
        uFwTxResponse = FWTX_PACKET_NACK;
    }
    uFwTxResponseId = sequence_id;
}


void MESHCOM_FIRMWARE_TX_MessageParser(MESHCOM_MSG_S *pMessage)
{
    if (pMessage->header.port == MESHCOM_PORT_UNICAST || pMessage->header.port == MESHCOM_PORT_UNICAST_NOACK)
    {
        if (pMessage->header.id.major == MESHCOM_MSG_ID_DATASTREAM && pMessage->header.id.minor == MESHCOM_COMMANDSTREAM_ID_ACK)
        {
            MESHCOM_DATASTREAM_T *pDataStream = (MESHCOM_DATASTREAM_T*)&pMessage->message.stream.payload;
            sDataStreamResponse.bUpdated = TRUE;
            sDataStreamResponse.eResponse = pDataStream->data.response.eResponse;
            sDataStreamResponse.uSequenceId = pDataStream->header.sequence_id;
        }
    }
}

void firmware_bc_callback(MESHCOM_RESULT_T ack, uint8_t seq_id)
{
    ack = ack;
    bBcSend = TRUE;
}


bool firmware_tx_SendCommandStart(void)
{
    MESHCOM_MSG_S message;

    memset(&message, 0, sizeof(MESHCOM_MSG_S));


    MESSAGE_HEADER_T* stream_params = &message.header;
    MESHCOM_COMMANDSTREAM_T* commandstream = (MESHCOM_COMMANDSTREAM_T *) &message.message.stream.payload;
    COMMANDSTREAM_FIRMWARE_HEADER_T* firmwareinfo_out = (COMMANDSTREAM_FIRMWARE_HEADER_T *) &commandstream->data.firmware_header;

    DPRINT(DEBUG_MESH, "FWTX: firmware_tx_SendCommandStart\n");

    stream_params->address.extAddr      = 0;
    stream_params->address.shortAddr    = uFwTxShortAddr;
    stream_params->id.minor             = MESHCOM_COMMANDSTREAM_ID_FIRMWARE_TX_INIT;
    stream_params->length               = sizeof(COMMANDSTREAM_FIRMWARE_HEADER_T);

    firmwareinfo_out->fw_type           = FwTxType;
    firmwareinfo_out->hw_id             = psFileInfo->hw_id;
    firmwareinfo_out->hw_revision       = psFileInfo->hw_revision;
    firmwareinfo_out->sw_version_major  = psFileInfo->sw_version_major;
    firmwareinfo_out->sw_version_minor  = psFileInfo->sw_version_minor;
    firmwareinfo_out->size              = psFileInfo->size;
    firmwareinfo_out->checksum          = psFileInfo->checksum;

    uFwTxResponse = FWTX_PACKET_RESPONSE_AWAITING;
    if (MESHCOM_PENDING >= MESHCOM_SendCommandstream(&message, MESHCOM_FIRMWARE_TX_Confirm, 0))
        return TRUE;

    return FALSE;
}


uint8_t firmware_tx_SendData(uint32_t uAddress, uint8_t uLen)
{
    MESHCOM_MSG_S message;

    memset(&message, 0, sizeof(MESHCOM_MSG_S));

    int8_t iReadResult;
    MESSAGE_HEADER_T* stream_params = &message.header;
    MESHCOM_DATASTREAM_T * datastream_out_payload = (MESHCOM_DATASTREAM_T *) message.message.stream.payload;
    DATASTREAM_FIRMWARE_T* firmware_out = (DATASTREAM_FIRMWARE_T *) &datastream_out_payload->data.firmware;

    stream_params->address.extAddr = 0;
    stream_params->address.shortAddr = uFwTxShortAddr;


    iReadResult = FISTO_Read(uAddress, uLen, firmware_out->aData);

    if (iReadResult > 0)
    {
        stream_params->id.minor = MESHCOM_DATASTREAM_ID_FIRMWARE;
        stream_params->length   = iReadResult + (MESHCOM_STREAM_PAYLOAD_MAX - DATASTREAM_FIRMWARE_DATA_LEN_MAX);
        datastream_out_payload->header.sequence_id = uAddress;
        firmware_out->uAddress  = uAddress;
        firmware_out->uLength   = iReadResult;

        DPRINT3(DEBUG_MESH, "FWTX: Send A:%u, L%u to 0x%04X\n", firmware_out->uAddress, firmware_out->uLength, uFwTxShortAddr);

        if (MESHCOM_PENDING >= MESHCOM_SendDatastream(&message, MESHCOM_PORT_UNICAST, MESHCOM_FIRMWARE_TX_Confirm, (uint8_t)uAddress))
            uFwTxResponse = FWTX_PACKET_RESPONSE_AWAITING;
        else
            uFwTxResponse = FWTX_PACKET_NO_RESPONSE;

        return (uint8_t)iReadResult;
    }
    else
        return 0;
}


void MESHCOM_FIRMWARE_TX_Tick(void)
{
//    static uint8_t uLastState;

    if (FWTX_SETTING_TX_MODE == SETTINGS_MESH_FWTX_MODE_OFF)
        return;
    /*
        if (uLastState != sFwTxState.current)
        {
            DPRINT1(DEBUG_MESH, "New State: %u", sFwTxState.current);
            uLastState = sFwTxState.current;
        }
    */
    if (uFwTxRetryCounter > FWTX_RETRY_COUNTER_MAX)
        state_set(&sFwTxState, FWTX_FAILED);

    // switch off beacon broadcasts if present.
    if (FWTX_SETTING_TX_MODE == SETTINGS_MESH_FWTX_MODE_ON_NOBC)
    {
        if ((sFwTxState.current > FWTX_START) && (_meshcom_status.bBroadcastReceived == TRUE))
        {
            _meshcom_status.bBroadcastReceived = FALSE;
            state_push(&sFwTxState, sFwTxState.current);
            sFwTxState.current = FWTX_STOP_BEACONS;
        }
    }

    switch (sFwTxState.current)
    {
    case FWTX_IDLE:
        if (state_is_new(&sFwTxState))
        {
            uFwTxResponse       = FWTX_PACKET_NO_RESPONSE;
            uFwTxRetryCounter   = 0;
        }
        break;

    case FWTX_STOP_BEACONS:             // intermediate state. pushed last state in new state and pops it back if done
        if (state_is_new(&sFwTxState))
        {
            DPRINT(DEBUG_MESH, "FWTX: meshcom_conn_send_broadcast_shutup\n");
            MESHCOM_SendBroadcastShutup(FWTX_BEACON_BC_SHUTUP_TIME, firmware_bc_callback);
            MESH_BroadcastShutUp(FWTX_BEACON_BC_SHUTUP_TIME);
        }

        if (bBcSend || state_timeout(&sFwTxState, FWTX_STOP_BEACON_TIMEOUT))
        {
            bBcSend = FALSE;
            sFwTxState.current = state_pop(&sFwTxState);
        }
        break;

    case FWTX_START:
        // check if local file is valid. go back to idle if not.
        if (TRUE == FISTO_Open(FwTxType))
        {
            DPRINT(DEBUG_MESH, "FWTX: fisto_open() successfull\n");
            sFwTxState.current = FWTX_INIT_TX;
        }
        else
        {
            DPRINT(DEBUG_MESH, "Fisto File is not valid!\n");
            state_init(&sFwTxState);
        }
        break;

    case FWTX_INIT_TX:
        if (state_is_new(&sFwTxState))
        {
            LOG(LOG_ID_MESHCOM_SENDING_FIRMWARE, uFwTxShortAddr, uFwTxSerial, 0, NULL);
            FISTO_GetFileInfo(&psFileInfo);
            // ToDo[medium] make sure that firmware from file is the same as installed! Otherwise stop transmission!
            uFwTxFileLength = psFileInfo->size;
            uTxLen      = DATASTREAM_FIRMWARE_DATA_LEN_MAX;
            uFwTxAddress    = 0;
            uFwTxRetryCounter = 0;
            firmware_tx_SendCommandStart();
        }

        if (uFwTxResponse == FWTX_PACKET_ACK)
        {
            if (state_timeout(&sFwTxState, FWTX_START_TIMEOUT))
            {
                sFwTxState.current = FWTX_SEND;
                break;
            }
        }
        else if (state_timeout(&sFwTxState, FWTX_RESEND_TIMEOUT))
        {
            uFwTxRetryCounter++;
            state_reset_timer(&sFwTxState);
            firmware_tx_SendCommandStart();
        }
        break;

    case FWTX_NEXT:
        uFwTxRetryCounter = 0;
    case FWTX_RESEND:
        uFwTxRetryCounter++;
    case FWTX_SEND:
        if (state_is_new(&sFwTxState))
        {
        }
        uTxLen = firmware_tx_SendData(uFwTxAddress, uTxLen);
        sFwTxState.current = FWTX_WAIT;
        break;

    case FWTX_WAIT:
        if (state_is_new(&sFwTxState))
        {

        }

        if (sDataStreamResponse.bUpdated == TRUE)
        {
            sDataStreamResponse.bUpdated = FALSE;
            if (sDataStreamResponse.eResponse == MESHCOM_CMD_RESPONSE_ACK && sDataStreamResponse.uSequenceId == uFwTxAddress)
            {
                DPRINT(DEBUG_MESH, "FWTX: ACK\n");
                uFwTxAddress += uTxLen;

                sFwTxState.current = FWTX_NEXT;

                if (uFwTxAddress + DATASTREAM_FIRMWARE_DATA_LEN_MAX < uFwTxFileLength)
                    uTxLen = DATASTREAM_FIRMWARE_DATA_LEN_MAX;
                else if (uFwTxFileLength > uFwTxAddress)
                    uTxLen = uFwTxFileLength - uFwTxAddress;
                else
                    sFwTxState.current = FWTX_IDLE;
            }
            else if (sDataStreamResponse.eResponse == MESHCOM_CMD_RESPONSE_NACK)
            {
                DPRINT(DEBUG_MESH, "FWTX: NACK\n");
                sFwTxState.current = FWTX_RESEND;
            }
            else if (sDataStreamResponse.eResponse == MESHCOM_CMD_RESPONSE_ABORT)
            {
                DPRINT(DEBUG_MESH, "FWTX: *** ABORT ***\n");
                sFwTxState.current = FWTX_IDLE;
            }
        }

        if (state_timeout(&sFwTxState, FWTX_WAIT_TIMEOUT))
            sFwTxState.current = FWTX_RESEND;
        break;

    case FWTX_FAILED:
    default:
        DPRINT(DEBUG_MESH, "FWTX: state_init(&sFwTxState)\n");
        uFwTxRetryCounter   = 0;

        // Reinit State Machine!
        state_init(&sFwTxState);
        break;
    }
}

void MESHCOM_FIRMWARE_TX_Init(void)
{
    state_init(&sFwTxState);
    MESHCOM_RegisterRxMessageParser(MESHCOM_FIRMWARE_TX_MessageParser);
}
