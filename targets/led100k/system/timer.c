
#include "stdheader.h"
#include "timer.h"

#include "clock.h"
#include "watchdog.h"

static TIMER_T *pTimerHead = NULL;

void timer_add(TIMER_T **head, TIMER_T *new, uint32_t sysTime)
{
    if (!*head)
    {
        *head = new;
        return;
    }

    TIMER_T *it, *prev = NULL;
    for (it = *head; it; it = it->service.next)
    {
        uint32_t remain = it->service.sysTimeLabel + it->interval - sysTime;
        if ((remain < INT32_MAX) && (remain >= new->interval))
            break;
        prev = it;
    }
    if (it == *head)
    {
        new->service.next = *head;
        *head = new;
    }
    else
    {
        prev->service.next = new;
        new->service.next = it;
    }
}

TIMER_T* timer_del(TIMER_T **head, TIMER_T *prev, TIMER_T *p)
{
    TIMER_T *t;

    if (p == *head)
    {
        // removing first element of list
        t = p->service.next;
        p->service.next = 0;
        *head = t;
        return *head;
    }
    else
    {
        prev->service.next = p->service.next;
        p->service.next = 0;
        return prev->service.next;
    }
}

TIMER_T *timer_findprev(TIMER_T **head, TIMER_T *p)
{
    TIMER_T *t = *head;

    for (; t;)
    {
        if (t->service.next == p)
            return t;
        t = t->service.next;
    }
    return NULL;
}

static bool timer_isrunning(TIMER_T *Timer)
{
    bool result = FALSE;
    TIMER_T *p; // p is bottom of list
    p = pTimerHead;

    while (NULL != p)
    {
        if (p == Timer)
        {
            result = TRUE;
            break;
        }
        p = (TIMER_T *) p->service.next;
    }
    return result;
}

int TIMER_Start(TIMER_T *Timer)
{
    uint32_t sysTime = (uint32_t)CLOCK_GetMillitimestamp();

    if (!Timer)
        return -1;
    if (TRUE == timer_isrunning(Timer))
        return 0;

    portENTER_CRITICAL();

    Timer->service.next = NULL;
    Timer->service.sysTimeLabel = sysTime;

    timer_add(&pTimerHead, Timer, sysTime);

    portEXIT_CRITICAL();

    return 0;
}

int TIMER_Stop(TIMER_T *Timer)
{
    TIMER_T *prev = 0;
    TIMER_T **t = &Timer;

    if (!Timer)
        return -1;

    portENTER_CRITICAL();

    if (pTimerHead != *t)
    {
        if (!(prev = timer_findprev(&pTimerHead, Timer)))
        {
            portEXIT_CRITICAL();
            return -1; // This timer is not in the list
        }
    }
    timer_del(&pTimerHead, prev, Timer);

    portEXIT_CRITICAL();

    return 0;
}

int TIMER_Restart(TIMER_T *Timer)
{
    TIMER_Stop(Timer);
    return TIMER_Start(Timer);
}

void TIMER_Task(void *pvParameters)
{
    uint32_t sysTime;

    while (1)
    {
        WATCHDOG_Heartbeat(WATCHDOG_TASK_ID_TIMER);

        sysTime = (uint32_t)CLOCK_GetMillitimestamp();
        // search for expired timers and call their callbacks
        if (pTimerHead != NULL)
        {
            while (pTimerHead && (sysTime - pTimerHead->service.sysTimeLabel) >= pTimerHead->interval)
            {
                portENTER_CRITICAL();
                TIMER_T *p = pTimerHead;
                timer_del(&pTimerHead, NULL, p);
                if (TIMER_REPEAT_MODE == p->mode)
                {
                    p->service.sysTimeLabel = sysTime;
                    timer_add(&pTimerHead, p, sysTime);
                }
                portEXIT_CRITICAL();
                p->callback(p->callback_parameter);
            }
        }

        vTaskDelay(1);
    }

}
