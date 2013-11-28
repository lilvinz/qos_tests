/*
 * state.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 30.04.2010
 *      Author:
 */

#ifndef STATE_H_
#define STATE_H_

#include "clock.h"

#define STATE_STACK_SIZE    4

typedef struct state_s
{
    uint8_t         current;
    uint8_t         last;
    CLOCK_MS_T      timestamp;
    uint8_t         stack[STATE_STACK_SIZE];
    uint8_t         stack_index;
} STATE_T;

void state_init(STATE_T *pState);
bool state_is_new(STATE_T *pState);
void state_set(STATE_T *pState, uint8_t NewState);
bool state_timeout_and_reset(STATE_T *pState, uint32_t timer_period);
bool state_timeout(STATE_T *pState, uint32_t timer_period);
void state_push(STATE_T *pState, uint8_t state);
uint8_t state_pop(STATE_T *pState);
void state_reset_timer(STATE_T *pState);


#endif /* STATE_H_ */
