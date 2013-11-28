#ifndef MESHCOM_CMD_H
#define MESHCOM_CMD_H

#include "log.h"
#include "dim.h"
#include "firmware_types.h"
#include "meshcom_conn.h"
#include "gloom_manager.h"

#include "meshcom.h"

#define MESHCOM_CMD_GET_PSU_DATA_TIMEOUT_MS 10  // 10 ms

// ***************************************
// ******* COMMAND CONFIGURATIONS ********

#define COMMAND_LED_LIGHTUP_DURATION_MAX    600 // max duration for light up in seconds

#define MESHCOM_COMMAND_LED_LIGHTUP_INTERVAL_MAX   4000  // 4000 s

// ***************************************
// ******* PROTOCOL & COMMAND DEFS********

#define MESHCOM_COMMANDSTREAM_ID_TEST1                   0x01

#define MESHCOM_COMMANDSTREAM_ID_GET_SETTING             0x11
#define MESHCOM_COMMANDSTREAM_ID_SET_SETTING             0x12
#define MESHCOM_COMMANDSTREAM_ID_GET_SETTING_GROUP       0x13
#define MESHCOM_COMMANDSTREAM_ID_SET_SETTING_GROUP       0x14
#define MESHCOM_COMMANDSTREAM_ID_SAVE_SETTINGS_AND_REBOOT 0x1D
#define MESHCOM_COMMANDSTREAM_ID_RESET_SETTINGS          0x1E
#define MESHCOM_COMMANDSTREAM_ID_SAVE_SETTINGS           0x1F

#define MESHCOM_COMMANDSTREAM_ID_GET_LOG_INFO            0x20
#define MESHCOM_COMMANDSTREAM_ID_GET_LOG_ENTRIES         0x21

#define MESHCOM_COMMANDSTREAM_ID_GET_NODEINFO            0x30
#define MESHCOM_COMMANDSTREAM_ID_GET_STATUS              0x31
#define MESHCOM_COMMANDSTREAM_ID_GET_POSITION            0x32

#define MESHCOM_COMMANDSTREAM_ID_FIRMWARE_GET            0x91
#define MESHCOM_COMMANDSTREAM_ID_FIRMWARE_TX_INIT        0x9A

#define MESHCOM_COMMANDSTREAM_LIGHTUP                    0xA1
#define MESHCOM_COMMANDSTREAM_ID_TEST_INDICATION         0xA2

#define MESHCOM_COMMANDSTREAM_ID_REBOOT                  0xE0

#define MESHCOM_COMMANDSTREAM_ID_ACK                     0xF0

#define MESHCOM_DATASTREAM_ID_PING                       0x01
#define MESHCOM_DATASTREAM_ID_PING_RESPONSE              0x02
#define MESHCOM_DATASTREAM_ID_FIRMWARE                   0x91

#define DATASTREAM_PAYLOAD_MAX                          (MESHCOM_STREAM_PAYLOAD_MAX - 4)                 // subtract datastream header size
#define DATASTREAM_SETTING_GROUP_ENTRY_COUNT_MAX        ((DATASTREAM_PAYLOAD_MAX - 1) / 4)
#define COMMANDSTREAM_SETTING_GROUP_ENTRY_COUNT_MAX     DATASTREAM_SETTING_GROUP_ENTRY_COUNT_MAX
#define DATASTREAM_LOG_ENTRY_COUNT                      ((DATASTREAM_PAYLOAD_MAX - 4) / LOG_ENTRY_SIZE)  // subtract index (uint32)
#define DATASTREAM_FIRMWARE_DATA_LEN_MAX                (DATASTREAM_PAYLOAD_MAX - 5)                    // subtract packet address and length

enum status_page_id_e
{
    STATUS_PAGE_GENERAL,
    STATUS_PAGE_LEDSTAGE_1,
    STATUS_PAGE_LEDSTAGE_2,
    STATUS_PAGE_LEDSTAGE_3,
    STATUS_PAGE_GPS,
    STATUS_PAGE_NETWORK,
    STATUS_PAGE_SENSORS,
    STATUS_PAGE_FIRMWARE,
    STATUS_PAGE_PSU_1,
    STATUS_PAGE_PSU_2,
    STATUS_PAGE_COUNT
};

// *************************************
// ******* COMMANDSTREAM STRUCT ********

