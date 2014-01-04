/**
 * @file    qflash.c
 * @brief   MCU internal flash driver code.
 *
 * @addtogroup FLASH
 * @{
 */

#include "ch.h"
#include "qhal.h"

#include "static_assert.h"

#if HAL_USE_FLASH || defined(__DOXYGEN__)

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
#if 0
static bool_t flash_write_enable(FLASHDriver* flashp)
{
    chDbgCheck((flashp != NULL), "flash_write_enable");

    spiSelect(flashp->config->spip);

    // send JEDEC WREN command
    static const uint8_t out[] =
    {
        FLASH_JEDEC_WREN,
    };

    spiSend(flashp->config->spip, NELEMS(out), out);

    spiUnselect(flashp->config->spip);

    return CH_SUCCESS;
}

static bool_t flash_write_disable(FLASHDriver* flashp)
{
    chDbgCheck((flashp != NULL), "flash_write_disable");

    spiSelect(flashp->config->spip);

    // send JEDEC WRDI command
    static const uint8_t out[] =
    {
        FLASH_JEDEC_WRDI,
    };

    spiSend(flashp->config->spip, NELEMS(out), out);

    spiUnselect(flashp->config->spip);

    return CH_SUCCESS;
}

static bool_t flash_page_program(FLASHDriver* flashp, uint32_t startaddr,
        uint32_t n, const uint8_t* buffer)
{
    chDbgCheck(flashp != NULL, "flash_page_program");

    if (fiSync(flashp) != CH_SUCCESS)
        return CH_FAILED;

    if (flash_write_enable(flashp) != CH_SUCCESS)
        return CH_FAILED;

    /* Write operation in progress.*/
    flashp->state = FLASH_WRITING;

    spiSelect(flashp->config->spip);

    const uint8_t out[] =
    {
        FLASH_JEDEC_PP,
        (startaddr >> 24) & 0xff,
        (startaddr >> 16) & 0xff,
        (startaddr >> 8) & 0xff,
        (startaddr >> 0) & 0xff,
    };

    /* command byte */
    spiSend(flashp->config->spip, 1, &out[0]);

    /* address bytes */
    spiSend(flashp->config->spip, flashp->config->addrbytes_num,
            &out[1 + (4 - flashp->config->addrbytes_num)]);

    /* data buffer */
    spiSend(flashp->config->spip, n, buffer);

    spiUnselect(flashp->config->spip);

    return CH_SUCCESS;
}

static bool_t flash_sector_erase(FLASHDriver* flashp, uint32_t startaddr)
{
    chDbgCheck(flashp != NULL, "flash_sector_erase");

    /* Check if device supports erase command. */
    if (flashp->config->sector_erase == 0x00)
        return CH_SUCCESS;

    if (fiSync(flashp) != CH_SUCCESS)
        return CH_FAILED;

    if (flash_write_enable(flashp) != CH_SUCCESS)
        return CH_FAILED;

    /* Erase operation in progress.*/
    flashp->state = FLASH_ERASING;

    spiSelect(flashp->config->spip);

    const uint8_t out[] =
    {
        flashp->config->sector_erase, /* Erase command is chip specific. */
        (startaddr >> 24) & 0xff,
        (startaddr >> 16) & 0xff,
        (startaddr >> 8) & 0xff,
        (startaddr >> 0) & 0xff,
        0x00,
    };

    /* command byte */
    spiSend(flashp->config->spip, 1, &out[0]);

    /* address bytes */
    spiSend(flashp->config->spip, flashp->config->addrbytes_num,
            &out[1 + (4 - flashp->config->addrbytes_num)]);

    spiUnselect(flashp->config->spip);

    return CH_SUCCESS;
}
#endif
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
    /* verify device status */
    chDbgAssert((flashp->state == NVM_STOP) || (flashp->state == NVM_READY),
            "flashStart(), #1", "invalid state");

    flashp->config = config;
    flashp->state = NVM_READY;
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
    /* verify device status */
    chDbgAssert((flashp->state == NVM_STOP) || (flashp->state == NVM_READY),
            "flashStop(), #1", "invalid state");

    flashp->state = NVM_STOP;
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
    /* verify device status */
    chDbgAssert(flashp->state >= NVM_READY, "flashRead(), #1",
            "invalid state");
#if 0
    /* verify range is within chip size */
    chDbgAssert(
            (startaddr + n <= flashp->config->sector_size * flashp->config->sector_num),
            "flashRead(), #2", "invalid parameters");

    if (fiSync(flashp) != CH_SUCCESS)
        return CH_FAILED;

    /* Read operation in progress.*/
    flashp->state = FLASH_READING;

    spiSelect(flashp->config->spip);

    const uint8_t out[] =
    {
        FLASH_JEDEC_FAST_READ,
        (startaddr >> 24) & 0xff,
        (startaddr >> 16) & 0xff,
        (startaddr >> 8) & 0xff,
        (startaddr >> 0) & 0xff,
        0x00, /* 1 dummy byte required for timing */
    };

    /* command byte */
    spiSend(flashp->config->spip, 1, &out[0]);

    /* address bytes */
    spiSend(flashp->config->spip,
            flashp->config->addrbytes_num + 1,
            &out[1 + (4 - flashp->config->addrbytes_num)]);

    spiReceive(flashp->config->spip, n, buffer);

    spiUnselect(flashp->config->spip);

    /* Read operation flashnished.*/
    flashp->state = NVM_READY;
#endif
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
    /* verify device status */
    chDbgAssert(flashp->state >= NVM_READY, "flashWrite(), #1",
            "invalid state");
