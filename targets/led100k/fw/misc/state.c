/*
 * state.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 30.04.2010
 *      Author:
 */

#include "stdheader.h"
#include "state.h"

#include "clock.h"

void state_init(STATE_T *pState)
{
    pState->current = 0;
    pState->last    = 255;
    for (uint8_t i = 0; i < STATE_STACK_SIZE; i++)
        pState->stack[i] = 0;
    pState->stack_index = 0;
    pState->timestamp   = 0;
}

bool state_is_new(STATE_T *pState)
{
    if (pState->current == pState->last)
        return FALSE;
    pState->last = pState->current;
    pState->timestamp = CLOCK_GetMillitimestamp();
    return TRUE;
}

bool state_timeout_and_reset(STATE_T *pState, uint32_t timer_period)
{
    if (state_timeout(pState, timer_period))
    {
        pState->timestamp = xTaskGetTickCount();
        return TRUE;
    }
    else
        return FALSE;
}

void state_set(STATE_T *pState, uint8_t NewState)
{
    pState->timestamp = xTaskGetTickCount();
    pState->current = NewState;
    pState->last = 0xFF;
}

void state_reset_timer(STATE_T *pState)
{
    pState->timestamp = xTaskGetTickCount();
}

bool state_timeout(STATE_T *pState, uint32_t timer_period)
{
    if (xTaskGetTickCount() >= pState->timestamp + timer_period)
        return TRUE;
    else
        return FALSE;
}

void state_push(STATE_T *pState, uint8_t state)
{
    if (pState->stack_index == (STATE_STACK_SIZE - 1))
        return;
    pState->stack[pState->stack_index++] = state;
}

uint8_t state_pop(STATE_T *pState)
{
    if (!pState->stack_index)
        return 0;
    return pState->stack[--pState->stack_index];
}