typedef struct
{
    uint32_t id;
    uint32_t value;
} PACK COMMANDSTREAM_SETTING_T;

typedef struct
{
    uint8_t group_id;
    uint32_t entry[COMMANDSTREAM_SETTING_GROUP_ENTRY_COUNT_MAX];
} PACK COMMANDSTREAM_SETTING_GROUP_T;

typedef struct
{
    uint32_t begin;
} PACK COMMANDSTREAM_LOG_T;

typedef struct
{
    uint8_t id; // status_page_id_e
} PACK COMMANDSTREAM_STATUS_T;

typedef struct
{
    uint32_t id;
} PACK COMMANDSTREAM_POSITION_T;

typedef struct
{
    uint8_t     fw_type;
    uint16_t    hw_id;
    uint16_t    hw_revision;
    uint8_t     sw_version_major;
    uint8_t     sw_version_minor;
    uint32_t    size;
    uint32_t    checksum;
    uint8_t     setup_bitmask;
} PACK COMMANDSTREAM_FIRMWARE_HEADER_T;

typedef struct
{
    uint32_t param1;
    uint32_t param2;
    uint32_t param3;
    uint32_t param4;
} PACK COMMANDSTREAM_COMMAND_T;

typedef struct
{
    uint8_t major;
    uint8_t minor;
    uint8_t ack;
} PACK COMMANDSTREAM_RESPONSE_T;

typedef struct
{
    uint32_t sequence_id;
} PACK COMMANDSTREAM_HEADER_T;

typedef struct
{
    COMMANDSTREAM_HEADER_T header;

    union
    {
        COMMANDSTREAM_SETTING_T         setting;
        COMMANDSTREAM_SETTING_GROUP_T   setting_group;
        COMMANDSTREAM_LOG_T             log;
        COMMANDSTREAM_STATUS_T          status;
        COMMANDSTREAM_POSITION_T        position;
        COMMANDSTREAM_FIRMWARE_HEADER_T firmware_header;
        COMMANDSTREAM_COMMAND_T         command;
        COMMANDSTREAM_RESPONSE_T        response;
    } data;
} PACK MESHCOM_COMMANDSTREAM_T;

// *************************************
// ******** DATASTREAM STRUCT **********

// Note: see system/local_texts.txt for descriptions of these structs!

typedef struct
{
    uint8_t id;
    union
    {
        struct
        {
            uint32_t time;
            uint32_t uptime;
            uint32_t logentries;
            uint8_t systemtype;
        } PACK general;
        struct
        {
            uint16_t led_fail_bm;
            GLOOM_MANAGER_LED_STATE_T led_state;
            uint16_t led_power_rel[DIM_LED_COUNT];
            uint16_t led_power_abs[DIM_LED_COUNT];
        } PACK ledstage_1;
        struct
        {
            uint32_t timemeter[DIM_LED_COUNT];
        } PACK ledstage_2;
        struct
        {
            uint32_t timemeter_comp[DIM_LED_COUNT];
        } PACK ledstage_3;
        struct
        {
            int16_t temperature[TEMPERATURE_COUNT]; // 2
            int8_t temperature_sensor_status[TEMPERATURE_COUNT]; // 1
            uint16_t gloom; // 2
            int8_t gloom_sensor_status; // 1
            uint16_t visibility; // 2
            int8_t visibility_sensor_status; // 1
            uint8_t visibility_cluster_id; // 1
        } sensors;
        struct
        {
            uint8_t status;
            uint8_t num_sat_fix;
            uint32_t sat_id_list;
            uint8_t fixtype;
            int32_t latitude;
            int32_t longitude;
            int32_t altitude;
        } PACK gps;
        struct
        {
            struct
            {
                uint16_t boardType;
                uint8_t softVersionMajor;
                uint8_t softVersionMinor;
                uint8_t nodeType;
                uint16_t shortAddr;
                uint16_t parentShortAddr;
                uint8_t lqi;
                int8_t rssi;
            } PACK node;
            uint8_t client_count;  // todo[HIGH] increase to 16 bit because we are currently supporting up to 400 clients!
            uint32_t bc_msgs_received;
            uint16_t gloom_network_avg;
        } PACK network;
        struct
        {
            uint16_t hw_id;
            uint16_t hw_revision;
            uint8_t sw_version_major;
            uint8_t sw_version_minor;
            uint32_t size;
            uint32_t checksum;
            uint8_t retry_bits;
        } PACK firmware;
        struct
        {
            uint32_t serial;
            uint16_t hw_id;
            uint16_t hw_revision;
            uint16_t sw_version_major;
            uint16_t sw_version_minor;
            int32_t sys_voltage;
            int32_t sys_temperature_value;
            uint8_t sys_temperature_status;
            uint8_t ups_status;
            uint8_t ups_raw_status;
            uint8_t ups_battery_status;
            uint8_t rating_temperature;
            uint8_t rating_thermal_fuse;
            uint8_t rating_voltage;
            uint32_t fan_time_meter;
            uint8_t fan_service_status;
            uint32_t fan_filter_time_meter;
            uint8_t fan_filter_service_status;
        } PACK psu_1;
        struct
        {
            int32_t battery_1_temperature_value;
            uint8_t battery_1_temperature_status;
            int32_t battery_1_time_meter_total;
            uint8_t battery_1_time_meter_total_status;
            int32_t battery_1_time_meter_hot;
            uint8_t battery_1_time_meter_hot_status;
            uint8_t battery_1_service_status;
            int32_t battery_2_temperature_value;
            uint8_t battery_2_temperature_status;
            int32_t battery_2_time_meter_total;
            uint8_t battery_2_time_meter_total_status;
            int32_t battery_2_time_meter_hot;
            uint8_t battery_2_time_meter_hot_status;
            uint8_t battery_2_service_status;
        } PACK psu_2;
    } data;

} PACK DATASTREAM_STATUS_T;

