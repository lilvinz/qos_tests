/***************************************************************************
 *   Copyright (C) Quantec Networks 2009
 ***************************************************************************/
/* Naming conventions
 *
 * Message = one member from MESHCOM_MESSAGE_S::uni which is transported as payload in packets
 * Header  = start sequence of each packet
 * Footer  = end sequence of each packet
 * Packet  = Header + Payload + Footer
 *
 */

#ifndef MESHCOM_H
#define MESHCOM_H

#include "settings.h"

#define MESHCOM_SETTING_TYPE                         _settings_list_ram[SETTINGS_MESH_TYPE]
#define MESHCOM_SETTING_CHANNEL_MASK                 _settings_list_ram[SETTINGS_MESH_CHANNEL_MASK]
#define MESHCOM_SETTING_PAN_ID                       _settings_list_ram[SETTINGS_MESH_PANID]
#define MESHCOM_SETTING_TRANSMITTER_POWER            _settings_list_ram[SETTINGS_MESH_TRANSMITTER_POWER]

#define SETTINGS_MESH_UID_BASE        0x5154454300000000ULL
#define SETTINGS_MESH_EXTPANID_BASE   0x5154454300000000ULL


#define MESHCOM_PROTOCOL_VERSION_MAJOR    3
#define MESHCOM_PROTOCOL_VERSION_MINOR    0

#define MESHCOM_MESSAGE_PARSE_TIMEOUT_SEC        60
#define MESHCOM_NETWORKDATA_PARSE_TIMEOUT_SEC    120

#define MESHCOM_MESH_PACKET_SIZE_MAX      51
#define MESHCOM_PRE_MSG_HEADER_SIZE       6    // Warning: This is the sizeof(MESSAGE_HEADER_T). Double check if it is set right!
// without address field size!
#define MESHCOM_STREAM_PAYLOAD_MAX         (MESHCOM_MESH_PACKET_SIZE_MAX - MESHCOM_PRE_MSG_HEADER_SIZE)        // Target Address will be cut out from packet in MeshRadio.
// don't count these 10 Bytes in Packet size calculation!

typedef uint8_t MESHCOM_MSG_ID_T;

#define MESHCOM_PORT_LOCAL              0x10
#define MESHCOM_PORT_UNICAST            0x20
#define MESHCOM_PORT_UNICAST_NOACK      0x21
#define MESHCOM_PORT_BROADCAST          0x40

#define MESHCOM_V1_MSG_ID_COMMAND       0x04
#define MESHCOM_V1_MSG_ID_SETUP         0x08
#define MESHCOM_V1_MSG_ID_STATUS        0x20
#define MESHCOM_V1_MSG_ID_BEACON_BC     0x30
#define MESHCOM_V1_MSG_ID_BOOTLOADER    'B'    // 0x42

#define MESHCOM_MSG_ID_COMMAND           0x10
#define MESHCOM_MSG_ID_SETUP             0x20
#define MESHCOM_MSG_ID_STATUS            0x40
#define MESHCOM_MSG_ID_COMMANDSTREAM     0x80
#define MESHCOM_MSG_ID_DATASTREAM        0xA0
#define MESHCOM_MSG_ID_BC_BEACON         0xB0
#define MESHCOM_MSG_ID_BC_NODEINFO       0xB8
#define MESHCOM_MSG_ID_BC_COMMAND        0xC0
#define MESHCOM_MSG_ID_RESPONSE          0xF0

#define MESHCOM_NODE_TYPE_COORDINATOR    0x00
#define MESHCOM_NODE_TYPE_ROUTER         0x01
#define MESHCOM_NODE_TYPE_END_DEVICE     0x02
#define MESHCOM_NODE_TYPE_UNKNOWN        0xFF

