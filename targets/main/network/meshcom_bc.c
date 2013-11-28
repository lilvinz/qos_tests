/*
 * meshcom_bc.c
 *
 *  Created on: 17.02.2011
 *      Author: cba
 */

#include "stdheader.h"
#include "meshcom_bc.h"

#include "meshcom.h"
#include "system.h"
#include "timeout.h"
#include "rtc.h"

#include "clock.h"
#include "version.h"
#include "debug.h"
#include "gpssync.h"
#include "gloom.h"
#include "gloom_manager.h"
#include "visibility.h"
#include "detector.h"
#include "bmm.h"
#include "sequence.h"
#include "random.h"


static bool bBroadcastEnable;
static uint16_t uBroadcastShutupTime = 0;
static bool bSendBeaconBroadcastFailed;
static bool bSendNodeinfoBroadcastFailed;
static bool bInitRequest;



void meshcom_send_beacon_bc_callback(MESHCOM_RESULT_T result, uint8_t sequence_id)
{
    if (result != MESHCOM_OK)
        bSendBeaconBroadcastFailed = TRUE;
}

void meshcom_send_nodeinfo_bc_callback(MESHCOM_RESULT_T result, uint8_t sequence_id)
{
    if (result != MESHCOM_OK)
        bSendNodeinfoBroadcastFailed = TRUE;
}

MESHCOM_RESULT_T MESHCOM_SendBeaconBroadcast(MESHCOM_CB_HANDLER_SEQ *callback)
{
    VISIBILITY_SENSOR_T     visibility_sensor_data;
    TEMPERATURE_SENSOR_T tempsensor;
    MESHCOM_MSG_S meshcom_send_message;

    memset(&meshcom_send_message, 0, sizeof(MESHCOM_MSG_S));

    meshcom_send_message.header.id.major = MESHCOM_MSG_ID_BC_BEACON;
    meshcom_send_message.header.id.minor = 0;
    meshcom_send_message.header.length = MESHCOM_MSG_BC_BEACON_SIZE;

    meshcom_send_message.message.bc_beacon.host.Serial = SERIAL_NUMBER;
    meshcom_send_message.message.bc_beacon.host.State = MESHCOM_STATUS_OK;
    meshcom_send_message.message.bc_beacon.host.FaultBM = ((uint16_t) _detector_status.uErrorFlags << 8) | (_detector_status.uWarningFlags & 0xff);

    meshcom_send_message.message.bc_beacon.sensors.gloom = (uint16_t)SENSORS_GetGloomEMA();
    meshcom_send_message.message.bc_beacon.sensors.gloom_sensor_status = GLOOM_MANAGER_GetReliability();

    VISIBILITY_GetVisibility(&visibility_sensor_data);
    meshcom_send_message.message.bc_beacon.sensors.visibility = visibility_sensor_data.value_rms_1min;
    meshcom_send_message.message.bc_beacon.sensors.visibility_sensor_status = visibility_sensor_data.state;
    meshcom_send_message.message.bc_beacon.sensors.visibility_cluster_id = VISIBILITY_CLUSTER_ID;


    SENSORS_GetTemperature(TEMPERATURE_MAIN, &tempsensor);
    meshcom_send_message.message.bc_beacon.sensors.temperature = (int16_t)tempsensor.value;
    meshcom_send_message.message.bc_beacon.sensors.temperature_sensor_status = tempsensor.state;

    meshcom_send_message.message.bc_beacon.opinions.led_on_off_opinion = GLOOM_MANAGER_GetGlobalLedOnOffOpinion();

    meshcom_send_message.message.bc_beacon.status_info.light_output_reduction = BMM_GetVisibilityRange();
    meshcom_send_message.message.bc_beacon.status_info.ledOperatingMode = GLOOM_MANAGER_GetLocalLedState();

    return MESHCOM_SendBroadcast(&meshcom_send_message, callback, 0);
}

MESHCOM_RESULT_T MESHCOM_SendNodeinfoBroadcast(MESHCOM_CB_HANDLER_SEQ *callback)
{
    MESHCOM_MSG_S meshcom_send_message;

    memset(&meshcom_send_message, 0, sizeof(MESHCOM_MSG_S));

    meshcom_send_message.header.id.major = MESHCOM_MSG_ID_BC_NODEINFO;
    meshcom_send_message.header.id.minor = 0;
    meshcom_send_message.header.length = MESHCOM_MSG_BC_NODEINFO_SIZE;

    memset(&meshcom_send_message.message.bc_nodeinfo.nodeinfo, 0xCC, sizeof(NODEINFO_T));

    meshcom_send_message.message.bc_nodeinfo.hostinfo.Serial = SERIAL_NUMBER;
    meshcom_send_message.message.bc_nodeinfo.hostinfo.State = MESHCOM_STATUS_OK;
    meshcom_send_message.message.bc_nodeinfo.hostinfo.VersionMajor = VERSION_MAJOR;
    meshcom_send_message.message.bc_nodeinfo.hostinfo.VersionMinor = VERSION_MINOR;
    meshcom_send_message.message.bc_nodeinfo.hostinfo.HardwareID = VERSION_HARDWARE_ID;

    meshcom_send_message.message.bc_nodeinfo.gps.latitude = _gpssync.position.latitude;
    meshcom_send_message.message.bc_nodeinfo.gps.longitude = _gpssync.position.longitude;

    return MESHCOM_SendBroadcast(&meshcom_send_message, callback, 0);
}

