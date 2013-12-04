/*
 * interrupts.c
 *
 *  Created on: 04.12.2013
 *      Author: vke
 */

#include "irq.h"
#include <stdint.h>
#if defined(FREERTOS)
#include "FreeRTOS.h"
#include "task.h"
#endif

/* The nesting counter ensures, that interrupts won't be enabled so long nested functions disable them */
static uint32_t nested_ctr;

/**
* Disables all interrupts (nested)
* \return < 0 On errors
*/
bool IRQ_Disable(void)
{
#if defined(FREERTOS)
    vTaskSuspendAll();
#endif

    ++nested_ctr;

    return true;
}

/**
* Enables all interrupts (nested)
* \return < 0 on errors
* \return -1 on nesting errors (Interrupts_Disable() hasn't been called before)
*/
bool IRQ_Enable(void)
{
    /* Check for nesting error */
    if (nested_ctr == 0)
    {
        /* Nesting error */
        return false;
    }

    /* Decrease nesting level */
    --nested_ctr;

    /* Set back previous priority once nested level reached 0 again */
    if (nested_ctr == 0)
    {
#if defined(FREERTOS)
        xTaskResumeAll();
#endif
    }

    return true;
}
