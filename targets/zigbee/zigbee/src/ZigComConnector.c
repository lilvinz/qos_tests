/*
 * ZigComConnector.c
 *
 *  Created on: 30.12.2008
 *      Author: Administrator
 */

#include <types.h>

#include <configServer.h>
#include <appFramework.h>
#include <resetReason.h>

#include "ZigComApp.h"
#include "ZigComNode.h"
#include "ZigComConnector.h"
#include "ZigComUARTManager.h"

static bool hostLostTimerStarted;
HAL_AppTimer_t hostLostTimer;
static uint8_t	hostLostEventCounter;

static appNodeSendBroadcastMessage_t sendBroadcastMessageParams;
static appNodeSendUnicastMessage_t sendUnicastMessageParams;

static DeviceType_t device_type;
static ZIGCOM_SHORT_MSG_S *pShortMessage;
static bool	bMessageSendResponse;
static uint8_t	uCode;

/** LOCAL FUNCTION PROTOTYPES **/
void appConnectorAckUartMessage(uint8_t message_id, bool state);
void appHostLostTimerFired(void);
void appConnectorSendVersion(void);


void appConnectorCommandAckReceived(uint8_t command_id)
{
    switch (command_id)
    {
    default:
        break;
    }
}

void appConnectorDecodeUARTMessage(ZIGCOM_MSG_S* pMessage)
{
    ZIGCOM_MSG_S response_message;

    // If we've received a message from host then host is not in BLF nor dead.
    hostLostEventCounter = 0;

    bMessageSendResponse = TRUE;			//Do we have to send a response at all?
    response_message.response.response      = ZIGCOM_RESPONSE_MSG_NACK;	//Response to received uart message
    response_message.response.message_id    = pMessage->header.id.major;
    uCode = 0;

    if (pMessage->header.version.major == ZIGCOM_PROTOCOL_VERSION_MAJOR)
    {
        switch (pMessage->header.port)
        {
        case ZIGCOM_PORT_LOCAL:
        {
            switch (pMessage->header.id.major)
            {
            case ZIGCOM_MSG_ID_COMMAND:
            {
                switch (pMessage->command.cmdid)
                {
                case ZIGCOM_CMD_RESET:
                    HAL_WarmReset();
                    break;
                case ZIGCOM_CMD_JOIN:
                    break;
                case ZIGCOM_CMD_LEAVE:
                    break;
                case ZIGCOM_CMD_ACK:
                    appConnectorCommandAckReceived(pMessage->command.cmdid);
                    break;
                case ZIGCOM_CMD_VERSION:
                    appConnectorSendVersion();
                    break;
                default:
                    break;
                }
                response_message.response.message_id  = ZIGCOM_MSG_ID_COMMAND;
                response_message.response.response    = ZIGCOM_RESPONSE_MSG_ACK;
                response_message.response.code        = 0;
                appConnectorSendResponse(ZIGCOM_PORT_LOCAL, &response_message);
            }
            break;

            case ZIGCOM_MSG_ID_SETUP:
            {
                switch (pMessage->setup.nodeType)
                {
                case ZIGCOM_NODE_TYPE_COORDINATOR:
                    device_type = DEVICE_TYPE_COORDINATOR;
                    break;
                case ZIGCOM_NODE_TYPE_ROUTER:
                    device_type = DEVICE_TYPE_ROUTER;
                    break;
                case ZIGCOM_NODE_TYPE_END_DEVICE:
                    device_type = DEVICE_TYPE_END_DEVICE;
                    break;
                default:
                    device_type = DEVICE_TYPE_ROUTER;
                    break;
                }

                CS_WriteParameter(CS_DEVICE_TYPE_ID,    &device_type);

                uint8_t channel_page = pMessage->setup.channelPage;
                CS_WriteParameter(CS_CHANNEL_PAGE_ID,	(void*)&channel_page);
                uint32_t channel_mask = pMessage->setup.channelMask;
                CS_WriteParameter(CS_CHANNEL_MASK_ID,	(void*)&channel_mask);

                // Setup PAN ID
                uint64_t ext_panid = pMessage->setup.extPanId;
                CS_WriteParameter(CS_EXT_PANID_ID,	(void*)&ext_panid);
                {
                    bool predefPANID;
                    uint16_t nwkPANID;

                    if ((ext_panid & 0xFFFF0000ULL) == 0ULL)
                    {
                        // if the upper 2Bytes of the lower 4Bytes PANID is 0 we'll use the lower part as the predefined PANID
                        predefPANID = TRUE;
                        CS_WriteParameter(CS_NWK_PREDEFINED_PANID_ID, (void*)&predefPANID);
                        nwkPANID	= (uint16_t)(ext_panid & 0xFFFFUL);
                        CS_WriteParameter(CS_NWK_PANID_ID, (void*)&nwkPANID);
                    }
                    else
                    {
                        // create the network dynamically with the extended PANID
                        predefPANID = FALSE;
                        CS_WriteParameter(CS_NWK_PREDEFINED_PANID_ID, (void*)&predefPANID);
                    }
                }

                uint64_t uid = pMessage->setup.uid;
                CS_WriteParameter(CS_UID_ID, (void*)&uid);

                int8_t rf_tx_power = pMessage->setup.txPower;
                CS_WriteParameter(CS_RF_TX_POWER_ID, (void*)&rf_tx_power);

#ifdef _SECURITY_
                CS_WriteParameter(CS_NETWORK_KEY_ID, (void*)pMessage->setup.nwkKey);

                uint64_t tcAddr = pMessage->setup.trustCenterAddr;
                CS_WriteParameter(CS_APS_TRUST_CENTER_ADDRESS_ID, (void*)&tcAddr);
    #ifdef _LINK_SECURITY_
        #warning(Info: Link security enabled)
    #else
        #warning(Info: Standard security enabled)
    #endif
#else
    #warning(Warn: Security disabled!!!)
#endif

                response_message.response.message_id  = ZIGCOM_MSG_ID_SETUP;
                response_message.response.response    = ZIGCOM_RESPONSE_MSG_ACK;
                response_message.response.code        = 0;
                appConnectorSendResponse(ZIGCOM_PORT_LOCAL, &response_message);

                appConfigReceived();	//Notify MainAp.
            }
            break;

            case ZIGCOM_MSG_ID_STATUS:
            {
                response_message.response.message_id  = ZIGCOM_MSG_ID_STATUS;
                response_message.response.response    = ZIGCOM_RESPONSE_MSG_ACK;
                response_message.response.code        = 0;
                appConnectorSendResponse(ZIGCOM_PORT_LOCAL, &response_message);
            }
            break;

            case ZIGCOM_MSG_ID_RESPONSE:
            {
            }
            break;

            default:
                break;
            }
        }
        break;

        case ZIGCOM_PORT_BROADCAST:
        {
            if (pMessage->header.id.major == ZIGCOM_MSG_ID_BC_NODEINFO)
            {
                // fill node info section.
                pMessage->beacon_bc.nodeinfo.boardType			= SYSTEM_BOARD_TYPE;
                pMessage->beacon_bc.nodeinfo.softVersionMajor	= SYSTEM_VERSION_MAJOR;
                pMessage->beacon_bc.nodeinfo.softVersionMinor	= SYSTEM_VERSION_MINOR;
                pMessage->beacon_bc.nodeinfo.nodeType			= appNodeInfo.DeviceType;
                pMessage->beacon_bc.nodeinfo.shortAddr 			= appNodeInfo.ShortAddr;
                pMessage->beacon_bc.nodeinfo.parentShortAddr	= appNodeInfo.ParentAddr;
                pMessage->beacon_bc.nodeinfo.lqi				= appNodeInfo.ParentLQI;
                pMessage->beacon_bc.nodeinfo.rssi				= appNodeInfo.ParentRSSI;
                pMessage->beacon_bc.nodeinfo.hostState			= ZIGCOM_STATUS_OK;
            }

            pShortMessage = (ZIGCOM_SHORT_MSG_S*)((uint8_t*)&pMessage->payload - ZIGCOM_MSG_SHORTHEADER_SIZE);
            pShortMessage->header.length 		= pMessage->header.length;
            pShortMessage->header.id.minor 		= pMessage->header.id.minor;
            pShortMessage->header.id.major		= pMessage->header.id.major;
            pShortMessage->header.port 			= pMessage->header.port;
            pShortMessage->header.version.minor	= pMessage->header.version.minor;
            pShortMessage->header.version.major	= pMessage->header.version.major;

            sendBroadcastMessageParams.message_id   = pMessage->header.id.major;
            sendBroadcastMessageParams.length       = (uint8_t)(pMessage->header.length + ZIGCOM_MSG_SHORTHEADER_SIZE);
            sendBroadcastMessageParams.payload      = (uint8_t*)pShortMessage;

            // send message via broadcast zigbee transmission.
            uCode = appNodeSendBroadcastMessage(&sendBroadcastMessageParams);
            if (uCode != MSG_OK)
            {
                response_message.response.message_id = pMessage->header.id.major;
                response_message.response.response = ZIGCOM_RESPONSE_MSG_STATUS;
                response_message.response.code = uCode;
                appConnectorSendResponse(ZIGCOM_PORT_BROADCAST, &response_message);
            }
        }
        break;

        case ZIGCOM_PORT_UNICAST:
        case ZIGCOM_PORT_UNICAST_NOACK:
        {
            if (pMessage->header.length > ZIGCOM_STREAM_PAYLOAD_LEN)
            {
                response_message.response.message_id  = pMessage->header.id.major;
                response_message.response.response    = ZIGCOM_RESPONSE_MSG_STATUS;
                response_message.response.code        = MSG_ERR_SIZE;
                appConnectorSendResponse(ZIGCOM_PORT_UNICAST, &response_message);
                break;
            }

            sendUnicastMessageParams.message_id     = pMessage->header.id.major;
            sendUnicastMessageParams.dstExtAddr     = pMessage->address.extAddr;
            sendUnicastMessageParams.dstShortAddr   = pMessage->address.shortAddr;

            if (pMessage->header.port == ZIGCOM_PORT_UNICAST)
                sendUnicastMessageParams.ack = 1;
            else
                sendUnicastMessageParams.ack = 0;

            pShortMessage = (ZIGCOM_SHORT_MSG_S*)((uint8_t*)&pMessage->payload - ZIGCOM_MSG_SHORTHEADER_SIZE);
            pShortMessage->header.length 		= pMessage->header.length;
            pShortMessage->header.id.minor 		= pMessage->header.id.minor;
            pShortMessage->header.id.major		= pMessage->header.id.major;
            pShortMessage->header.port 			= pMessage->header.port;
            pShortMessage->header.version.minor	= pMessage->header.version.minor;
            pShortMessage->header.version.major	= pMessage->header.version.major;

            sendUnicastMessageParams.length     = (uint8_t)(pMessage->header.length + ZIGCOM_MSG_SHORTHEADER_SIZE);
            sendUnicastMessageParams.payload    = (uint8_t*)pShortMessage;

            // send message via unicast zigbee transmission.
            uCode = appNodeSendUnicastMessage(&sendUnicastMessageParams);
            if (uCode != MSG_OK)
            {
                response_message.response.message_id  = pMessage->header.id.major;
                response_message.response.response    = ZIGCOM_RESPONSE_MSG_STATUS;
                response_message.response.code        = uCode;
                appConnectorSendResponse(ZIGCOM_PORT_UNICAST, &response_message);
            }
        }
        break;
        default:
            asm("nop");
            break;
        }
    } //end if (pMessage->version.major == VERSION_MAJOR)
    appHelloAlive();
}

