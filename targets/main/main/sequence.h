#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "dim.h"

/***************************************/
/************** SEQUENCE ***************/

#define SEQUENCE_LENGTH     10

#define SEQUENCE_TESTDRIVE_LEDPOWER 1000

#define SEQUENCE_GLOOM_SENSOR_CAPTURE_TIME 700  // 700ms, needed for gloom measurement during led off time fixme algorithm for capturing gloom values has to be changed to reduce time (sensor needs only 250 ms)

typedef struct seq_table_t
{
    uint16_t    timespan;
    uint16_t    dim_time;
    DIM_VALUE_T dim_value;
} SEQUENCE_TABLE_T; // 12

typedef struct sequence_t
{
    uint8_t             length;
    SEQUENCE_TABLE_T    table[SEQUENCE_LENGTH]; // 120
    uint16_t            delay;
} SEQUENCE_T; // 128 (not packed)

void SEQUENCE_RTC_TickHandlerFromISR(void);
void SEQUENCE_FlatOn(uint16_t timeout);
void SEQUENCE_KickStart(void);
void SEQUENCE_TestDriveStartBlocking(uint32_t uStepTimeMs);
void SEQUENCE_TestDriveStart(uint32_t uStepTimeMs);
void SEQUENCE_Task(void *pvParameters);
void SEQUENCE_Init(void);

#endif
