/*
 * watchdog.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 29.10.2010
 *      Author:DHA
 *
 * Purpose:
 * Watchdog
 *
 */
#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "version.h"


#define WATCHDOG_TASK_WAIT_TIME     1000  // wait 1 second
#define WATCHDOG_QUEUE_SIZE         20  // todo check size
#define WATCHDOG_TASK_WAIT_TIME_MS  1000  // todo check size

#define WATCHDOG_MAGIC_VARIABLE_INIT_VALUE  0xA518245A

enum
{
    WDVAL_INIT,
    WDVAL_RESET
};

typedef enum
{
    WATCHDOG_TASK_ID_LOG = 0,
    WATCHDOG_TASK_ID_DRIVER,
    WATCHDOG_TASK_ID_TIMER,
    WATCHDOG_TASK_ID_SENSORS,
    WATCHDOG_TASK_ID_GPSSYNC,  // at least 30 seconds
    WATCHDOG_TASK_ID_GLOOM_MANAGER,
    WATCHDOG_TASK_ID_DETECTOR,
    WATCHDOG_TASK_ID_INDICATION,
    WATCHDOG_TASK_ID_MESH,
    WATCHDOG_TASK_ID_SYNDICATION,
    WATCHDOG_TASK_ID_DIM,
    WATCHDOG_TASK_ID_SEQUENCE,
    WATCHDOG_TASK_ID_ALARM_CLOCK,
    WATCHDOG_TASK_ID_SVINFO_MANAGER,
#ifdef HWP_CAN_MASTER
    WATCHDOG_TASK_ID_CAN_MANAGER,
#endif
//    WATCHDOG_TASK_ID_TELNET,
    WATCHDOG_TASK_ID_COUNT
} WATCHDOG_TASK_ID_T;

void WATCHDOG_Task(void *pvParameters);
void WATCHDOG_Heartbeat(WATCHDOG_TASK_ID_T task_id);

#endif
