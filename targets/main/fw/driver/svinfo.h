/*
 * svinfo.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 31.10.2011
 *      Author: DHA
 *
 * Purpose:
 * Receive messages from svinfo
 *
 */

#ifndef SVINFO_H
#define SVINFO_H

#define SVINFO_SIGNAL_UART_BAUDRATE         300  // Baudrate
#define SVINFO_SIGNAL_TIMER_MAX_COUNT       20000  // maximum counter value

void SVINFO_Create(void);
void SVINFO_Deinit(void);
void SVINFO_Init(void);
void SVINFO_ExtIRQHandler(void);
bool SVINFO_CharAvail(void);
int SVINFO_GetChar(void);
bool SVINFO_GetTestMode(void);
int SVINFO_NumCharAvail(void);

#endif
