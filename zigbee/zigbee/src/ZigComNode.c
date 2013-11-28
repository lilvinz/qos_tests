/************************************************************************
 ZBNRouterEndDeviceApp.c
 Copyrights Meshnetics (TBD)

 History:
 13/06/07 I. Kalganova - Modified
 **************************************************************************/

#include <string.h>
#include <ZigComApp.h>
#include "ZigComConnector.h"
#include "ZigComNode.h"

DeviceState_t appDeviceState = INITIAL_DEVICE_STATE;

appMessageBroadcastRequest_t 		appMessageBroadcast;
static uint8_t appMessageBroadcastCurrentMessageID;
static bool appMessageBroadcastBusy;

appMessageUnicastRequest_t			appMessageUnicast;
static uint8_t appMessageUnicastCurrentMessageID;
//static bool appMessageUnicastBusy;

appMessageNetstatRequest_t          appMessageNetstat;


APS_DataReq_t messageBroadcastParams;
APS_DataReq_t messageUnicastParams;
APS_DataReq_t messageNetstatParams;

//static ZDO_ZdpReq_t zdpReq;
//static bool zdpReqBusy;

extern AppState_t appState;
extern bool subTaskRequested;

extern ZIGCOM_MSG_S commsg;

static uint8_t transmissionRetryCount = 0;

#ifdef PING_PARENT
static HAL_AppTimer_t deviceAliveTimer;
static uint8_t deviceAliveRetryCounter = 0;
static void zdpIeeeAddrResp(ZDO_ZdpResp_t *resp);
static void deviceAliveTimerFired(void);
#endif

static HAL_AppTimer_t deviceTimeoutTimer;
//static void deviceTimeoutTimerFired(void);

static void APS_BroadcastDataConf(APS_DataConf_t *confInfo);
static void APS_UnicastDataConf(APS_DataConf_t *confInfo);
static void APS_NetstatDataConf(APS_DataConf_t *confInfo);
//static void zdpNwkAddrResp(ZDO_ZdpResp_t *resp);
//static bool doShortAddressRequest(ExtAddr_t extAddress);


void appInitDeviceNode(void);
void appNodeReadLqiRssi(void);


/***********************************************************************************
 Process router or end device functionality.

 Parameters:
 none

 Return:
 none

 ***********************************************************************************/
void appNodeTaskHandler(AppEvent_t event, void *param)
{
    switch (appDeviceState)
    {
    case WAITING_DEVICE_STATE:
        break;

    case INITIAL_DEVICE_STATE:
        switch (event)
        {
        case APP_PROCESS:

#ifdef PING_PARENT
            HAL_StopAppTimer(&deviceAliveTimer); // Have to be stopped before start
            deviceAliveTimer.interval = CONFIG_ALIVE_INTERVAL;
            deviceAliveTimer.mode = TIMER_REPEAT_MODE;
            deviceAliveTimer.callback = deviceAliveTimerFired;
            HAL_StartAppTimer(&deviceAliveTimer);
            deviceAliveRetryCounter = 0;
#endif

           // appMessageUnicastBusy = FALSE;

            appConnectorInit();

            appDeviceState = WAITING_DEVICE_STATE;
            appPostSubTaskTask();
            break;
        default:
            break;
        }
        break;
    case REINITIAL_DEVICE_STATE:
        switch (event)
        {
        case APP_PROCESS:
            break;
        default:
            appDeviceState = INITIAL_DEVICE_STATE;
            appPostSubTaskTask();
            break;
        }
        break;
    default:
        break;
    }
    param = param;	// suppress compiler warning if parameter not used
}

// appRouterTaskHandler must be called from main state machine after this function
void appInitDeviceNode()
{
    if (WAITING_DEVICE_STATE == appDeviceState)
        appDeviceState = REINITIAL_DEVICE_STATE;
    else
    {
        appDeviceState = INITIAL_DEVICE_STATE;
        appPostSubTaskTask();
    }
}


MsgRet_t appNodeSendBroadcastMessage(appNodeSendBroadcastMessage_t *params)
{
    if (appMessageBroadcastBusy == TRUE)
        return MSG_ERR_BUSY;

    if (params->length > MESH_BROADCAST_RAWDATA_LEN)
        return MSG_ERR_SIZE;

    // Prefilling Beacon Broadcast request parameters
    messageBroadcastParams.profileId = simpleDescriptorForBroadcast.AppProfileId;
    messageBroadcastParams.dstAddrMode = APS_SHORT_ADDRESS;
    messageBroadcastParams.dstAddress.shortAddress = BROADCAST_ADDR_ALL; //BROADCAST_ADDR_ALL;
    messageBroadcastParams.dstEndpoint = END_POINT_BEACON_BC;
    messageBroadcastParams.clusterId = 1;
    messageBroadcastParams.srcEndpoint = END_POINT_BEACON_BC;
    messageBroadcastParams.asduLength = sizeof(appMessageBroadcast.rawdata);
    messageBroadcastParams.asdu = (uint8_t *)(&appMessageBroadcast.rawdata);
    messageBroadcastParams.txOptions.acknowledgedTransmission = 0;
    messageBroadcastParams.radius = 5;
    messageBroadcastParams.APS_DataConf = APS_BroadcastDataConf;
    appMessageBroadcastBusy = FALSE;

    memcpy(&appMessageBroadcast.rawdata[0], params->payload, (params->length));
    appMessageBroadcastCurrentMessageID = params->message_id;

    APS_DataReq(&messageBroadcastParams);
    appMessageBroadcastBusy = TRUE;
    return MSG_OK;
}

