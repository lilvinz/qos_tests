#ifndef MESHCOM_CONN_H
#define MESHCOM_CONN_H


#include "settings.h"
#include "rtc.h"
#include "uart.h"
#include "meshcom.h"




#define MESHCOM_SETTING_TYPE                    _settings_list_ram[SETTINGS_MESH_TYPE]
#define MESHCOM_CHANNEL_PAGE                 0  // channel page is fixed
#define MESHCOM_SETTING_CHANNEL_MASK            _settings_list_ram[SETTINGS_MESH_CHANNEL_MASK]
#define MESHCOM_SETTING_PAN_ID              _settings_list_ram[SETTINGS_MESH_PANID]
#define MESHCOM_SETTING_BROADCAST_INTERVAL  _settings_list_ram[SETTINGS_MESH_BROADCAST_INTERVAL]
#define MESHCOM_SETTING_TRANSMITTER_POWER   _settings_list_ram[SETTINGS_MESH_TRANSMITTER_POWER]

#define MESHCOM_SETTING_NWK_KEY_1            _settings_list_ram[SETTINGS_SYS_MESH_NWK_KEY_1]
#define MESHCOM_SETTING_NWK_KEY_2            _settings_list_ram[SETTINGS_SYS_MESH_NWK_KEY_2]
#define MESHCOM_SETTING_NWK_KEY_3            _settings_list_ram[SETTINGS_SYS_MESH_NWK_KEY_3]
#define MESHCOM_SETTING_NWK_KEY_4            _settings_list_ram[SETTINGS_SYS_MESH_NWK_KEY_4]

#define MESHCOM_CALLBACK_TIMEOUT_SEC                    6
#define MESHCOM_MESSAGEQUEUE_COUNT_MAX               5
#define MESHCOM_MESSAGE_QUEUE_CONFIRM_TIMEOUT_SEC    30

#define MESHCOM_NETWORK_TIMEOUT_S            180
#define MESHCOM_NETWORK_FAIL_COUNT_MAX       20
#define MESHCOM_NETWORK_NACK_COUNT_MAX       30
#define MESHCOM_FW_UPGRADE_RETRY_COUNT_MAX   10


typedef enum
{
    MESHCOM_TIMEOUT = -3,
    MESHCOM_ERROR_PARAM = -2,
    MESHCOM_ERROR = -1,
    MESHCOM_BUSY = 0,
    MESHCOM_OK = 1,
    MESHCOM_PENDING = 2
} MESHCOM_RESULT_T;

typedef void (MESHCOM_CB_HANDLER)(MESHCOM_RESULT_T result);
typedef void (MESHCOM_CB_HANDLER_SEQ)(MESHCOM_RESULT_T result, uint8_t sequence_id);
typedef void (MESHCOM_MESSAGE_PARSER)(MESHCOM_MSG_S *pMessage);

typedef struct
{
    MESHCOM_CB_HANDLER *pCallBackFunc;
} MESHCOM_MESSAGE_PARAMS_T;

typedef struct meshcom_status_s
{
    bool bProtocolVersionMismatch;
    bool bNetworkFailed;
    uint8_t uMessageNackCount;
    bool bBroadcastReceived;
    uint32_t uBroadcastMessagesReceivedCount;
    uint32_t uBroadcastMessageReceiveTimestamp;
    uint32_t uBroadcastMessagesSendCount;
} PACK MESHCOM_STATUS_S;

extern MESHCOM_STATUS_S _meshcom_status;


MESHCOM_RESULT_T MESHCOM_SendLocalCommand(uint8_t command, uint16_t parameter16, uint32_t parameter32);
MESHCOM_RESULT_T MESHCOM_SendDatastream(MESHCOM_MSG_S *meshcom_send_message, uint8_t port, MESHCOM_CB_HANDLER_SEQ callback, uint8_t seq_id);
MESHCOM_RESULT_T MESHCOM_SendCommandstream(MESHCOM_MSG_S *meshcom_send_message, MESHCOM_CB_HANDLER_SEQ callback, uint8_t seq_id);
MESHCOM_RESULT_T MESHCOM_SendBroadcast(MESHCOM_MSG_S *meshcom_send_message, MESHCOM_CB_HANDLER_SEQ *callback, uint8_t seq_id);
MESHCOM_RESULT_T MESHCOM_SendBroadcastCommand(MESHCOM_MSG_S *meshcom_send_message, MESHCOM_CB_HANDLER_SEQ *callback, uint8_t seq_id);
MESHCOM_RESULT_T MESHCOM_SendBroadcastShutup(uint16_t timeout, MESHCOM_CB_HANDLER_SEQ *callback);
MESHCOM_RESULT_T MESHCOM_SendSetup(void);
void MESHCOM_RegisterRxMessageParser(MESHCOM_MESSAGE_PARSER *parser);
void MESHCOM_RegisterTxMessageParser(MESHCOM_MESSAGE_PARSER *parser);
void MESHCOM_UnregisterRxMessageParser(MESHCOM_MESSAGE_PARSER *parser);
void MESHCOM_UnregisterTxMessageParser(MESHCOM_MESSAGE_PARSER *parser);

void MESHCOM_Task(void *pvParameters);
void MESHCOM_Init(void);
void MESHCOM_UnInit(void);


#endif

