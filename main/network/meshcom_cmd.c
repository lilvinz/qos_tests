// headers needed for command execution support

#include "stdheader.h"
#include "meshcom_cmd.h"

#include "settings.h"
#include "meshcom_conn.h"
#include "meshcom.h"

#include "log.h"
#include "clock.h"
#include "system.h"
#include "systemconfig.h"
#include "time_meter.h"
#include "rtc.h"
#include "aguard.h"
#include "sequence.h"
#include "image_information.h"
#include "firmware_store.h"
#include "meshcom_firmware_tx.h"
#include "debug.h"
#include "gloom.h"
#include "visibility.h"
#include "gpssync.h"
#include "syndication.h"
#include "detector.h"
#include "indication.h"
#include "fram_mirror.h"
#include "time_meter.h"
#include "psu_monitoring.h"

static bool bFirmwareRxTxEnabled;

void MESHCOM_CMD_SendDatastreamCallback(MESHCOM_RESULT_T result, uint8_t sequence_id);

void MESHCOM_CMD_SendResponse(uint8_t major, uint8_t minor, bool bAck)
{
    // ToDo [medium] implement
    (void)major;
    (void)minor;
    (void)bAck;
}

void MESHCOM_CMD_FirmwareRxTxEnable(bool b_enable)
{
    if (bFirmwareRxTxEnabled != b_enable)
    {
        if (b_enable == TRUE)
            MESHCOM_FIRMWARE_TX_Init();
    }

    bFirmwareRxTxEnabled = b_enable;
}


