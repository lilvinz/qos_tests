/*
 * sv_uni_com.h
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

/* Naming conventions
 *
 * Message = one member from SV_UNI_COM_MESSAGE_S::uni which is transported as payload in packets
 * Header  = start sequence of each packet
 * Footer  = end sequence of each packet
 * Packet  = Header + Payload + Footer
 *
 */

#ifndef SV_UNI_COM_H
#define SV_UNI_COM_H

#include "sensor.h"
#include "svinfo.h"
#include "svwire.h"

#define SV_UNI_UART_SEND(c) SVINFO_PutChar(c)

#define SV_UNI_COM_V1_MESSAGESIZE_MAX   128
#define SV_UNI_COM_MAX_DROPPED_BYTES_ALLOWED    (2 * SV_UNI_COM_V1_MESSAGESIZE_MAX)

#define SV_UNI_COM_V1_FRAME_BOARDERSIZE 7        // start+len+checksum+end
#define POS_START_SEQ_1                 0        // message start mark 1    (0xA0)
#define POS_START_SEQ_2                 1        // message start mark 2    (0xA2)
#define POS_LENGTH                      2        // length
#define POS_PAYLOAD                     3        // start of payload
#define POS_OFFSET_CHECKSUM_UW          3        // offset from end of msg for checksum upper byte
#define POS_OFFSET_CHECKSUM_LW          2        // offset from end of msg for checksum lower byte

#define SV_UNI_COM_ERR_CNT_MAX 10

#define SV_UNI_COM_PROTOCOL_VERSION_MAJOR    1

#define SV_UNI_COM_SET_DATA_TIMEOUT_MS 10 // 10 ms
#define SV_UNI_COM_MESSAGE_PARSE_TIMEOUT_S 60

typedef enum
{
    SV_UNI_COM_STATUS_FAILED = -1,
    SV_UNI_COM_STATUS_INIT = 0,
    SV_UNI_COM_STATUS_NOT_AVAILABLE,
    SV_UNI_COM_STATUS_OK,
} SV_UNI_COM_STATUS_T;

typedef struct
{
    float           value;
    SENSOR_STATUS_T  state;
} PACK SV_UNI_COM_TEMPERATURE_SENSOR_T;

typedef enum
{
    SV_UNI_COM_MSG_ID_INFO = 0,
    SV_UNI_COM_MSG_ID_STATUS,
    SV_UNI_COM_MSG_ID_SENSORS,
    SV_UNI_COM_MSG_ID_COUNT
} SV_UNI_COM_MSG_ID_T;

typedef struct
{
    uint16_t hw_id;  // 2 Bytes
    uint16_t hw_revision;  // 2 Bytes
    uint16_t sw_version_major;  // 2 Bytes
    uint16_t sw_version_minor;  // 2 Bytes
    uint32_t serial;  // 4 Bytes
} PACK SV_UNI_COM_MSG_INFO_S;  // 12 Bytes

typedef struct
{
    SVWIRE_UPS_CONTROL_UPS_STATUS_T             ups_status;  // 1 Byte
    uint8_t                                     ups_raw_status;  // 1 Byte
    SVWIRE_UPS_CONTROL_BATTERY_STATUS_T         ups_battery_status;  // 1 Byte
    SVWIRE_DETECTOR_TEMPERATURE_RATING_T        temperature_rating_system;  // 1 Byte
    SVWIRE_DETECTOR_TEMPERATURE_FUSE_RATING_T   temperature_fuse_rating;  // 1 Byte
    SVWIRE_DETECTOR_VOLTAGE_RATING_T            voltage_rating;  // 1 Byte
    SVWIRE_DETECTOR_SERVICE_STATUS_T            service_fan;  // 1 Byte
    SVWIRE_DETECTOR_SERVICE_STATUS_T            service_fan_filter;  // 1 Byte
    SVWIRE_DETECTOR_SERVICE_STATUS_T            service_battery_1;  // 1 Byte
    SVWIRE_DETECTOR_SERVICE_STATUS_T            service_battery_2;  // 1 Byte
} PACK SV_UNI_COM_MSG_STATUS_S;  // 10 Bytes

