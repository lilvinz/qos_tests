/**
 * @file    qflash_partition.c
 * @brief   FLASH partition driver code.
 *
 * @addtogroup FLASH_PARTITION
 * @{
 */

#include "ch.h"
#include "qhal.h"

#include "static_assert.h"

#include <string.h>

#if HAL_USE_FLASH_PARTITION || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   Virtual methods table.
 */
static const struct FlashPartitionDriverVMT flash_partition_vmt =
{
    .read = (bool_t (*)(void*, uint32_t, uint32_t, uint8_t*))fpartRead,
    .write = (bool_t (*)(void*, uint32_t, uint32_t, const uint8_t*))fpartWrite,
    .erase = (bool_t (*)(void*, uint32_t, uint32_t))fpartErase,
    .sync = (bool_t (*)(void*))fpartSync,
#if FLASH_PARTITION_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
    .acquire = (bool_t (*)(void*))fpartAcquireBus,
    .release = (bool_t (*)(void*))fpartReleaseBus,
#endif
    .get_info = (bool_t (*)(void*, FlashDeviceInfo *))fpartGetInfo,
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   FLASH partition driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void fpartInit(void)
{
}

/**
 * @brief   Initializes an instance.
 *
 * @param[out] fpartp   pointer to the @p FlashPartitionDriver object
 *
 * @init
 */
void fpartObjectInit(FlashPartitionDriver* fpartp)
{
    fpartp->vmt = &flash_partition_vmt;
    fpartp->state = FLASH_STOP;
    fpartp->config = NULL;
#if FLASH_PARTITION_USE_MUTUAL_EXCLUSION
#if CH_USE_MUTEXES
    chMtxInit(&fpartp->mutex);
#else
    chSemInit(&fpartp->semaphore, 1);
#endif
#endif /* FLASH_PARTITION_USE_MUTUAL_EXCLUSION */
}

/**
 * @brief   Configures and activates the FLASH partition.
 *
 * @param[in] fpartp    pointer to the @p FlashPartitionDriver object
 * @param[in] config    pointer to the @p FlashPartitionConfig object.
 *
 * @api
 */
void fpartStart(FlashPartitionDriver* fpartp, const FlashPartitionConfig* config)
{
    chDbgCheck((fpartp != NULL) && (config != NULL), "fpartStart");
    /* verify device status */
    chDbgAssert((fpartp->state == FLASH_STOP) || (fpartp->state == FLASH_READY),
            "fpartStart(), #1", "invalid state");

    fpartp->config = config;
    flashGetInfo(fpartp->config->flashp, &fpartp->llfdi);
    fpartp->state = FLASH_READY;
}

/**
 * @brief   Disables the FLASH partition.
 *
 * @param[in] fpartp    pointer to the @p FlashPartitionDriver object
 *
 * @api
 */
void fpartStop(FlashPartitionDriver* fpartp)
{
    chDbgCheck(fpartp != NULL, "fpartStop");
    /* verify device status */
    chDbgAssert((fpartp->state == FLASH_STOP) || (fpartp->state == FLASH_READY),
            "fpartStop(), #1", "invalid state");

    fpartp->state = FLASH_STOP;
}

/**
 * @brief   Reads data crossing sector boundaries if required.
 *
 * @param[in] fpartp    pointer to the @p FlashPartitionDriver object
 * @param[in] startaddr address to start reading from
 * @param[in] n         number of bytes to read
 * @param[in] buffer    pointer to data buffer
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t fpartRead(FlashPartitionDriver* fpartp, uint32_t startaddr, uint32_t n, uint8_t *buffer)
{
    chDbgCheck(fpartp != NULL, "fpartRead");
    /* verify device status */
    chDbgAssert(fpartp->state >= FLASH_READY, "fpartRead(), #1",
            "invalid state");
    /* verify range is within partition size */
    chDbgAssert((startaddr + n <= fpartp->llfdi.sector_size * fpartp->config->sector_num), "fpartRead(), #2",
            "invalid parameters");

    fpartp->state = FLASH_READING;

    bool_t result = flashRead(fpartp->config->flashp,
            fpartp->llfdi.sector_size * fpartp->config->sector_offset + startaddr,
            n,
            buffer);

    fpartp->state = FLASH_READY;

    return result;
}

