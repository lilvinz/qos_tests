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

#include "mod_test_cpp.hpp"

#if MOD_TEST_CPP

#include "ch_tools.h"
#include "watchdog.h"
#include "module_init_cpp.h"

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
ModTestCpp ModTestCpp::modInstance = ModTestCpp();


/*===========================================================================*/
/* Local functions                                                           */
/*===========================================================================*/

tprio_t ModTestCpp::GetThreadPrio() const
{
    return NORMALPRIO + 1;
}

/**
 * Module thread, should not return.
 */
void ModTestCpp::ThreadMain()
{
    chRegSetThreadName("mod_test_cpp");

    systime_t lastSysTime = chibios_rt::System::getTimeX();

    while (chibios_rt::BaseThread::shouldTerminate() == false)
    {
        watchdog_reload(WATCHDOG_MOD_TEST);

        ledToggle(LED_STATUS);

        chThdSleepPeriod(&lastSysTime, TIME_MS2I(250));
    }
}

/*===========================================================================*/
/* Exported functions                                                        */
/*===========================================================================*/

ModTestCpp::ModTestCpp()
{

}

ModTestCpp::~ModTestCpp()
{

}

/**
 * stage 1 - Initialize the module, called on startup.
 * @note: This is being called from main.c while scheduler is running
 */

void ModTestCpp::Init()
{
    watchdog_register(WATCHDOG_MOD_TEST);
}

/**
 * stage 2 - Start the module, called after all modules have been initialized
 * @note: This is being called from main.c while scheduler is running
 */
void ModTestCpp::Start()
{
    Super::Start();
}

/**
 * stage 3 - Stop the module, called after all modules have been started
 * @note: This is being called from main.c while scheduler is running
 */
 void ModTestCpp::Shutdown()
{
     Super::Shutdown();
}

MODULE_INITCALL(0, qos::ModuleInit<ModTestCpp>::Init, qos::ModuleInit<ModTestCpp>::Start, qos::ModuleInit<ModTestCpp>::Shutdown)


#endif /* MOD_TEST_CPP */