#if 0
    /* verify range is within chip size */
    chDbgAssert(
            (startaddr + n <= flashp->config->sector_size * flashp->config->sector_num),
            "flashWrite(), #2", "invalid parameters");

    uint32_t written = 0;

    while (written < n)
    {
        uint32_t n_chunk = flashp->config->page_size
                - ((startaddr + written) % flashp->config->page_size);
        if (n_chunk > n - written)
            n_chunk = n - written;

        if (flash_page_program(flashp,
                startaddr + written, n_chunk, buffer + written) != CH_SUCCESS)
            return CH_FAILED;

        written += n_chunk;
    }
#endif
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
    /* verify device status */
    chDbgAssert(flashp->state >= NVM_READY, "flashErase(), #1",
            "invalid state");
#if 0
    /* verify range is within chip size */
    chDbgAssert(
            (startaddr + n <= flashp->config->sector_size * flashp->config->sector_num),
            "flashErase(), #2", "invalid parameters");

    uint32_t flashrst_sector_addr = startaddr - (startaddr % flashp->config->sector_size);
    uint32_t last_sector_addr = (startaddr + n) - ((startaddr + n) % flashp->config->sector_size);

    for (uint32_t addr = flashrst_sector_addr; addr < last_sector_addr; addr += flashp->config->sector_size)
    {
        if (flash_sector_erase(flashp, addr) != CH_SUCCESS)
            return CH_FAILED;
    }
#endif
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
    /* verify device status */
    chDbgAssert(flashp->state >= NVM_READY, "flashMassErase(), #1",
            "invalid state");
#if 0
    /* Check if device supports erase command. */
    if (flashp->config->sector_erase == 0x00)
        return CH_SUCCESS;

    if (fiSync(flashp) != CH_SUCCESS)
        return CH_FAILED;

    if (flash_write_enable(flashp) != CH_SUCCESS)
        return CH_FAILED;

    /* Erase operation in progress.*/
    flashp->state = FLASH_ERASING;

    spiSelect(flashp->config->spip);

    static const uint8_t out[] =
    {
        FLASH_JEDEC_MASS_ERASE,
    };

    /* command byte */
    spiSend(flashp->config->spip, NELEMS(out), out);

    spiUnselect(flashp->config->spip);
#endif
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
    /* verify device status */
    chDbgAssert(flashp->state >= NVM_READY, "flashSync(), #1",
            "invalid state");
#if 0
    if (flashp->state == NVM_READY)
        return CH_SUCCESS;

    spiSelect(flashp->config->spip);

    static const uint8_t out[] =
    {
        FLASH_JEDEC_RDSR,
    };

    spiSend(flashp->config->spip, NELEMS(out), out);

    uint8_t in;
    for (uint8_t i = 0; i < 16; ++i)
    {
        spiReceive(flashp->config->spip, sizeof(in), &in);
        if ((in & 0x01) == 0x00)
            break;
    }

    /* Looks like it is a long wait.*/
    while ((in & 0x01) != 0x00)
    {
#ifdef FLASH_NICE_WAITING
        /* Trying to be nice with the other threads.*/
        chThdSleep(1);
#endif
        spiReceive(flashp->config->spip, sizeof(in), &in);
    }

    spiUnselect(flashp->config->spip);

    flashp->state = NVM_READY;
#endif
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
    /* verify device status */
    chDbgAssert(flashp->state >= NVM_READY, "flashGetInfo(), #1",
            "invalid state");
#if 0
    nvmdip->sector_num = flashp->config->sector_num;
    nvmdip->sector_size = flashp->config->sector_size;

    spiSelect(flashp->config->spip);

    static const uint8_t out[] =
    {
        FLASH_JEDEC_RDID,
    };
    spiSend(flashp->config->spip, NELEMS(out), out);

    // skip JEDEC continuation id
    nvmdip->identification[0] = 0x7f;
    while (nvmdip->identification[0] == 0x7f)
        spiReceive(flashp->config->spip, 1, nvmdip->identification);

    spiReceive(flashp->config->spip, NELEMS(nvmdip->identification) - 1, nvmdip->identification + 1);

    spiUnselect(flashp->config->spip);
#endif
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
    /* verify device status */
    chDbgAssert(flashp->state >= NVM_READY, "flashWriteUnprotect(), #1",
            "invalid state");
#if 0
    if (flash_write_enable(flashp) != CH_SUCCESS)
        return CH_FAILED;

    spiSelect(flashp->config->spip);

    static const uint8_t out[] =
    {
        FLASH_JEDEC_WRSR,
        0x00,
    };

    /* command byte */
    spiSend(flashp->config->spip, NELEMS(out), out);

    /* address bytes */
    spiSend(flashp->config->spip, flashp->config->addrbytes_num,
            &out[1 + (4 - flashp->config->addrbytes_num)]);

    spiUnselect(flashp->config->spip);

    if (flash_write_disable(flashp) != CH_SUCCESS)
        return CH_FAILED;
#endif
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
    /* verify device status */
    chDbgAssert(flashp->state >= NVM_READY, "flashWriteProtect(), #1",
            "invalid state");
#if 0
    if (flash_write_enable(flashp) != CH_SUCCESS)
        return CH_FAILED;

    spiSelect(flashp->config->spip);

    static const uint8_t out[] =
    {
        FLASH_JEDEC_WRSR,
        0x3c, /* set BP0 ... BP3 */
    };

    /* command byte */
    spiSend(flashp->config->spip, NELEMS(out), out);

    /* address bytes */
    spiSend(flashp->config->spip, flashp->config->addrbytes_num,
            &out[1 + (4 - flashp->config->addrbytes_num)]);

    spiUnselect(flashp->config->spip);

    if (flash_write_disable(flashp) != CH_SUCCESS)
        return CH_FAILED;
#endif
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
