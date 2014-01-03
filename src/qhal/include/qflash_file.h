/**
 * @file    qflash_file.h
 * @brief   FLASH emulation through binary file.
 *
 * @addtogroup FLASH_FILE
 * @{
 */

#ifndef _FLASH_FILE_H_
#define _FLASH_FILE_H_

#if HAL_USE_FLASH_FILE || defined(__DOXYGEN__)

#include <stdio.h>

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    FLASH_FILE configuration options
 * @{
 */
/**
 * @brief   Enables the @p ffileAcquireBus() and @p ffileReleaseBus() APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(FLASH_FILE_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define FLASH_FILE_USE_MUTUAL_EXCLUSION     TRUE
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if FLASH_FILE_USE_MUTUAL_EXCLUSION && !CH_USE_MUTEXES && !CH_USE_SEMAPHORES
#error "FLASH_FILE_USE_MUTUAL_EXCLUSION requires CH_USE_MUTEXES and/or CH_USE_SEMAPHORES"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Flash emulation driver configuration structure.
 */
typedef struct
{
    /**
    * @brief Name of the underlying binary file.
    */
    const char*   file_name;
    /**
     * @brief Smallest erasable sector size in bytes.
     */
    uint32_t      sector_size;
    /**
     * @brief Total number of sectors.
     */
    uint32_t      sector_num;
} FlashFileConfig;

/**
 * @brief   @p FlashFileDriver specific methods.
 */
#define _flash_file_driver_methods                                            \
    _base_flash_device_methods

/**
 * @extends BaseFLashDeviceVMT
 *
 * @brief   @p FlashFileDriver virtual methods table.
 */
struct FlashFileDriverVMT
{
    _flash_file_driver_methods
};

/**
 * @extends BaseFlashDevice
 *
 * @brief   Structure representing a FLASH file driver.
 */
typedef struct
{
    /**
    * @brief Virtual Methods Table.
    */
    const struct FlashFileDriverVMT*     vmt;
    _base_flash_device_data
    /**
    * @brief Current configuration data.
    */
    const FlashFileConfig*               config;
    /**
     * @brief Current configuration data.
    */
    FILE*                                file;
#if FLASH_FILE_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
#if CH_USE_MUTEXES || defined(__DOXYGEN__)
    /**
     * @brief Mutex protecting the device.
     */
    Mutex                                mutex;
#elif CH_USE_SEMAPHORES
    Semaphore                            semaphore;
#endif
#endif /* FLASH_FILE_USE_MUTUAL_EXCLUSION */
} FlashFileDriver;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
    void ffileInit(void);
    void ffileObjectInit(FlashFileDriver* ffilep);
    void ffileStart(FlashFileDriver* ffilep, const FlashFileConfig* config);
    void ffileStop(FlashFileDriver* ffilep);
    bool_t ffileRead(FlashFileDriver* ffilep, uint32_t startaddr, uint32_t n, uint8_t* buffer);
    bool_t ffileWrite(FlashFileDriver* ffilep, uint32_t startaddr, uint32_t n, const uint8_t* buffer);
    bool_t ffileErase(FlashFileDriver* ffilep, uint32_t startaddr, uint32_t n);
    bool_t ffileMassErase(FlashFileDriver* ffilep);
    bool_t ffileSync(FlashFileDriver* ffilep);
    bool_t ffileGetInfo(FlashFileDriver* ffilep, FlashDeviceInfo* fdip);
#if FLASH_FILE_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
    void ffileAcquireBus(FlashFileDriver* ffilep);
    void ffileReleaseBus(FlashFileDriver* ffilep);
#endif /* FLASH_FILE_USE_MUTUAL_EXCLUSION */
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_FLASH_FILE */

#endif /* _FLASH_FILE_H_ */

/** @} */
