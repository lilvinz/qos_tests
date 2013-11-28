#include <taskManager.h>
#include <resetReason.h>
#include <wdtCtrl.h>
#include <zdo.h>
#include <zdoZib.h>
#include <ZigComApp.h>
#include <pdsDataServer.h>
#include <configServer.h>
#include <aps.h>
#include <mac.h>

#include "leds.h"

#include "ZigComNode.h"
#include "ZigComConnector.h"
#include "ZigComUARTManager.h"
#include "bootloaderupdater.h"
#include "FactoryTest.h"

/***********************************************************************************
 Defines
 ***********************************************************************************/
#define APP_STATE_CONFIG_TIMEOUT	5000

typedef enum
{
    LED_BLINK_START,
    LED_BLINK_STOP,
}LED_BLINK_STATE_T;

/***********************************************************************************
 Prototypes
 ***********************************************************************************/
void appPostSubTaskTask(void);
void appPostGlobalTask(void);
/***********************************************************************************
 Global variables
 ***********************************************************************************/
AppState_t appState = APP_INITING_STATE;//application state

uint8_t failedInNetworkState = 0;

appNodeInfo_t appNodeInfo;

/***********************************************************************************
 Local variables
 ***********************************************************************************/
static ZDO_StartNetworkReq_t networkParams; //request params for ZDO_StartNetworkReq

//endpoint parameters
SimpleDescriptor_t simpleDescriptorForBroadcast	=   { END_POINT_BEACON_BC, 1, 1, 1, 0, 0, NULL, 0, NULL };
SimpleDescriptor_t simpleDescriptorForUnicast   =   { END_POINT_UNICAST, 1, 1, 1, 0, 0, NULL, 0, NULL };
SimpleDescriptor_t simpleDescriptorForNetstat   =   { END_POINT_NETSTAT, 1, 1, 1, 0, 0, NULL, 0, NULL };

static APS_RegisterEndpointReq_t broadcastEndpointParams;
static APS_RegisterEndpointReq_t unicastEndpointParams;
static APS_RegisterEndpointReq_t netstatEndpointParams;

// Timer indicating starting network
static ZDO_ZdpReq_t leaveReq;
static AppEvent_t appLastEvent = 0;

HAL_AppTimer_t deviceStatusMessageTimer;
HAL_AppTimer_t appStateTimeoutTimer;
HAL_AppTimer_t appWatchdogTimer;
HAL_AppTimer_t ledBlinkTimer;

static uint32_t appAliveCounter = 2;

/***********************************************************************************
 Local functions
 ***********************************************************************************/
static void ZDO_StartNetworkConf(ZDO_StartNetworkConf_t *confirmInfo);
static void zdpLeaveResp(ZDO_ZdpResp_t *zdpResp);
static void initSubStateMachine(void);
static void appTaskHandler(AppEvent_t event, void *param);
static void appNotifyStatus(void);
static void deviceStatusMessageTimerInit(void);
static void deviceStatusMessageTimerFired(void);
static void appConfigStateTimeoutTimerFired(void);
static void appWatchdogTimerFired(void);
static void appSendStatusMessage(void);



/***********************************************************************************
 Implementation
 ***********************************************************************************/


/*************************************************************//**
  \brief Main - C program main start function

  \param none
  \return none
************************************************************/
int main(void)
{
    BootloaderUpdater();
    FactoryTest_CTTM();

    SYS_SysInit();

    for(;;)
    {
        SYS_RunTask();
    }
}



#ifdef _BINDING_
/***********************************************************************************
  Stub for ZDO Binding Indication

  Parameters:
    bindInd - indication

  Return:
    none

 ***********************************************************************************/
void ZDO_BindIndication(ZDO_BindInd_t *bindInd)
{
    (void)bindInd;
}
void ZDO_UnbindIndication(ZDO_UnbindInd_t *unbindInd)
{
    (void)unbindInd;
}
#endif //_BINDING_



/***********************************************************************************
 Application task.

 Parameters:
 none

 Return:
 none

 ***********************************************************************************/
void APL_TaskHandler(void)
{
    appTaskHandler(APP_PROCESS, NULL);
}

