
#include "semaphore.h"
#include "irq.h"
#include "delay.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

struct semaphore_s* Semaphore_Create(void)
{
    struct semaphore_s* sema = malloc(sizeof(struct semaphore_s));

    if (sema == NULL)
        return NULL;

    /*
     * The initial state of a binary semaphore is "given".
     * FreeRTOS executes a "give" upon creation.
     */
#if defined(FREERTOS)
    vSemaphoreCreateBinary(sema->sema_handle);
#else
    sema->sema_count = 1;
#endif

    return sema;
}

bool Semaphore_Take(struct semaphore_s* sema, uint32_t timeout_ms)
{
    assert(sema != NULL);

#if defined(FREERTOS)
    portTickType timeout_ticks;
    if (timeout_ms == SEMAPHORE_TIMEOUT_MAX)
        timeout_ticks = portMAX_DELAY;
    else
        timeout_ticks = timeout_ms / portTICK_RATE_MS;

    return xSemaphoreTake(sema->sema_handle, timeout_ticks) == pdTRUE;
#else
    uint32_t start = Delay_GetRaw();

    uint32_t temp_sema_count;
    do
    {
        IRQ_Disable();
        if ((temp_sema_count = sema->sema_count) != 0)
            --sema->sema_count;
        IRQ_Enable();
    } while (temp_sema_count == 0 && Delay_DiffuS(start) < timeout_ms * 1000);

    return temp_sema_count != 0;
#endif
}

bool Semaphore_Give(struct semaphore_s *sema)
{
    assert(sema != NULL);
#if defined(FREERTOS)
    return xSemaphoreGive(sema->sema_handle) == pdTRUE;
#else
    bool result = true;

    IRQ_Disable();

    if (sema->sema_count == 0)
        ++sema->sema_count;
    else
        result = false;

    IRQ_Enable();

    return result;
#endif
}

bool Semaphore_Take_FromISR(struct semaphore_s *sema, bool *woken)
{
    assert(sema != NULL);

#if defined(FREERTOS)
    assert(woken != NULL);

    signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    bool result = xSemaphoreTakeFromISR(sema->sema_handle, &xHigherPriorityTaskWoken) == pdTRUE;

    *woken = *woken || xHigherPriorityTaskWoken == pdTRUE;

    return result;
#else
    bool result = true;

    IRQ_Disable();

    if (sema->sema_count != 0)
        --sema->sema_count;
    else
        result = false;

    IRQ_Enable();

    return result;
#endif
}

bool Semaphore_Give_FromISR(struct semaphore_s *sema, bool *woken)
{
    assert(sema != NULL);

#if defined(FREERTOS)
    assert(woken != NULL);

    signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    bool result = xSemaphoreGiveFromISR(sema->sema_handle, &xHigherPriorityTaskWoken) == pdTRUE;

    *woken = *woken || xHigherPriorityTaskWoken == pdTRUE;

    return result;
#else
    bool result = true;

    IRQ_Disable();

    if (sema->sema_count == 0)
        ++sema->sema_count;
    else
        result = false;

    IRQ_Enable();

    return result;
#endif
}
