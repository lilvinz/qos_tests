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

#ifndef BOARD_CFG_H_
#define BOARD_CFG_H_

#include "static_assert.h"

#if HAL_USE_PAL || defined(__DOXYGEN__)
/**
 * @brief   PAL setup.
 * @details Digital I/O ports static configuration as defined in @p board.h.
 *          This variable is used by the HAL when initializing the PAL driver.
 */
const PALConfig pal_default_config = {
#if STM32_HAS_GPIOA
  {VAL_GPIOA_MODER, VAL_GPIOA_OTYPER, VAL_GPIOA_OSPEEDR, VAL_GPIOA_PUPDR,
   VAL_GPIOA_ODR,   VAL_GPIOA_AFRL,   VAL_GPIOA_AFRH},
#endif
#if STM32_HAS_GPIOB
  {VAL_GPIOB_MODER, VAL_GPIOB_OTYPER, VAL_GPIOB_OSPEEDR, VAL_GPIOB_PUPDR,
   VAL_GPIOB_ODR,   VAL_GPIOB_AFRL,   VAL_GPIOB_AFRH},
#endif
#if STM32_HAS_GPIOC
  {VAL_GPIOC_MODER, VAL_GPIOC_OTYPER, VAL_GPIOC_OSPEEDR, VAL_GPIOC_PUPDR,
   VAL_GPIOC_ODR,   VAL_GPIOC_AFRL,   VAL_GPIOC_AFRH},
#endif
#if STM32_HAS_GPIOD
  {VAL_GPIOD_MODER, VAL_GPIOD_OTYPER, VAL_GPIOD_OSPEEDR, VAL_GPIOD_PUPDR,
   VAL_GPIOD_ODR,   VAL_GPIOD_AFRL,   VAL_GPIOD_AFRH},
#endif
#if STM32_HAS_GPIOE
  {VAL_GPIOE_MODER, VAL_GPIOE_OTYPER, VAL_GPIOE_OSPEEDR, VAL_GPIOE_PUPDR,
   VAL_GPIOE_ODR,   VAL_GPIOE_AFRL,   VAL_GPIOE_AFRH},
#endif
#if STM32_HAS_GPIOF
  {VAL_GPIOF_MODER, VAL_GPIOF_OTYPER, VAL_GPIOF_OSPEEDR, VAL_GPIOF_PUPDR,
   VAL_GPIOF_ODR,   VAL_GPIOF_AFRL,   VAL_GPIOF_AFRH},
#endif
#if STM32_HAS_GPIOG
  {VAL_GPIOG_MODER, VAL_GPIOG_OTYPER, VAL_GPIOG_OSPEEDR, VAL_GPIOG_PUPDR,
   VAL_GPIOG_ODR,   VAL_GPIOG_AFRL,   VAL_GPIOG_AFRH},
#endif
#if STM32_HAS_GPIOH
  {VAL_GPIOH_MODER, VAL_GPIOH_OTYPER, VAL_GPIOH_OSPEEDR, VAL_GPIOH_PUPDR,
   VAL_GPIOH_ODR,   VAL_GPIOH_AFRL,   VAL_GPIOH_AFRH},
#endif
#if STM32_HAS_GPIOI
  {VAL_GPIOI_MODER, VAL_GPIOI_OTYPER, VAL_GPIOI_OSPEEDR, VAL_GPIOI_PUPDR,
   VAL_GPIOI_ODR,   VAL_GPIOI_AFRL,   VAL_GPIOI_AFRH}
#endif
};
#endif

/* Watchdog */
#if HAL_USE_WDG
static const WDGConfig WDGD1_cfg =
{
    /* WDG uses LSI clock which can be between 30 kHz and 60 kHz.
     * Using a prescaler of 256, this leads to a worst case count
     * time of 4.26 ms and thus to a worst case total timeout of 17.472 s.
     */
    .pr = STM32_IWDG_PR_256,
    .rlr = 0xfff,
};
#endif

/* Status LEDs */
#if HAL_USE_LED
LedDriver led_green;
static const LedConfig led_green_cfg =
{
    .ledport = GPIOD,
    .ledpad = GPIOD_LED4,
    .drive = LED_ACTIVE_HIGH,
};

LedDriver led_red;
static const LedConfig led_red_cfg =
{
    .ledport = GPIOD,
    .ledpad = GPIOD_LED5,
    .drive = LED_ACTIVE_HIGH,
};
#endif /* HAL_USE_LED */

#if HAL_USE_FLASH
static const FLASHConfig FLASHD_cfg =
{
};

#if HAL_USE_NVM_PARTITION
NVMPartitionDriver nvm_part_internal_flash_bl;
static const NVMPartitionConfig nvm_part_internal_flash_bl_cfg =
{
    .nvmp = (BaseNVMDevice*)&FLASHD,
    .sector_offset = (BL_ORIGIN - EF_ORIGIN) / 16 / 1024,
    .sector_num = BL_SIZE / 16 / 1024,
};

NVMPartitionDriver nvm_part_internal_flash_ee;
static const NVMPartitionConfig nvm_part_internal_flash_ee_cfg =
{
    .nvmp = (BaseNVMDevice*)&FLASHD,
    .sector_offset = (EE_ORIGIN - EF_ORIGIN) / 16 / 1024,
    .sector_num = EE_SIZE / 16 / 1024,
};

NVMPartitionDriver nvm_part_internal_flash_fw;
static const NVMPartitionConfig nvm_part_internal_flash_fw_cfg =
{
    .nvmp = (BaseNVMDevice*)&FLASHD,
    .sector_offset = (FW_ORIGIN - EF_ORIGIN) / 16 / 1024,
    .sector_num = FW_SIZE / 16 / 1024,
};

#if HAL_USE_NVM_FEE
NVMFeeDriver nvm_fee;
static const NVMFeeConfig nvm_fee_cfg =
{
    .nvmp = (BaseNVMDevice*)&nvm_part_internal_flash_ee,
};
#endif /* HAL_USE_NVM_FEE */
#endif /* HAL_USE_NVM_PARTITION */
#endif /* HAL_USE_FLASH */

#if HAL_USE_NVM_MEMORY
#if defined(BL_BIN)
extern uint8_t bl_bin[];
extern uint32_t bl_bin_size;
NVMMemoryDriver nvm_memory_bl_bin;
static NVMMemoryConfig nvm_memory_bl_bin_cfg =
{
    .memoryp = bl_bin,
    .sector_size = 1,
    /* @note    sector_num has to be filled in at runtime. */
    .sector_num = 0,
};
#endif /* defined(BL_BIN) */

#if STM32_BKPRAM_ENABLE == TRUE
NVMMemoryDriver nvm_memory_bkpsram;
static const NVMMemoryConfig nvm_memory_bkpsram_cfg =
{
    .memoryp = (void*)0x40024000,
    .sector_size = 1,
    .sector_num = 4096,
};
#endif /* STM32_BKPRAM_ENABLE == TRUE */
#endif /* HAL_USE_NVM_MEMORY */

#if HAL_USE_RTC
static const RTCWakeup RTCWakeup_cfg =
{
    /*
     * clock = 32768
     * prescaler = 16
     * interval = 5 s
     */
    .wutr = (STM32_RTCCLK / 16) * 2 - 1,
};
#endif /* HAL_USE_RTC */

#endif /* BOARD_CFG_H_ */
