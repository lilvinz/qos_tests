/**
 * @file    qflash_mirror.c
 * @brief   FLASH mirror driver code.
 *
 * @addtogroup FLASH_MIRROR
 * @{
 */

#include "ch.h"
#include "qhal.h"

#include "static_assert.h"

#include <string.h>

#if HAL_USE_FLASH_MIRROR || defined(__DOXYGEN__)

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
static const struct FlashMirrorDriverVMT flash_mirror_vmt =
{
    .read = (bool_t (*)(void*, uint32_t, uint32_t, uint8_t*))fmirrorRead,
    .write = (bool_t (*)(void*, uint32_t, uint32_t, const uint8_t*))fmirrorWrite,
    .erase = (bool_t (*)(void*, uint32_t, uint32_t))fmirrorErase,
    .sync = (bool_t (*)(void*))fmirrorSync,
#if FLASH_MIRROR_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
    .acquire = (bool_t (*)(void*))fmirrorAcquireBus,
    .release = (bool_t (*)(void*))fmirrorReleaseBus,
#endif
    .get_info = (bool_t (*)(void*, FlashDeviceInfo *))fmirrorGetInfo,
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   FLASH mirror driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void fmirrorInit(void)
{
}

/**
 * @brief   Initializes an instance.
 *
 * @param[out] fmirrorp pointer to the @p FlashMirrorDriver object
 *
 * @init
 */
void fmirrorObjectInit(FlashMirrorDriver* fmirrorp)
{
    fmirrorp->vmt = &flash_mirror_vmt;
    fmirrorp->state = FLASH_STOP;
    fmirrorp->config = NULL;
#if FLASH_MIRROR_USE_MUTUAL_EXCLUSION
#if CH_USE_MUTEXES
    chMtxInit(&fmirrorp->mutex);
#else
    chSemInit(&fmirrorp->semaphore, 1);
#endif
#endif /* FLASH_JEDEC_SPI_USE_MUTUAL_EXCLUSION */
}

/**
 * @brief   Configures and activates the FLASH mirror.
 *
 * @param[in] fmirrorp  pointer to the @p FlashMirrorDriver object
 * @param[in] config    pointer to the @p FlashMirrorConfig object.
 *
 * @api
 */
void fmirrorStart(FlashMirrorDriver* fmirrorp, const FlashMirrorConfig* config)
{
    chDbgCheck((fmirrorp != NULL) && (config != NULL), "fmirrorStart");
    /* verify device status */
    chDbgAssert((fmirrorp->state == FLASH_STOP) || (fmirrorp->state == FLASH_READY),
            "fmirrorStart(), #1", "invalid state");

    fmirrorp->config = config;
    flashGetInfo(fmirrorp->config->flashp, &fmirrorp->llfdi);
    fmirrorp->state = FLASH_READY;
}

/**
 * @brief   Disables the FLASH mirror.
 *
 * @param[in] fmirrorp  pointer to the @p FlashMirrorDriver object
 *
 * @api
 */
void fmirrorStop(FlashMirrorDriver* fmirrorp)
{
    chDbgCheck(fmirrorp != NULL, "fmirrorStop");
    /* verify device status */
    chDbgAssert((fmirrorp->state == FLASH_STOP) || (fmirrorp->state == FLASH_READY),
            "fmirrorStop(), #1", "invalid state");

    fmirrorp->state = FLASH_STOP;
}

/**
 * @brief   Reads data crossing sector boundaries if required.
 *
 * @param[in] fmirrorp  pointer to the @p FlashMirrorDriver object
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
bool_t fmirrorRead(FlashMirrorDriver* fmirrorp, uint32_t startaddr, uint32_t n, uint8_t *buffer)
{
    chDbgCheck(fmirrorp != NULL, "fmirrorRead");
    /* verify device status */
    chDbgAssert(fmirrorp->state >= FLASH_READY, "fmirrorRead(), #1",
            "invalid state");
#if 0
    /* verify range is within mirror size */
    chDbgAssert((startaddr + n <= fmirrorp->llfdi.sector_size * fmirrorp->config->sector_num), "fmirrorRead(), #2",
            "invalid parameters");

    return flashRead(fmirrorp->config->flashp,
            fmirrorp->llfdi.sector_size * fmirrorp->config->sector_offset + startaddr,
            n,
            buffer);
#endif
    return CH_SUCCESS;
}

/**
 * @brief   Writes data crossing sector boundaries if required.
 *
 * @param[in] fmirrorp  pointer to the @p FlashMirrorDriver object
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
bool_t fmirrorWrite(FlashMirrorDriver* fmirrorp, uint32_t startaddr, uint32_t n, const uint8_t* buffer)
{
    chDbgCheck(fmirrorp != NULL, "fmirrorWrite");
    /* verify device status */
    chDbgAssert(fmirrorp->state >= FLASH_READY, "fmirrorWrite(), #1",
            "invalid state");
