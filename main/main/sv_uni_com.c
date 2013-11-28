/*
 * sv_uni_com.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 31.10.2011
 *      Author: DHA
 *
 * Purpose:
 * Receive messages from svinfo
 *
 */
#include "stdheader.h"
#include "sv_uni_com.h"

#include "debug.h"
#include "psu_monitoring.h"
#include "version.h"
#include "clock.h"
#include "timeout.h"
#include "log.h"

#include "systemconfig.h"

static uint8_t sv_uni_combuffer[SV_UNI_COM_V1_MESSAGESIZE_MAX];
static uint8_t sv_uni_combuffer_index = 0;

static uint32_t num_bytes_dropped = 0;  // enough for one day at 300 baud
static uint16_t num_buffer_clearances = 0;
static uint16_t num_crc_errors = 0;

static TIMEOUT_T MessageParseTimeout = 0;
//static uint8_t MessageParseErrorCount = 0;
static SV_UNI_COM_STATUS_T sv_uni_com_status;  // used to manage module status

/**
  * @brief  Receive byte. Return pointer to frame if reception completed.
  * @param  byte: byte to put into buffer
  * @retval Pointer to frame if frame completed . Otherwise returns NULL.
*/
SV_UNI_COM_MSG_S* SV_UNI_COM_Receive(uint8_t byte)
{
    if (sv_uni_combuffer_index >= SV_UNI_COM_V1_MESSAGESIZE_MAX)
    {
        if (num_buffer_clearances < UINT16_T_MAX)
            num_buffer_clearances++;
        sv_uni_combuffer_index = 0;
    }

    sv_uni_combuffer[sv_uni_combuffer_index++] = byte;

    // '0xA0' '0xA2' marks the beginning of a message frame
    switch (sv_uni_combuffer_index)
    {
    case 0:
        break;
    case 1:
        if (sv_uni_combuffer[POS_START_SEQ_1] != 0xA0)
        {
            if (num_bytes_dropped < UINT32_T_MAX)
                num_bytes_dropped++;
            sv_uni_combuffer_index = 0;
        }
        break;  // Check for 1st Frame Start Mark
    case 2:
        if (sv_uni_combuffer[POS_START_SEQ_2] != 0xA2)
        {
            if (num_bytes_dropped < (UINT32_T_MAX - 1))
                num_bytes_dropped += 2;
            sv_uni_combuffer_index = 0;
        }
        break;  // Check for 2nd Frame Start Mark
//    case 3:
//        if (sv_uni_combuffer[POS_LENGTH]  != 0x00) sv_uni_combuffer_index = 0;
//        break;    // Reject messages larger than 255 Bytes
    default:
        if ((sv_uni_combuffer[POS_LENGTH] + SV_UNI_COM_V1_FRAME_BOARDERSIZE) < sv_uni_combuffer_index)
        {
            if (num_buffer_clearances < UINT16_T_MAX)
                num_buffer_clearances++;
            sv_uni_combuffer_index = 0;
        }
        break; // Prevent buffer overflow
    }

    if ((sv_uni_combuffer_index > (SV_UNI_COM_V1_FRAME_BOARDERSIZE - 1))
            && ((sv_uni_combuffer[POS_LENGTH] + SV_UNI_COM_V1_FRAME_BOARDERSIZE) == sv_uni_combuffer_index)
            && (byte == 0xB3))
    {
        // Frame received! Check frame checksum
        uint16_t checksum = 0;
        for (uint8_t i = POS_PAYLOAD; i < (sv_uni_combuffer[POS_LENGTH] + POS_PAYLOAD); ++i)
        {
            checksum += sv_uni_combuffer[i];
        }
        checksum &= 0x7FFF;

        if ((sv_uni_combuffer[sv_uni_combuffer_index - POS_OFFSET_CHECKSUM_UW - 1] == (0xFF & (checksum >> 8)))
                && (sv_uni_combuffer[sv_uni_combuffer_index - POS_OFFSET_CHECKSUM_LW - 1] == (0xFF & checksum)))
        {
            // checksum is ok, process packet
            // sv_uni_com_V1_parse(&sv_uni_combuffer[POS_PAYLOAD]);
            sv_uni_combuffer_index = 0;
            return (SV_UNI_COM_MSG_S*) &sv_uni_combuffer[POS_PAYLOAD];
        }
        else
        {
            if (num_crc_errors < UINT16_T_MAX)
                num_crc_errors++;
        }
        sv_uni_combuffer_index = 0;
    }
    return 0;
}