static void appTaskHandler(AppEvent_t event, void *param)
{
    bool rxOnWhenIdleFlag = true;

    appLastEvent = event;

    switch (appState)
    {
    case APP_IN_NETWORK_STATE:
    {
        switch (event)
        {
        case APP_PROCESS:
        {
            appNodeTaskHandler(APP_PROCESS, NULL);
            break;
        }
        case APP_NETWORK_STATE_UPDATED:
        {
            ZDO_MgmtNwkUpdateNotf_t *updateParam = param;

            switch (updateParam->status)
            {
            case ZDO_NETWORK_LOST_STATUS:
                appState = APP_STOP_STATE;
                break;
            case ZDO_NETWORK_LEFT_STATUS:
                appState = APP_STARTING_NETWORK_STATE;
                appPostGlobalTask();
                break;
            case ZDO_NWK_UPDATE_STATUS:
            case ZDO_NETWORK_STARTED_STATUS:
                // Update info parameters
                appNodeInfo.ShortAddr   = updateParam->nwkUpdateInf.shortAddr;
                appNodeInfo.PANId       = updateParam->nwkUpdateInf.panId;
                appNodeInfo.ParentAddr  = updateParam->nwkUpdateInf.parentShortAddr;
                appNodeInfo.Channel     = updateParam->nwkUpdateInf.currentChannel;
                break;
            default:
                break;
            }

            appNodeInfo.Status = updateParam->status;
            appNotifyStatus();
            break;
        }
        case APP_CONFIG_UPDATED:
        {
            break;
        }

        default:
            break;
        }
        break;
    }
    case APP_INITING_STATE: //node has initial state
    {
        switch (event)
        {
        case APP_PROCESS:
            appWatchdogTimer.interval = CONFIG_WATCHDOG_TIMER_INTERVAL;
            appWatchdogTimer.mode = TIMER_REPEAT_MODE;
            appWatchdogTimer.callback = appWatchdogTimerFired;
            HAL_StartAppTimer(&appWatchdogTimer);

            //Init environment
            failedInNetworkState = 0;

            BSP_OpenLeds();
            appStartUARTManager(); //init UART manager

            appNodeInfo.State	= ZIGCOM_NODE_STATUS_INITING_STATE;
            appNotifyStatus();

            appInitDeviceNode();

            CS_ReadParameter(CS_DEVICE_TYPE_ID, &appNodeInfo.DeviceType);
            CS_ReadParameter(CS_UID_ID, &appNodeInfo.Uid);

            appNodeInfo.ExtAddr = appNodeInfo.Uid;

            // always keep rx on!
            rxOnWhenIdleFlag = true;
            CS_WriteParameter(CS_RX_ON_WHEN_IDLE_ID, &rxOnWhenIdleFlag);

            csZIB.powerDescriptor.currentPowerMode = RECEIVER_SYNCRONIZED_WHEN_ON_IDLE;
            csZIB.powerDescriptor.availablePowerSources = MAINS_POWER;
            csZIB.powerDescriptor.currentPowerSource = MAINS_POWER;
            csZIB.powerDescriptor.currentPowerSourceLevel = PERCENT_100;

            // register beacon broadcast endpoint
            broadcastEndpointParams.simpleDescriptor = &simpleDescriptorForBroadcast;
            broadcastEndpointParams.APS_DataInd = APS_DataIndBroadcast;
            APS_RegisterEndpointReq(&broadcastEndpointParams);

            // register command endpoint
            unicastEndpointParams.simpleDescriptor = &simpleDescriptorForUnicast;
            unicastEndpointParams.APS_DataInd = APS_DataIndUnicast;
            APS_RegisterEndpointReq(&unicastEndpointParams);

            // register netstat endpoint
            netstatEndpointParams.simpleDescriptor = &simpleDescriptorForNetstat;
            netstatEndpointParams.APS_DataInd = APS_DataIndNetstat;
            APS_RegisterEndpointReq(&netstatEndpointParams);

            appState = APP_WAIT_FOR_INITIAL_CONFIG;
            appPostGlobalTask();
            break;
        default:
            break;
        }
        break;
    }
    case APP_WAIT_FOR_INITIAL_CONFIG:
    {
        switch (event)
        {
        case APP_PROCESS:
            appNodeInfo.State	= ZIGCOM_NODE_STATUS_CONFIG_STATE;
            appNotifyStatus();
            HAL_StopAppTimer(&appStateTimeoutTimer); // Have to be stopped before start
            appStateTimeoutTimer.interval = APP_STATE_CONFIG_TIMEOUT;
            appStateTimeoutTimer.mode = TIMER_REPEAT_MODE;
            appStateTimeoutTimer.callback = appConfigStateTimeoutTimerFired;
            HAL_StartAppTimer(&appStateTimeoutTimer);
            break;
        case APP_CONFIG_UPDATED:
            HAL_StopAppTimer(&appStateTimeoutTimer);
            CS_ReadParameter(CS_DEVICE_TYPE_ID, &appNodeInfo.DeviceType);
            CS_ReadParameter(CS_UID_ID, &appNodeInfo.Uid);
            appNodeInfo.ExtAddr = appNodeInfo.Uid;
            appState = APP_STARTING_NETWORK_STATE;
            appPostGlobalTask();
            break;

        default:
            break;
        }
        break;
    }
    case APP_STARTING_NETWORK_STATE:
    {
        switch (event)
        {
        case APP_PROCESS:
            appNodeInfo.State	= ZIGCOM_NODE_STATUS_STARTING_NETWORK_STATE;
            appNotifyStatus();
            networkParams.ZDO_StartNetworkConf = ZDO_StartNetworkConf;
            ZDO_StartNetworkReq(&networkParams);
            break;
        case APP_NETWORK_STARTING_DONE:
        {
            ZDO_StartNetworkConf_t *startInfo = param;
            switch (startInfo->status)
            {
            case ZDO_SUCCESS_STATUS:
            {
                appNodeInfo.State 		= ZIGCOM_NODE_STATUS_IN_NETWORK_STATE;
                appNodeInfo.ShortAddr 	= startInfo->shortAddr;
                appNodeInfo.PANId 		= startInfo->PANId;
                appNodeInfo.ParentAddr 	= startInfo->parentAddr;
                appNodeInfo.Channel 	= startInfo->activeChannel;
                appNodeInfo.Status      = startInfo->status;
                appNotifyStatus();
                deviceStatusMessageTimerInit();
                appState = APP_IN_NETWORK_STATE;
                initSubStateMachine();
            }
            break;
/*
            case ZDO_EXTPANID_ALREADY_EXIST_STATUS:
            {
                // There is a network with the same extended PANID. I will lose my position as Coordinator
                CS_ReadParameter(CS_DEVICE_TYPE_ID, &appNodeInfo.DeviceType);
                if (appNodeInfo.DeviceType == DEVICE_TYPE_COORDINATOR)
                {
                    appNodeInfo.DeviceType = DEVICE_TYPE_ROUTER;
                    CS_WriteParameter(CS_DEVICE_TYPE_ID, &appNodeInfo.DeviceType);
                }
                appPostGlobalTask();
            }
            break;
            case ZDO_NO_NETWORKS_STATUS:
            {
                // No network was found. I will become the Coordinator. Yeah, I rock!
                CS_ReadParameter(CS_DEVICE_TYPE_ID, &appNodeInfo.DeviceType);
                if (appNodeInfo.DeviceType != DEVICE_TYPE_COORDINATOR)
                {
                    appNodeInfo.DeviceType = DEVICE_TYPE_COORDINATOR;
                    CS_WriteParameter(CS_DEVICE_TYPE_ID, &appNodeInfo.DeviceType);
                }
                appPostGlobalTask();
            }
            break;
*/
            default:
                appNodeInfo.State       = ZIGCOM_NODE_STATUS_START_NETWORK_FAILED;
                appNodeInfo.Status      = startInfo->status;
                appNotifyStatus();
                appPostGlobalTask();
                break;
            }
        }
        case APP_NETWORK_STATE_UPDATED:
        {
            ZDO_MgmtNwkUpdateNotf_t *updateParam = param;
            switch (updateParam->status)
            {
            case ZDO_NETWORK_STARTED_STATUS:
                appNodeInfo.State 		= ZIGCOM_NODE_STATUS_NETWORK_UPDATE;
                appNodeInfo.ShortAddr 	= updateParam->nwkUpdateInf.shortAddr;
                appNodeInfo.PANId 		= updateParam->nwkUpdateInf.panId;
                appNodeInfo.ParentAddr 	= updateParam->nwkUpdateInf.parentShortAddr;
                appNodeInfo.Channel 	= updateParam->nwkUpdateInf.currentChannel;
                appNodeInfo.Status      = updateParam->status;
                appNotifyStatus();
                deviceStatusMessageTimerInit();
                appState = APP_IN_NETWORK_STATE;
                initSubStateMachine();
                break;
            default:
                break;
            }
        }
        default:
            break;
        }
        break;
    }
    case APP_LEAVING_NETWORK_STATE:
    {
        ZDO_MgmtLeaveReq_t *zdpLeaveReq = &leaveReq.req.reqPayload.mgmtLeaveReq;

        appNodeInfo.State = ZIGCOM_NODE_STATUS_LEAVING_NETWORK_STATE;
        appNotifyStatus();

        switch (event)
        {
        case APP_PROCESS:
            leaveReq.ZDO_ZdpResp = zdpLeaveResp;
            leaveReq.reqCluster = MGMT_LEAVE_CLID;
            leaveReq.dstAddrMode = EXT_ADDR_MODE;
            leaveReq.dstExtAddr = 0;
            zdpLeaveReq->deviceAddr = 0;
            zdpLeaveReq->rejoin = 0;
            zdpLeaveReq->removeChildren = 1;
            zdpLeaveReq->reserved = 0;
            ZDO_ZdpReq(&leaveReq);
            break;
        case APP_LEAVE_DONE:
            if (ZDO_SUCCESS_STATUS == ((ZDO_ZdpResp_t *) param)->respPayload.status)
            {
                appState = APP_STOP_STATE;
            }
            appPostGlobalTask();
            break;
        case APP_NETWORK_STATE_UPDATED:
        {
            ZDO_MgmtNwkUpdateNotf_t *updateParam = param;

            switch (updateParam->status)
            {
            case ZDO_NETWORK_LOST_STATUS:
                appState = APP_STOP_STATE;
                break;
            case ZDO_NETWORK_LEFT_STATUS:
                appState = APP_STARTING_NETWORK_STATE;
                appPostGlobalTask();
                break;
            default:
                break;
            }
            break;
        }
        default:
            break;
        }
        break;
    }
    case APP_STOP_STATE:
    {
        appNodeInfo.State = ZIGCOM_NODE_STATUS_STOP_STATE;
        appNotifyStatus();

        switch (event)
        {
        case APP_NETWORK_STATE_UPDATED:
        {
            ZDO_MgmtNwkUpdateNotf_t *updateParam = param;

            switch (updateParam->status)
            {
            case ZDO_NETWORK_STARTED_STATUS:
                appNodeInfo.State 		= ZIGCOM_NODE_STATUS_IN_NETWORK_STATE;
                appNodeInfo.ShortAddr 	= updateParam->nwkUpdateInf.shortAddr;
                appNodeInfo.PANId 		= updateParam->nwkUpdateInf.panId;
                appNodeInfo.ParentAddr 	= updateParam->nwkUpdateInf.parentShortAddr;
                appNodeInfo.Channel     = updateParam->nwkUpdateInf.currentChannel;
                appNodeInfo.Status      = ZDO_NETWORK_STARTED_STATUS;
                appNotifyStatus();

                appState = APP_IN_NETWORK_STATE;
                appPostGlobalTask();
                break;
            case ZDO_NETWORK_LEFT_STATUS:
                appState = APP_STARTING_NETWORK_STATE;
                appPostGlobalTask();
                break;
            default:
                break;
            }
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

// The response means that the command has been received successfully but not processed yet
static void zdpLeaveResp(ZDO_ZdpResp_t *zdpResp)
{
    appTaskHandler(APP_LEAVE_DONE, zdpResp);
}

/***********************************************************************************
 ZDO_StartNetwork primitive confirmation was received.

 Parameters:
 confirmInfo - confirmation information

 Return:
 none

 ***********************************************************************************/

static void ZDO_StartNetworkConf(ZDO_StartNetworkConf_t *confirmInfo)
{
    appTaskHandler(APP_NETWORK_STARTING_DONE, confirmInfo);
}

void ZDO_MgmtNwkUpdateNotf(ZDO_MgmtNwkUpdateNotf_t *nwkParams)
{
    appTaskHandler(APP_NETWORK_STATE_UPDATED, nwkParams);
}

void initSubStateMachine(void)
{
    appInitDeviceNode();
}

void appConfigReceived(void)
{
    appTaskHandler(APP_CONFIG_UPDATED, NULL);
}

// For blinking

static void manageBlinking(LED_BLINK_STATE_T blinkaction)
{
    if (blinkaction == LED_BLINK_START)
    {
        HAL_StopAppTimer(&ledBlinkTimer);
        HAL_StartAppTimer(&ledBlinkTimer);
    }
    if (blinkaction == LED_BLINK_STOP)
    {
        HAL_StopAppTimer(&ledBlinkTimer);
    }
}

static void ledBlinkTimerFired(void)
{
    APP_STATUS_LED_TOGGLE();
}

void appNotifyStatus(void)
{
    static uint8_t nodeStatusOld;
    if (appNodeInfo.State != nodeStatusOld)
    {
        nodeStatusOld = appNodeInfo.State;

        //Show status via LED
        switch(appNodeInfo.State)
        {
        case ZIGCOM_NODE_STATUS_INITING_STATE:
            APP_STATUS_LED_OFF();
            break;
        case ZIGCOM_NODE_STATUS_CONFIG_STATE:
            ledBlinkTimer.interval = APP_LED_TIMER_WAITING_FOR_CONFIG_INTERVAL;
            ledBlinkTimer.mode     = TIMER_REPEAT_MODE;
            ledBlinkTimer.callback = ledBlinkTimerFired;
            manageBlinking(LED_BLINK_START);
            break;
        case ZIGCOM_NODE_STATUS_STARTING_NETWORK_STATE:
            ledBlinkTimer.interval = APP_LED_TIMER_STARTING_NETWORK_INTERVAL;
            ledBlinkTimer.mode     = TIMER_REPEAT_MODE;
            ledBlinkTimer.callback = ledBlinkTimerFired;
            manageBlinking(LED_BLINK_START);
            break;
        case ZIGCOM_NODE_STATUS_IN_NETWORK_STATE:
            manageBlinking(LED_BLINK_STOP);
            APP_STATUS_LED_ON();
            break;
        case ZIGCOM_NODE_STATUS_LEAVING_NETWORK_STATE:
            manageBlinking(LED_BLINK_STOP);
            APP_STATUS_LED_OFF();
            break;
        case ZIGCOM_NODE_STATUS_STOP_STATE:
            manageBlinking(LED_BLINK_STOP);
            APP_STATUS_LED_OFF();
            break;
        default:
            break;
        }
    }
    appConnectorSendStatus();
}


// Post tasks functions
void appPostGlobalTask()
{
    SYS_PostTask(APL_TASK_ID);
}

void appPostSubTaskTask()
{
    SYS_PostTask(APL_TASK_ID);
}

void appLeaveNetwork()
{
    if (APP_IN_NETWORK_STATE == appState)
    {
        appState = APP_LEAVING_NETWORK_STATE;
        appPostGlobalTask();
    }
}

void appReadLqiRssi(void)
{
    ZDO_GetLqiRssi_t lqiRssi;

    lqiRssi.nodeAddr = appNodeInfo.ParentAddr;
    ZDO_GetLqiRssi(&lqiRssi);

    appNodeInfo.ParentLQI = lqiRssi.lqi;
    appNodeInfo.ParentRSSI = lqiRssi.rssi;
}

static void deviceStatusMessageTimerInit(void)
{
    HAL_StopAppTimer(&deviceStatusMessageTimer); // Have to be stopped before start
    deviceStatusMessageTimer.interval = CONFIG_STATUS_MESSAGE_INTERVAL;
    deviceStatusMessageTimer.mode = TIMER_REPEAT_MODE;
    deviceStatusMessageTimer.callback = deviceStatusMessageTimerFired;
    HAL_StartAppTimer(&deviceStatusMessageTimer);
}

static void deviceStatusMessageTimerFired(void)
{
    appSendStatusMessage();
}

static void appSendStatusMessage(void)
{
    ZDO_NwkStatus_t NwkStatus;
    appReadLqiRssi();
    appConnectorSendStatus();

    NwkStatus = ZDO_GetNwkStatus();
    if (NwkStatus == ZDO_OUT_NETWORK_STATUS)
    {
        failedInNetworkState++;
        if (failedInNetworkState >= (CONFIG_OUT_OF_NETWORK_FATAL_TIMEOUT / CONFIG_STATUS_MESSAGE_INTERVAL))
        {
            HAL_WarmReset();
        }
    }
    else
        failedInNetworkState = 0;
}

static void appConfigStateTimeoutTimerFired(void)
{
    if (appState == APP_WAIT_FOR_INITIAL_CONFIG)
    {
        appPostGlobalTask();
    }
}

static void appWatchdogTimerFired(void)
{
    if (appAliveCounter > 0)
    {
        appAliveCounter--;
        HAL_ResetWdt();
    }

}

void appHelloAlive(void)
{
    appAliveCounter = (uint32_t)((float)CONFIG_ALIVE_COUNTER_RESET_VALUE / ((float)CONFIG_WATCHDOG_TIMER_INTERVAL / 1000.0));
}

//eof
