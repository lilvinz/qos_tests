/************************************************************************
 UARTManager.c
 **************************************************************************/

#include <types.h>
#include <usart.h>
#include "ascii.h"
#include "rs232Controller.h"
#include <ZigComApp.h>
#include "ZigComUARTManager.h"
#include "ZigComConnector.h"

#define PARSER(x)	appConnectorDecodeUARTMessage(x)

typedef struct
{
    uint8_t payload[UART_MESSAGESIZE_MAX];
    uint16_t size;
} UartMessage_t;

static struct
{
    UartMessage_t uartMessageQueue[MAX_UART_MESSAGE_QUEUE_COUNT];
    uint8_t head, tail, size;
} zbn2uart;

static uint8_t 	rxBuffer[UART_RXBUFFER_SIZE];

static void readByteEvent(uint16_t bytesToRead);
static void writeConfirm(void);
static void sendNextMessage(void);
static HAL_UsartDescriptor_t uartDescriptor;

static uint8_t recv_frame_buffer[UART_MESSAGESIZE_MAX];
static uint16_t recv_frame_buffer_index;
static uint8_t inBuffer[UART_RXBUFFER_SIZE];
static uint16_t inBufferIndex;

static uint16_t buffer_index;
static uint8_t byte;

/***********************************************************************************
 Init UART, register UART callbacks.

 Parameters: none
 Return: none
 ***********************************************************************************/
void appStartUARTManager(void)
{
    uartDescriptor.tty = USART_CHANNEL_0;
    uartDescriptor.mode = USART_MODE_ASYNC;
    uartDescriptor.flowControl = USART_FLOW_CONTROL_NONE;
    uartDescriptor.baudrate = USART_BAUDRATE_115200;
    uartDescriptor.dataLength = USART_DATA8;
    uartDescriptor.parity = USART_PARITY_NONE;
    uartDescriptor.stopbits = USART_STOPBIT_1;
    uartDescriptor.rxBuffer = rxBuffer;
    uartDescriptor.rxBufferLength = UART_RXBUFFER_SIZE;
    uartDescriptor.txBuffer = NULL;
    uartDescriptor.txBufferLength = 0;
    uartDescriptor.rxCallback = readByteEvent;
    uartDescriptor.txCallback = writeConfirm;

    BSP_EnableRs232();
    HAL_OpenUsart(&uartDescriptor);

    memset(&zbn2uart, 0, sizeof(zbn2uart));
}

void appStopUARTManager(void)
{
    BSP_DisableRs232();
    HAL_CloseUsart(&uartDescriptor);
}


/***********************************************************************************
 New UART bytes were received.

 Parameters: readBytesLen - count of received bytes
 Return: none
 ***********************************************************************************/
