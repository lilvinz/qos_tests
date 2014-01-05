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
     * @brief Pointer to flash peripheral base.
     */
    FLASH_TypeDef* flashp;
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
 * @brief   Structure representing a FLASH driver.
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
    const FLASHConfig* config;
    /**
     * @brief Pointer to the FLASH registers block.
     */
    FLASH_TypeDef* flash;
#if FLASH_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
#if CH_USE_MUTEXES || defined(__DOXYGEN__)
    /**
     * @brief Mutex protecting the device.
     */
    Mutex mutex;
#elif CH_USE_SEMAPHORES
    Semaphore semaphore;
#endif
#endif /* FLASH_USE_MUTUAL_EXCLUSION */
} FLASHDriver;

typedef struct
{
    uint32_t sector;
    uint32_t origin;
    uint32_t size;
} FLASHSectorInfo;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern FLASHDriver FLASHD;

#ifdef __cplusplus
extern "C" {
#endif
    void flash_lld_init(void);
    void flash_lld_start(FLASHDriver* flashp);
    void flash_lld_stop(FLASHDriver* flashp);
    void flash_lld_read(FLASHDriver* flashp, uint32_t startaddr, uint32_t n, uint8_t* buffer);
    void flash_lld_write(FLASHDriver* flashp, uint32_t startaddr, uint32_t n, const uint8_t* buffer);
    void flash_lld_erase(FLASHDriver* flashp, uint32_t startaddr);
    void flash_lld_masserase(FLASHDriver* flashp);
    void flash_lld_sync(FLASHDriver* flashp);
    void flash_lld_get_info(FLASHDriver* flashp, NVMDeviceInfo* nvmdip);
    void flash_lld_write_unprotect(FLASHDriver* flashp);
    void flash_lld_write_protect(FLASHDriver* flashp);
    bool_t flash_lld_addr_to_sector(uint32_t addr, FLASHSectorInfo* sinfo);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_FLASH */

#endif /* _FLASH_LLD_H_ */

/** @} */
