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

#include "target_cfg.h"

#include "nelems.h"

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Module local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables.                                                   */
/*===========================================================================*/

/* MAY ONLY BE INCLUDED ONCE! */
#include "board_cfg.h"

/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Early initialization code.
 * @details This initialization must be performed just after stack setup
 *          and before any other initialization.
 */
void __early_init(void)
{
    stm32_clock_init();
}

/**
 * @brief   Prepare all driver configurations
 */
void boardInit(void)
{
    /**
     * Initialize custom drivers as boardInit() is being called at the end of halInit()
     */
    qhalInit();

    /**
     * call *ObjectInit() for all device instances which are created in here.
     */

    /* Start status LED driver */
#if HAL_USE_LED
    ledObjectInit(&led_green);
    ledObjectInit(&led_red);
#endif /* HAL_USE_LED */

    /* Internal flash */
#if HAL_USE_FLASH
#if HAL_USE_NVM_PARTITION
    nvmpartObjectInit(&nvm_part_internal_flash_bl);
    nvmpartObjectInit(&nvm_part_internal_flash_ee);
    nvmpartObjectInit(&nvm_part_internal_flash_fw);
#endif /* HAL_USE_NVM_PARTITION */
#if HAL_USE_NVM_FEE
    nvmfeeObjectInit(&nvm_fee);
#endif /* HAL_USE_NVM_FEE */
#endif /* HAL_USE_FLASH */

    /* nvm memory drivers */
#if HAL_USE_NVM_MEMORY
#if defined(BL_BIN)
    nvm_memory_bl_bin_cfg.sector_num = bl_bin_size;
    nvmmemoryObjectInit(&nvm_memory_bl_bin);
#endif /* defined(BL_BIN) */
#if STM32_BKPRAM_ENABLE
    nvmmemoryObjectInit(&nvm_memory_bkpsram);
#endif /* STM32_BKPRAM_ENABLE */
#endif /* HAL_USE_NVM_MEMORY */
}

/**
 * @brief   Start all drivers
 */
void boardStart(void)
{
    /* Start watchdog */
#if HAL_USE_WDG
    wdgStart(&WDGD1, &WDGD1_cfg);
#endif /* HAL_USE_WDG */

    /* Start status LED driver */
#if HAL_USE_LED
    ledStart(&led_green, &led_green_cfg);
    ledStart(&led_red, &led_red_cfg);
#endif /* HAL_USE_LED */

    /* Internal flash */
#if HAL_USE_FLASH
    flashStart(&FLASHD, &FLASHD_cfg);
#if HAL_USE_NVM_PARTITION
    nvmpartStart(&nvm_part_internal_flash_bl, &nvm_part_internal_flash_bl_cfg);
    nvmpartStart(&nvm_part_internal_flash_ee, &nvm_part_internal_flash_ee_cfg);
    nvmpartStart(&nvm_part_internal_flash_fw, &nvm_part_internal_flash_fw_cfg);
#endif /* HAL_USE_NVM_PARTITION */
#if HAL_USE_NVM_FEE
    nvmfeeStart(&nvm_fee, &nvm_fee_cfg);
#endif /* HAL_USE_NVM_FEE */
#endif /* HAL_USE_FLASH */

    /* nvm memory drivers */
#if HAL_USE_NVM_MEMORY
#if defined(BL_BIN)
    nvmmemoryStart(&nvm_memory_bl_bin, &nvm_memory_bl_bin_cfg);
#endif /* defined(BL_BIN) */
#if STM32_BKPRAM_ENABLE
    nvmmemoryStart(&nvm_memory_bkpsram, &nvm_memory_bkpsram_cfg);
#endif /* STM32_BKPRAM_ENABLE */
#endif /* HAL_USE_NVM_MEMORY */
}

/**
 * @brief   Stop all drivers in the reverse order of their start.
 * @note    Pin settings must match the initial state in board.h
 */
void boardStop(void)
{
    /* nvm memory drivers */
#if HAL_USE_NVM_MEMORY
#if STM32_BKPRAM_ENABLE
    nvmmemorySync(&nvm_memory_bkpsram);
    nvmmemoryStop(&nvm_memory_bkpsram);
#endif /* STM32_BKPRAM_ENABLE */
#if defined(BL_BIN)
    nvmmemorySync(&nvm_memory_bl_bin);
    nvmmemoryStop(&nvm_memory_bl_bin);
#endif /* defined(BL_BIN) */
#endif /* HAL_USE_NVM_MEMORY */

    /* Internal flash */
#if HAL_USE_FLASH
#if HAL_USE_NVM_FEE
    nvmfeeSync(&nvm_fee);
    nvmfeeStop(&nvm_fee);
#endif /* HAL_USE_NVM_FEE */
#if HAL_USE_NVM_PARTITION
    nvmpartSync(&nvm_part_internal_flash_fw);
    nvmpartStop(&nvm_part_internal_flash_fw);
    nvmpartSync(&nvm_part_internal_flash_ee);
    nvmpartStop(&nvm_part_internal_flash_ee);
    nvmpartSync(&nvm_part_internal_flash_bl);
    nvmpartStop(&nvm_part_internal_flash_bl);
#endif /* HAL_USE_NVM_PARTITION */
    flashStop(&FLASHD);
#endif /* HAL_USE_FLASH */

    /* Stop status LED driver */
#if HAL_USE_LED
    ledOff(&led_red);
    ledStop(&led_red);
    ledOff(&led_green);
    ledStop(&led_green);
#endif /* HAL_USE_LED */

    /* Stop watchdog */
#if HAL_USE_WDG
    /*
     * The watchdog cannot be stopped again once started.
     * So the only thing we can do here is reload the counter.
     */
    wdgReset(&WDGD1);
#endif /* HAL_USE_WDG */
}

void boardReset(void)
{
    /* Execute reset */
    NVIC_SystemReset();
}