/**
  * @brief  Parse message.
  * @param  pMessage:  pointer to message.
  * @retval None
*/
void SV_UNI_COM_Parse(SV_UNI_COM_MSG_S *pMessage)
{
    //save timestamp for timeout detection
    TIMEOUT_Reset(MessageParseTimeout);  // todo[high] add timeout

    // If major version does not match, return immediately to prevent errors.
    if (pMessage->header.version != SV_UNI_COM_PROTOCOL_VERSION_MAJOR)
        return;

    switch (pMessage->header.id)
    {
    case SV_UNI_COM_MSG_ID_INFO:
    {
        PSU_MONITORING_PSU_INFO_S psu_info;
        psu_info.serial = pMessage->message.info.serial;
        psu_info.hw_id = pMessage->message.info.hw_id;
        psu_info.hw_revision = pMessage->message.info.hw_revision;
        psu_info.sw_version_major = pMessage->message.info.sw_version_major;
        psu_info.sw_version_minor = pMessage->message.info.sw_version_minor;
        if (PSU_MONITORING_LockTimeout(SV_UNI_COM_SET_DATA_TIMEOUT_MS))
        {
            PSU_MONITORING_Set_PSU_Info(&psu_info);
            PSU_MONITORING_Unlock();
        }
        break;
    }
    case SV_UNI_COM_MSG_ID_STATUS:
    {
        PSU_MONITORING_PSU_STATUS_S psu_status;
        psu_status.ups_status = pMessage->message.status.ups_status;
        psu_status.ups_raw_status = pMessage->message.status.ups_raw_status;
        psu_status.ups_battery_status = pMessage->message.status.ups_battery_status;
        psu_status.temperature_rating_system = pMessage->message.status.temperature_rating_system;
        psu_status.temperature_fuse_rating = pMessage->message.status.temperature_fuse_rating;
        psu_status.voltage_rating = pMessage->message.status.voltage_rating;
        psu_status.service_fan = pMessage->message.status.service_fan;
        psu_status.service_fan_filter = pMessage->message.status.service_fan;
        psu_status.service_battery_1 = pMessage->message.status.service_battery_1;
        psu_status.service_battery_2 = pMessage->message.status.service_battery_2;
        if (PSU_MONITORING_LockTimeout(SV_UNI_COM_SET_DATA_TIMEOUT_MS))
        {
            PSU_MONITORING_Set_PSU_Status(&psu_status);
            PSU_MONITORING_Unlock();
        }
        break;
    }

    case SV_UNI_COM_MSG_ID_SENSORS:
    {
        if (PSU_MONITORING_LockTimeout(SV_UNI_COM_SET_DATA_TIMEOUT_MS))
        {
            // Attention:
            // Here we locked psu data!
            // Do not add functions which need lots of time (a few assignments should be ok).
            PSU_MONITORING_PSU_SENSORS_S psu_sensors;
            psu_sensors.temperature_sensor_system.state = pMessage->message.sensors.temperature_sensor_system.state;
            psu_sensors.temperature_sensor_system.value = pMessage->message.sensors.temperature_sensor_system.value;
            psu_sensors.supply_voltage = pMessage->message.sensors.supply_voltage;
            PSU_MONITORING_Set_PSU_Sensors(&psu_sensors);

            PSU_MONITORING_PSU_FAN_INFO_S psu_fan_info;
            psu_fan_info.timemeter_fan = pMessage->message.sensors.timemeter_fan;
            psu_fan_info.timemeter_fan_filter = pMessage->message.sensors.timemeter_fan_filter;
            PSU_MONITORING_Set_PSU_FanInfo(&psu_fan_info);

            PSU_MONITORING_BATTERY_INFO_S battery_info;
            battery_info.temperature_sensor_battery.state = pMessage->message.sensors.temperature_sensor_battery_1.state;
            battery_info.temperature_sensor_battery.value = pMessage->message.sensors.temperature_sensor_battery_1.value;
            battery_info.timemeter_battery_total = pMessage->message.sensors.timemeter_battery_1_total;
            battery_info.timemeter_battery_hot = pMessage->message.sensors.timemeter_battery_1_hot;
            battery_info.timemeter_battery_total_status = pMessage->message.sensors.timemeter_battery_1_total_status;
            battery_info.timemeter_battery_hot_status = pMessage->message.sensors.timemeter_battery_1_hot_status;
            PSU_MONITORING_Set_PSU_BatteryInfo(0, &battery_info);

            battery_info.temperature_sensor_battery.state = pMessage->message.sensors.temperature_sensor_battery_2.state;
            battery_info.temperature_sensor_battery.value = pMessage->message.sensors.temperature_sensor_battery_2.value;
            battery_info.timemeter_battery_total = pMessage->message.sensors.timemeter_battery_2_total;
            battery_info.timemeter_battery_hot = pMessage->message.sensors.timemeter_battery_2_hot;
            battery_info.timemeter_battery_total_status = pMessage->message.sensors.timemeter_battery_2_total_status;
            battery_info.timemeter_battery_hot_status = pMessage->message.sensors.timemeter_battery_2_hot_status;
            PSU_MONITORING_Set_PSU_BatteryInfo(1, &battery_info);

            PSU_MONITORING_Unlock();
        }
        break;
    }
    break;
    default:
        break;
    }
}

