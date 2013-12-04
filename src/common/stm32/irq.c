/*
 * interrupts.c
 *
 *  Created on: 04.12.2013
 *      Author: vke
 */

#include "irq.h"
#include <stdint.h>

/* The nesting counter ensures, that interrupts won't be enabled so long nested functions disable them */
static uint32_t nested_ctr;

/* Stored priority level before IRQ has been disabled (important for co-existence with vPortEnterCritical) */
static uint32_t prev_primask;

/**
* Disables all interrupts (nested)
* \return < 0 On errors
*/
bool IRQ_Disable(void)
{
    /* Get current priority if nested level == 0 */
    if (!nested_ctr)
    {
        __asm volatile("   mrs %0, primask\n":"=r"(prev_primask));
    }

    /* Disable interrupts */
    __asm volatile("     mov r0, #1     \n" "     msr primask, r0\n":::"r0");

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
        __asm volatile("   msr primask, %0\n"::"r"(prev_primask));
    }

    return true;
}
