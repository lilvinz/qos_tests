#include "stdheader.h"
#include "sequence.h"
#include "watchdog.h"
#include "dim.h"

/*-------------- Local Type Definitions ------------*/

/*

*/

/*-------------- Local Variables ------------*/

uint32_t sequence_pwm;
uint32_t sequence_time;


/*-------------- Local Functions ------------*/


void SEQUENCE_Task(void *pvParameters)
{
    sequence_pwm = 0;
    sequence_time = 0;

    DIM_Init();

    while(1)
    {
        WATCHDOG_Heartbeat(WATCHDOG_TASK_ID_SEQUENCE);

        if (sequence_time > SEQUENCE_MAX_ON_TIME)
            sequence_time = SEQUENCE_MAX_ON_TIME;

        if (sequence_time != 0)
        {
            DIM_Set((DIM_VALUE_T)sequence_pwm);

            vTaskDelay(sequence_time);
            DIM_Reset();

            vTaskDelay(sequence_time * SEQUENCE_MIN_OFF_FACTOR);  // minimal downtime = SEQUENCE_MIN_OFF_FACTOR * on time
            sequence_time = 0;
        }

        vTaskDelay(1);
    }
}


void SEQUENCE_Init(void)
{

}
