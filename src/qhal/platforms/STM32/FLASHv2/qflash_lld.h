/**
 * @file    STM32/FLASHv2/qflash.c
 * @brief   STM32 low level FLASH driver header.
 *
 * @addtogroup FLASH
 * @{
 */

#ifndef _FLASH_LLD_H_
#define _FLASH_LLD_H_

#if HAL_USE_FLASH || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   XXX driver enable switch.
 * @details If set to @p TRUE the support for XXX1 is included.
 */
#if !defined(PLATFORM_XXX_USE_XXX1) || defined(__DOXYGEN__)
#define PLATFORM_XXX_USE_XXX1             FALSE
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Flash internal driver configuration structure.
 */
typedef struct
{
    /**
     * @brief Smallest erasable sector size in bytes.
     */
    uint32_t      sector_size;
} FLASHConfig;

/**
 * @brief   @p FLASHDriver specific methods.
 */
#define _flash_driver_methods                                                 \
    _base_nvm_device_methods

/**
 * @extends BaseFlashDeviceVMT
 *
 * @brief   @p FLASHDriver virtual methods table.
 */
struct FLASHDriverVMT
{
    _flash_driver_methods
};

/**
 * @extends BaseFlashDevice
 *
 * @brief   Structure representing a FLASH JEDEC over SPI driver.
 */
typedef struct
{
    /**
    * @brief Virtual Methods Table.
    */
    const struct FLASHDriverVMT* vmt;
    _base_nvm_device_data
    /**
    * @brief Current configuration data.
    */
    const FLASHConfig*                  config;
#if FLASH_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
#if CH_USE_MUTEXES || defined(__DOXYGEN__)
    /**
     * @brief Mutex protecting the device.
     */
    Mutex                               mutex;
#elif CH_USE_SEMAPHORES
    Semaphore                           semaphore;
#endif
#endif /* FLASH_USE_MUTUAL_EXCLUSION */
} FLASHDriver;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if PLATFORM_XXX_USE_XXX1 && !defined(__DOXYGEN__)
extern FLASHDriver XXXD1;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void flash_lld_init(void);
  void flash_lld_start(FLASHDriver* flashp);
  void flash_lld_stop(FLASHDriver* flashp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_FLASH */

#endif /* _FLASH_LLD_H_ */

/** @} */
