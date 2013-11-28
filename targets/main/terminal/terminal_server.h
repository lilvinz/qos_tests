/*
 * terminal_server.h
 *
 *  Created on: 03.05.2011
 *      Author: cba
 */

#ifndef TERMINAL_SERVER_H_
#define TERMINAL_SERVER_H_

#include "uart.h"
#include "settings.h"

#define TERMINAL_SERVER_UART_CHANNEL     UART_CHANNEL_5
#define TERMINAL_SERVER_UART_BAUDRATE    _settings_list_ram[SETTINGS_TERMINAL_BAUDRATE]
#define TERMINAL_SERVER_UART_FLAGS       UART_CTRL_8N1

#define TERMINAL_USE_TELNET     _settings_list_ram[SETTINGS_TERMINAL_TELNET]



#define GETCHAR(x) uart_getchar(x)
#define PUTCHAR(x) uart_putchar(x)
#define CAVAIL(x)  uart_cavail(x)


void TERMINAL_SERVER_GetExclusiveIO(void);
void TERMINAL_SERVER_LoseExclusiveIO(void);
void TERMINAL_SERVER_Init(void);
void TERMINAL_SERVER_Output(uint8_t c);


#endif /* TERMINAL_SERVER_H_ */
