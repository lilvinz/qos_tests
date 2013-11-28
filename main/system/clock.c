
#include "stdheader.h"
#include "clock.h"

static volatile CLOCK_MS_T clock_millitimestamp;
static volatile CLOCK_S_T clock_timestamp;

CLOCK_MS_T CLOCK_GetMillitimestamp(void)
{
    taskENTER_CRITICAL();
    CLOCK_MS_T result = clock_millitimestamp;
    taskEXIT_CRITICAL();
    return result;
}

CLOCK_S_T CLOCK_GetTimestamp(void)
{
    CLOCK_S_T result = clock_timestamp;
    return result;
}

CLKIV CLOCK_GetInterval(uint32_t invseconds)
{
    CLKIV clk;

    clk.iv_days = invseconds / 86400;
    invseconds %= 86400;
    clk.iv_hours = invseconds / 3600;
    invseconds %= 3600;
    clk.iv_mins = invseconds / 60;
    invseconds %= 60;
    clk.iv_secs = invseconds;

    return clk;
}

void vApplicationTickHook(void)
{
    //tick is 1/1000 s
    clock_millitimestamp++;
    if (clock_millitimestamp % 1000 == 0)
        clock_timestamp++;
}

