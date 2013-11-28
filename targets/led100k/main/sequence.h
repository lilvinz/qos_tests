#ifndef SEQUENCE_H
#define SEQUENCE_H

/***************************************/
/************** SEQUENCE ***************/

#define SEQUENCE_MAX_ON_TIME    200
#define SEQUENCE_MIN_OFF_FACTOR 4

typedef struct
{
    uint32_t    pwm;
    uint32_t    time;
} SEQUENCE_T;


void SEQUENCE_Task(void *pvParameters);
void SEQUENCE_Init(void);

#endif