MsgRet_t appNodeSendUnicastMessage(appNodeSendUnicastMessage_t *params)
{
    if ((params->length == 0) || (params->length > MESH_UNICAST_RAWDATA_LEN))
        return MSG_ERR_SIZE;

    // Prefilling Command Stream parameters
    messageUnicastParams.profileId = simpleDescriptorForUnicast.AppProfileId;
    messageUnicastParams.dstAddrMode = APS_SHORT_ADDRESS;
    messageUnicastParams.dstAddress.shortAddress = 0x0000;  //prefill coordinators address. later target address will be used
    messageUnicastParams.dstEndpoint = END_POINT_UNICAST;
    messageUnicastParams.clusterId = 1;
    messageUnicastParams.srcEndpoint = END_POINT_UNICAST;
    messageUnicastParams.asduLength = sizeof(appMessageUnicast.rawdata);
    messageUnicastParams.asdu = (uint8_t *)(&appMessageUnicast.rawdata);
    messageUnicastParams.txOptions.acknowledgedTransmission = 1;
    messageUnicastParams.radius = 0x0;
    messageUnicastParams.APS_DataConf = APS_UnicastDataConf;

    if (params->dstExtAddr > 0ULL)
    {
        if (params->dstExtAddr == appNodeInfo.ExtAddr)
            return MSG_ERR_ADDR;
        messageUnicastParams.dstAddrMode = APS_EXT_ADDRESS;
        messageUnicastParams.dstAddress.extAddress = params->dstExtAddr;
    }
    else
    {
        if (params->dstShortAddr == appNodeInfo.ShortAddr)
            return MSG_ERR_ADDR;
        messageUnicastParams.dstAddrMode = APS_SHORT_ADDRESS;
        messageUnicastParams.dstAddress.shortAddress = params->dstShortAddr;
    }

    messageUnicastParams.txOptions.acknowledgedTransmission = params->ack;

    memcpy(&appMessageUnicast.rawdata[0], params->payload, params->length);
    appMessageUnicastCurrentMessageID = params->message_id;

    APS_DataReq(&messageUnicastParams);
    return MSG_OK;
}


/***********************************************************************************
 Incoming message handling
 ***********************************************************************************/

void APS_DataIndBroadcast(APS_DataInd_t *indData)
{
    appConnectorMessageToUart(indData);
}

void APS_DataIndUnicast(APS_DataInd_t *indData)
{
    appConnectorMessageToUart(indData);
}

void APS_DataIndNetstat(APS_DataInd_t *indData)
{
    // check message id and initiate neighbor table transfer
    //if ()
    appConnectorMessageToUart(indData);
}

/***********************************************************************************
 Confirmation of message sent
 ***********************************************************************************/

static void APS_BroadcastDataConf(APS_DataConf_t *confInfo)
{
    ZIGCOM_MSG_S response_message;

    appMessageBroadcastBusy = FALSE;

    //ToDo add real message id from send message
    response_message.response.message_id 	= appMessageBroadcastCurrentMessageID;

    if (confInfo->status == APS_SUCCESS_STATUS)	response_message.response.response = ZIGCOM_RESPONSE_MSG_ACK;
    else										response_message.response.response = ZIGCOM_RESPONSE_MSG_NACK;

    response_message.response.code = confInfo->status;
    appConnectorSendResponse(ZIGCOM_PORT_BROADCAST, &response_message);
}