void appHostLostTimerFired(void)
{

    if (hostLostEventCounter++ >= CONFIG_HOSTLOST_COUNT_MAX)
    {
        //appMessageBroadcast.data.data.nodeinfo.hostState = MESH_NODESTATE_HOST_LOST;
    }
    /*
    	sendBroadcastMessageParams.id = ZIGCOM_BEACON_ID_AUTOMSG;
    	sendBroadcastMessageParams.length = 0;
    	sendBroadcastMessageParams.payload = 0;

    	appNodeSendBroadcastMessage(&sendBroadcastMessageParams);
    	 * */
}


void appConnectorSendResponse(uint8_t port, ZIGCOM_MSG_S * msg)
{
    msg->header.port 			= port;
    msg->header.id.major 		= ZIGCOM_MSG_ID_RESPONSE;
    msg->header.id.minor 		= 0;
    msg->header.version.major	= ZIGCOM_PROTOCOL_VERSION_MAJOR;
    msg->header.version.minor	= ZIGCOM_PROTOCOL_VERSION_MINOR;
    msg->header.length          = ZIGCOM_MSG_RESPONSE_SIZE;
    msg->address.extAddr		= 0;
    msg->address.shortAddr      = 0;

    appSendMessageToUart((uint8_t*)msg, ZIGCOM_MSG_HEADER_SIZE + ZIGCOM_MSG_RESPONSE_SIZE);
    appHelloAlive();
}


