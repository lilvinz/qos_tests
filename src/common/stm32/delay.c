
#include "delay.h"
#include <assert.h>
#if defined(STM32F2XX)
#include <stm32f2xx_spi.h>
#elif defined(STM32F4XX)
#include <stm32f4xx_spi.h>
#else
#error "Unsupported chip"
#endif

static const uint32_t MAX_TICK_COUNT = 0xffffffff;
static uint32_t ticks_per_us;
static uint32_t modulo_us;

/**
* Initialises the timer used by delay functions.
*/
void Delay_Init(void)
{
    RCC_ClocksTypeDef clocks;

    /* compute the number of system clocks per microsecond */
    RCC_GetClocksFreq(&clocks);
    ticks_per_us = clocks.SYSCLK_Frequency / 1000000;
    assert(ticks_per_us > 1);

    // Split this into two steps to avoid 64bit maths
    modulo_us = MAX_TICK_COUNT / ticks_per_us;
    modulo_us += ((MAX_TICK_COUNT % ticks_per_us) + 1) / ticks_per_us;

    // ensure that the us_module is smaller than half of uint32_t max to make modulo operation possible
    assert(modulo_us < MAX_TICK_COUNT / 2);

    // enable the CPU cycle counter
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
 * Get the raw delay timer, useful for timing
 */
uint32_t Delay_GetRaw(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    return DWT->CYCCNT;
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
