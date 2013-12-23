/**
 * @file    io_flash.h
 * @brief   I/O flash devices access.
 * @details This header defines an abstract interface useful to access generic
 *          I/O flash devices in a standardized way.
 *
 * @addtogroup IO_FLASH
 * @details This module defines an abstract interface for accessing generic
 *          flash devices.<br>
 *          Note that no code is present, just abstract interfaces-like
 *          structures, you should look at the system as to a set of
 *          abstract C++ classes (even if written in C). This system
 *          has then advantage to make the access to flash devices
 *          independent from the implementation logic.
 * @{
 */

#ifndef _IO_FLASH_H_
#define _IO_FLASH_H_

/**
 * @brief   Driver state machine possible states.
 */
typedef enum
{
    FLASH_UNINIT = 0,                 /**< Not initialized.                   */
    FLASH_STOP = 1,                   /**< Stopped.                           */
    FLASH_READY = 2,                  /**< Device ready.                      */
    FLASH_READING = 3,                /**< Read operation in progress.        */
    FLASH_WRITING = 4,                /**< Write operation in progress.       */
    FLASH_ERASING = 5,               /**< Erase operation in progress.       */
} flashstate_t;

/**
 * @brief   Flash device info.
 */
typedef struct
{
    uint32_t      sector_size;        /**< @brief Sector size in bytes.       */
    uint32_t      sector_num;         /**< @brief Total number of sectors.    */
    uint8_t       identification[3];  /**< @brief Jedec device identification.*/
} FlashDeviceInfo;

/**
 * @brief   @p BaseFlashDevice specific methods.
 */
#define _base_flash_device_methods                                          \
    /* Reads one or more bytes crossing sectors when required.*/            \
    bool_t (*read)(void *instance, uint32_t startaddr,                      \
            uint32_t n, uint8_t *buffer);                                   \
    /* Writes one or more bytes crossing sectors when required.*/           \
    bool_t (*write)(void *instance, uint32_t startaddr,                     \
            uint32_t n, const uint8_t *buffer);                             \
    /* Erase one or more sectors.*/                                         \
    bool_t (*erase)(void *instance, uint32_t startaddr,                     \
            uint32_t n);                                                    \
    /* Write / erase operations synchronization.*/                          \
    bool_t (*sync)(void *instance);                                         \
    /* Obtains info about the media.*/                                      \
    bool_t (*get_info)(void *instance, FlashDeviceInfo *bdip);

/**
 * @brief   @p BaseFlashDevice specific data.
 */
#define _base_flash_device_data                                             \
    /* Driver state.*/                                                      \
    flashstate_t            state;

/**
 * @brief   @p BaseFlashDevice virtual methods table.
 */
struct BaseFlashDeviceVMT
{
    _base_flash_device_methods
};

/**
 * @brief   Base flash device class.
 * @details This class represents a generic, block-accessible, device.
 */
typedef struct
{
    /** @brief Virtual Methods Table.*/
    const struct BaseFlashDeviceVMT *vmt;
    _base_flash_device_data
} BaseFlashDevice;

/**
 * @name    Macro Functions (BaseFlashDevice)
 * @{
 */
/**
 * @brief   Returns the driver state.
 * @note    Can be called in ISR context.
 *
 * @param[in] ip        pointer to a @p BaseFlashDevice or derived class
 *
 * @return              The driver state.
 *
 * @special
 */
#define flashGetDriverState(ip) ((ip)->state)

/**
 * @brief   Determines if the device is transferring data.
 * @note    Can be called in ISR context.
 *
 * @param[in] ip        pointer to a @p BaseFlashDevice or derived class
 *
 * @return              The driver state.
 * @retval FALSE        the device is not transferring data.
 * @retval TRUE         the device not transferring data.
 *
 * @special
 */
#define flashIsTransferring(ip) ((((ip)->state) == FLASH_READING) ||          \
                               (((ip)->state) == FLASH_WRITING) ||            \
                               (((ip)->state) == FLASH_ERASING))

/**
 * @brief   Reads one or more bytes crossing sectors when required.
 *
 * @param[in] ip        pointer to a @p BaseFlashDevice or derived class
 * @param[in] startaddr first address to read
 * @param[in] n         number of bytes to read
 * @param[out] buffer   pointer to the read buffer
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   operation succeeded.
 * @retval CH_FAILED    operation failed.
 *
 * @api
 */
#define flashRead(ip, startaddr, n, buffer)                                   \
    ((ip)->vmt->read(ip, startaddr, n, buffer))

/**
 * @brief   Writes one or more bytes crossing sectors when required.
 *
 * @param[in] ip        pointer to a @p BaseFlashDevice or derived class
 * @param[in] startaddr first address to write
 * @param[in] n         number of bytes to write
 * @param[out] buffer   pointer to the write buffer
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   operation succeeded.
 * @retval CH_FAILED    operation failed.
 *
 * @api
 */
#define flashWrite(ip, startaddr, n, buffer)                                  \
    ((ip)->vmt->write(ip, startaddr, n, buffer))

/**
 * @brief   Erases one or more sectors.
 *
 * @param[in] ip        pointer to a @p BaseFlashDevice or derived class
 * @param[in] startaddr address of sector to erase
 * @param[in] n         number of bytes to erase
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   operation succeeded.
 * @retval CH_FAILED    operation failed.
 *
 * @api
 */
#define flashErase(ip, startaddr, n)                                          \
    ((ip)->vmt->erase(ip, startaddr, n))

/**
 * @brief   Ensures write / erase synchronization.
 *
 * @param[in] ip        pointer to a @p BaseFlashDevice or derived class
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   operation succeeded.
 * @retval CH_FAILED    operation failed.
 *
 * @api
 */
#define flashSync(ip) ((ip)->vmt->sync(ip))

/**
 * @brief   Returns a media information structure.
 *
 * @param[in] ip        pointer to a @p BaseFlashDevice or derived class
 * @param[out] bdip     pointer to a @p FlashDeviceInfo structure
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   operation succeeded.
 * @retval CH_FAILED    operation failed.
 *
 * @api
 */
#define flashGetInfo(ip, bdip) ((ip)->vmt->get_info(ip, bdip))

/** @} */

#endif /* _IO_FLASH_H_ */

/** @} */
