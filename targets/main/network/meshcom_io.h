/*
 * meshcom_io.h
 *
 *  Created on: 23.12.2011
 *      Author: cba
 */

#ifndef MESHCOM_IO_H
#define MESHCOM_IO_H

#include "meshcom.h"


#define MESHCOM_UART_CHANNEL         UART_CHANNEL_2
#define MESHCOM_UART_SEND(c)         UART2_PutChar(c)
#define MESHCOM_UART_CHAR_AVAIL()    UART2_CharAvail()
#define MESHCOM_UART_RECV()          UART2_GetChar()

#define MESHCOM_SEND(x) MESHCOM_UART_SEND(x)




#define MESHCOM_V1_MESSAGESIZE_MAX   128

#define MESHCOM_V1_FRAME_BOARDERSIZE    (8)        // start+len+checksum+end
#define POS_START_SEQ_1              0        // message start mark 1    (0xA0)
#define POS_START_SEQ_2              1        // message start mark 2    (0xA2)
#define POS_LENGTH_UW                2        // length upper byte
#define POS_LENGTH_LW                3        // length lower byte
#define POS_PAYLOAD                  4        // start of payload
#define POS_OFFSET_CHECKSUM_UW       3        // offset from end of msg for checksum upper byte
#define POS_OFFSET_CHECKSUM_LW       2        // offset from end of msg for checksum lower byte


#define MESHCOM_ERR_CNT_MAX 10


typedef struct
{
    uint32_t bytesProcessed;
    uint32_t bytesReceived;
    uint32_t errorChecksum;
    uint32_t errorSize;
    uint32_t packetsProcessed;
} meshcomRcvStats_t;

extern meshcomRcvStats_t meshcomRcvStats;


MESHCOM_MSG_S* MESHCOM_IO_Receive(uint8_t byte);
void MESHCOM_IO_Dispatch(uint8_t *message, uint8_t length);




#endif
