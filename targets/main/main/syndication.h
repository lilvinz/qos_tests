#ifndef SYNDICATION_H
#define SYNDICATION_H

#include "network.h"
#include "sensor.h"
#include "gloom_manager.h"

#define SYNDICATION_QUEUE_SIZE          10
#define SYNDICATION_QUEUE_TIMEOUT_PUT   100  // do not block longer than 100ms
#define SYNDICATION_QUEUE_TIMEOUT_GET   100  // do not block longer than 100ms

#define SDC_INTEGRATION_VALUE_TIMEOUT   180 //integrated value timeout [s]
#define SDC_SETTING_NODE_TIMEOUT        (_settings_list_ram[SETTINGS_MESH_NODE_TIMEOUT])  // time to live for client in list in [s]. client will be removed from known clients list.
#define SDC_CLIENT_VALUE_TIMEOUT        (SDC_SETTING_NODE_TIMEOUT - (SDC_SETTING_NODE_TIMEOUT / 10))
#define SDC_MAX_NO_OF_CLIENTS           400  // todo set to final value //maximum no of clients handled by syndicate.
#define SDC_MAX_NUM_VISIBILITY_DEV      8   //maximum no of visibility sensors
#define SDC_CLIENT_VISIBILITY_TRUST_AGE_MAX 180 //max age for value [s]

#define SDC_GET_ASSIMILATED_DATA_MUTEX_TIMEOUT_MS 100
#define SDC_GET_SYNDICATE_DATA_MUTEX_TIMEOUT_MS 100

#define DEBUG_SYNDICATION   0

#if (DEBUG_SYNDICATION == 1)

//#include "debug.h"

#define SYNDICATION_TPIN_ON(x)              TPIN_ON(x)
#define SYNDICATION_TPIN_OFF(x)             TPIN_OFF(x)
#define SYNDICATION_TPIN_TOGGLE(x)          TPIN_TOGGLE(x)

#define SYNDICATION_DEBUG(x)                DEBUG(x)
#define SYNDICATION_DEBUG1(x,a)             DEBUG1(x,a)
#define SYNDICATION_DEBUG2(x,a,b)           DEBUG2(x,a,b)
#define SYNDICATION_DEBUG3(x,a,b,c)         DEBUG3(x,a,b,c)
#define SYNDICATION_DEBUG4(x,a,b,c,d)       DEBUG4(x,a,b,c,d)
#define SYNDICATION_DEBUG5(x,a,b,c,d,e)     DEBUG5(x,a,b,c,d,e)
#define SYNDICATION_DEBUG6(x,a,b,c,d,e,f)   DEBUG6(x,a,b,c,d,e,f)
#define SYNDICATION_DEBUG7(x,a,b,c,d,e,f,g) DEBUG7(x,a,b,c,d,e,f,g)

#else

#define SYNDICATION_TPIN_ON(x)
#define SYNDICATION_TPIN_OFF(x)
#define SYNDICATION_TPIN_TOGGLE(x)

#define SYNDICATION_DEBUG(x)
#define SYNDICATION_DEBUG1(x,a)
#define SYNDICATION_DEBUG2(x,a,b)
#define SYNDICATION_DEBUG3(x,a,b,c)
#define SYNDICATION_DEBUG4(x,a,b,c,d)
#define SYNDICATION_DEBUG5(x,a,b,c,d,e)
#define SYNDICATION_DEBUG6(x,a,b,c,d,e,f)
#define SYNDICATION_DEBUG7(x,a,b,c,d,e,f,g)

#endif

typedef enum
{
    SDC_CVID_SERIAL,
    SDC_CVID_NETWORKID,
} SDC_CVID_T;

////
//syndication
typedef struct
{
    int16_t         value;         // 2 Bytes
    SENSOR_STATUS_T  sensor_state;  // 1 Byte
} SDC_TEMPERATURE_T;  // 3 Bytes

typedef struct
{
    float           value;        // 4 Bytes
    SENSOR_STATUS_T  sensor_state; // 1 Byte
} SDC_GLOOM_T;  // 5 Bytes

typedef struct
{
    uint16_t value;  // 2 Bytes
    SENSOR_STATUS_T sensor_status;  // 1 Byte
    uint8_t cluster_id;  // id of visibility sensor, 1 Byte
} SDC_VISIBILITY_T;  // 4 Byte

typedef struct
{
    SDC_GLOOM_T gloom;  // 5 Bytes
    SDC_TEMPERATURE_T temperature;  // 3 Bytes
    SDC_VISIBILITY_T visibility;
} SDC_OPINIONS_T;  // 8 Bytes

typedef struct
{
    uint32_t        serial_number;      // 4 Bytes
    uint16_t        TimeToLive;         // 2 Bytes
    SDC_OPINIONS_T  opinion;            // 8 Bytes
} SDC_CLIENT_S; // size = 14 Bytes

typedef struct
{
    SDC_CLIENT_S        client[SDC_MAX_NO_OF_CLIENTS];
    uint16_t            client_count;
} SDC_SYNDICATE_T;

typedef struct
{
    uint32_t        serial_number;
    GLOOM_MANAGER_GLOBAL_LED_ON_OFF_STATE_T led_on_off_opinion;

    SDC_GLOOM_T         gloom;        // 5 Bytes
    SDC_TEMPERATURE_T   temperature;  // 3 Bytes
    SDC_VISIBILITY_T visibility;  // 4 Byte
} SDC_CLIENT_DATA_T;

/////
//assimilation

typedef struct
{
    float       average;
    float       minimum;
    float       maximum;
    uint32_t    timestamp;
    uint16_t    count;
} SDC_A_GLOOM_T;

typedef struct
{
    int16_t     average;
    int16_t     minimum;
    int16_t     maximum;
    uint32_t    timestamp;
    uint16_t    count;
} SDC_A_TEMPERATURE_T;

typedef struct
{
    uint16_t    average;
    uint16_t    minimum;
    uint16_t    maximum;
    uint32_t    timestamp;
    uint16_t    count;
} SDC_A_VISIBILITY_T;

typedef struct
{
    SDC_A_GLOOM_T       gloom;
    SDC_A_TEMPERATURE_T temperature;
    SDC_A_VISIBILITY_T  visibility;
    uint32_t            time;
} SDC_ASSIMILATE_T;

////////////////////////
// umstrukturieren. temerature.min, .max, .avrg, .timestamp, .count

bool SYNDICATION_GetAssimilatedGloom(SDC_A_GLOOM_T *pdata);
bool SYNDICATION_GetAssimilatedTemperature(SDC_A_TEMPERATURE_T *pdata);
bool SYNDICATION_GetAssimilatedVisibility(SDC_A_VISIBILITY_T *pdata);
bool SYNDICATION_GetNextClientGloomInfo(uint16_t *client, SDC_GLOOM_T *p_data);
bool SYNDICATION_GetSyndicateClientData(uint16_t client, SDC_CLIENT_S *pdata);
bool SYNDICATION_GetSyndicateClientCount(uint16_t *pdata);

void SYNDICATION_Submit(SDC_CLIENT_DATA_T *client_data);
void SYNDICATION_Create(void);
void SYNDICATION_Task(void *pvParameters);

#endif
