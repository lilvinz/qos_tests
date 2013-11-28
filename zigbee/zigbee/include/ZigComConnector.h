/*
 * ZigComConnector.h
 *
 *  Created on: 30.12.2008
 *      Author: Carsten Bartels
 */

#ifndef ZIGCOMCONNECTOR_H
#define ZIGCOMCONNECTOR_H

#include "ZigComNode.h"
#include "aps.h"

#define ZIGCOM_PROTOCOL_VERSION_MAJOR	3
#define ZIGCOM_PROTOCOL_VERSION_MINOR	0

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef PACK
#define PACK __attribute__((__aligned__ (8), __packed__))
#endif

#define ZIGCOM_MSG_HEADER_PRE_SIZE			6
#define ZIGCOM_BROADCAST_PAYLOAD_LEN		MESH_BROADCAST_RAWDATA_LEN - ZIGCOM_MSG_HEADER_PRE_SIZE
#define ZIGCOM_STREAM_PAYLOAD_LEN			MESH_UNICAST_RAWDATA_LEN - ZIGCOM_MSG_HEADER_PRE_SIZE

#define ZIGCOM_PORT_LOCAL			0x10
#define ZIGCOM_PORT_UNICAST			0x20
#define ZIGCOM_PORT_UNICAST_NOACK	0x21
#define ZIGCOM_PORT_BROADCAST		0x40

#define ZIGCOM_MSG_ID_VERSION       0x01
#define ZIGCOM_MSG_ID_COMMAND		0x10
#define ZIGCOM_MSG_ID_SETUP 		0x20
#define ZIGCOM_MSG_ID_STATUS 		0x40
#define ZIGCOM_MSG_ID_BC_BEACON_BC	0xB0
#define ZIGCOM_MSG_ID_BC_NODEINFO   0xB8

#define ZIGCOM_MSG_ID_RESPONSE		0xF0


typedef  uint8_t ZIGCOM_MSG_ID_T;

#define ZIGCOM_NODE_TYPE_COORDINATOR	0x00
#define ZIGCOM_NODE_TYPE_ROUTER			0x01
#define ZIGCOM_NODE_TYPE_END_DEVICE		0x02

#define ZIGCOM_NODE_STATUS_INITING_STATE			0x10
#define ZIGCOM_NODE_STATUS_CONFIG_STATE				0x12
#define ZIGCOM_NODE_STATUS_STARTING_NETWORK_STATE	0x20
#define ZIGCOM_NODE_STATUS_START_NETWORK_FAILED     0x28
#define ZIGCOM_NODE_STATUS_IN_NETWORK_STATE			0x30
#define ZIGCOM_NODE_STATUS_NETWORK_UPDATE           0x31
#define ZIGCOM_NODE_STATUS_LEAVING_NETWORK_STATE	0x40
#define ZIGCOM_NODE_STATUS_STOP_STATE				0x50

#define ZIGCOM_CMD_VERSION  0x01        // Request Version Information
#define ZIGCOM_CMD_RESET	0x10		// Soft-reset of ZigBee Stack
#define ZIGCOM_CMD_JOIN		0x20		// Join Network
#define ZIGCOM_CMD_LEAVE	0x21		// Leave Network
#define ZIGCOM_CMD_ACK		0x80		// ACK of last Command. Last Command returned as Parameter.
#define ZIGCOM_CMD_NACK		0x81


typedef struct zigcom_msg_version_s
{
    uint16_t    boardType;              // 2
    uint8_t     softVersionMajor;       // 1
    uint8_t     softVersionMinor;       // 1
    uint8_t     protocolVersionMajor;   // 1
    uint8_t     protocolVersionMinor;   // 1
    uint16_t    hardwareId;             // 2
    uint16_t    hardwareRevision;       // 2
} PACK ZIGCOM_MSG_VERSION_S;

typedef struct zigcom_msg_command_s
{
    uint8_t cmdid;
    uint16_t parameter16;
    uint32_t parameter32;
} PACK ZIGCOM_MSG_COMMAND_S;

#define ZIGCOM_NWK_KEY_LEN  16

typedef struct zigcom_msg_setup_s
{
    uint8_t		nodeType;
    uint8_t		channelPage;
    uint32_t	channelMask;
    uint64_t	uid;
    uint64_t	extPanId;
    int8_t		txPower;
    uint8_t     nwkKey[ZIGCOM_NWK_KEY_LEN];
    uint64_t    trustCenterAddr;
} PACK ZIGCOM_MSG_SETUP_S;


typedef struct zigcom_msg_status_s
{
    uint8_t		nodeStatus;			// 1
    uint16_t 	boardType;			// 2
    uint8_t		softVersionMajor;	// 1
    uint8_t		softVersionMinor;	// 1
    uint8_t		nodeType;			// 1
    uint8_t		workingChannel;		// 1
    uint16_t	shortPanId;			// 2
    uint64_t	extAddr;			// 8
    uint16_t	shortAddr;			// 2
    uint16_t	parentShortAddr;	// 2
    uint8_t		lqi;				// 1
    int8_t		rssi;				// 1
    uint8_t     status;             // 1
} PACK ZIGCOM_MSG_STATUS_S;  // Size = 24 Bytes