/**
 * @brief   Writes data crossing sector boundaries if required.
 *
 * @param[in] fpartp    pointer to the @p FlashPartitionDriver object
 * @param[in] startaddr address to start writing to
 * @param[in] n         number of bytes to write
 * @param[in] buffer    pointer to data buffer
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t fpartWrite(FlashPartitionDriver* fpartp, uint32_t startaddr, uint32_t n, const uint8_t* buffer)
{
    chDbgCheck(fpartp != NULL, "fpartWrite");
    /* verify device status */
    chDbgAssert(fpartp->state >= FLASH_READY, "fpartWrite(), #1",
            "invalid state");
    /* verify range is within partition size */
    chDbgAssert((startaddr + n <= fpartp->llfdi.sector_size * fpartp->config->sector_num), "fpartRead(), #2",
            "invalid parameters");

    fpartp->state = FLASH_WRITING;

    return flashWrite(fpartp->config->flashp,
            fpartp->llfdi.sector_size * fpartp->config->sector_offset + startaddr,
            n,
            buffer);
}

/**
 * @brief   Erases one or more sectors.
 *
 * @param[in] fpartp    pointer to the @p FlashPartitionDriver object
 * @param[in] startaddr address within to be erased sector
 * @param[in] n         number of bytes to erase
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t fpartErase(FlashPartitionDriver* fpartp, uint32_t startaddr, uint32_t n)
{
    chDbgCheck(fpartp != NULL, "fpartErase");
    /* verify device status */
    chDbgAssert(fpartp->state >= FLASH_READY, "fpartErase(), #1",
            "invalid state");
    /* verify range is within partition size */
    chDbgAssert((startaddr + n <= fpartp->llfdi.sector_size * fpartp->config->sector_num), "fpartRead(), #2",
            "invalid parameters");

    fpartp->state = FLASH_ERASING;

    return flashErase(fpartp->config->flashp,
            fpartp->llfdi.sector_size * fpartp->config->sector_offset + startaddr,
            n);
}

/**
 * @brief   Waits for idle condition.
 *
 * @param[in] fpartp    pointer to the @p FlashPartitionDriver object
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t fpartSync(FlashPartitionDriver* fpartp)
{
    chDbgCheck(fpartp != NULL, "fpartSync");
    /* verify device status */
    chDbgAssert(fpartp->state >= FLASH_READY, "fpartSync(), #1",
            "invalid state");

    bool_t result = flashSync(fpartp->config->flashp);

    fpartp->state = FLASH_READY;

    return result;
}

/**
 * @brief   Returns media info.
 *
 * @param[in] fpartp    pointer to the @p FlashPartitionDriver object
 * @param[out] fdip     pointer to a @p FlashDeviceInfo structure
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t fpartGetInfo(FlashPartitionDriver* fpartp, FlashDeviceInfo* fdip)
{
    chDbgCheck(fpartp != NULL, "fpartGetInfo");
    /* verify device status */
    chDbgAssert(fpartp->state >= FLASH_READY, "fpartGetInfo(), #1",
            "invalid state");

    fdip->sector_num = fpartp->config->sector_num;
    fdip->sector_size = fpartp->llfdi.sector_size;
    memcpy(fdip->identification, fpartp->llfdi.identification, sizeof(fdip->identification));

    return CH_SUCCESS;
}

#if FLASH_PARTITION_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
/**
 * @brief   Gains exclusive access to the flash device.
 * @details This function tries to gain ownership to the flash device, if the
 *          device is already being used then the invoking thread is queued.
 * @pre     In order to use this function the option
 *          @p FLASH_PARTITION_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] fpartp    pointer to the @p FlashPartitionDriver object
 *
 * @api
 */
void fpartAcquireBus(FlashPartitionDriver* fpartp)
{
    chDbgCheck(fpartp != NULL, "fpartAcquireBus");

#if CH_USE_MUTEXES
    chMtxLock(&fpartp->mutex);
#elif CH_USE_SEMAPHORES
    chSemWait(&fpartp->semaphore);
#endif

    /* Lock the underlying device as well */
    flashAcquire(fpartp->config->flashp);
}

/**
 * @brief   Releases exclusive access to the flash device.
 * @pre     In order to use this function the option
 *          @p ADC_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] fpartp    pointer to the @p FlashPartitionDriver object
 *
 * @api
 */
void fpartReleaseBus(FlashPartitionDriver* fpartp)
{
    chDbgCheck(fpartp != NULL, "fpartReleaseBus");

#if CH_USE_MUTEXES
    (void)fpartp;
    chMtxUnlock();
#elif CH_USE_SEMAPHORES
    chSemSignal(&fpartp->semaphore);
#endif

    /* Release the underlying device as well */
    flashRelease(fpartp->config->flashp);
}
#endif /* FLASH_PARTITION_USE_MUTUAL_EXCLUSION */

#endif /* HAL_USE_FLASH_PARTITION */

/** @} */