static void readByteEvent(uint16_t bytesToRead)
{
    if (bytesToRead == 0)
        return;

    if (bytesToRead > UART_RXBUFFER_SIZE)
        bytesToRead = UART_RXBUFFER_SIZE;

    inBufferIndex = 0;

    HAL_ReadUsart(&uartDescriptor, &inBuffer[0], bytesToRead);

    do
    {
        byte = inBuffer[inBufferIndex++];

        recv_frame_buffer[recv_frame_buffer_index++] = byte;


        if (recv_frame_buffer_index >= UART_MESSAGESIZE_MAX)
            recv_frame_buffer_index = 0;

        // '0xA0' '0xA2' marks the beginning of a message frame
        switch (recv_frame_buffer_index)
        {
        case 0:
            break;
        case 1:
            if (recv_frame_buffer[POS_START_SEQ_1] != 0xA0) recv_frame_buffer_index = 0;
            break;	// Check for 1st Frame Start Mark
        case 2:
            if (recv_frame_buffer[POS_START_SEQ_2] != 0xA2) recv_frame_buffer_index = 0;
            break;	// Check for 2nd Frame Start Mark
        case 3:
            if (recv_frame_buffer[POS_LENGTH_UW]	!= 0x00) recv_frame_buffer_index = 0;
            break;	// Reject messages larger than 255 Bytes
        default:
            if ((recv_frame_buffer[POS_LENGTH_LW] + FRAME_BORDERSIZE) < recv_frame_buffer_index) recv_frame_buffer_index = 0;
            break; // Prevent buffer overflow
        }

        if ((recv_frame_buffer_index > (FRAME_BORDERSIZE - 1))
                && ((recv_frame_buffer[POS_LENGTH_LW] + FRAME_BORDERSIZE) == recv_frame_buffer_index)
                && (byte == 0xB3))
        {
            // Frame received! Check frame checksum
            uint16_t chksum = 0;

            for (buffer_index = POS_PAYLOAD; buffer_index < (recv_frame_buffer[POS_LENGTH_LW] + POS_PAYLOAD); buffer_index++)
            {
                chksum += recv_frame_buffer[buffer_index];
            }
            chksum &= 0x7FFF;

            if ((recv_frame_buffer[recv_frame_buffer_index - POS_OFFSET_CHECKSUM_UW - 1] == (0xFF & (chksum >> 8)))
                    && (recv_frame_buffer[recv_frame_buffer_index - POS_OFFSET_CHECKSUM_LW - 1] == (0xFF & chksum)))
            {
                // checksum is ok, process packet
                PARSER((ZIGCOM_MSG_S*)&recv_frame_buffer[POS_PAYLOAD]);
                memset(recv_frame_buffer, 0, UART_MESSAGESIZE_MAX);
            }
            recv_frame_buffer_index = 0;
        }

    }
    while (inBufferIndex < bytesToRead);
}


/***********************************************************************************
 Send next message from queue.

 Parameters: none
 Return: none
 ***********************************************************************************/
static void sendNextMessage(void)
{
    if (zbn2uart.size)
    {
        HAL_WriteUsart(&uartDescriptor,
                       zbn2uart.uartMessageQueue[zbn2uart.head].payload,
                       zbn2uart.uartMessageQueue[zbn2uart.head].size);
    }
}


/***********************************************************************************
 Writing confirmation has been received. New message can be sent.

 Parameters: none
 Return: none
 ***********************************************************************************/
static void writeConfirm(void)
{
    if (zbn2uart.size)
    {
        zbn2uart.size--;
        if (++zbn2uart.head >= MAX_UART_MESSAGE_QUEUE_COUNT)
            zbn2uart.head -= MAX_UART_MESSAGE_QUEUE_COUNT;
    }
    //send next message
    sendNextMessage();
}


/***********************************************************************************
 New message being sent into UART has to be put into queue.

 Parameters: newMessage - new message fields.
 Return: none
 ***********************************************************************************/
void appSendMessageToUart(uint8_t *data, uint16_t len)
{
    UartMessage_t *msg;
    uint8_t *p;
    uint16_t chksum;

    chksum = 0;

    if (len > UART_MESSAGESIZE_MAX - 8)
        return;

    if (zbn2uart.size < MAX_UART_MESSAGE_QUEUE_COUNT)
    {
        zbn2uart.size++;
        msg = &zbn2uart.uartMessageQueue[zbn2uart.tail];
        if (++zbn2uart.tail >= MAX_UART_MESSAGE_QUEUE_COUNT)
            zbn2uart.tail -= MAX_UART_MESSAGE_QUEUE_COUNT;

        p = msg->payload;

        // Add Frame Start Sequence
        *p++ = 0xA0;
        *p++ = 0xA2;
        // Add Frame Payload Length
        *p++ = 0x00;
        *p++ = len;
        msg->size = len + 4;

        // Add Frame Payload
        while (len-- > 0)
        {
            *p = *data;
            chksum += *data;
            p++;
            data++;
        }

        // Add Frame Payload Checksum
        chksum &= 0x7FFF;
        *p++ = (uint8_t)(0xFF & (chksum >> 8));
        *p++ = (uint8_t)(0xFF & chksum);
        *p++ = 0xB0;
        *p++ = 0xB3;
        msg->size += 4;
    }

    //check sending state
    sendNextMessage();
}

//eof UARTManager.c
