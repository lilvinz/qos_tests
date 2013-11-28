/*
 * ZigComNode.h
 *
 *  Created on: 30.12.2008
 *      Author: Carsten Bartels
 */

#ifndef ZIGCOMNODE_H
#define ZIGCOMNODE_H

#include <macAddr.h>
#include <aps.h>

#ifndef PACK
#define PACK __attribute__((__packed__))
#endif

#define END_POINT_ALIVE			 5
#define END_POINT_BEACON_BC		10
#define END_POINT_UNICAST		20
#define END_POINT_NETSTAT		40

// Maximum ASDU payload length is 53 Bytes for secure and 84 Bytes for non-secure transmissions
#define MESH_MAX_ADSU_LENGTH		51

#define CONFIG_BROADCAST_INTERVAL_MINIMUM	3000	// minimum time between broadcasts.
#define CONFIG_INTERVAL_HOSTLOST			60000	// check intervall for host status.
#define CONFIG_HOSTLOST_COUNT_MAX			10		// max no of check intervalls with no news from host.
#define CONFIG_ALIVE_INTERVAL				60000	// minimum time between alive messages to parent.
#define CONFIG_ALIVE_RETRY_MAX              10

typedef enum
{
    INITIAL_DEVICE_STATE,
    SENDING_DEVICE_STATE,
    WAITING_DEVICE_STATE,
    REINITIAL_DEVICE_STATE
} DeviceState_t;

typedef enum
{
    MSG_ERR_ADDR	= 0xF0,
    MSG_ERR_BUSY	= 0xFE,
    MSG_ERR_SIZE	= 0xFF,
    MSG_OK			= 1
} MsgRet_t;

// ***************Defines for *************************************
// *********************** Broadcast Message **********************

#define MESH_NODESTATE_OK			0x00
#define MESH_NODESTATE_HOST_LOST	0x30

#define MESH_BROADCAST_RAWDATA_LEN				(MESH_MAX_ADSU_LENGTH)

typedef struct
{
    uint8_t header[APS_ASDU_OFFSET];
    uint8_t rawdata[MESH_BROADCAST_RAWDATA_LEN];
    uint8_t footer[APS_AFFIX_LENGTH - APS_ASDU_OFFSET];
} PACK appMessageBroadcastRequest_t;

typedef struct
{
    uint8_t message_id;
    uint8_t length;
    uint8_t *payload;
} PACK appNodeSendBroadcastMessage_t;


// ***************Defines for *************************************
// *********************** Unicast Message ************************

#define MESH_UNICAST_RAWDATA_LEN				(MESH_MAX_ADSU_LENGTH)

typedef struct
{
    uint8_t header[APS_ASDU_OFFSET];
    uint8_t rawdata[MESH_UNICAST_RAWDATA_LEN];
    uint8_t footer[APS_AFFIX_LENGTH - APS_ASDU_OFFSET];
} PACK appMessageUnicastRequest_t;

typedef struct
{
    uint8_t message_id;
    uint64_t dstExtAddr;
    uint16_t dstShortAddr;
    uint8_t ack;
    uint8_t	length;
    uint8_t *payload;
} PACK appNodeSendUnicastMessage_t;


// ***************Defines for *************************************
// *********************** Netstat Message ************************

#define MESH_NETSTAT_RAWDATA_LEN                (MESH_MAX_ADSU_LENGTH)

typedef struct
{
    ShortAddr_t networkAddr;
    Relationship_t relationship;
    Lqi_t lqi;
    Rssi_t rssi;
    /** The cost of an outgoing link as measured by the neighbor. */
    unsigned outgoingCost  :3;
    /** The cost of an incoming link as measured by this device. */
    unsigned incomingCost  :3;
}appMessageNetstatNeighbours_t;

enum
{
    NETSTAT_MSG_NEIGHBOURS_REQUEST  = 0x10,
    NETSTAT_MSG_NEIGHBOURS_RESPONSE = 0x11,
};

typedef struct
{
    uint8_t msgid;
    union
    {
        appMessageNetstatNeighbours_t   Neightbours;
    }data;
}appMessageNetstatData_t;

typedef struct
{
    uint8_t header[APS_ASDU_OFFSET];
    appMessageNetstatData_t data;
    uint8_t footer[APS_AFFIX_LENGTH - APS_ASDU_OFFSET];
} PACK appMessageNetstatRequest_t;

extern appMessageBroadcastRequest_t appMessageBroadcast;

MsgRet_t appNodeSendBroadcastMessage(appNodeSendBroadcastMessage_t *params);
MsgRet_t appNodeSendUnicastMessage(appNodeSendUnicastMessage_t *params);
MsgRet_t appNodeSendNetstatMessage(appNodeSendUnicastMessage_t *params);

void appInitDeviceNode(void);
void APS_DataIndBroadcast(APS_DataInd_t *indData);
void APS_DataIndUnicast(APS_DataInd_t *indData);
void APS_DataIndNetstat(APS_DataInd_t *indData);


#endif /* ZIGCOMNODE_H */
