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
#if !defined(NDEBUG)
    DBGMCU->CR =
            DBGMCU_CR_DBG_STANDBY |
            DBGMCU_CR_DBG_STOP |
            DBGMCU_CR_DBG_SLEEP;
    DBGMCU->APB1FZ =
            DBGMCU_APB1_FZ_DBG_TIM2_STOP |
            DBGMCU_APB1_FZ_DBG_TIM3_STOP |
            DBGMCU_APB1_FZ_DBG_TIM4_STOP |
            DBGMCU_APB1_FZ_DBG_TIM5_STOP |
            DBGMCU_APB1_FZ_DBG_TIM6_STOP |
            DBGMCU_APB1_FZ_DBG_TIM7_STOP |
            DBGMCU_APB1_FZ_DBG_TIM12_STOP |
            DBGMCU_APB1_FZ_DBG_TIM13_STOP |
            DBGMCU_APB1_FZ_DBG_TIM14_STOP |
            DBGMCU_APB1_FZ_DBG_RTC_STOP |
            DBGMCU_APB1_FZ_DBG_WWDG_STOP |
            DBGMCU_APB1_FZ_DBG_IWDG_STOP |
            DBGMCU_APB1_FZ_DBG_I2C1_SMBUS_TIMEOUT |
            DBGMCU_APB1_FZ_DBG_I2C2_SMBUS_TIMEOUT |
            DBGMCU_APB1_FZ_DBG_I2C3_SMBUS_TIMEOUT |
            DBGMCU_APB1_FZ_DBG_CAN1_STOP |
            DBGMCU_APB1_FZ_DBG_CAN2_STOP;
    DBGMCU->APB2FZ =
            DBGMCU_APB2_FZ_DBG_TIM1_STOP |
            DBGMCU_APB2_FZ_DBG_TIM8_STOP |
            DBGMCU_APB2_FZ_DBG_TIM9_STOP |
            DBGMCU_APB2_FZ_DBG_TIM10_STOP |
            DBGMCU_APB2_FZ_DBG_TIM11_STOP;
#else
    /* @note: These registers are only reset by a power cycle.
     * To prevent issues with this, we set them to a defined value.
     */
    DBGMCU->CR = 0;
    DBGMCU->APB1FZ = 0;
    DBGMCU->APB2FZ = 0;
#endif

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
    nvmpartObjectInit(&nvm_part_internal_flash_ee);
    nvmpartObjectInit(&nvm_part_internal_flash_fw);
#if HAL_USE_NVM_FEE
    nvmfeeObjectInit(&nvm_fee);
#endif /* HAL_USE_NVM_FEE */
#endif /* HAL_USE_NVM_PARTITION */
#endif /* HAL_USE_FLASH */

    /* nvm memory drivers */
#if HAL_USE_NVM_MEMORY
#if STM32_BKPRAM_ENABLE == TRUE
    nvmmemoryObjectInit(&nvm_memory_bkpsram);
#endif /* STM32_BKPRAM_ENABLE == TRUE */
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
    nvmpartStart(&nvm_part_internal_flash_ee, &nvm_part_internal_flash_ee_cfg);
    nvmpartStart(&nvm_part_internal_flash_fw, &nvm_part_internal_flash_fw_cfg);
#if HAL_USE_NVM_FEE
    nvmfeeStart(&nvm_fee, &nvm_fee_cfg);
#endif /* HAL_USE_NVM_FEE */
#endif /* HAL_USE_NVM_PARTITION */
#endif /* HAL_USE_FLASH */

    /* nvm memory drivers */
#if HAL_USE_NVM_MEMORY
#if STM32_BKPRAM_ENABLE == TRUE
    nvmmemoryStart(&nvm_memory_bkpsram, &nvm_memory_bkpsram_cfg);
#endif /* STM32_BKPRAM_ENABLE == TRUE */
#endif /* HAL_USE_NVM_MEMORY */

    /*
     * Set low level board options
     */
