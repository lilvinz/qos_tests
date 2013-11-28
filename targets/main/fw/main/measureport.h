/*
 * measureport.h
 *
 *  Created on: 28.11.2011
 *      Author: cba
 */

#ifndef MEASUREPORT_H_
#define MEASUREPORT_H_

#define MEASUREPORT_USAGE _settings_list_ram[SETTINGS_TERMINAL_RS485_PORT2_FUNCTION]

#define MEASUREPORT_PUTCHAR(x) UART4_PutChar(x)

#define MEASUREPORT_INTERVAL        1

void MEASUREPORT_Tick(void);
void MEASUREPORT_Init(void);
void MEASUREPORT_SendSync(void);

#endif /* MEASUREPORT_H_ */
