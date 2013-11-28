#ifndef TIME_METER_H
#define TIME_METER_H

#include "dim.h"

#define TIME_METER_MAX_TIME UINT32_T_MAX

typedef enum
{
    TIME_METER_COUNTER_LED_NONE = 0,
    TIME_METER_COUNTER_LED_WHITE,  // white
    TIME_METER_COUNTER_LED_RED_1,  // top row
    TIME_METER_COUNTER_LED_RED_2,  // second row
    TIME_METER_COUNTER_LED_RED_3,  // third row
    TIME_METER_COUNTER_LED_RED_4,  // bottom row
    TIME_METER_COUNTER_LED_RED_C,  // middle
    TIME_METER_COUNTER_LED_WHITE_COMP,  // temperature compensated white
    TIME_METER_COUNTER_LED_RED_1_COMP,  // temperature compensated top row
    TIME_METER_COUNTER_LED_RED_2_COMP,  // temperature compensated second row
    TIME_METER_COUNTER_LED_RED_3_COMP,  // temperature compensated third row
    TIME_METER_COUNTER_LED_RED_4_COMP,  // temperature compensated bottom row
    TIME_METER_COUNTER_LED_RED_C_COMP,  // temperature compensated middle
    TIME_METER_COUNTER_SYSTEM,
    TIME_METER_COUNTER_NETWORK,
    TIME_METER_COUNTER_COUNT
} TIME_METER_COUNTER_T;

void TIME_METER_Init(void);
void TIME_METER_Tick(void);
uint32_t TIME_METER_GetSystemOperatingTime(void);
uint32_t TIME_METER_GetNetworkOperatingTime(void);
uint32_t TIME_METER_GetLedOverallOperatingTime(DIM_LED_T led);
uint32_t TIME_METER_GetLedCompensatedOverallOperatingTime(DIM_LED_T led);
bool TIME_METER_SetOperatingTime(TIME_METER_COUNTER_T timer, uint32_t value);
#endif