#if defined(NDEBUG)
#if HAL_USE_FLASH
    /* Enable brown out detection.
     * note: the discoveryf407 board provides 3V so
     * we can only use LEVEL 2 here */
    flash_lld_ob_bor(&FLASHD, OB_BOR_LEVEL_2);

    /* Set user option byte. */
    flash_lld_ob_user(&FLASHD,
            OB_USER_no_WDG_HW |
            OB_USER_no_RST_STOP |
            OB_USER_no_RST_STDBY);

    /* Enable chip readout protection. */
    flash_lld_ob_rdp(&FLASHD, OB_RDP_LEVEL_0);
#endif /* HAL_USE_FLASH */
#endif /* defined(NDEBUG) */
}

/**
 * @brief   Stop all drivers in the reverse order of their start.
 * @note    Pin settings must match the initial state in board.h
 */
void boardStop(void)
{
    /* nvm memory drivers */
#if HAL_USE_NVM_MEMORY
#if STM32_BKPRAM_ENABLE == TRUE
    nvmmemorySync(&nvm_memory_bkpsram);
    nvmmemoryStop(&nvm_memory_bkpsram);
#endif /* STM32_BKPRAM_ENABLE == TRUE */
#endif /* HAL_USE_NVM_MEMORY */

    /* Internal flash */
#if HAL_USE_FLASH
#if HAL_USE_NVM_PARTITION
#if HAL_USE_NVM_FEE
    nvmfeeSync(&nvm_fee);
    nvmfeeStop(&nvm_fee);
#endif /* HAL_USE_NVM_FEE */
    nvmpartSync(&nvm_part_internal_flash_fw);
    nvmpartStop(&nvm_part_internal_flash_fw);
    nvmpartSync(&nvm_part_internal_flash_ee);
    nvmpartStop(&nvm_part_internal_flash_ee);
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

void boardJumpToApplication(uint32_t address)
{
    /* Clear pending Systick and SVC interrrupts. */
    SCB->ICSR = SCB_ICSR_PENDSVCLR_Msk | SCB_ICSR_PENDSTCLR_Msk;

    /* Disable and clear all NVIC interrupts. */
    for (size_t i = 0; i < NELEMS(NVIC->ICER); i++)
    {
        NVIC->ICER[i] = 0xffffffff;
        NVIC->ICPR[i] = 0xffffffff;
    }

    const volatile uint32_t* ApplicationAddress = (volatile uint32_t*)address;
    typedef void (*pFunction)(void);
    pFunction Jump_To_Application = (pFunction)ApplicationAddress[1];

    /* Set stack pointer as in application's vector table. */
    __set_MSP((uint32_t)(ApplicationAddress[0]));

    /* Clear pending Systick and SVC interrrupts. */
    SCB->ICSR = SCB_ICSR_PENDSVCLR_Msk | SCB_ICSR_PENDSTCLR_Msk;

    /* Disable and clear all NVIC interrupts. */
    for (size_t i = 0; i < NELEMS(NVIC->ICER); i++)
    {
        NVIC->ICER[i] = 0xffffffff;
        NVIC->ICPR[i] = 0xffffffff;
    }

    /* Bye bye... */
    Jump_To_Application();
}

void boardMassErase(void)
{
    /* This function assumes that FLASHD has been started once. */
#if HAL_USE_FLASH
    /* Write unprotect whole internal flash. */
    flash_lld_writeunprotect_mass(&FLASHD);

    /* Disable brown out detection. */
    flash_lld_ob_bor(&FLASHD, OB_BOR_LEVEL_OFF);

    /* Reset user option byte to default. */
    flash_lld_ob_user(&FLASHD,
            OB_USER_no_WDG_HW |
            OB_USER_no_RST_STOP |
            OB_USER_no_RST_STDBY);

    /* Start mass erase by setting rdp level 0. */
    flash_lld_ob_rdp(&FLASHD, OB_RDP_LEVEL_0);

    /* This place should never be reached if protection was enabled.
     * But if not, make sure we reach factory defaults regardless.
     */
    flash_lld_erase_mass(&FLASHD);
#endif /* HAL_USE_FLASH */

    /* This should never be reached. */
    boardReset();
}
