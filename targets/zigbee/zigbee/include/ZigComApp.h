#ifndef _ZIGCOMAPP_H
#define _ZIGCOMAPP_H

#include <macAddr.h>
#include <appFramework.h>
#include <configServer.h>
#include <appTimer.h>
#include <aps.h>
#include <uid.h>
#include <appTimer.h>
#include <zdo.h>
#include <dbg.h>

#include "ZigComConnector.h"
#include "version.h"

#define CONFIG_STATUS_MESSAGE_INTERVAL			   5000U    // interval of status message send to host
#define CONFIG_OUT_OF_NETWORK_FATAL_TIMEOUT		600000UL    // timeout of out of network state. zigbit will be restarted
#define CONFIG_WATCHDOG_TIMER_INTERVAL              500U    // watchdog reset timer. BitClouds WDT interval is 1s (see halInit.c)
#define CONFIG_ALIVE_COUNTER_RESET_VALUE           3600U    // Alive counter lasts for x seconds.
#define APP_LED_TIMER_STARTING_NETWORK_INTERVAL     100U    // led toggle interval for starting network state
#define APP_LED_TIMER_WAITING_FOR_CONFIG_INTERVAL   500U    // led toggle interval for config state

#define APP_STATUS_LED_ON()         BSP_OffLed(LED_FIRST)
#define APP_STATUS_LED_OFF(id)      BSP_OnLed(LED_FIRST)
#define APP_STATUS_LED_TOGGLE(id)   BSP_ToggleLed(LED_FIRST)

enum
{
    BOARD_TYPE_UNKNOWN,
    BOARD_TYPE_ZIGBEE_900,
    BOARD_TYPE_ZIGBEE_2400,
};

#define SYSTEM_BOARD_TYPE		BOARD_TYPE_ZIGBEE_2400

#define SYSTEM_VERSION_MAJOR	VERSION_MAJOR
#define SYSTEM_VERSION_MINOR	VERSION_MINOR

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

typedef enum
{
    APP_PROCESS,
    APP_CONFIG_UPDATED,
    APP_CONFIG_TIMEOUT,
    APP_NETWORK_STARTING_DONE,
    APP_NETWORK_STATE_UPDATED,
    APP_LEAVE_DONE,
    APP_SENDING_DONE,
    APP_SENDING_ALIVE_DONE,
    APP_SENDING_BROADCAST_DONE,
    APP_SENDING_UNICAST_DONE,
    APP_SLEEP_DONE,
    APP_WOKEUP,
    APP_SEND_ALIVE_NOW,
    APP_SEND_STATUS,
} AppEvent_t;

typedef enum
{
    APP_INITING_STATE,
    APP_WAIT_FOR_INITIAL_CONFIG,
    APP_STARTING_NETWORK_STATE,
    APP_IN_NETWORK_STATE,
    APP_LEAVING_NETWORK_STATE,
    APP_STOP_STATE
} AppState_t;

typedef struct
{
    uint8_t			State;
    DeviceType_t 	DeviceType;
    uint16_t 		PANId;
    uint64_t 		Uid;
    ShortAddr_t 	ShortAddr;
    ExtAddr_t		ExtAddr;
    uint8_t 		Channel;
    ShortAddr_t 	ParentAddr;
    uint8_t			ParentLQI;
    int8_t			ParentRSSI;
    uint8_t         Status;
} appNodeInfo_t;


extern SimpleDescriptor_t simpleDescriptorForBroadcast;
extern SimpleDescriptor_t simpleDescriptorForUnicast;


extern appNodeInfo_t appNodeInfo;

extern void appNodeTaskHandler(AppEvent_t event, void *param);

extern void appStopRouter(void);
extern void appPostSubTaskTask(void);
extern void SYS_InitLog(void);
extern void appReadLqiRssi(void);
extern void SYS_StartLog(void);
extern void SYS_StopLog(void);


void appLeaveNetwork(void);
void appConfigReceived(void);
void appHelloAlive(void);

//dummy func to suppress linker error. see source
void BSP_TaskHandler(void);

#endif /*_ZIGCOMAPP_H*/