#define MESHCOM_NODE_STATUS_INITING_STATE            0x10
#define MESHCOM_NODE_STATUS_CONFIG_STATE             0x12
#define MESHCOM_NODE_STATUS_STARTING_NETWORK_STATE   0x20
#define MESHCOM_NODE_STATUS_START_NETWORK_FAILED     0x28
#define MESHCOM_NODE_STATUS_IN_NETWORK_STATE         0x30
#define MESHCOM_NODE_STATUS_NETWORK_UPDATE           0x31
#define MESHCOM_NODE_STATUS_LEAVING_NETWORK_STATE    0x40
#define MESHCOM_NODE_STATUS_STOP_STATE               0x50
#define MESHCOM_NODE_STATUS_UNKNOWN                  0xFF

/***************** COMMAND MESSAGE *****************/

#define MESHCOM_CMD_RESET    0x10        // Soft-reset of Mesh Stack
#define MESHCOM_CMD_JOIN     0x20        // Join Network
#define MESHCOM_CMD_LEAVE    0x21        // Leave Network
#define MESHCOM_CMD_ACK      0x80        // ACK of last Command. Last Command returned as Parameter.
#define MESHCOM_CMD_NACK     0x81

typedef struct meshcom_msg_command_s
{
    uint8_t cmdid;
    uint16_t parameter16;
    uint32_t parameter32;
} PACK MESHCOM_MSG_COMMAND_S;

/***************** SETUP MESSAGE *****************/

#define MESHCOM_NWK_KEY_LEN  16

typedef struct
{
    uint8_t nodeType;
    uint8_t channelPage;
    uint32_t channelMask;
    uint64_t uid;
    uint64_t extPanId;
    int8_t txPower;
    uint8_t nwkKey[MESHCOM_NWK_KEY_LEN];
    uint64_t trustCenterAddr;
} PACK MESHCOM_MSG_SETUP_S;

/***************** STATUS MESSAGE *****************/

typedef struct meshcom_msg_status_s
{
    uint8_t nodeStatus; // 1
    uint16_t boardType; // 2
    uint8_t softVersionMajor; // 1
    uint8_t softVersionMinor; // 1
    uint8_t nodeType; // 1
    uint8_t workingChannel; // 1
    uint16_t shortPanId; // 2
    uint64_t extAddr; // 8
    uint16_t shortAddr; // 2
    uint16_t parentShortAddr; // 2
    uint8_t lqi; // 1
    int8_t rssi; // 1
    uint8_t status; // 1
} PACK MESHCOM_MSG_STATUS_S; // Size = 24 Bytes


/***************** BEACON BROADCAST MESSAGE *****************/

typedef struct meshcom_bc_host_s
{
    uint32_t Serial;
    uint8_t State;
    uint16_t FaultBM;
} PACK MESHCOM_BC_HOST_S; // 7 Bytes

typedef struct meshcom_bc_hostinfo_s
{
    uint32_t Serial;
    uint16_t HardwareID;
    uint8_t State;
    uint8_t VersionMajor;
    uint8_t VersionMinor;
} PACK MESHCOM_BC_HOSTINFO_S; // 9 Bytes

typedef struct meshcom_bc_sensor_s
{
    uint16_t gloom; // 2
    int8_t gloom_sensor_status; // 1
    int16_t temperature; // 2
    int8_t temperature_sensor_status; // 1
    uint16_t visibility; // 2
    int8_t visibility_sensor_status; // 1
    uint8_t visibility_cluster_id; // 1
} PACK MESHCOM_BC_SENSOR_S; // 10 Bytes

typedef struct meshcom_bc_gps_s
{
    int32_t latitude;
    int32_t longitude;
} PACK MESHCOM_BC_GPS_S; // 8 Bytes

typedef struct meshcom_bc_opinions_s
{
    uint8_t led_on_off_opinion;
} PACK MESHCOM_BC_OPINIONS_S; // 1 Byte

typedef struct meshcom_bc_status_info_s
{
    unsigned sequence_running :1; // 0 : sequence is sleeping, 1 : sequence is running
    unsigned light_output_reduction :2; // 0 : no reduction / full power, 1 : reduction range 1, 2 : reduction range 2, 3 : not used
    unsigned unused :5; // unused
    uint8_t ledOperatingMode; // Gloom Manager LED Stage
} PACK MESHCOM_BC_STATUS_INFO_S; // 1 Byte

