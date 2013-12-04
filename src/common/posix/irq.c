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

/**
* Disables all interrupts (nested)
* \return < 0 On errors
*/
bool IRQ_Disable(void)
{
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

    return true;
}
