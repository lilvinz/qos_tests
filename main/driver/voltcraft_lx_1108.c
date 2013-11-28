/*
 * voltcraft_lx_1108.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 21.03.2011
 *      Author: DHA
 *
 * Purpose:
 * Collect measured values from VOLTCRAFT LX-1108
 *
 */

#include "stdheader.h"
#include "voltcraft_lx_1108.h"

#include "state.h"
#include "ascii.h"
#include "uart.h"
#include "systemconfig.h"
#include "log.h"
#include "clock.h"

static VOLTCRAFT_LX_1108_SENSOR_T brightness;
static CLOCK_MS_T timestamp;

void VOLTCRAFT_LX_1108_Message_RECV(char byte);

void VOLTCRAFT_LX_1108_Get_Brightness(VOLTCRAFT_LX_1108_SENSOR_T *p_data)
{
    taskENTER_CRITICAL();
    memcpy(p_data, &brightness, sizeof(VOLTCRAFT_LX_1108_SENSOR_T));
    taskEXIT_CRITICAL();
}

void VOLTCRAFT_LX_1108_Set_Brightness(VOLTCRAFT_LX_1108_SENSOR_T *p_data)
{
    taskENTER_CRITICAL();
    memcpy(&brightness, p_data, sizeof(VOLTCRAFT_LX_1108_SENSOR_T));
    taskEXIT_CRITICAL();
}

void VOLTCRAFT_LX_1108_Message_Parse(uint8_t * msgbuffer, uint8_t length)
{
    char* p_msgbuffer = (char*) msgbuffer;

    uint8_t i_index = 0;
    VOLTCRAFT_LX_1108_SENSOR_T local_brightness;

    if (length > VOLTCRAFT_LX_1108_RECV_BUFFER_SIZE)
        return;

    //search for End Of Text
    while (*++p_msgbuffer != ASCII_CR)
    {
        if (i_index++ >= length)
            return;             // End of Text not found, return
    }
    *p_msgbuffer = 0x00;        // terminate ASCII string here

    p_msgbuffer -= VOLTCRAFT_LX_1108_MSG_POS_NUM_DECIMALS;
    if (*p_msgbuffer != '0')
    {
        // decimals available --> remove decimals from string
        uint8_t num_decimals;
        num_decimals = *p_msgbuffer - '0';
        if (num_decimals < VOLTCRAFT_LX_1108_MSG_NUM_DATABYTES)
            *(p_msgbuffer + VOLTCRAFT_LX_1108_MSG_POS_NUM_DECIMALS - num_decimals)= 0x00;        // terminate ASCII string here
    }

    p_msgbuffer++;

    local_brightness.value = strtol(p_msgbuffer, NULL, 10);
    local_brightness.state = SENSOR_OK;

    VOLTCRAFT_LX_1108_Set_Brightness(&local_brightness);

    timestamp = CLOCK_GetMillitimestamp();
}

void VOLTCRAFT_LX_1108_Message_RECV(char byte)
{
    static uint8_t brightness_recv_buffer_index;
    static uint8_t brightness_recv_buffer[VOLTCRAFT_LX_1108_RECV_BUFFER_SIZE];

    if (brightness_recv_buffer_index >= VOLTCRAFT_LX_1108_RECV_BUFFER_SIZE)
        brightness_recv_buffer_index = 0;

    brightness_recv_buffer[brightness_recv_buffer_index++] = byte;

    //Check Start/Header
    switch (brightness_recv_buffer_index)
    {
    case 0:
        break;
    case 1:
        if (brightness_recv_buffer[0] != ASCII_STX)
            brightness_recv_buffer_index = 0;
        break;  // Check for Frame Start Mark
    case 2:
        if (brightness_recv_buffer[1] != '4')
            brightness_recv_buffer_index = 0;
        break;  // Check for "4"
    case 3:
        if ((brightness_recv_buffer[2]  != '1') && (brightness_recv_buffer[2]    != '2'))
            brightness_recv_buffer_index = 0;
        break;  // Check for "1" or "2"
    default:
        break;
    }

    if ((byte == ASCII_CR)
            && (brightness_recv_buffer_index >= 16))
    {
        VOLTCRAFT_LX_1108_Message_Parse(&brightness_recv_buffer[VOLTCRAFT_LX_1108_MSG_POS_PAYLOAD], brightness_recv_buffer_index);
        //reset buffer and index
        while (brightness_recv_buffer_index-- > 0)
            brightness_recv_buffer[brightness_recv_buffer_index] = 0xFF;
    }
}

void VOLTCRAFT_LX_1108_Tick(void)
{
    if (CLOCK_GetMillitimestamp() > timestamp + VOLTCRAFT_LX_1108_TIMEOUT)
        brightness.state = SENSOR_NOT_AVAILABLE;
}

void VOLTCRAFT_LX_1108_Init(void)
{
    taskENTER_CRITICAL();
    brightness.state = SENSOR_NOT_AVAILABLE;
    brightness.value = UINT16_T_MAX;
    taskEXIT_CRITICAL();

    UART_ChannelInitTypeDef UartChannelInitStruct;
    UartChannelInitStruct.baudrate          = VOLTCRAFT_LX_1108_BAUDRATE;
    UartChannelInitStruct.ctrl_flags        = UART_CTRL_8E1;
    UartChannelInitStruct.rx_callback_func  = VOLTCRAFT_LX_1108_Message_RECV;
    UART_ChannelInit(VOLTCRAFT_LX_1108_UART_CHANNEL, &UartChannelInitStruct);

    timestamp = CLOCK_GetMillitimestamp();
}

void VOLTCRAFT_LX_1108_DeInit(void)
{
    taskENTER_CRITICAL();
    brightness.state = SENSOR_NOT_AVAILABLE;
    brightness.value = UINT16_T_MAX;
    taskEXIT_CRITICAL();
}
