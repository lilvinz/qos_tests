/*
 * watchdog.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 29.10.2010
 *      Author:DHA
 *
 * Purpose:
 * Watchdog
 *
 */

#include "stdheader.h"
#include "watchdog.h"

#include "version.h"
#include "debug.h"


const int32_t  watchdog_value[WATCHDOG_TASK_ID_COUNT][2] =
{
//Value: Init, Reload
        {  60,  60 },    // WATCHDOG_TASK_ID_DRIVER
        {  60,  60 },    // WATCHDOG_TASK_ID_TIMER
        {  60,  60 },    // WATCHDOG_TASK_ID_SEQUENCE
//        {  60,  60 },    // WATCHDOG_TASK_ID_DETECTOR
        {  120,  120 },    // WATCHDOG_ID_CAN_RX
};

int32_t watchdog_ctr[WATCHDOG_TASK_ID_COUNT];
int32_t watchdog_heartbeat_ctr[WATCHDOG_TASK_ID_COUNT];

//#define MEMCHECK_SIZE 2800

#ifdef MEMCHECK_SIZE
uint32_t memcheck[MEMCHECK_SIZE];
#endif
uint32_t *kill = (uint32_t*)0x8001000;

/**
  * @brief  Send heartbeat to watchdog task.
  * @param  task_id: id of task
  * @retval None
  */
void WATCHDOG_Heartbeat(WATCHDOG_TASK_ID_T task_id)
{
    watchdog_ctr[task_id] = watchdog_value[task_id][WDVAL_RESET];
    watchdog_heartbeat_ctr[task_id]++;
}

/**
  * @brief  Init watchdog.
  * @param  None
  * @retval None
  */
void WATCHDOG_Init(void)
{
    // initialize counter with start values
    for (uint8_t i_ctr = 0; i_ctr < WATCHDOG_TASK_ID_COUNT; i_ctr++)
    {
        watchdog_ctr[i_ctr]             = watchdog_value[i_ctr][WDVAL_INIT];
        watchdog_heartbeat_ctr[i_ctr]   = 0;
    }
#ifdef MEMCHECK_SIZE
    // initialize memcheck
    for (uint16_t i_ctr = 0; i_ctr < MEMCHECK_SIZE; i_ctr++)
    {
        memcheck[i_ctr] = 0xDEADBEEF;
    }
#endif

    // Reload IWDG counter
    IWDG_ReloadCounter();

    // Enable write access to IWDG_PR and IWDG_RLR registers
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

#if (VERSION_IS_DEBUG > 0)
    // IWDG counter clock: 32KHz(LSI) / 128 = 250 Hz
	// -> with a reload value of 0xFFF we get a watchdog timout of 16.4sec
    IWDG_SetPrescaler(IWDG_Prescaler_128);
#else
    // IWDG counter clock: 32KHz(LSI) / 32 = 1 KHz
	// -> with a reload value of 0xFFF we get a watchdog timout of 4.1sec
    IWDG_SetPrescaler(IWDG_Prescaler_32);
#endif

    // Set counter reload value to 0xFFF
    IWDG_SetReload(0xFFF);

    // Reload IWDG counter
    IWDG_ReloadCounter();

    // Enable IWDG (the LSI oscillator will be enabled by hardware)
    IWDG_Enable();

#if (VERSION_IS_DEBUG > 0)
    /* Stop IWDG when breaking into debugger */
    DBGMCU_APB1PeriphConfig(DBGMCU_IWDG_STOP, ENABLE);
#endif
}

/**
  * @brief  Watch all tasks.
  * @param  None
  * @retval None
  */
void WATCHDOG_Task(void *pvParameters)
{
//    kill = (uint32_t*) 0x0;
    uint32_t magic_variable;
    magic_variable = WATCHDOG_MAGIC_VARIABLE_INIT_VALUE;

    WATCHDOG_Init();

    while (1)
    {
#ifdef MEMCHECK_SIZE
        // memcheck
        for (uint16_t i_ctr = 0; i_ctr < MEMCHECK_SIZE; i_ctr++)
        {
            if (memcheck[i_ctr] != 0xDEADBEEF)
            {
                *kill = 0;
            }

        }
#endif
        // decrement counter
        for (uint8_t i_ctr = 0; i_ctr < WATCHDOG_TASK_ID_COUNT; i_ctr++)
        {
            watchdog_ctr[i_ctr]--;
        }

        // check timeouts
        for (uint8_t i_ctr = 0; i_ctr < WATCHDOG_TASK_ID_COUNT; i_ctr++)
        {
            if ((watchdog_ctr[i_ctr] <= 0) || (watchdog_ctr[i_ctr] > watchdog_value[i_ctr][WDVAL_INIT]))
            {
                DPRINT1(DEBUG_SYSTEM, "Watchdog: Task Timeout for ID %u\n", i_ctr);
                magic_variable = 0;
            }
        }

        if (magic_variable == WATCHDOG_MAGIC_VARIABLE_INIT_VALUE)
        {
            // reset watchdog if variable is still ok
            IWDG_ReloadCounter();
        }

        vTaskDelay(WATCHDOG_TASK_WAIT_TIME);
    }
}
