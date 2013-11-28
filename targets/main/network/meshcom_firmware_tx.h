#ifndef MESHCOM_FIRMWARE_TX_H
#define MESHCOM_FIRMWARE_TX_H

#include "firmware_types.h"
#include "meshcom_conn.h"


#define FWTX_SETTING_TX_MODE   _settings_list_ram[SETTINGS_ADVANCED_MESH_FIRMWARE_TX]

#define FWTX_STOP_BEACON_TIMEOUT        20000
#define FWTX_STOP_BEACON_RESEND_COUNT   3
#define FWTX_BEACON_BC_SHUTUP_TIME      180

#define FWTX_START_TIMEOUT  15000
#define FWTX_RESEND_TIMEOUT 20000
#define FWTX_WAIT_TIMEOUT   15000

#define FWTX_RETRY_COUNTER_MAX  30



enum
{
    FWTX_IDLE,          // 0
    FWTX_STOP_BEACONS,  // 1
    FWTX_START,         // 2
    FWTX_INIT_TX,       // 3
    FWTX_START_TX,      // 4
    FWTX_NEXT,          // 5
    FWTX_RESEND,        // 6
    FWTX_SEND,          // 7
    FWTX_WAIT,          // 8
    FWTX_FAILED         // 9
};

enum
{
    FWTX_PACKET_NO_RESPONSE,
    FWTX_PACKET_RESPONSE_AWAITING,
    FWTX_PACKET_ACK,
    FWTX_PACKET_NACK,
};


bool MESHCOM_FIRMWARE_TX_Request(uint16_t sAddr, FWT_SELECTED_FW_T FwType, uint32_t uSerial);
void MESHCOM_FIRMWARE_TX_Abort(void);  // todo [medium] use or remove
void MESHCOM_FIRMWARE_TX_Confirm(MESHCOM_RESULT_T result, uint8_t sequence_id);
void MESHCOM_FIRMWARE_TX_Tick(void);
void MESHCOM_FIRMWARE_TX_Init(void);



#endif
