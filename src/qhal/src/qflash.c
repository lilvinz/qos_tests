/**
 * @file    qflash.c
 * @brief   MCU internal flash driver code.
 *
 * @addtogroup FLASH
 * @{
 */

#include "ch.h"
#include "qhal.h"

#if HAL_USE_FLASH || defined(__DOXYGEN__)

#include "static_assert.h"

/*
 * @todo    - add error detection and handling
 *          - add option bytes programming
 *          - add readout protection programming and clearing
 */

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
static const struct FLASHDriverVMT flash_vmt =
{
    .read = (bool_t (*)(void*, uint32_t, uint32_t, uint8_t*))flashRead,
    .write = (bool_t (*)(void*, uint32_t, uint32_t, const uint8_t*))flashWrite,
    .erase = (bool_t (*)(void*, uint32_t, uint32_t))flashErase,
    .sync = (bool_t (*)(void*))flashSync,
    .write_protect = (bool_t (*)(void*))flashWriteProtect,
    .write_unprotect = (bool_t (*)(void*))flashWriteUnprotect,
#if FLASH_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
    .acquire = (bool_t (*)(void*))flashAcquireBus,
    .release = (bool_t (*)(void*))flashReleaseBus,
#endif
    .get_info = (bool_t (*)(void*, NVMDeviceInfo*))flashGetInfo,
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   FLASH INTERNAL driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void flashInit(void)
{
    flash_lld_init();
}

/**
 * @brief   Initializes an instance.
 *
 * @param[out] flashp   pointer to the @p FLASHDriver object
 *
 * @init
 */
void flashObjectInit(FLASHDriver* flashp)
{
    flashp->vmt = &flash_vmt;
    flashp->state = NVM_STOP;
    flashp->config = NULL;
#if FLASH_USE_MUTUAL_EXCLUSION
#if CH_USE_MUTEXES
    chMtxInit(&flashp->mutex);
#else
    chSemInit(&flashp->semaphore, 1);
#endif
#endif /* FLASH_USE_MUTUAL_EXCLUSION */
}

/**
 * @brief   Configures and activates the FLASH peripheral.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 * @param[in] config    pointer to the @p FLASHConfig object.
 *
 * @api
 */
void flashStart(FLASHDriver* flashp, const FLASHConfig* config)
{
    chDbgCheck((flashp != NULL) && (config != NULL), "flashStart");

    chSysLock();
    /* Verify device status. */
    chDbgAssert((flashp->state == NVM_STOP) || (flashp->state == NVM_READY),
            "flashStart(), #1", "invalid state");

    flashp->config = config;
    flash_lld_start(flashp);
    flashp->state = NVM_READY;
    chSysUnlock();
}

/**
 * @brief   Disables the FLASH peripheral.
 *
 * @param[in] flashp      pointer to the @p FLASHDriver object
 *
 * @api
 */
void flashStop(FLASHDriver* flashp)
{
    chDbgCheck(flashp != NULL, "flashStop");

    chSysLock();
    /* verify device status */
    chDbgAssert((flashp->state == NVM_STOP) || (flashp->state == NVM_READY),
            "flashStop(), #1", "invalid state");

    flash_lld_stop(flashp);
    flashp->state = NVM_STOP;
    chSysUnlock();
}

/**
 * @brief   Reads data crossing sector boundaries if required.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
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
bool_t flashRead(FLASHDriver* flashp, uint32_t startaddr, uint32_t n,
        uint8_t* buffer)
{
    chDbgCheck(flashp != NULL, "flashRead");

    chSysLock();

    /* Verify device status. */
    chDbgAssert(flashp->state >= NVM_READY, "flashRead(), #1",
            "invalid state");

    /* Verify range is within chip size. */
    chDbgAssert(
            flash_lld_addr_to_sector(startaddr, NULL) == CH_SUCCESS
            && flash_lld_addr_to_sector(startaddr + n, NULL) == CH_SUCCESS,
            "flashRead(), #2", "invalid parameters");

    /* Read operation in progress. */
    flashp->state = NVM_READING;

    flash_lld_sync(flashp);

    flash_lld_read(flashp, startaddr, n, buffer);

    chSysUnlock();

    return CH_SUCCESS;
}

/**
 * @brief   Writes data crossing sector boundaries if required.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
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
bool_t flashWrite(FLASHDriver* flashp, uint32_t startaddr, uint32_t n,
        const uint8_t* buffer)
{
    chDbgCheck(flashp != NULL, "flashWrite");

    chSysLock();

    /* Verify device status. */
    chDbgAssert(flashp->state >= NVM_READY, "flashWrite(), #1",
            "invalid state");

    /* Verify range is within chip size. */
    chDbgAssert(
            flash_lld_addr_to_sector(startaddr, NULL) == CH_SUCCESS
            && flash_lld_addr_to_sector(startaddr + n, NULL) == CH_SUCCESS,
            "flashWrite(), #2", "invalid parameters");

    /* Write operation in progress. */
    flashp->state = NVM_WRITING;

    flash_lld_sync(flashp);

    flash_lld_write(flashp, startaddr, n, buffer);

    chSysUnlock();

    return CH_SUCCESS;
}