#if 0
    /* verify range is within mirror size */
    chDbgAssert((startaddr + n <= fmirrorp->llfdi.sector_size * fmirrorp->config->sector_num), "fmirrorRead(), #2",
            "invalid parameters");

    return flashWrite(fmirrorp->config->flashp,
            fmirrorp->llfdi.sector_size * fmirrorp->config->sector_offset + startaddr,
            n,
            buffer);
#endif
    return CH_SUCCESS;
}

/**
 * @brief   Erases one or more sectors.
 *
 * @param[in] fmirrorp  pointer to the @p FlashMirrorDriver object
 * @param[in] startaddr address within to be erased sector
 * @param[in] n         number of bytes to erase
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t fmirrorErase(FlashMirrorDriver* fmirrorp, uint32_t startaddr, uint32_t n)
{
    chDbgCheck(fmirrorp != NULL, "fmirrorErase");
    /* verify device status */
    chDbgAssert(fmirrorp->state >= FLASH_READY, "fmirrorErase(), #1",
            "invalid state");
#if 0
    /* verify range is within mirror size */
    chDbgAssert((startaddr + n <= fmirrorp->llfdi.sector_size * fmirrorp->config->sector_num), "fmirrorRead(), #2",
            "invalid parameters");

    return flashErase(fmirrorp->config->flashp,
            fmirrorp->llfdi.sector_size * fmirrorp->config->sector_offset + startaddr,
            n);
#endif
    return CH_SUCCESS;
}

/**
 * @brief   Waits for idle condition.
 *
 * @param[in] fmirrorp  pointer to the @p FlashMirrorDriver object
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t fmirrorSync(FlashMirrorDriver* fmirrorp)
{
    chDbgCheck(fmirrorp != NULL, "fmirrorSync");
    /* verify device status */
    chDbgAssert(fmirrorp->state >= FLASH_READY, "fmirrorSync(), #1",
            "invalid state");

    return flashSync(fmirrorp->config->flashp);
}

/**
 * @brief   Returns media info.
 *
 * @param[in] fmirrorp  pointer to the @p FlashMirrorDriver object
 * @param[out] fdip     pointer to a @p FlashDeviceInfo structure
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t fmirrorGetInfo(FlashMirrorDriver* fmirrorp, FlashDeviceInfo* fdip)
{
    chDbgCheck(fmirrorp != NULL, "fmirrorGetInfo");
    /* verify device status */
    chDbgAssert(fmirrorp->state >= FLASH_READY, "fmirrorGetInfo(), #1",
            "invalid state");

    fdip->sector_num = (fmirrorp->llfdi.sector_size) - 1 / 2;
    fdip->sector_size = fmirrorp->llfdi.sector_size;
    memcpy(fdip->identification, fmirrorp->llfdi.identification, sizeof(fdip->identification));

    return CH_SUCCESS;
}

#if FLASH_MIRROR_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
/**
 * @brief   Gains exclusive access to the flash device.
 * @details This function tries to gain ownership to the flash device, if the
 *          device is already being used then the invoking thread is queued.
 * @pre     In order to use this function the option
 *          @p FLASH_MIRROR_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] fmirrorp  pointer to the @p FlashMirrorDriver object
 *
 * @api
 */
void fmirrorAcquireBus(FlashMirrorDriver* fmirrorp)
{
    chDbgCheck(fmirrorp != NULL, "fmirrorAcquireBus");

#if CH_USE_MUTEXES
    chMtxLock(&fmirrorp->mutex);
#elif CH_USE_SEMAPHORES
    chSemWait(&fmirrorp->semaphore);
#endif

    /* Lock the underlying device as well */
    flashAcquire(fmirrorp->config->flashp);
}

/**
 * @brief   Releases exclusive access to the flash device.
 * @pre     In order to use this function the option
 *          @p ADC_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] fmirrorp  pointer to the @p FlashMirrorDriver object
 *
 * @api
 */
void fmirrorReleaseBus(FlashMirrorDriver* fmirrorp)
{
    chDbgCheck(fmirrorp != NULL, "fmirrorReleaseBus");

#if CH_USE_MUTEXES
    (void)fmirrorp;
    chMtxUnlock();
#elif CH_USE_SEMAPHORES
    chSemSignal(&fmirrorp->semaphore);
#endif

    /* Release the underlying device as well */
    flashRelease(fmirrorp->config->flashp);
}
#endif /* FLASH_MIRROR_USE_MUTUAL_EXCLUSION */

#endif /* HAL_USE_FLASH_MIRROR */

/** @} */