typedef struct
{
    uint32_t id;
    uint32_t value;
} PACK DATASTREAM_SETTING_T;

typedef struct
{
    uint8_t group_id;
    uint32_t entry[DATASTREAM_SETTING_GROUP_ENTRY_COUNT_MAX];
} PACK DATASTREAM_SETTING_GROUP_T;

typedef struct
{
    uint32_t first_index;
    LOG_ENTRY_T entry[DATASTREAM_LOG_ENTRY_COUNT];
} PACK DATASTREAM_LOG_ENTRIES_T;

typedef struct
{
    uint32_t first; // first entry available in log memory
    uint32_t last; // last entry
    uint8_t pagesize; // num of log entries per datastream packet
    uint16_t overflows; // amount of log-entry memory overflows
    uint16_t failures; // amount of defektive eeprom blocks (0 = everything OK)
} PACK DATASTREAM_LOG_INFO_T;

typedef struct
{
    uint32_t uAddress;
    uint8_t uLength;
    uint8_t aData[DATASTREAM_FIRMWARE_DATA_LEN_MAX];
} PACK DATASTREAM_FIRMWARE_T;

typedef enum
{
    MESHCOM_CMD_RESPONSE_ACK,
    MESHCOM_CMD_RESPONSE_NACK,
    MESHCOM_CMD_RESPONSE_ABORT,
    MESHCOM_CMD_RESPONSE_ERR_SIZE,
} DATASTREAM_RESPONSE_ENUM_T;

typedef struct
{
    uint8_t     eResponse;
} PACK DATASTREAM_RESPONSE_T;

typedef struct
{
    uint32_t sequence_id;
} PACK DATASTREAM_HEADER_T;

typedef struct
{
    DATASTREAM_HEADER_T header;

    union
    {
        DATASTREAM_SETTING_T setting;
        DATASTREAM_SETTING_GROUP_T setting_group;
        DATASTREAM_LOG_INFO_T log_info;
        DATASTREAM_LOG_ENTRIES_T log_entries;
        DATASTREAM_FIRMWARE_T firmware;
        DATASTREAM_STATUS_T status;
        DATASTREAM_RESPONSE_T response;
        uint8_t rawdata[DATASTREAM_PAYLOAD_MAX];
    } data;
} MESHCOM_DATASTREAM_T;

void MESHCOM_CMD_FirmwareRxTxEnable(bool b_enable);  // todo[medium] note for cba: please check
bool MESHCOM_CMD_CommandstreamReceive(MESHCOM_MSG_S *pMessage);
bool MESHCOM_CMD_DatastreamReceive(MESHCOM_MSG_S *pMessage);
void MESHCOM_CMD_RequestFirmware(uint8_t uShortAddr, FWT_SELECTED_FW_T FwType);
void MESHCOM_CMD_Tick(void);
void MESHCOM_CMD_Init(void);

#endif