bool MESHCOM_CMD_CommandstreamReceive(MESHCOM_MSG_S *pMessage)
{
    MESHCOM_COMMANDSTREAM_T * pParameters = (MESHCOM_COMMANDSTREAM_T *) pMessage->message.stream.payload;
    MESHCOM_DATASTREAM_T * datastream_out_payload;
    uint8_t responseLength;

    MESHCOM_MSG_S meshcom_send_message;
    MESSAGE_HEADER_T* stream_params;
    MESHCOM_MSG_STREAM_S* stream_out;

    memset(&meshcom_send_message, 0, sizeof(MESHCOM_MSG_S));

    stream_params = (MESSAGE_HEADER_T*) & meshcom_send_message.header;
    stream_out = &meshcom_send_message.message.stream;
    datastream_out_payload = (MESHCOM_DATASTREAM_T *) stream_out->payload;

    // id will be command identifier
    stream_params->id.minor = pMessage->header.id.minor;
    // send message back to originating source
    stream_params->address.shortAddr = pMessage->header.address.shortAddr;
    stream_params->address.extAddr = 0;
    // init part and length
    responseLength = 0;

    switch (stream_params->id.minor)
    {
    case MESHCOM_COMMANDSTREAM_ID_GET_SETTING:
        DPRINT1(DEBUG_MESH, "MESHCOM <COMMANDSTREAM Get Setting ID: %u>\n", pParameters->data.setting.id);
        responseLength = 4;
        datastream_out_payload->data.setting.id = pParameters->data.setting.id;
        datastream_out_payload->data.setting.value = _settings_list_ram[pParameters->data.setting.id];
        break;

    case MESHCOM_COMMANDSTREAM_ID_SET_SETTING:
        DPRINT1(DEBUG_MESH, "MESHCOM <COMMANDSTREAM Set Setting ID: %u>\n", pParameters->data.setting.id);
        LOG(LOG_ID_MESHCOM_SET_SETTING, pParameters->data.setting.id, _settings_list_ram[pParameters->data.setting.id], pParameters->data.setting.value, NULL);
        SETTINGS_SetRaw(pParameters->data.setting.id, pParameters->data.setting.value);
        break;

    case MESHCOM_COMMANDSTREAM_ID_RESET_SETTINGS:
        DPRINT(DEBUG_MESH, "MESHCOM <COMMANDSTREAM Reset Settings>\n");
        LOG(LOG_ID_MESHCOM_RESET_SETTINGS, 0, 0, 0, NULL);
        SETTINGS_LoadAll();
        break;

    case MESHCOM_COMMANDSTREAM_ID_SAVE_SETTINGS:
        DPRINT(DEBUG_MESH, "MESHCOM <COMMANDSTREAM Save Settings>\n");
        LOG(LOG_ID_MESHCOM_SAVE_SETTINGS, 0, 0, 0, NULL);
        SETTINGS_SaveAll();
        break;

    case MESHCOM_COMMANDSTREAM_ID_SAVE_SETTINGS_AND_REBOOT:
        DPRINT(DEBUG_MESH, "MESHCOM <COMMANDSTREAM Save Settings AND Reboot>\n");
        LOG(LOG_ID_MESHCOM_SAVE_SETTINGS, 0, 0, 0, NULL);
        SETTINGS_SaveAll();
        LOG(LOG_ID_MESHCOM_REBOOT, 0, 0, 0, NULL);
        SYSTEM_Reboot();
        break;

    case MESHCOM_COMMANDSTREAM_ID_GET_SETTING_GROUP:
    {
        uint8_t     setting_idx;

        DPRINT1(DEBUG_MESH, "MESHCOM <COMMANDSTREAM Get Setting Group ID: %u>\n", pParameters->data.setting_group.group_id);

        if (pParameters->data.setting_group.group_id >= SETTINGS_GROUP_COUNT)
            break;

        datastream_out_payload->data.setting_group.group_id = pParameters->data.setting_group.group_id;
        responseLength = 1;

        setting_idx = SETTINGS_GROUP_SIZE * pParameters->data.setting_group.group_id;

        for (uint8_t i = 0; i < SETTINGS_GROUP_SIZE; i++)
        {
            if (i >= DATASTREAM_SETTING_GROUP_ENTRY_COUNT_MAX)
                break;
            datastream_out_payload->data.setting_group.entry[i] = _settings_list_ram[setting_idx + i];
            responseLength += 4;
        }
    }
    break;

    case MESHCOM_COMMANDSTREAM_ID_SET_SETTING_GROUP:
    {
        uint8_t     setting_idx;

        DPRINT1(DEBUG_MESH, "MESHCOM <COMMANDSTREAM Set Setting Group ID: %u>\n", pParameters->data.setting_group.group_id);

        if (pParameters->data.setting_group.group_id >= SETTINGS_GROUP_COUNT)
            break;
        if (pParameters->data.setting_group.group_id <= SETTINGS_GROUP_SYSTEM)
            break;

        LOG(LOG_ID_MESHCOM_SET_SETTING_GROUP, pParameters->data.setting_group.group_id, 0, 0, NULL);

        setting_idx = SETTINGS_GROUP_SIZE * pParameters->data.setting_group.group_id;

        for (uint8_t i = 0; i < SETTINGS_GROUP_SIZE; i++)
        {
            if (i >= DATASTREAM_SETTING_GROUP_ENTRY_COUNT_MAX)
                break;
            DPRINT2(DEBUG_MESH, "MESHCOM <COMMANDSTREAM Set Setting Group ID: Setting %u Value %u > \n", setting_idx + i, pParameters->data.setting_group.entry[i]);
            LOG(LOG_ID_MESHCOM_SET_SETTING, setting_idx + i, _settings_list_ram[setting_idx + i], pParameters->data.setting_group.entry[i], NULL);
            SETTINGS_SetRaw(setting_idx + i, pParameters->data.setting_group.entry[i]);
        }
        break;
    }

    case MESHCOM_COMMANDSTREAM_ID_GET_LOG_INFO:
    {
        DPRINT(DEBUG_MESH, "MESHCOM <COMMANDSTREAM Get Log Info>\n");
        datastream_out_payload->data.log_info.pagesize  = DATASTREAM_LOG_ENTRY_COUNT;
        datastream_out_payload->data.log_info.first     = LOG_GetFirstEntryIndex();  // first entry
        datastream_out_payload->data.log_info.last      = LOG_GetNextEntryIndex();   // next free entry, number of entries = last - first
        datastream_out_payload->data.log_info.failures  = 0;                            // amount of defektive eeprom blocks (0 = everything OK)
        DPRINT3(DEBUG_MESH, "MESHCOM <COMMANDSTREAM Get Log Info pagesize, first, last: %hu, %u, %u >\n", datastream_out_payload->data.log_info.pagesize, datastream_out_payload->data.log_info.first, datastream_out_payload->data.log_info.last);
        responseLength = sizeof(DATASTREAM_LOG_INFO_T);
    }
    break;

    case MESHCOM_COMMANDSTREAM_ID_GET_LOG_ENTRIES:
    {
        DPRINT1(DEBUG_MESH, "MESHCOM <COMMANDSTREAM Get Log Entries Start: %d>\n", pParameters->data.log.begin);

        datastream_out_payload->data.log_entries.first_index = pParameters->data.log.begin;
        uint8_t num = DATASTREAM_LOG_ENTRY_COUNT;
        memset(datastream_out_payload->data.log_entries.entry, 0xff, DATASTREAM_LOG_ENTRY_COUNT * sizeof(LOG_ENTRY_T));
        if (!LOG_GetLogEntry(&pParameters->data.log.begin, datastream_out_payload->data.log_entries.entry, &num, TRUE))
        {
            DPRINT1(DEBUG_MESH, "MESHCOM <COMMANDSTREAM LOG Entry not available: %d>\n", pParameters->data.log.begin);
        }


        responseLength = sizeof(DATASTREAM_LOG_ENTRIES_T);//DATASTREAM_LOG_ENTRY_COUNT * LOG_ENTRYSIZE;
    }
    break;

    case MESHCOM_COMMANDSTREAM_ID_GET_STATUS:
    {
        DPRINT1(DEBUG_MESH, "MESHCOM <COMMANDSTREAM Get Status Page %u>\n", pParameters->data.status.id);

        datastream_out_payload->data.status.id = pParameters->data.status.id;

        switch (pParameters->data.status.id)
        {
        case STATUS_PAGE_GENERAL:
            datastream_out_payload->data.status.data.general.time       = RTC_GetUnixTime();
            datastream_out_payload->data.status.data.general.uptime     = CLOCK_GetTimestamp();
            datastream_out_payload->data.status.data.general.logentries = LOG_GetLogSize();
            datastream_out_payload->data.status.data.general.systemtype = _settings_list_ram[SETTINGS_SYS_SYSTEMTYPE];
            break;

        case STATUS_PAGE_LEDSTAGE_1:
            datastream_out_payload->data.status.data.ledstage_1.led_fail_bm = DETECTOR_GetLedStageFailBitmask(0);
            datastream_out_payload->data.status.data.ledstage_1.led_state   = GLOOM_MANAGER_GetLocalLedState();
            for(DIM_LED_T idx = 0; idx < DIM_LED_COUNT; idx++)
            {
                uint16_t power_level;
                uint16_t power_level_rect;
                uint32_t power_level_rel;
                power_level         = DIM_GetLedPowerLevel(idx);
                power_level_rect    = DIM_GetRectifiedLedPowerLevel(idx);
                if (power_level > 0)
                    power_level_rel = (uint32_t) power_level_rect * 100 / power_level;
                else
                    power_level_rel = 0;

                datastream_out_payload->data.status.data.ledstage_1.led_power_abs[idx] = power_level;
                datastream_out_payload->data.status.data.ledstage_1.led_power_rel[idx] = power_level_rel;
            }
            break;

        case STATUS_PAGE_LEDSTAGE_2:
            for(DIM_LED_T idx = 0; idx < DIM_LED_COUNT; idx++)
            {
                datastream_out_payload->data.status.data.ledstage_2.timemeter[idx] = TIME_METER_GetLedOverallOperatingTime(idx);
            }
            break;

        case STATUS_PAGE_LEDSTAGE_3:
            for(DIM_LED_T idx = 0; idx < DIM_LED_COUNT; idx++)
            {
                datastream_out_payload->data.status.data.ledstage_3.timemeter_comp[idx] = TIME_METER_GetLedCompensatedOverallOperatingTime(idx);
            }
            break;

        case STATUS_PAGE_GPS:
            datastream_out_payload->data.status.data.gps.status         = _gpssync.datetime.status;  // 0: invalid, 1: valid, 2: confirmed
            datastream_out_payload->data.status.data.gps.num_sat_fix    = _gpssync.position.num_of_sv_in_fix;
            datastream_out_payload->data.status.data.gps.sat_id_list    = _gpssync.position.sat_id_list;  // bit mask
            datastream_out_payload->data.status.data.gps.fixtype        = _gpssync.position.nav_type;  // 1: no navigation fix, 2: 2D, 3: 3D
            datastream_out_payload->data.status.data.gps.latitude       = _gpssync.position.latitude;  // outdated if fixtype is < 1
            datastream_out_payload->data.status.data.gps.longitude      = _gpssync.position.longitude;  // outdated if fixtype is < 1
            datastream_out_payload->data.status.data.gps.altitude       = _gpssync.position.altitude;  // outdated if fixtype is < 3
            break;

        case STATUS_PAGE_NETWORK:
        {
            uint16_t syndicate_client_count = 0;
            SYNDICATION_GetSyndicateClientCount(&syndicate_client_count);  // return value is not evaluated because variable is initialized to 0
            datastream_out_payload->data.status.data.network.node.boardType         = _meshcom_node_status.boardType;
            datastream_out_payload->data.status.data.network.node.softVersionMajor  = _meshcom_node_status.softVersionMajor;
            datastream_out_payload->data.status.data.network.node.softVersionMinor  = _meshcom_node_status.softVersionMinor;
            datastream_out_payload->data.status.data.network.node.nodeType          = _meshcom_node_status.nodeType;
            datastream_out_payload->data.status.data.network.node.shortAddr         = _meshcom_node_status.shortAddr;
            datastream_out_payload->data.status.data.network.node.parentShortAddr   = _meshcom_node_status.parentShortAddr;
            datastream_out_payload->data.status.data.network.node.lqi               = _meshcom_node_status.lqi;
            datastream_out_payload->data.status.data.network.node.rssi              = _meshcom_node_status.rssi;
            datastream_out_payload->data.status.data.network.client_count        = syndicate_client_count;
            datastream_out_payload->data.status.data.network.bc_msgs_received    = _meshcom_status.uBroadcastMessagesReceivedCount;
            datastream_out_payload->data.status.data.network.gloom_network_avg   = GLOOM_MANAGER_GetGlobalGloomValueAvg();
            break;
        }

        case STATUS_PAGE_SENSORS:
        {
            TEMPERATURE_SENSOR_T    temperature;
            VISIBILITY_SENSOR_T     visibility;
            for (TEMPERATURE_T idx = 0; idx < TEMPERATURE_COUNT; idx++)
            {
                SENSORS_GetTemperature(idx, &temperature);
                datastream_out_payload->data.status.data.sensors.temperature[idx]               = (int16_t)temperature.value;
                datastream_out_payload->data.status.data.sensors.temperature_sensor_status[idx] = (int16_t)temperature.state;
            }
            datastream_out_payload->data.status.data.sensors.gloom                  = (uint16_t)SENSORS_GetGloomEMA();
            datastream_out_payload->data.status.data.sensors.gloom_sensor_status    = GLOOM_MANAGER_GetReliability();;
            VISIBILITY_GetVisibility(&visibility);
            datastream_out_payload->data.status.data.sensors.visibility                 = visibility.value_rms_1min;
            datastream_out_payload->data.status.data.sensors.visibility_sensor_status   = visibility.state;
            datastream_out_payload->data.status.data.sensors.visibility_cluster_id      = VISIBILITY_CLUSTER_ID;
        }
        break;

        case STATUS_PAGE_FIRMWARE:
        {
            IMAGE_INFORMATION_T *psFileInfo;
            FISTO_GetFileInfo(&psFileInfo);
            datastream_out_payload->data.status.data.firmware.hw_id             = psFileInfo->hw_id;
            datastream_out_payload->data.status.data.firmware.hw_revision       = psFileInfo->hw_revision;
            datastream_out_payload->data.status.data.firmware.sw_version_major  = psFileInfo->sw_version_major;
            datastream_out_payload->data.status.data.firmware.sw_version_minor  = psFileInfo->sw_version_minor;
            datastream_out_payload->data.status.data.firmware.size              = psFileInfo->size;
            datastream_out_payload->data.status.data.firmware.checksum          = psFileInfo->checksum;
            uint8_t retry_bits;
            if (!FRAM_MIRROR_ReadByte(FRAM_FW_UPDATE_RETRIES, &retry_bits))
                retry_bits = 0;
            datastream_out_payload->data.status.data.firmware.retry_bits        = retry_bits;
            break;
        }

#if (SVINFOCOM_IMPLEMENTATION_FINISHED == 1)
        case STATUS_PAGE_PSU_1:
        {
            // attention this needs a lot of memory from stack!
            // todo[medium] check if this needs to be changed
            PSU_MONITORING_PSU_INFO_S psu_info;
            PSU_MONITORING_PSU_SENSORS_S sensors;
            PSU_MONITORING_PSU_STATUS_S psu_status;
            PSU_MONITORING_PSU_FAN_INFO_S fan_info;

            memset(&datastream_out_payload->data.status.data.psu_1, 0xff, sizeof(datastream_out_payload->data.status.data.psu_1));

            bool b_info_valid = FALSE;

            if (PSU_MONITORING_LockTimeout(MESHCOM_CMD_GET_PSU_DATA_TIMEOUT_MS))
            {
                b_info_valid  = PSU_MONITORING_Get_PSU_Info(&psu_info);
                b_info_valid &= PSU_MONITORING_Get_PSU_Sensors(&sensors);
                b_info_valid &= PSU_MONITORING_Get_PSU_Status(&psu_status);
                b_info_valid &= PSU_MONITORING_Get_PSU_FanInfo(&fan_info);
                PSU_MONITORING_Unlock();
            }

            if (b_info_valid)
            {
                datastream_out_payload->data.status.data.psu_1.serial = psu_info.serial;
                datastream_out_payload->data.status.data.psu_1.hw_id = psu_info.hw_id;
                datastream_out_payload->data.status.data.psu_1.hw_revision = psu_info.hw_revision;
                datastream_out_payload->data.status.data.psu_1.sw_version_major = psu_info.sw_version_major;
                datastream_out_payload->data.status.data.psu_1.sw_version_minor = psu_info.sw_version_minor;
                datastream_out_payload->data.status.data.psu_1.sys_voltage = sensors.supply_voltage;
                datastream_out_payload->data.status.data.psu_1.sys_temperature_value = sensors.temperature_sensor_system.value;
                datastream_out_payload->data.status.data.psu_1.sys_temperature_status = sensors.temperature_sensor_system.state;
                datastream_out_payload->data.status.data.psu_1.ups_status = psu_status.ups_status;
                datastream_out_payload->data.status.data.psu_1.ups_raw_status = psu_status.ups_raw_status;
                datastream_out_payload->data.status.data.psu_1.ups_battery_status = psu_status.ups_battery_status;
                datastream_out_payload->data.status.data.psu_1.rating_temperature = psu_status.temperature_rating_system;
                datastream_out_payload->data.status.data.psu_1.rating_thermal_fuse = psu_status.temperature_fuse_rating;
                datastream_out_payload->data.status.data.psu_1.rating_voltage = psu_status.voltage_rating;
                datastream_out_payload->data.status.data.psu_1.fan_time_meter = fan_info.timemeter_fan;
                datastream_out_payload->data.status.data.psu_1.fan_service_status = psu_status.service_fan;
                datastream_out_payload->data.status.data.psu_1.fan_filter_time_meter = fan_info.timemeter_fan_filter;
                datastream_out_payload->data.status.data.psu_1.fan_filter_service_status = psu_status.service_fan_filter;
            }
            break;
        }

        case STATUS_PAGE_PSU_2:
        {
            PSU_MONITORING_BATTERY_INFO_S battery_info;
            PSU_MONITORING_PSU_STATUS_S psu_status;

            memset(&datastream_out_payload->data.status.data.psu_2, 0xff, sizeof(datastream_out_payload->data.status.data.psu_2));

            if (PSU_MONITORING_Get_PSU_Status(&psu_status))
            {
                if (PSU_MONITORING_Get_PSU_BatteryInfo(0, &battery_info))
                {
                    datastream_out_payload->data.status.data.psu_2.battery_1_temperature_value = battery_info.temperature_sensor_battery.value;
                    datastream_out_payload->data.status.data.psu_2.battery_1_temperature_status = battery_info.temperature_sensor_battery.state;
                    datastream_out_payload->data.status.data.psu_2.battery_1_time_meter_total = battery_info.timemeter_battery_total;
                    datastream_out_payload->data.status.data.psu_2.battery_1_time_meter_total_status = battery_info.timemeter_battery_total_status;
                    datastream_out_payload->data.status.data.psu_2.battery_1_time_meter_hot = battery_info.timemeter_battery_hot;
                    datastream_out_payload->data.status.data.psu_2.battery_1_time_meter_hot_status = battery_info.timemeter_battery_hot_status;
                    datastream_out_payload->data.status.data.psu_2.battery_1_service_status = psu_status.service_battery_1;
                }

                if (PSU_MONITORING_Get_PSU_BatteryInfo(1, &battery_info))
                {
                    datastream_out_payload->data.status.data.psu_2.battery_2_temperature_value = battery_info.temperature_sensor_battery.value;
                    datastream_out_payload->data.status.data.psu_2.battery_2_temperature_status = battery_info.temperature_sensor_battery.state;
                    datastream_out_payload->data.status.data.psu_2.battery_2_time_meter_total = battery_info.timemeter_battery_total;
                    datastream_out_payload->data.status.data.psu_2.battery_2_time_meter_total_status = battery_info.timemeter_battery_total_status;
                    datastream_out_payload->data.status.data.psu_2.battery_2_time_meter_hot = battery_info.timemeter_battery_hot;
                    datastream_out_payload->data.status.data.psu_2.battery_2_time_meter_hot_status = battery_info.timemeter_battery_hot_status;
                    datastream_out_payload->data.status.data.psu_2.battery_2_service_status = psu_status.service_battery_2;
                }
            }
            break;
        }
#endif

        default:
            break;
        }

        responseLength = sizeof(DATASTREAM_STATUS_T);
    }
    break;

    case MESHCOM_COMMANDSTREAM_ID_FIRMWARE_GET:
        // peer requested firmware. enable stream out data.
        DPRINT(DEBUG_MESH, "MESHCOM <COMMANDSTREAM Firmware Get>\n");
        MESHCOM_FIRMWARE_TX_Request(pMessage->header.address.shortAddr, pParameters->data.command.param1, pParameters->data.command.param2);
        break;

    case MESHCOM_COMMANDSTREAM_ID_FIRMWARE_TX_INIT:
    {
        DPRINT(DEBUG_MESH, "MESHCOM <COMMANDSTREAM Firmware TX Init>\n");
        IMAGE_INFORMATION_T sFileInfo;
        sFileInfo.hw_id             = pParameters->data.firmware_header.hw_id;
        sFileInfo.hw_revision       = pParameters->data.firmware_header.hw_revision;
        sFileInfo.sw_version_major  = pParameters->data.firmware_header.sw_version_major;
        sFileInfo.sw_version_minor  = pParameters->data.firmware_header.sw_version_minor;
        sFileInfo.size              = pParameters->data.firmware_header.size;
        sFileInfo.checksum          = pParameters->data.firmware_header.checksum;

        if (bFirmwareRxTxEnabled)
            FISTO_StartReceiver(&sFileInfo, (FWT_SELECTED_FW_T)pParameters->data.firmware_header.fw_type);
    }
    break;

    case MESHCOM_COMMANDSTREAM_ID_REBOOT:
        LOG(LOG_ID_MESHCOM_REBOOT, 0, 0, 0, NULL);
        DPRINT(DEBUG_MESH, "MESHCOM <COMMANDSTREAM Reboot>\n");
        switch (pParameters->data.command.param1)
        {
        case 1:
            SYSTEM_Reboot();
            break;
        case 2:
            SYSTEM_InvokeBootloader();
            break;
        default:
            break;
        }
        break;

    case MESHCOM_COMMANDSTREAM_LIGHTUP:
        DPRINT1(DEBUG_MESH, "MESHCOM <COMMANDSTREAM Test Lightup with an interval of %u seconds>\n", pParameters->data.command.param1);
        if (pParameters->data.command.param1 == 0)
            break;
        if (pParameters->data.command.param1 > MESHCOM_COMMAND_LED_LIGHTUP_INTERVAL_MAX)
            pParameters->data.command.param1 = MESHCOM_COMMAND_LED_LIGHTUP_INTERVAL_MAX;
        SEQUENCE_TestDriveStart(pParameters->data.command.param1);
        break;

    case MESHCOM_COMMANDSTREAM_ID_TEST_INDICATION:
        if (pParameters->data.command.param2 == 0)
            break;
        if (pParameters->data.command.param2 > 300)
            pParameters->data.command.param2 = 300;

        LOG(LOG_ID_MESHCOM_TEST_INDICATION, pParameters->data.command.param1, 0, 0, NULL);

        if (pParameters->data.command.param1 & 1<<IND_A)
            INDICATION_Set(IND_A, (IND_TIMEOUT_T)pParameters->data.command.param2);
        else
            INDICATION_Reset(IND_A);

        if (pParameters->data.command.param1 & 1<<IND_B)
            INDICATION_Set(IND_B, (IND_TIMEOUT_T)pParameters->data.command.param2);
        else
            INDICATION_Reset(IND_B);

        break;

    default:
        break;
    }

    if (responseLength > 0)
    {
        datastream_out_payload->header.sequence_id = pParameters->header.sequence_id;
        stream_params->length = responseLength + sizeof(DATASTREAM_HEADER_T);
        MESHCOM_SendDatastream(&meshcom_send_message, pMessage->header.port, MESHCOM_CMD_SendDatastreamCallback, 0);
    }
    return TRUE;
}