void appConnectorSendStatus(void)
{
    ZIGCOM_MSG_S commsg;

    commsg.header.port 			= ZIGCOM_PORT_LOCAL;
    commsg.header.id.major 		= ZIGCOM_MSG_ID_STATUS;
    commsg.header.id.minor 		= 0;
    commsg.header.version.major	= ZIGCOM_PROTOCOL_VERSION_MAJOR;
    commsg.header.version.minor	= ZIGCOM_PROTOCOL_VERSION_MINOR;
    commsg.header.length 		= ZIGCOM_MSG_STATUS_SIZE;
    commsg.address.extAddr		= 0;
    commsg.address.shortAddr	= 0;

    commsg.status.softVersionMajor 	= VERSION_MAJOR;
    commsg.status.softVersionMinor 	= VERSION_MINOR;
    // ...status report for our Host
    commsg.status.nodeStatus		= appNodeInfo.State;
    commsg.status.nodeType		    = appNodeInfo.DeviceType;
    commsg.status.shortPanId 		= appNodeInfo.PANId;
    commsg.status.extAddr 		    = appNodeInfo.ExtAddr;
    commsg.status.shortAddr 		= appNodeInfo.ShortAddr;
    commsg.status.parentShortAddr   = appNodeInfo.ParentAddr;
    commsg.status.workingChannel	= appNodeInfo.Channel;
    commsg.status.lqi 			    = appNodeInfo.ParentLQI;
    commsg.status.rssi              = appNodeInfo.ParentRSSI;
    commsg.status.status            = appNodeInfo.Status;

    appSendMessageToUart((uint8_t*)&commsg, ZIGCOM_MSG_HEADER_SIZE + ZIGCOM_MSG_STATUS_SIZE);
    appHelloAlive();
}