typedef enum MeshComStatus_e
{
    MESHCOM_STATUS_FAILED = -4,
    MESHCOM_STATUS_NOT_AVAILABLE = -3,
    MESHCOM_STATUS_WAIT_FOR_INIT = -2,
    MESHCOM_STATUS_RUNAWAY_VALUE = -1,
    MESHCOM_STATUS_OK = 0,
    MESHCOM_STATUS_UPPER_LIMIT = 1,
    MESHCOM_STATUS_LOWER_LIMIT = 2
} MESHCOM_STATUS_E;

typedef struct
{
    uint16_t boardType;
    uint8_t softVersionMajor;
    uint8_t softVersionMinor;
    uint8_t nodeType;
    uint16_t shortAddr;
    uint16_t parentShortAddr;
    uint8_t lqi;
    int8_t rssi;
    int8_t hostState; //MESHCOM_STATUS_E value
} PACK NODEINFO_T; // 12


typedef struct
{
    MESHCOM_BC_HOST_S host; // 7
    MESHCOM_BC_SENSOR_S sensors; // 10
    MESHCOM_BC_OPINIONS_S opinions; // 1
    MESHCOM_BC_STATUS_INFO_S status_info; // 1
} PACK MESHCOM_MSG_BC_BEACON_S; // 19 total max 44 (max. 53)

typedef struct
{
    NODEINFO_T nodeinfo; // 12
    MESHCOM_BC_HOSTINFO_S hostinfo; // 9
    MESHCOM_BC_GPS_S gps; // 8
} PACK MESHCOM_MSG_BC_NODEINFO_S; // 29 total max 44 (max. 53)

/***************** COMMAND BROADCAST MESSAGE *****************/

#define MESHCOM_BC_COMMAND_BROADCAST_SHUTUP  0x10
#define MESHCOM_BC_COMMAND_LIGHTUP           0x20
#define MESHCOM_BC_COMMAND_SWITCH_ON_OFF     0x30

typedef struct
{
    uint8_t command; // 1
    uint16_t parameter16; // 2
    uint32_t parameter32; // 4
    uint8_t reserved[11]; // 11
    uint32_t serial; // 4    // ToDo: Clean Up on major protocol change
} PACK MESHCOM_MSG_BC_COMMAND_S; // 22 total max 44 (max. 53)

/***************** STREAM MESSAGE *****************/

typedef struct
{
    // hier kommt je nach major-id eines von beiden aus meshcom_cmd.h
    // major-id == MESHCOM_MSG_ID_DATASTREAM    : MESHCOM_DATASTREAM_T payload
    // major-id == MESHCOM_MSG_ID_COMMANDSTREAM : MESHCOM_COMMANDSTREAM_T payload

    uint8_t payload[MESHCOM_STREAM_PAYLOAD_MAX];
} PACK MESHCOM_MSG_STREAM_S;

/***************** RESPONSE MESSAGE *****************/

#define MESHCOM_RESPONSE_MSG_ACK      1
#define MESHCOM_RESPONSE_MSG_NACK     0
#define MESHCOM_RESPONSE_MSG_STATUS   2
#define MESHCOM_RESPONSE_MSG_ABORT    0xff

typedef enum MsgResponseStatusCode_t
{
    MESHCOM_RESPONSE_MSG_STATUS_ERR_ADDR = 0xF0,
    MESHCOM_RESPONSE_MSG_STATUS_ERR_BUSY = 0xFE,
    MESHCOM_RESPONSE_MSG_STATUS_ERR_SIZE = 0xFF,
    MESHCOM_RESPONSE_MSG_STATUS_OK = 1
} MSGRESPONSESTATUSCODE_T;

typedef struct
{
    uint8_t message_id;
    uint8_t response;
    uint8_t code;
} PACK MESHCOM_MSG_RESPONSE_S;