void MESHCOM_CMD_SendDatastreamCallback(MESHCOM_RESULT_T result, uint8_t sequence_id)
{
    // ToDo [medium] implement
    (void)sequence_id;
    (void)result;
}

bool MESHCOM_CMD_DatastreamReceive(MESHCOM_MSG_S *pMessage)
{
    bool bSend = FALSE;

    MESHCOM_DATASTREAM_T *pData = (MESHCOM_DATASTREAM_T*)pMessage->message.stream.payload;
    MESHCOM_MSG_S MessageOut;
    MESHCOM_DATASTREAM_T *pDataOut = (MESHCOM_DATASTREAM_T*)&MessageOut.message.stream.payload;

    // send response
    MessageOut.header.address.shortAddr = pMessage->header.address.shortAddr;
    MessageOut.header.address.extAddr = 0;
    MessageOut.header.port = MESHCOM_PORT_UNICAST_NOACK;
    MessageOut.header.id.minor = MESHCOM_COMMANDSTREAM_ID_ACK;
    MessageOut.header.length = sizeof(pDataOut->data.response) + sizeof(DATASTREAM_HEADER_T);
    pDataOut->header.sequence_id = pData->header.sequence_id;


    if ((pMessage->header.length - sizeof(DATASTREAM_HEADER_T)) <= DATASTREAM_PAYLOAD_MAX)
    {
        switch (pMessage->header.id.minor)
        {
        case MESHCOM_DATASTREAM_ID_PING:
            DPRINT2(DEBUG_MESH, "MESHCOM <DATASTREAM Ping SeqNumber: %u Payload: %u Bytes>\n", pData->header.sequence_id, pMessage->header.length - sizeof(DATASTREAM_HEADER_T));
            MessageOut.header.id.minor = MESHCOM_DATASTREAM_ID_PING_RESPONSE;
            MessageOut.header.length = pMessage->header.length;
            memcpy(pDataOut->data.rawdata, pData->data.rawdata, pMessage->header.length - sizeof(DATASTREAM_HEADER_T));
            bSend = TRUE;
            break;

        case MESHCOM_DATASTREAM_ID_FIRMWARE:
            if (bFirmwareRxTxEnabled)
            {
                switch (FISTO_Write(pData->data.firmware.uAddress, pData->data.firmware.uLength, pData->data.firmware.aData))
                {
                case FISTO_OK:
                    pDataOut->data.response.eResponse = MESHCOM_CMD_RESPONSE_ACK;
                    break;
                case FISTO_ABORT:
                    pDataOut->data.response.eResponse = MESHCOM_CMD_RESPONSE_ABORT;
                    break;
                default:
                    pDataOut->data.response.eResponse = MESHCOM_CMD_RESPONSE_NACK;
                    break;
                }
                bSend = TRUE;
            }
            break;
        default:
            break;
        }
    }
    else
    {
        // max datastream payload size exceeded.
        DPRINT3(DEBUG_MESH, "MESHCOM DATASTREAM Invalid Payload size! MinorID: 0x%02x SeqNumber: %u Size: %u Bytes\n", pMessage->header.id.minor, pData->header.sequence_id, pMessage->header.length - sizeof(DATASTREAM_HEADER_T));
        pDataOut->data.response.eResponse = MESHCOM_CMD_RESPONSE_ERR_SIZE;
        bSend = TRUE;
    }

    if (bSend)
    {
        MESHCOM_SendDatastream(&MessageOut, MessageOut.header.port, MESHCOM_CMD_SendDatastreamCallback, 0);
    }

    return TRUE;
}