enum
{
    ZIGCOM_STATUS_FAILED			= -4,
    ZIGCOM_STATUS_NOT_AVAILABLE		= -3,
    ZIGCOM_STATUS_WAIT_FOR_INIT		= -2,
    ZIGCOM_STATUS_RUNAWAY_VALUE		= -1,
    ZIGCOM_STATUS_OK				= 0,
    ZIGCOM_STATUS_UPPER_LIMIT		= 1,
    ZIGCOM_STATUS_LOWER_LIMIT		= 2
} PACK ZIGCOM_STATUS_E;

typedef struct
{
    uint16_t 	boardType;
    uint8_t		softVersionMajor;
    uint8_t		softVersionMinor;
    uint8_t 	nodeType;
    uint16_t	shortAddr;
    uint16_t	parentShortAddr;
    uint8_t 	lqi;
    int8_t 		rssi;
    int8_t		hostState;
} PACK NODEINFO_T;						// 12

typedef struct
{
    NODEINFO_T	nodeinfo;
    uint8_t 	rawdata[ZIGCOM_BROADCAST_PAYLOAD_LEN - sizeof(NODEINFO_T)];
} PACK ZIGCOM_MSG_BEACON_BC_S;

typedef struct
{
    uint8_t 	payload[ZIGCOM_STREAM_PAYLOAD_LEN];
} PACK ZIGCOM_MSG_STREAM_S;


#define ZIGCOM_RESPONSE_MSG_ACK		1
#define ZIGCOM_RESPONSE_MSG_NACK	0
#define ZIGCOM_RESPONSE_MSG_STATUS	2

typedef struct
{
    uint8_t		message_id;
    uint8_t		response;
    uint8_t		code;
} PACK ZIGCOM_MSG_RESPONSE_S;

/***************** MESSAGE HEADER *****************/

typedef struct
{
    uint8_t     major;
    uint8_t     minor;
} PACK ProtocolVersion_t;

typedef struct
{
    uint8_t  major;
    uint8_t  minor;
} PACK MessageID_t;

typedef struct
{
    uint64_t	extAddr;
    uint16_t	shortAddr;
} PACK MESSAGE_ADDRESS_T;

typedef struct
{
    ProtocolVersion_t 	    version;
    uint8_t					port; 		// port number of zigbee stack
    MessageID_t				id;   		// value for big switch statement
    uint8_t					length;
} PACK MESSAGE_HDR_T;

typedef struct zigcom_msg_s
{
    MESSAGE_HDR_T	header;

    union
    {
        MESSAGE_ADDRESS_T	address;
        uint8_t				shortpacket_payload;
    }PACK;

    union
    {
        ZIGCOM_MSG_VERSION_S        version;
        ZIGCOM_MSG_COMMAND_S		command;
        ZIGCOM_MSG_SETUP_S			setup;
        ZIGCOM_MSG_STATUS_S			status;
        ZIGCOM_MSG_BEACON_BC_S		beacon_bc;
        ZIGCOM_MSG_STREAM_S			stream;
        ZIGCOM_MSG_RESPONSE_S		response;
        uint8_t						payload;
    }PACK;
} PACK ZIGCOM_MSG_S;

typedef struct zigcom_short_msg_s
{
    MESSAGE_HDR_T	header;
    uint8_t			shortpacket_payload;
} PACK ZIGCOM_SHORT_MSG_S;


#define ZIGCOM_MSG_HEADER_SIZE			(sizeof(MESSAGE_HDR_T) + sizeof(MESSAGE_ADDRESS_T))
#define ZIGCOM_MSG_SHORTHEADER_SIZE		(sizeof(MESSAGE_HDR_T))
#define ZIGCOM_MSG_VERSION_SIZE         (sizeof(ZIGCOM_MSG_VERSION_S))
#define ZIGCOM_MSG_COMMAND_SIZE         (sizeof(ZIGCOM_MSG_COMMAND_S))
#define ZIGCOM_MSG_SETUP_SIZE			(sizeof(ZIGCOM_MSG_SETUP_S))
#define ZIGCOM_MSG_STATUS_SIZE			(sizeof(ZIGCOM_MSG_STATUS_S))
#define ZIGCOM_MSG_BEACON_BC_SIZE		(sizeof(ZIGCOM_MSG_BEACON_BC_S))
#define ZIGCOM_MSG_STREAM_SIZE			(sizeof(ZIGCOM_MSG_STREAM_S))
#define ZIGCOM_MSG_RESPONSE_SIZE		(sizeof(ZIGCOM_MSG_RESPONSE_S))


void appConnectorDecodeUARTMessage(ZIGCOM_MSG_S* pMessage);
void appConnectorMessageToUart(APS_DataInd_t *indData);
void appConnectorSendResponse(uint8_t port, ZIGCOM_MSG_S * msg);
void appConnectorSendStatus(void);

void appConnectorInit(void);

#endif /* ZIGCOMCONNECTOR_H */
