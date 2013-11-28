/*
 * visibility_vaisala_pwd20.c
 *
 *  Created on: 18.08.2011
 *      Author: cba
 */

#include "stdheader.h"
#include "visibility.h"
#include "visibility_vaisala_pwd20.h"

#include "ascii.h"

static Visibility_Data_In_Callback_t *pDataInFunc;
static bool b_acquire;

const uint8_t visi_pwd20_message_request[1][10]  =
{
    { ASCII_CR, ASCII_ENQ, 'P', 'W', ' ', '1', ' ', '0', ASCII_CR, 0x00 },
};


void messagereceive(char byte);
void messageparse(uint8_t * msgbuffer, uint8_t length);
void sendrequest(uint8_t msgid);


void messageparse(uint8_t * msgbuffer, uint8_t length)
{
    char* p_msgbuffer = (char*) msgbuffer;
    uint8_t i_index = 0;
    VISIBILITY_SENSOR_T local_visibility;

    if (length > VISI_RECV_BUFFER_SIZE)
        return;

    //search for End Of Text
    while (*++p_msgbuffer != ASCII_ETX)
    {
        if (i_index++ >= length)
            return;             // End of Text not found, return
    }
    *p_msgbuffer = 0x00;        // terminate ASCII string here

    p_msgbuffer -= 6;
    if (*(p_msgbuffer + 1) != '/')
        local_visibility.value_rms_10min = strtol(p_msgbuffer, NULL, 10);
    else
        local_visibility.value_rms_10min = UINT16_T_MAX;

    *p_msgbuffer = 0x00;        // terminate ASCII string here

    p_msgbuffer -= 7;
    if (*(p_msgbuffer + 1) != '/')
        local_visibility.value_rms_1min = strtol(p_msgbuffer, NULL, 10);
    else
        local_visibility.value_rms_1min = UINT16_T_MAX;

    *p_msgbuffer = 0x00;        // terminate ASCII string here

    p_msgbuffer -= 2;

    //get status
    switch ((uint8_t)(*(p_msgbuffer + 1)))
    {
    case '0':
        local_visibility.status = VISIBILITY_SENSOR_STATUS_OK;
        break;
    case '1':
        local_visibility.status = VISIBILITY_SENSOR_STATUS_HARDWARE_ERROR;
        break;
    case '2':
        local_visibility.status = VISIBILITY_SENSOR_STATUS_HARDWARE_WARNING;
        break;
    case '3':
        local_visibility.status = VISIBILITY_SENSOR_STATUS_BACKSCATTER_ALARM;
        break;
    case '4':
        local_visibility.status = VISIBILITY_SENSOR_STATUS_BACKSCATTER_WARNING;
        break;
    default:
        local_visibility.status = VISIBILITY_SENSOR_STATUS_UNKNOWN;
        break;
    }

    b_acquire = FALSE;

    if (local_visibility.value_rms_10min != UINT16_T_MAX)
    {
        // wait until 10 minute value is updated
        local_visibility.state = SENSOR_OK;
    }
    else
    {
        local_visibility.state = SENSOR_WAIT_FOR_INIT;
    }

    pDataInFunc(&local_visibility);
}


void messagereceive(char byte)
{
    static uint8_t visi_recv_buffer_index;
    static uint8_t visi_recv_buffer[VISI_RECV_BUFFER_SIZE];

    if (visi_recv_buffer_index >= VISI_RECV_BUFFER_SIZE)
        visi_recv_buffer_index = 0;

    visi_recv_buffer[visi_recv_buffer_index++] = byte;

    //Check Start/Header
    switch (visi_recv_buffer_index)
    {
    case 0:
        break;
    case 1:
        if (visi_recv_buffer[0] != ASCII_SOH) visi_recv_buffer_index = 0;
        break;  // Check for Frame Start Mark
    case 2:
        if (visi_recv_buffer[1] != 'P') visi_recv_buffer_index = 0;
        break;  // Check for "P"
    case 3:
        if (visi_recv_buffer[2] != 'W') visi_recv_buffer_index = 0;
        break;  // Check for "W"
    default:
        break;
    }

    if ((byte == ASCII_LF)
            && (visi_recv_buffer_index > 5))
    {
        if (visi_recv_buffer[visi_recv_buffer_index - 2] == ASCII_CR)
        {
            messageparse(&visi_recv_buffer[VISI_MSG_POS_PAYLOAD], visi_recv_buffer_index);
            //reset buffer and index
            while (visi_recv_buffer_index-- > 0)
                visi_recv_buffer[visi_recv_buffer_index] = 0xFF;
        }
    }
}

void sendrequest(uint8_t msgid)
{
    uint8_t idx;
    char c;

    idx = 0;
    do
    {
        c = visi_pwd20_message_request[msgid][idx++];
        if (c == 0)
            break;
        VISIBILITY_UART_SEND(c);
    }
    while (1);
}

bool VISIBILITY_VAISALA_PWD20_Acquire(Visibility_Data_In_Callback_t *pFunc)
{
    if (b_acquire)
    {
        // request still running, no answer received --> error
        b_acquire = FALSE;
        return FALSE;
    }

    //send out request
    b_acquire = TRUE;
    pDataInFunc = pFunc;
    sendrequest(VISI_MSG_ID_0);
    return TRUE;
}

void VISIBILITY_VAISALA_PWD20_Init(void)
{
    UART_ChannelInitTypeDef UartChannelInitStruct;
    UartChannelInitStruct.baudrate          = VISIBILITY_BAUDRATE;
    UartChannelInitStruct.ctrl_flags        = UART_CTRL_7E1;
    UartChannelInitStruct.rx_callback_func  = messagereceive;
    UART_ChannelInit(VISIBILITY_UART_CHANNEL, &UartChannelInitStruct);
}

void VISIBILITY_VAISALA_PWD20_DeInit(void)
{
    UART_ChannelUnInit(VISIBILITY_UART_CHANNEL);
}