void SV_UNI_COM_CheckReceiveErrors(void)
{
    // this should be executed one time a day
    if ((num_bytes_dropped > SV_UNI_COM_MAX_DROPPED_BYTES_ALLOWED) || (num_buffer_clearances > 0) || (num_crc_errors > 0))
    {
        // we accept no crc errors and no buffer cleareances but a few dropped bytes because svwire and sirius need time to synchronize
        LOG(LOG_ID_SV_UNI_COM_RECEIVE_ERROR, num_crc_errors, num_buffer_clearances, num_bytes_dropped, NULL);
    }
    num_bytes_dropped = 0;  // enough for one day at 300 baud
    num_buffer_clearances = 0;
    num_crc_errors = 0;
}

void SV_UNI_COM_CheckTimeouts(void)
{
    if (TIMEOUT_Check(MessageParseTimeout, SV_UNI_COM_MESSAGE_PARSE_TIMEOUT_S))
    {
        TIMEOUT_Reset(MessageParseTimeout);

//        sv_uni_com_status  todo[high]
//        if (MessageParseErrorCount < UINT8_T_MAX)
//            MessageParseErrorCount++;
    }
}

/**
  * @brief  Manage svinfo communication.
  * @param  None
  * @retval None
*/
void SV_UNI_COM_Tick(void)
{
    SV_UNI_COM_MSG_S* pRecvMessage;

    // handle received characters
    while(SVINFO_CharAvail())
    {
        if ((pRecvMessage = SV_UNI_COM_Receive(SVINFO_GetChar())) != NULL)
        {
            SV_UNI_COM_Parse(pRecvMessage);
        }
    }

    // check for timeouts
}

/**
  * @brief  Enable or disable external interrupt for svinfo input.
  * @param  new_state:  - enable: external interrupt will be enabled.
  * @param              - disable: external interrupt will be disabled.
  * @retval None
*/
void SV_UNI_COM_Init(void)
{
    sv_uni_com_status = SV_UNI_COM_STATUS_INIT;
    num_bytes_dropped = 0;
    num_buffer_clearances = 0;
    num_crc_errors = 0;

    SVINFO_Create();
    SVINFO_Init();
    TIMEOUT_Reset(MessageParseTimeout);
}
