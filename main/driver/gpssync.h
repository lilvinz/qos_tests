#ifndef GPSSYNC_H
#define GPSSYNC_H


#define GPSSYNC_UART_CHANNEL            UART_CHANNEL_1
#define GPSSYNC_UART_SEND(c)            UART1_PutChar(c)

#define GPSSYNC_ERROR_FLAG_1PPS_MESSAGE 0x01
#define GPSSYNC_ERROR_FLAG_1PPS_SIGNAL  0x02
#define GPSSYNC_ERROR_FLAG_SUBMIT       0x04

#define GPS_STATE_INIT_TIMEOUT_SEC      (5)
#define GPS_STATE_NMEA_TIMEOUT_SEC      (3)
#define GPS_STATE_FAILED_TIMEOUT_SEC    (10)

#define GPS_NMEA_PARSE_TIMEOUT_SEC          (300)  // set timeout to 5 minutes to avoid resetting gps to early
#define GPS_NMEA_PARSE_TIMEOUT_SHORT_SEC    (6)

#define GPS_1PPS_TIME_SUBMIT_TIMEOUT_S      (300)  // set timeout to 5 minutes to avoid resetting gps to early

// minimum delay time between 1PPS time message receive timestamp and corresponding 1PPS
#define GPS_1PPS_MESSAGE_RECV_DELAY_MIN     (100)
// safety margin between time message and next 1PPS pulse which is used to submit time to rtc
#define GPS_1PPS_MESSAGE_SUBMIT_SAFETY_MARGIN   (200)
// timeout to detect missing 1PPS pulse -> trigger rtc by crystal if 1PPS fails.
#define GPS_1PPS_INTERVAL_TIMEOUT_MS    (300000)  // set timeout to 5 minutes to avoid resetting gps to early
#define GPS_1PPS_INTERVAL_TIMEOUT_SHORT_MS   (1500)
// maximum time to first GPS time lock after power up. If timeout reached reset gps
#define GPS_COLD_START_LOCK_TIMEOUT_SEC  (1200)

// **** GPS MODULE SPECS ****
#define GPSSYNC_GPS_MAX_NO_OF_CH        12

#define GPS_DEGREE_FACTOR   1000000L

enum gpssync_sequence_e
{
    GPS_STATE_INIT,
    GPS_STATE_IDLE,
    GPS_STATE_FAILED
};

typedef enum _gpssync_status_e
{
    INIT,
    WAITING_FOR_INITIAL_SYNC,
    SYNCED,
    FAILED
} GPSSYNC_STATUS_T;

typedef struct
{
    bool b_error;
    uint16_t error_ctr;
} GPSSYNC_ERROR_STRUCT_T;

typedef struct
{
    uint8_t error_bm;
    uint32_t gps_power_up_timestamp;
    uint32_t last_submitted_time;
    uint64_t gps_1pps_timestamp;
    GPSSYNC_ERROR_STRUCT_T error_1pps_signal;
    GPSSYNC_ERROR_STRUCT_T error_1pps_message;
} GPSSYNC_DATA_STRUCT_T;

#define GEONAVDATA_NAV_TYPE_FIXTYPE_NONAVFIX    1   //no navigation fix
#define GEONAVDATA_NAV_TYPE_FIXTYPE_2D          2   //2D
#define GEONAVDATA_NAV_TYPE_FIXTYPE_3D          3   //3D

enum
{
    GPSSYNC_DATETIME_INVALID,
    GPSSYNC_DATETIME_VALID,
    GPSSYNC_DATETIME_CONFIRMED,
};

enum
{
    GPSSYNC_MEASUREMENTS_STATE_LOST = 0,
    GPSSYNC_MEASUREMENTS_STATE_TRACKED,
    GPSSYNC_MEASUREMENTS_STATE_LOCKED,
};

typedef struct gpssync_datetime_s
{
    uint8_t     hour;
    uint8_t     minute;
    uint8_t     second;
    uint8_t     day;
    uint8_t     month;
    uint16_t    year;
    uint8_t     status;
    bool        time_is_valid;
} GPSSYNC_DATETIME_T;

typedef struct gpssync_position_s
{
    uint8_t     nav_type;
    uint8_t     num_of_sv_in_fix;
    uint32_t    sat_id_list;
    int32_t     latitude;
    int32_t     longitude;
    int32_t     altitude;
    bool        valid;
} GPSSYNC_POSITION_T;

typedef struct GPSSYNC_MEASUREMENTS_S
{
    uint8_t     svid;
    uint8_t     state;
    uint8_t     cno;
} GPSSYNC_MEASUREMENTS_T;

typedef struct
{
    GPSSYNC_POSITION_T      position;
    GPSSYNC_DATETIME_T      datetime;
    GPSSYNC_MEASUREMENTS_T  svmeasurements[GPSSYNC_GPS_MAX_NO_OF_CH];
    GPSSYNC_STATUS_T        status;
} GPSSYNC_T;

extern GPSSYNC_T _gpssync;

void GPSSYNC_GPS_IRQHandler(void);
void GPSSYNC_Task(void *pvParameters);

#endif