typedef struct
{
    SV_UNI_COM_TEMPERATURE_SENSOR_T                     temperature_sensor_system;  // 5 Bytes
    SV_UNI_COM_TEMPERATURE_SENSOR_T                     temperature_sensor_battery_1;  // 5 Bytes
    SV_UNI_COM_TEMPERATURE_SENSOR_T                     temperature_sensor_battery_2;  // 5 Bytes
    float                                               supply_voltage;  // 4 Bytes
    uint32_t                                            timemeter_fan;  // 4 Bytes
    uint32_t                                            timemeter_fan_filter;  // 4 Bytes
    uint32_t                                            timemeter_battery_1_total;  // 4 Bytes
    uint32_t                                            timemeter_battery_1_hot;  // 4 Bytes
    SVWIRE_BATTERY_MANAGER_BATTERY_TIMEMETER_STATUS_T   timemeter_battery_1_total_status;  // 1 Byte
    SVWIRE_BATTERY_MANAGER_BATTERY_TIMEMETER_STATUS_T   timemeter_battery_1_hot_status;  // 1 Byte
    uint32_t                                            timemeter_battery_2_total;  // 4 Bytes
    uint32_t                                            timemeter_battery_2_hot;  // 4 Bytes
    SVWIRE_BATTERY_MANAGER_BATTERY_TIMEMETER_STATUS_T   timemeter_battery_2_total_status;  // 1 Byte
    SVWIRE_BATTERY_MANAGER_BATTERY_TIMEMETER_STATUS_T   timemeter_battery_2_hot_status;  // 1 Byte
} PACK SV_UNI_COM_MSG_SENSORS_S;  // 47 Bytes

typedef struct
{
    uint8_t version;  //  1 Byte protocol version
    SV_UNI_COM_MSG_ID_T id;  //  1 Byte value for big switch statement
    uint8_t length;  //  1 Byte length of following message
} PACK SV_UNI_COM_MSG_HEADER_T;  // 3 Bytes HEADER

typedef struct
{
    SV_UNI_COM_MSG_HEADER_T header;  // 3 Bytes
    union
    {
        SV_UNI_COM_MSG_INFO_S info;  // 12 Bytes
        SV_UNI_COM_MSG_STATUS_S status;  // 10 Bytes
        SV_UNI_COM_MSG_SENSORS_S sensors;  // 47 Bytes
    } PACK message;
} PACK SV_UNI_COM_MSG_S;

typedef struct
{
    uint8_t Start1;
    uint8_t Start2;
    uint8_t Length;
} PACK SV_UNI_COM_FRAME_HEADER_S;

typedef struct
{
    uint8_t CRC_High;
    uint8_t CRC_Low;
    uint8_t Stop1;
    uint8_t Stop2;
} PACK SV_UNI_COM_FRAME_FOOTER_S;

typedef struct
{
    SV_UNI_COM_FRAME_HEADER_S header;
    SV_UNI_COM_MSG_S payload;
    SV_UNI_COM_FRAME_FOOTER_S footer; // Note: payload has varying size, do not access footer in here!
} PACK SV_UNI_COM_PACKET_S;

#define SV_UNI_COM_MSG_HEADER_SIZE          (sizeof(SV_UNI_COM_MSG_HEADER_T))
#define SV_UNI_COM_MSG_INFO_SIZE            (sizeof(SV_UNI_COM_MSG_INFO_S))
#define SV_UNI_COM_MSG_STATUS_SIZE          (sizeof(SV_UNI_COM_MSG_STATUS_S))
#define SV_UNI_COM_MSG_SENSORS_SIZE         (sizeof(SV_UNI_COM_MSG_SENSORS_S))

typedef enum
{
    SV_UNI_COM_RESULT_TIMEOUT = -3,
    SV_UNI_COM_RESULT_ERROR_PARAM = -2,
    SV_UNI_COM_RESULT_ERROR = -1,
    SV_UNI_COM_RESULT_BUSY = 0,
    SV_UNI_COM_RESULT_OK = 1,
//    SV_UNI_COM_RESULT_PENDING = 2
} SV_UNI_COM_RESULT_T;

void SV_UNI_COM_Tick(void);
void SV_UNI_COM_Init(void);

#endif
