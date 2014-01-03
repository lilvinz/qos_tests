/**
 * @file    qflash_mirror.h
 * @brief   FLASH mirror driver header.
 *
 * @addtogroup FLASH_MIRROR
 * @{
 */

#ifndef _FLASH_MIRROR_H_
#define _FLASH_MIRROR_H_

#if HAL_USE_FLASH_MIRROR || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    FLASH_MIRROR configuration options
 * @{
 */
/**
 * @brief   Enables the @p fmirrorAcquireBus() and @p fmirrorReleaseBus() APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(FLASH_MIRROR_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define FLASH_MIRROR_USE_MUTUAL_EXCLUSION     TRUE
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if FLASH_MIRROR_USE_MUTUAL_EXCLUSION && !CH_USE_MUTEXES && !CH_USE_SEMAPHORES
#error "FLASH_MIRROR_USE_MUTUAL_EXCLUSION requires CH_USE_MUTEXES and/or CH_USE_SEMAPHORES"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Flash mirror driver configuration structure.
 */
typedef struct
{
    /**
    * @brief Flash driver associated to this mirror.
    */
    BaseFlashDevice*    flashp;
    /*
     * @brief number of sectors to assign to metadata header
     */
    uint32_t            sector_header_num;
} FlashMirrorConfig;

/**
 * @brief   @p FlashMirrorDriver specific methods.
 */
#define _flash_mirror_driver_methods                                          \
    _base_flash_device_methods

/**
 * @extends BaseFlashDeviceVMT
 *
 * @brief   @p FlashMirrorDriver virtual methods table.
 */
struct FlashMirrorDriverVMT
{
    _flash_mirror_driver_methods
};

/**
 * @brief   Enum representing the internal state of the mirror
 */
typedef enum
{
    STATE_INVALID = 0,
    STATE_DIRTY_A,
    STATE_DIRTY_B,
    STATE_SYNCED,
    STATE_DIRTY_COUNT,
} FlashMirrorState;

/**
 * @extends BaseFlashDevice
 *
 * @brief   Structure representing a FLASH mirror driver.
 */
typedef struct
{
    /**
    * @brief Virtual Methods Table.
    */
    const struct FlashMirrorDriverVMT* vmt;
    _base_flash_device_data
    /**
    * @brief Current configuration data.
    */
    const FlashMirrorConfig*              config;
    /**
    * @brief Device info of underlying flash device.
    */
    FlashDeviceInfo                       llfdi;
#if FLASH_MIRROR_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
#if CH_USE_MUTEXES || defined(__DOXYGEN__)
    /**
     * @brief Mutex protecting the device.
     */
    Mutex                                 mutex;
#elif CH_USE_SEMAPHORES
    Semaphore                             semaphore;
#endif
#endif /* FLASH_MIRROR_USE_MUTUAL_EXCLUSION */
    FlashMirrorState                      mirror_state;
    uint32_t                              mirror_state_addr;
} FlashMirrorDriver;

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
    void fmirrorInit(void);
    void fmirrorObjectInit(FlashMirrorDriver* fmirrorp);
    void fmirrorStart(FlashMirrorDriver* fmirrorp, const FlashMirrorConfig* config);
    void fmirrorStop(FlashMirrorDriver* fmirrorp);
    bool_t fmirrorRead(FlashMirrorDriver* fmirrorp, uint32_t startaddr, uint32_t n, uint8_t* buffer);
    bool_t fmirrorWrite(FlashMirrorDriver* fmirrorp, uint32_t startaddr, uint32_t n, const uint8_t* buffer);
    bool_t fmirrorErase(FlashMirrorDriver* fmirrorp, uint32_t startaddr, uint32_t n);
    bool_t fmirrorSync(FlashMirrorDriver* fmirrorp);
    bool_t fmirrorGetInfo(FlashMirrorDriver* fmirrorp, FlashDeviceInfo* fdip);
#if FLASH_MIRROR_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
    void fmirrorAcquireBus(FlashMirrorDriver* fmirrorp);
    void fmirrorReleaseBus(FlashMirrorDriver* fmirrorp);
#endif /* FLASH_MIRROR_USE_MUTUAL_EXCLUSION */
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_FLASH_MIRROR */

#endif /* _FLASH_MIRROR_H_ */

/** @} */
