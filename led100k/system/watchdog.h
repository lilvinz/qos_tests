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


#define WATCHDOG_TASK_WAIT_TIME     1000  // wait 1 second
#define WATCHDOG_QUEUE_SIZE         20
#define WATCHDOG_TASK_WAIT_TIME_MS  1000

#define WATCHDOG_MAGIC_VARIABLE_INIT_VALUE  0xA518245A

enum
{
    WDVAL_INIT,
    WDVAL_RESET
};

typedef enum
{
    WATCHDOG_TASK_ID_DRIVER = 0,
    WATCHDOG_TASK_ID_TIMER,
    WATCHDOG_TASK_ID_SEQUENCE,
//    WATCHDOG_TASK_ID_DETECTOR,
    WATCHDOG_ID_CAN_RX,
    WATCHDOG_TASK_ID_COUNT
}WATCHDOG_TASK_ID_T;

void WATCHDOG_Task(void *pvParameters);
void WATCHDOG_Heartbeat(WATCHDOG_TASK_ID_T task_id);

#endif
