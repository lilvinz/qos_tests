/*
 * terminal_server.c
 *
 *  Created on: 03.05.2011
 *      Author: cba
 */

#include "stdheader.h"
#include "terminal_server.h"

#include "vcp_uart.h"
#include "telnet.h"
#include "cli.h"


typedef enum
{
    TERMINAL_SOURCE_UART,
    TERMINAL_SOURCE_VCPUART
} TERMINAL_SOURCE_T;

static bool b_exclusive_io = FALSE;
static TERMINAL_SOURCE_T   terminal_source;


void TERMINAL_SERVER_Input(char c);


void TERMINAL_SERVER_Input(char c)
{
    if (TERMINAL_USE_TELNET)
        telnet_input(c);
    else
        cli_input(c);
}

void terminal_server_in_vcpuart(char c)
{
    terminal_source = TERMINAL_SOURCE_VCPUART;
    TERMINAL_SERVER_Input(c);
}

void terminal_server_in_uart(char c)
{
    terminal_source = TERMINAL_SOURCE_UART;
    TERMINAL_SERVER_Input(c);
}

void TERMINAL_SERVER_Output(uint8_t c)
{
    switch(terminal_source)
    {
    case TERMINAL_SOURCE_UART:
        if (!b_exclusive_io)
            PUTCHAR(c);
        break;
    case TERMINAL_SOURCE_VCPUART:
        VCP_UART_PutChar(VCP_UART_CHANNEL_1, c);
        break;
    default:
        break;
    }
}


void TERMINAL_SERVER_GetExclusiveIO(void)
{
    UART_ChannelInitTypeDef UartChannelInitStruct;

    UartChannelInitStruct.baudrate = TERMINAL_SERVER_UART_BAUDRATE;
    UartChannelInitStruct.ctrl_flags = TERMINAL_SERVER_UART_FLAGS;
    UartChannelInitStruct.rx_callback_func = NULL;
    UART_ChannelInit(TERMINAL_SERVER_UART_CHANNEL, &UartChannelInitStruct);

    b_exclusive_io = TRUE;
}


void TERMINAL_SERVER_LoseExclusiveIO(void)
{
    TERMINAL_SERVER_Init();
}


void TERMINAL_SERVER_Init(void)
{
    UART_ChannelInitTypeDef UartChannelInitStruct;

    UartChannelInitStruct.baudrate = TERMINAL_SERVER_UART_BAUDRATE;
    UartChannelInitStruct.ctrl_flags = TERMINAL_SERVER_UART_FLAGS;
    UartChannelInitStruct.rx_callback_func = terminal_server_in_uart;
    UART_ChannelInit(TERMINAL_SERVER_UART_CHANNEL, &UartChannelInitStruct);

    VCP_UART_RegisterCallback(VCP_UART_CHANNEL_1, terminal_server_in_vcpuart);

    b_exclusive_io = FALSE;
}
