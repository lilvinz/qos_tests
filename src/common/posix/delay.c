
#include "delay.h"
#include <assert.h>
#include <time.h>

static const uint32_t ticks_per_us = 1000;

/**
* Initialises the timer used by delay functions.
*/
void Delay_Init(void)
{
    // stub
}

/**
 * Get the raw delay timer, useful for timing
 */
uint32_t Delay_GetRaw(void)
{
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return (uint32_t)(tp.tv_sec * 1000 * 1000 + tp.tv_nsec);
}

/**
 * Waits for a specific number of uS
 */
void Delay_WaituS(uint32_t uS)
{
    uint32_t elapsed = 0;

    uint32_t last_count = Delay_GetRaw();

    for (;;)
    {
        uint32_t current_count = Delay_GetRaw();
        uint32_t elapsed_uS;

        /* Measure the time elapsed since the last time we checked.
         * This is overflow safe.
         */
        elapsed += current_count - last_count;
        last_count = current_count;

        /* convert to microseconds */
        elapsed_uS = elapsed / ticks_per_us;

        if (elapsed_uS >= uS)
            break;

        /* reduce the delay by the elapsed time */
        uS -= elapsed_uS;

        /* keep fractional microseconds for the next iteration */
        elapsed %= ticks_per_us;
    }
}

/**
 * Waits for a specific number of mS
 */
void Delay_WaitmS(uint32_t mS)
{
    while (mS--)
    {
        Delay_WaituS(1000);
    }
}

uint32_t Delay_DiffuS(uint32_t raw)
{
    return (Delay_GetRaw() - raw) / ticks_per_us;
}
