
#include "stdheader.h"
#include "alarm_clock.h"

#include "clock.h"
#include "rtc.h"
#include "debug.h"
#include "watchdog.h"
#include "statistics.h"
#include "time_meter.h"
#include "debug_freertos.h"

static volatile xSemaphoreHandle    xAlarmClockTaskWaitMutex;

/**
 * \brief give mutex
 *
 * \param none
 * \return none
*/
void ALARM_CLOCK_GiveMutexFromISR(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if (xAlarmClockTaskWaitMutex != NULL)
    {
        xSemaphoreGiveFromISR(xAlarmClockTaskWaitMutex, &xHigherPriorityTaskWoken);
        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
    }
}

/**
 * \brief initialize variables
 *
 * \param none
 * \return none
*/
void ALARM_CLOCK_Init(void)
{
    TIME_METER_Init();
}

/**
 * \brief Task to call time-dependent functions
 *
 * \param none
 * \return none
*/
void ALARM_CLOCK_Task(void *pvParameters)
{
    bool b_stored_today = FALSE;

    xAlarmClockTaskWaitMutex   = xSemaphoreCreateMutex();
    DQueueAddToRegistry(xAlarmClockTaskWaitMutex, "xAlarmClockTaskWaitMutex");

    assert_param(xAlarmClockTaskWaitMutex != NULL);

    ALARM_CLOCK_Init();

    while (1)
    {
        if (pdTRUE == xSemaphoreTake(xAlarmClockTaskWaitMutex, portMAX_DELAY))
        {
            WATCHDOG_Heartbeat(WATCHDOG_TASK_ID_ALARM_CLOCK);  // send heartbeat

            TIME_METER_Tick();

            if (RTC_TimeTrusted() == TRUE)  // ensure RTC time is current time (gps time valid)
            {
                RTCTM local_time;
                RTC_GetLocalTime(&local_time);

                if ((local_time.tm_hour == 0) && (local_time.tm_min == 0) && (b_stored_today == FALSE))
                {
                    b_stored_today = TRUE;  // set flag to ensure writing log entries only once a day
                    STATISTICS_Store();
                }

                if ((local_time.tm_hour == 0) && (local_time.tm_min == 1) && (b_stored_today == TRUE))
                {
                    b_stored_today = FALSE;  // reset flag
                }
            }
        }
    }
}
