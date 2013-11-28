/*
 * ZigComUARTManager.h
 *
 *  Created on: 30.12.2008
 *      Author: cba
 */

#ifndef ZIGCOMUARTMANAGER_H_
#define ZIGCOMUARTMANAGER_H_


// ******** ZIGCOM Frame description *********
#define FRAME_BORDERSIZE		(8)		// start+len+checksum+end
#define POS_START_SEQ_1			0		// message start mark 1	(0xA0)
#define POS_START_SEQ_2			1		// message start mark 2	(0xA2)
#define POS_LENGTH_UW			2		// length upper byte
#define POS_LENGTH_LW			3		// length lower byte
#define POS_PAYLOAD				4		// start of payload
#define POS_OFFSET_CHECKSUM_UW	3		// offset from end of msg for checksum upper byte
#define POS_OFFSET_CHECKSUM_LW	2		// offset from end of msg for checksum lower byte


#define MAX_UART_MESSAGE_QUEUE_COUNT 30U
#define UART_MESSAGESIZE_MAX	sizeof(ZIGCOM_MSG_S) + FRAME_BORDERSIZE + 20
#define UART_RXBUFFER_SIZE      (2 * UART_MESSAGESIZE_MAX)

//#define UART_MESSAGESIZE_MAX	90

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif


void appStartUARTManager(void);
void appSendMessageToUart(uint8_t *data, uint16_t len);


#endif /* ZIGCOMUARTMANAGER_H_ */