void appConnectorSendVersion(void)
{
    ZIGCOM_MSG_S commsg;

    commsg.header.port          = ZIGCOM_PORT_LOCAL;
    commsg.header.id.major      = ZIGCOM_MSG_ID_VERSION;
    commsg.header.id.minor      = 0;
    commsg.header.version.major = ZIGCOM_PROTOCOL_VERSION_MAJOR;
    commsg.header.version.minor = ZIGCOM_PROTOCOL_VERSION_MINOR;
    commsg.header.length        = ZIGCOM_MSG_STATUS_SIZE;
    commsg.address.extAddr      = 0;
    commsg.address.shortAddr    = 0;

    commsg.version.hardwareId           = VERSION_HARDWARE_ID;
    commsg.version.hardwareRevision     = VERSION_HARDWARE_REVISION;
    commsg.version.softVersionMajor     = VERSION_MAJOR;
    commsg.version.softVersionMinor     = VERSION_MINOR;
    commsg.version.protocolVersionMajor = ZIGCOM_PROTOCOL_VERSION_MAJOR;
    commsg.version.protocolVersionMinor = ZIGCOM_PROTOCOL_VERSION_MINOR;
    commsg.version.boardType            = SYSTEM_BOARD_TYPE;

    appSendMessageToUart((uint8_t*)&commsg, ZIGCOM_MSG_HEADER_SIZE + ZIGCOM_MSG_VERSION_SIZE);
    appHelloAlive();
}

void appConnectorMessageToUart(APS_DataInd_t *indData)
{
    ZIGCOM_MSG_S commsg;

    if (indData->asduLength > MESH_MAX_ADSU_LENGTH)
        return;

    // copy first part from asdu to header section
    memcpy(&commsg, indData->asdu, ZIGCOM_MSG_SHORTHEADER_SIZE);

    // set address field to source address of packet
    commsg.address.shortAddr	= indData->srcAddress.shortAddress;

    if (indData->dstAddrMode >= APS_EXT_ADDRESS)
        commsg.address.extAddr = indData->srcAddress.extAddress;
    else
        commsg.address.extAddr		= 0ULL;

    // prevent buffer overflow
    //if (commsg.header.length > sizeof(commsg.payload))
    //    return;

    // copy rest of asdu to payload section
    memcpy(&commsg.payload, indData->asdu + ZIGCOM_MSG_SHORTHEADER_SIZE, commsg.header.length);

    appSendMessageToUart((uint8_t*)&commsg, (uint8_t)(commsg.header.length + ZIGCOM_MSG_HEADER_SIZE));
    appHelloAlive();
}

void appConnectorInit(void)
{
    HAL_StopAppTimer(&hostLostTimer); // Have to be stopped before start
    hostLostTimer.interval 	= CONFIG_INTERVAL_HOSTLOST;
    hostLostTimer.mode 		= TIMER_REPEAT_MODE;
    hostLostTimer.callback 	= appHostLostTimerFired;
    hostLostTimerStarted 	= TRUE;

    appConnectorSendVersion();
}