MESHCOM_RESULT_T MESHCOM_SendSwitchOnOffMessage(bool b_on_or_off, RTCTIME time, MESHCOM_CB_HANDLER *callback)
{
    MESHCOM_MSG_S meshcom_send_message;

    // todo[high] change function, this function uses variables (mesh message queue) which are use by other tasks, please check it to avoid undefined behavior in case of a context switch
    MESHCOM_MSG_BC_COMMAND_S *bc_command = &meshcom_send_message.message.bc_command;
    bc_command->command = MESHCOM_BC_COMMAND_SWITCH_ON_OFF;
    bc_command->parameter16 = b_on_or_off;
    bc_command->parameter32 = time;

    bc_command->serial = SERIAL_NUMBER;

    return MESHCOM_SendBroadcastCommand(&meshcom_send_message, NULL, 0);
}

void MESH_BroadcastShutUp(uint16_t uTimeout)
{
    if (uTimeout > MESH_BROADCAST_SHUTUP_TIMEOUT_MAX)
        uTimeout = MESH_BROADCAST_SHUTUP_TIMEOUT_MAX;

    //add some pseudo random offset and set timer
    random_init();
    uBroadcastShutupTime = uTimeout + random_get(0, (int32_t) MESH_BROADCAST_INTERVAL);;
}

void MESHCOM_BC_Start(void)
{
    bBroadcastEnable = TRUE;
}

void MESHCOM_BC_Stop(void)
{
    bBroadcastEnable = FALSE;
}

void MESHCOM_BC_Init(void)
{
    bInitRequest = TRUE;
}

void MESHCOM_BC_Tick(void)
{
    static TIMEOUT_T SendNextBeaconBroadcastTimeout;
    static TIMEOUT_T SendNextNodeinfoBroadcastTimeout;
    static int32_t BeaconBroadcastInterval = 0;
    static int32_t NodeinfoBroadcastInterval = 0;

    if (bInitRequest == TRUE)
    {
        bInitRequest = FALSE;
        SendNextBeaconBroadcastTimeout = 0;
        SendNextNodeinfoBroadcastTimeout = 0;
    }

    if (bBroadcastEnable == TRUE)
    {
        if (uBroadcastShutupTime > 0)
        {
            // if shutup time set add it to the current interval of broadcast timers.
            BeaconBroadcastInterval = uBroadcastShutupTime;
            TIMEOUT_Reset(SendNextBeaconBroadcastTimeout);
            NodeinfoBroadcastInterval = uBroadcastShutupTime;
            TIMEOUT_Reset(SendNextNodeinfoBroadcastTimeout);
            uBroadcastShutupTime = 0;
        }

        if (TIMEOUT_Check(SendNextBeaconBroadcastTimeout, BeaconBroadcastInterval))
        {
            TIMEOUT_Reset(SendNextBeaconBroadcastTimeout);
            BeaconBroadcastInterval = random_jitter(MESH_BROADCAST_INTERVAL, MESH_BROADCAST_JITTER);
            MESHCOM_SendBeaconBroadcast(meshcom_send_beacon_bc_callback);
        }

        if (bSendBeaconBroadcastFailed)
        {
            bSendBeaconBroadcastFailed = FALSE;
            BeaconBroadcastInterval = random_jitter(MESH_BROADCAST_INTERVAL_RETRY, MESH_BROADCAST_JITTER);
        }

        if (TIMEOUT_Check(SendNextNodeinfoBroadcastTimeout, NodeinfoBroadcastInterval))
        {
            TIMEOUT_Reset(SendNextNodeinfoBroadcastTimeout);
            NodeinfoBroadcastInterval = random_jitter(MESH_BROADCAST_NODEINFO_INTERVAL, MESH_BROADCAST_JITTER);
            MESHCOM_SendNodeinfoBroadcast(meshcom_send_nodeinfo_bc_callback);
        }

        if (bSendNodeinfoBroadcastFailed)
        {
            bSendNodeinfoBroadcastFailed = FALSE;
            NodeinfoBroadcastInterval = random_jitter(MESH_BROADCAST_NODEINFO_INTERVAL_RETRY, MESH_BROADCAST_JITTER);
        }

        // make sure interval is not 0
        if (BeaconBroadcastInterval == 0)
            BeaconBroadcastInterval = 1;
        if (NodeinfoBroadcastInterval == 0)
            NodeinfoBroadcastInterval = 1;

    }
}
