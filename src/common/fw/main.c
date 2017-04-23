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

#include "module_init.h"
#include "nvm_tools.h"

#include <stdbool.h>
#include "target_cfg.h"

BSEMAPHORE_DECL(main_shutdown_sema, true);

/*
 * Application entry point.
 */
int main(void)
{
    /*
     * System initializations.
     * - HAL initialization, this also initializes the configured device drivers
     *   and performs the board-specific initializations.
     * - Kernel initialization, the main() function becomes a thread and the
     *   RTOS is active.
     */
    halInit();
    chSysInit();

    boardStart();

#if HAL_USE_LED && !defined(NDEBUG)
    ledOn(LED_STATUS);
#endif /* HAL_USE_LED && !defined(NDEBUG) */

#if defined(PARTITION_BL) && defined(PARTITION_BL_UPDATE)
    /*
     * Check and automagically update bootloader if necessary.
     * It is assumed that this process will not take longer
     * than the watchdog period.
     */
    {
        NVMDeviceInfo di;

        /* Omitting this breaks strict aliasing rule but i don't know why. */
        BaseNVMDevice* partition_bl_update = PARTITION_BL_UPDATE;

        if (nvmGetInfo(partition_bl_update, &di) == HAL_SUCCESS)
        {
            uint32_t n = di.sector_size * di.sector_num;
            if (nvmcmp(PARTITION_BL_UPDATE, PARTITION_BL, n) != 0)
            {
                ledOn(LED_WARNING);
                nvmMassWriteUnprotect(PARTITION_BL);
                nvmMassErase(PARTITION_BL);
                nvmcpy(PARTITION_BL, PARTITION_BL_UPDATE, n);
                boardStop();
                chSysDisable();
                boardReset();
            }
        }
    }
#endif /* defined(PARTITION_BL) && defined(PARTITION_BL_UPDATE) */

    MODULE_INITIALISE_ALL();

    MODULE_START_ALL();

    /* Assign ourselves the highest possible priority and wait for the
     * shutdown signal.
     */
    chThdSetPriority(HIGHPRIO);
    chBSemWait(&main_shutdown_sema);

#if HAL_USE_LED && !defined(NDEBUG)
    ledOff(LED_STATUS);
#endif /* HAL_USE_LED && !defined(NDEBUG) */

    MODULE_STOP_ALL();

    boardStop();
    chSysDisable();

    chThdExit(0);
}