static void APS_UnicastDataConf(APS_DataConf_t *confInfo)
{
    ZIGCOM_MSG_S response_message;

    //ToDo add real message id
    response_message.response.message_id 	= appMessageUnicastCurrentMessageID;

    if (confInfo->status == APS_SUCCESS_STATUS)	response_message.response.response = ZIGCOM_RESPONSE_MSG_ACK;
    else										response_message.response.response = ZIGCOM_RESPONSE_MSG_NACK;
/*
    if (transmissionRetryCount < 3 && confInfo->status == APS_NO_SHORT_ADDRESS_STATUS)
    {
        if (doShortAddressRequest(messageUnicastParams.dstAddress.extAddress))
            transmissionRetryCount++;
        else
        {
            response_message.response.code = confInfo->status;
            appConnectorSendResponse(ZIGCOM_PORT_UNICAST, &response_message);
            HAL_StopAppTimer(&deviceTimeoutTimer);
            appMessageUnicastBusy = FALSE;
            transmissionRetryCount = 0;
        }
    }
    else
    {
    */
        response_message.response.code = confInfo->status;
        appConnectorSendResponse(ZIGCOM_PORT_UNICAST, &response_message);
        HAL_StopAppTimer(&deviceTimeoutTimer);
       // appMessageUnicastBusy = FALSE;
        transmissionRetryCount = 0;
//    }
}

/*
static void deviceTimeoutTimerFired(void)
{
    appMessageUnicastBusy = FALSE;
    zdpReqBusy = false;
}
*/



/***********************************************************************************
 Short Address Request and Unicast Retry Request.
 ***********************************************************************************/
/*
static void zdpNwkAddrResp(ZDO_ZdpResp_t *resp)
{
    ZDO_NwkAddrResp_t *nwkAddrResp = (ZDO_NwkAddrResp_t *) &resp->respPayload.nwkAddrResp;

    zdpReqBusy = false;
    if (ZDO_SUCCESS_STATUS == resp->respPayload.status)
    {
        NWK_LinkShortAndExtAddress(nwkAddrResp->nwkAddrRemote, nwkAddrResp->ieeeAddrRemote);
        messageUnicastParams.dstAddrMode = APS_SHORT_ADDRESS;
        messageUnicastParams.dstAddress.shortAddress = nwkAddrResp->nwkAddrRemote;
    }
    appNodeSendUnicastMessageRetry();
}
*/
/*
static bool doShortAddressRequest(ExtAddr_t extAddress)
{
    if (zdpReqBusy == false)
    {
        ZDO_NwkAddrReq_t *nwkAddrReq = &zdpReq.req.reqPayload.nwkAddrReq;

        zdpReq.ZDO_ZdpResp = zdpNwkAddrResp;
        zdpReq.reqCluster = NWK_ADDR_CLID;

        nwkAddrReq->ieeeAddrOfInterest = extAddress;
        nwkAddrReq->reqType = SINGLE_RESPONSE_REQUESTTYPE;
        nwkAddrReq->startIndex = 0;

        zdpReqBusy = true;
        ZDO_ZdpReq(&zdpReq);
        return true;
    }
    else
        return false;
}
*/
static void getNeighbors(void)
{
    /** idea & todo **/
    // spill out neighbors to host

    //ZDO_Neib_t NeighborTable[CS_NEIB_TABLE_SIZE];

    //ZDO_GetNeibTable(NeighborTable);


}


#ifdef PING_PARENT

/***********************************************************************************
 Device ALIVE timer event.
 ***********************************************************************************/
static void zdpIeeeAddrResp(ZDO_ZdpResp_t *resp)
{
    if (ZDO_SUCCESS_STATUS == resp->respPayload.status)
    {
        deviceAliveRetryCounter = 0;
    }
    else
    {
        deviceAliveRetryCounter++;
    }
    zdpReqBusy = false;
}

static void deviceAliveTimerFired(void)
{
    // periodic network alive beacon
    NodeAddr_t parentAddr;
    ZDO_IeeeAddrReq_t *ieeeAddrReq = &zdpReq.req.reqPayload.ieeeAddrReq;

    if (appNodeInfo.DeviceType == DEVICE_TYPE_END_DEVICE)
    {
        if (deviceAliveRetryCounter > CONFIG_ALIVE_RETRY_MAX)
        {
            appLeaveNetwork();
            return;
        }
        else if (zdpReqBusy == false)
        {
            //set parents address as destination
            ZDO_GetParentAddr(&parentAddr);
            zdpReq.ZDO_ZdpResp = zdpIeeeAddrResp;
            zdpReq.reqCluster = IEEE_ADDR_CLID;

            ieeeAddrReq->nwkAddrOfInterest = parentAddr.shortAddr;
            ieeeAddrReq->reqType = SINGLE_RESPONSE_REQUESTTYPE;
            ieeeAddrReq->startIndex = 0;

            zdpReqBusy = true;
            ZDO_ZdpReq(&zdpReq);
        }
        else
            deviceAliveRetryCounter++;
    }
}
#endif

void ZDO_SleepConf(ZDO_SleepConf_t *conf)
{
    conf = conf;	// suppress compiler warning if parameter not used
    //appNodeTaskHandler(APP_SLEEP_DONE, conf);
}

void ZDO_WakeUpInd()
{
    //appNodeTaskHandler(APP_WOKEUP, NULL);
}

//eof ZBNRouterEndDeviceApp.c