/***************** MESSAGE HEADER *****************/

typedef struct
{
    uint8_t major;
    uint8_t minor;
} PACK MESSAGE_VERSION_T; // 2 Byte

typedef struct
{
    uint8_t major;
    uint8_t minor;
} PACK MESSAGE_ID_T; // 2 Byte

typedef struct
{
    uint64_t extAddr;
    uint16_t shortAddr;
} PACK MESSAGE_ADDRESS_T; // 10 Byte

typedef union
{
    struct
    {
        unsigned char major;
        unsigned char port;
        unsigned char minor;
        unsigned char pad; // 32-bit padding
    } fields;
    unsigned long value;
} PACK UNIFIED_MESSAGE_ID_T;

typedef struct
{
    MESSAGE_VERSION_T version; //  2 Byte protocol version
    uint8_t port; //  1 Byte port number of mesh stack (message_id for protocol v1.x)
    MESSAGE_ID_T id; //  2 Byte value for big switch statement
    uint8_t length; //  1 Byte length of following message
    MESSAGE_ADDRESS_T address; // 10 Byte mesh source or destination address of packet
} PACK MESSAGE_HEADER_T; // 16 Bytes HEADER

typedef struct meshcom_msg_s
{
    MESSAGE_HEADER_T header; // 16 Bytes
    union
    {
        MESHCOM_MSG_COMMAND_S command;
        MESHCOM_MSG_SETUP_S setup;
        MESHCOM_MSG_STATUS_S status;
        MESHCOM_MSG_BC_BEACON_S bc_beacon;
        MESHCOM_MSG_BC_NODEINFO_S bc_nodeinfo;
        MESHCOM_MSG_BC_COMMAND_S bc_command;
        MESHCOM_MSG_STREAM_S stream;
        MESHCOM_MSG_RESPONSE_S response;
    } PACK message;
} PACK MESHCOM_MSG_S;

typedef struct meshcom_frame_header_s
{
    uint8_t Start1;
    uint8_t Start2;
    uint8_t Length_High;
    uint8_t Length_Low;
} PACK MESHCOM_FRAME_HEADER_S;

typedef struct meshcom_frame_footer_s
{
    uint8_t CRC_High;
    uint8_t CRC_Low;
    uint8_t Stop1;
    uint8_t Stop2;
} PACK MESHCOM_FRAME_FOOTER_S;

typedef struct meshcom_packet_s
{
    MESHCOM_FRAME_HEADER_S header;
    MESHCOM_MSG_S payload;
    MESHCOM_FRAME_FOOTER_S footer; // Note: payload has varying size, do not access footer in here!
} PACK MESHCOM_PACKET_S;

#define MESHCOM_MSG_HEADER_SIZE          (sizeof(MESSAGE_HEADER_T))
#define MESHCOM_MSG_COMMAND_SIZE         (sizeof(MESHCOM_MSG_COMMAND_S))
#define MESHCOM_MSG_SETUP_SIZE           (sizeof(MESHCOM_MSG_SETUP_S))
#define MESHCOM_MSG_STATUS_SIZE          (sizeof(MESHCOM_MSG_STATUS_S))
#define MESHCOM_MSG_BC_BEACON_SIZE       (sizeof(MESHCOM_MSG_BC_BEACON_S))
#define MESHCOM_MSG_BC_NODEINFO_SIZE     (sizeof(MESHCOM_MSG_BC_NODEINFO_S))
#define MESHCOM_MSG_BC_COMMAND_SIZE      (sizeof(MESHCOM_MSG_BC_COMMAND_S))
#define MESHCOM_MSG_STREAM_SIZE          (MESHCOM_STREAM_PAYLOAD_MAX)
#define MESHCOM_MSG_RESPONSE_SIZE        (sizeof(MESHCOM_MSG_RESPONSE_S))
#define MESHCOM_MESSAGESIZE_MAX          (sizeof(MESHCOM_MSG_S))

extern MESHCOM_MSG_STATUS_S _meshcom_node_status;



#endif