void MESHCOM_CMD_RequestFirmware(uint8_t uShortAddr, FWT_SELECTED_FW_T FwType)
{
    MESHCOM_COMMANDSTREAM_T* commandstream;
    MESHCOM_MSG_S meshcom_send_message;

    memset(&meshcom_send_message, 0, sizeof(MESHCOM_MSG_S));

    MESSAGE_HEADER_T* stream_params;

    stream_params = (MESSAGE_HEADER_T*) & meshcom_send_message.header;
    commandstream = (MESHCOM_COMMANDSTREAM_T *) meshcom_send_message.message.stream.payload;

    stream_params->id.minor             = MESHCOM_COMMANDSTREAM_ID_FIRMWARE_GET;
    stream_params->length               = sizeof(COMMANDSTREAM_COMMAND_T);
    stream_params->address.extAddr      = 0;
    stream_params->address.shortAddr    = uShortAddr;
    commandstream->data.command.param1  = FwType;
    commandstream->data.command.param2  = SERIAL_NUMBER;

    MESHCOM_SendCommandstream(&meshcom_send_message, MESHCOM_FIRMWARE_TX_Confirm, 0);
}

void MESHCOM_CMD_Tick(void)
{
    // FWTX is switched off in current release via advanced setting. Functionality has to be proven and tested. CBA ToDo
    if (bFirmwareRxTxEnabled)
        MESHCOM_FIRMWARE_TX_Tick();
}

void MESHCOM_CMD_Init(void)
{
    MESHCOM_CMD_FirmwareRxTxEnable(TRUE);
}

