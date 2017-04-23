/*
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "mod_test.h"

#if MOD_TEST

#include "module_init.h"
#include "ch_tools.h"
#include "watchdog.h"

#include "qhal.h"

/*===========================================================================*/
/* Local definitions                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Imported variables                                                        */
/*===========================================================================*/

/*===========================================================================*/
/* Exported variables                                                        */
/*===========================================================================*/

/*===========================================================================*/
/* Local types                                                               */
/*===========================================================================*/

/*===========================================================================*/
/* Local variables                                                           */
/*===========================================================================*/

static thread_reference_t threadp;

/*===========================================================================*/
/* Local functions                                                           */
/*===========================================================================*/

/**
 * Module thread, should not return.
 */
static void mod_test_thread(void* parameters)
{
    chRegSetThreadName("mod_test");

    systime_t lastSysTime = chVTGetSystemTimeX();

    while (chThdShouldTerminateX() == false)
    {
        watchdog_reload(WATCHDOG_MOD_TEST);

        ledToggle(LED_STATUS);

        chThdSleepPeriod(&lastSysTime, OSAL_MS2ST(250));
    }
}

/*===========================================================================*/
/* Exported functions                                                        */
/*===========================================================================*/

/**
 * stage 1 - Initialize the module, called on startup.
 * @note: This is being called from main.c while scheduler is running
 */
void mod_test_init(void)
{
    watchdog_register(WATCHDOG_MOD_TEST);
}

/**
 * stage 2 - Start the module, called after all modules have been initialized
 * @note: This is being called from main.c while scheduler is running
 */
void mod_test_start(void)
{
    static THD_WORKING_AREA(wa_mod_test, 4096);
    threadp = chThdCreateStatic(wa_mod_test, sizeof(wa_mod_test),
            NORMALPRIO + 1, mod_test_thread, NULL);
}

/**
 * stage 3 - Stop the module, called after all modules have been started
 * @note: This is being called from main.c while scheduler is running
 */
void mod_test_stop(void)
{
    chThdTerminate(threadp);
    /* Wait for thread to terminate and release it from registry. */
    chThdWait(threadp);
}

MODULE_INITCALL(0, mod_test_init, mod_test_start, mod_test_stop)

#endif /* MOD_TEST */
