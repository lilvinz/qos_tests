/*
 * usb_vcp.c
 *
 *  Created on: 29.09.2010
 *      Author: vinz
 */

#ifndef _VCP_UART_H
#define _VCP_UART_H

/*
 * This is a simple queue based layer to drive a virtual usb com port
 *
 * please notice:
 * - data gets lost if rx queue is full on receiving new data
 *
 *
 */

//timeout for user api in ms
#define VCP_UART_TIMEOUT                100

// Interval between sending IN packets in frame number (1 frame = 1ms)
#define VCOMPORT_IN_FRAME_INTERVAL      1


typedef enum
{
    VCP_UART_CHANNEL_1,
    VCP_UART_CHANNEL_COUNT
} VCP_UART_CHANNEL_E;

typedef void (VcpUartRxCallBackHandler_t)(char c);

typedef struct
{
    VcpUartRxCallBackHandler_t *rx_callback_func;
} VCP_UART_ChannelConfig_T;

void VCP_UART_Init(VCP_UART_CHANNEL_E Channel, uint16_t RxQueueSize, uint16_t TxQueueSize);
void VCP_UART_DeInit(VCP_UART_CHANNEL_E Channel);

bool VCP_UART_CharAvail(VCP_UART_CHANNEL_E Channel);
uint8_t VCP_UART_GetChar(VCP_UART_CHANNEL_E Channel);
void VCP_UART_PutChar(VCP_UART_CHANNEL_E Channel, uint8_t Byte);

void VCP_UART_AddReceivedChar_FromISR(uint8_t *pData);

void VCP_UART_RegisterCallback(VCP_UART_CHANNEL_E channel, VcpUartRxCallBackHandler_t *rx_callback_func);
void VCP_UART_Tick(void);

#endif /* _VCP_UART_H */
