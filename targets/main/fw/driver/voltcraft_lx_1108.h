/*
 * voltcraft_lx_1108.h
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

#ifndef VOLTCRAFT_LX_1108_H
#define VOLTCRAFT_LX_1108_H

#include "sensor.h"

#define VOLTCRAFT_LX_1108_MSG_POS_PAYLOAD       2
#define VOLTCRAFT_LX_1108_MSG_POS_NUM_DECIMALS  9
#define VOLTCRAFT_LX_1108_MSG_NUM_DATABYTES     8

#define VOLTCRAFT_LX_1108_RECV_BUFFER_SIZE  50
#define VOLTCRAFT_LX_1108_TIMEOUT           2000

#define VOLTCRAFT_LX_1108_UART_CHANNEL              UART_CHANNEL_4
#define VOLTCRAFT_LX_1108_UART_SEND(c)              UART4_PutChar(c)

#define VOLTCRAFT_LX_1108_BAUDRATE                  9600

typedef struct
{
    uint32_t        value;
    SENSOR_STATUS_T state;
} VOLTCRAFT_LX_1108_SENSOR_T;

void VOLTCRAFT_LX_1108_Get_Brightness(VOLTCRAFT_LX_1108_SENSOR_T *p_data);
void VOLTCRAFT_LX_1108_DeInit(void);
void VOLTCRAFT_LX_1108_Init(void);
void VOLTCRAFT_LX_1108_Tick(void);
#endif

