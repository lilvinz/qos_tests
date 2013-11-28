/*
 * random.c
 *
 *  Created on: 28.02.2011
 *      Author: cba
 */

#include "stdheader.h"
#include "random.h"

#include "system.h"
#include "clock.h"

#define IA ((signed long)16807)
#define IM ((signed long)2147483647)
#define IQ ((signed long)127773)
#define IR ((signed long)2836)
#define MASK ((signed long)123459876)

static int32_t idum = 1;

/**
 * \brief Initialize random number generator
 *
 * \param none
 * \return none
*/
void random_init(void)
{
    idum = (uint32_t)(CLOCK_GetMillitimestamp() + SERIAL_NUMBER);
}

uint32_t my_rand()
{
    int32_t k;
    idum^=MASK;
    k=idum/IQ;
    idum=IA*(idum-k*IQ)-IR*k;
    if (idum<0) idum+=IM;
    k=idum;
    idum^=MASK;
    return (uint32_t)(k);
}

/**
 * \brief Get random value within certain range
 *
 * \param lower and upper random value boundary
 * \return random value
*/
int32_t random_get(int32_t min, int32_t max)
{
    return (min + ((int32_t)my_rand() % (max - min)));
}

/**
 * \brief Add a jitter to a given value
 *
 * \param given base value and fraction used as jitter range
 * \return distorted value
*/
int32_t random_jitter(int32_t base, uint16_t jitter)
{
    if (jitter == 0)
        return base;
    if (base / jitter == 0)
        return base;

    return (int32_t)(base - (base / ((int32_t)jitter * 2)) + ((int32_t)my_rand() % (base / (int32_t)jitter)));
}

