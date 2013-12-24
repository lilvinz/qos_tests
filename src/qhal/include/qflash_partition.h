/**
 * @file    qflash_partition.h
 * @brief   FLASH partition driver header.
 *
 * @addtogroup FLASH_PARTITION
 * @{
 */

#ifndef _FLASH_PARTITION_H_
#define _FLASH_PARTITION_H_

#if HAL_USE_FLASH_PARTITION || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    FLASH_PARTITION configuration options
 * @{
 */
/**
 * @brief   Enables the @p fpartAcquireBus() and @p fpartReleaseBus() APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(FLASH_PARTITION_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define FLASH_PARTITION_USE_MUTUAL_EXCLUSION     TRUE
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if FLASH_PARTITION_USE_MUTUAL_EXCLUSION && !CH_USE_MUTEXES && !CH_USE_SEMAPHORES
#error "FLASH_PARTITION_USE_MUTUAL_EXCLUSION requires CH_USE_MUTEXES and/or CH_USE_SEMAPHORES"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Flash JEDEC over SPI driver configuration structure.
 */
typedef struct
{
    /**
    * @brief Flash driver associated to this partition.
    */
    BaseFlashDevice*    flashp;
    /**
     * @brief Offset in sectors.
     */
    uint32_t            sector_offset;
    /**
     * @brief Total number of sectors.
     */
    uint32_t            sector_num;
} FlashPartitionConfig;

/**
 * @brief   @p FlashPartitionDriver specific methods.
 */
#define _flash_partition_driver_methods                                        \
    _base_flash_device_methods

/**
 * @extends BaseFlashDeviceVMT
 *
 * @brief   @p FlashPartitionDriver virtual methods table.
 */
struct FlashPartitionDriverVMT
{
    _flash_partition_driver_methods
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
    const struct FlashPartitionDriverVMT* vmt;
    _base_flash_device_data
    /**
    * @brief Current configuration data.
    */
    const FlashPartitionConfig*           config;
    /**
    * @brief Device info of underlying flash device.
    */
    FlashDeviceInfo                       llfdi;
#if FLASH_PARTITION_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
#if CH_USE_MUTEXES || defined(__DOXYGEN__)
    /**
     * @brief Mutex protecting the device.
     */
    Mutex                                 mutex;
#elif CH_USE_SEMAPHORES
    Semaphore                             semaphore;
#endif
#endif /* FLASH_PARTITION_USE_MUTUAL_EXCLUSION */
} FlashPartitionDriver;

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
    void fpartInit(void);
    void fpartObjectInit(FlashPartitionDriver* fpartp);
    void fpartStart(FlashPartitionDriver* fpartp, const FlashPartitionConfig* config);
    void fpartStop(FlashPartitionDriver* fpartp);
    bool_t fpartRead(FlashPartitionDriver* fpartp, uint32_t startaddr, uint32_t n, uint8_t *buffer);
    bool_t fpartWrite(FlashPartitionDriver* fpartp, uint32_t startaddr, uint32_t n, const uint8_t *buffer);
    bool_t fpartErase(FlashPartitionDriver* fpartp, uint32_t startaddr, uint32_t n);
    bool_t fpartSync(FlashPartitionDriver* fpartp);
    bool_t fpartGetInfo(FlashPartitionDriver* fpartp, FlashDeviceInfo* fdip);
#if FLASH_PARTITION_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
    void fpartAcquireBus(FlashPartitionDriver* fjsp);
    void fpartReleaseBus(FlashPartitionDriver* fjsp);
#endif /* FLASH_PARTITION_USE_MUTUAL_EXCLUSION */
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_FLASH_PARTITION */

#endif /* _FLASH_PARTITION_H_ */

/** @} */