/**
 * @brief   Erases one or more sectors.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 * @param[in] startaddr address within to be erased sector
 * @param[in] n         number of bytes to erase
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t flashErase(FLASHDriver* flashp, uint32_t startaddr, uint32_t n)
{
    chDbgCheck(flashp != NULL, "flashErase");

    chSysLock();
    /* Verify device status. */
    chDbgAssert(flashp->state >= NVM_READY, "flashErase(), #1",
            "invalid state");

    /* Verify range is within chip size. */
    chDbgAssert(
            flash_lld_addr_to_sector(startaddr, NULL) == CH_SUCCESS
            && flash_lld_addr_to_sector(startaddr + n, NULL) == CH_SUCCESS,
            "flashErase(), #2", "invalid parameters");

    /* Erase operation in progress. */
    flashp->state = NVM_ERASING;

    FLASHSectorInfo sector;

    for (sector.origin = startaddr;
            sector.origin < startaddr + n;
            sector.origin += sector.size)
    {
        if (flash_lld_addr_to_sector(sector.origin, &sector) != CH_SUCCESS)
        {
            chSysUnlock();
            return CH_FAILED;
        }

        flash_lld_sync(flashp);

        flash_lld_erase(flashp, sector.origin);
    }

    chSysUnlock();

    return CH_SUCCESS;
}

/**
 * @brief   Erases whole chip.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t flashMassErase(FLASHDriver* flashp)
{
    chDbgCheck(flashp != NULL, "flashMassErase");

    chSysLock();
    /* verify device status */
    chDbgAssert(flashp->state >= NVM_READY, "flashMassErase(), #1",
            "invalid state");

    /* Erase operation in progress. */
    flashp->state = NVM_ERASING;

    flash_lld_sync(flashp);

    flash_lld_masserase(flashp);

    chSysUnlock();

    return CH_SUCCESS;
}

/**
 * @brief   Waits for idle condition.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t flashSync(FLASHDriver* flashp)
{
    chDbgCheck(flashp != NULL, "flashSync");

    chSysLock();
    /* verify device status */
    chDbgAssert(flashp->state >= NVM_READY, "flashSync(), #1",
            "invalid state");

    flash_lld_sync(flashp);

    flashp->state = NVM_READY;

    chSysUnlock();
    return CH_SUCCESS;
}

/**
 * @brief   Returns media info.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 * @param[out] nvmdip   pointer to a @p NVMDeviceInfo structure
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t flashGetInfo(FLASHDriver* flashp, NVMDeviceInfo* nvmdip)
{
    chDbgCheck(flashp != NULL, "flashGetInfo");

    chSysLock();
    /* verify device status */
    chDbgAssert(flashp->state >= NVM_READY, "flashGetInfo(), #1",
            "invalid state");

    flash_lld_get_info(flashp, nvmdip);

    chSysUnlock();
    return CH_SUCCESS;
}

/**
 * @brief   Write unprotects the whole chip.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t flashWriteUnprotect(FLASHDriver* flashp)
{
    chDbgCheck(flashp != NULL, "flashWriteUnprotect");

    chSysLock();
    /* Verify device status. */
    chDbgAssert(flashp->state >= NVM_READY, "flashWriteUnprotect(), #1",
            "invalid state");

    if (flashSync(flashp) != CH_SUCCESS)
    {
        chSysUnlock();
        return CH_FAILED;
    }

    flash_lld_write_unprotect(flashp);

    chSysUnlock();
    return CH_SUCCESS;
}

/**
 * @brief   Write protects the whole chip.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t flashWriteProtect(FLASHDriver* flashp)
{
    chDbgCheck(flashp != NULL, "flashWriteProtect");

    chSysLock();
    /* Verify device status. */
    chDbgAssert(flashp->state >= NVM_READY, "flashWriteProtect(), #1",
            "invalid state");

    if (flashSync(flashp) != CH_SUCCESS)
    {
        chSysUnlock();
        return CH_FAILED;
    }

    flash_lld_write_protect(flashp);

    chSysUnlock();
    return CH_SUCCESS;
}

#if FLASH_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
/**
 * @brief   Gains exclusive access to the flash device.
 * @details This function tries to gain ownership to the flash device, if the
 *          device is already being used then the invoking thread is queued.
 * @pre     In order to use this function the option
 *          @p FLASH_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 *
 * @api
 */
void flashAcquireBus(FLASHDriver* flashp)
{
    chDbgCheck(flashp != NULL, "flashAcquireBus");

#if CH_USE_MUTEXES
    chMtxLock(&flashp->mutex);
#elif CH_USE_SEMAPHORES
    chSemWait(&flashp->semaphore);
#endif
}

/**
 * @brief   Releases exclusive access to the flash device.
 * @pre     In order to use this function the option
 *          @p FLASH_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 *
 * @api
 */
void flashReleaseBus(FLASHDriver* flashp)
{
    chDbgCheck(flashp != NULL, "flashReleaseBus");

#if CH_USE_MUTEXES
    (void)flashp;
    chMtxUnlock();
#elif CH_USE_SEMAPHORES
    chSemSignal(&flashp->semaphore);
#endif
}
#endif /* FLASH_USE_MUTUAL_EXCLUSION */

#endif /* HAL_USE_FLASH */

/** @} */
