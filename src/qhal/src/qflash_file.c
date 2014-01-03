/**
 * @file    qflash_file.c
 * @brief   FLASH emulation through binary file code.
 *
 * @addtogroup FLASH_FILE
 * @{
 */

#include "ch.h"
#include "qhal.h"

#include "static_assert.h"

#include <string.h>

#if HAL_USE_FLASH_FILE || defined(__DOXYGEN__)

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
static const struct FlashFileDriverVMT flash_file_vmt =
{
    .read = (bool_t (*)(void*, uint32_t, uint32_t, uint8_t*))ffileRead,
    .write = (bool_t (*)(void*, uint32_t, uint32_t, const uint8_t*))ffileWrite,
    .erase = (bool_t (*)(void*, uint32_t, uint32_t))ffileErase,
    .sync = (bool_t (*)(void*))ffileSync,
#if FLASH_FILE_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
    .acquire = (bool_t (*)(void*))ffileAcquireBus,
    .release = (bool_t (*)(void*))ffileReleaseBus,
#endif
    .get_info = (bool_t (*)(void*, FlashDeviceInfo *))ffileGetInfo,
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   FLASH emulation through binary file initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void ffileInit(void)
{
}

/**
 * @brief   Initializes an instance.
 *
 * @param[out] ffilep   pointer to the @p FlashFileDriver object
 *
 * @init
 */
void ffileObjectInit(FlashFileDriver* ffilep)
{
    ffilep->vmt = &flash_file_vmt;
    ffilep->state = FLASH_STOP;
    ffilep->config = NULL;
#if FLASH_FILE_USE_MUTUAL_EXCLUSION
#if CH_USE_MUTEXES
    chMtxInit(&ffilep->mutex);
#else
    chSemInit(&ffilep->semaphore, 1);
#endif
#endif /* FLASH_FILE_USE_MUTUAL_EXCLUSION */
}

/**
 * @brief   Configures and activates the FLASH emulation.
 *
 * @param[in] ffilep    pointer to the @p FlashFileDriver object
 * @param[in] config    pointer to the @p FlashFileConfig object.
 *
 * @api
 */
void ffileStart(FlashFileDriver* ffilep, const FlashFileConfig* config)
{
    chDbgCheck((ffilep != NULL) && (config != NULL), "ffileStart");
    /* verify device status */
    chDbgAssert((ffilep->state == FLASH_STOP) || (ffilep->state == FLASH_READY),
            "ffileStart(), #1", "invalid state");

    if (ffilep->state == FLASH_READY)
        ffileStop(ffilep);

    ffilep->config = config;

    ffilep->file = fopen(ffilep->config->file_name, "r+b");

    if (ffilep->file == NULL)
    {
        /* Create file as it seems to not exist */
        ffilep->file = fopen(ffilep->config->file_name, "w+b");
    }

    chDbgAssert(ffilep->file != NULL, "ffileStart(), #2", "invalid file pointer");

    if (ffilep->file == NULL)
        return;

    /* grow file according to configuration if necessary */
    if (fseek(ffilep->file, 0, SEEK_END) != 0)
        return;

    size_t current_size = ftell(ffilep->file);
    size_t desired_size = ffilep->config->sector_size * ffilep->config->sector_num;

    if (current_size < desired_size)
    {
        static const uint8_t erased = 0xff;
        for (size_t i = current_size; i < desired_size; ++i)
            if (fwrite(&erased, 1, sizeof(erased), ffilep->file) != sizeof(erased))
                return;
        if (fflush(ffilep->file) != 0)
            return;
    }

    ffilep->state = FLASH_READY;
}

/**
 * @brief   Disables the FLASH peripheral.
 *
 * @param[in] ffilep    pointer to the @p FlashFileDriver object
 *
 * @api
 */
void ffileStop(FlashFileDriver* ffilep)
{
    chDbgCheck(ffilep != NULL, "ffileStop");
    /* verify device status */
    chDbgAssert((ffilep->state == FLASH_STOP) || (ffilep->state == FLASH_READY),
            "ffileStop(), #1", "invalid state");

    if (ffilep->file != NULL)
        fclose(ffilep->file);

    ffilep->file = NULL;
    ffilep->state = FLASH_STOP;
}

/**
 * @brief   Reads data crossing sector boundaries if required.
 *
 * @param[in] ffilep    pointer to the @p FlashFileDriver object
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
bool_t ffileRead(FlashFileDriver* ffilep, uint32_t startaddr, uint32_t n, uint8_t* buffer)
{
    chDbgCheck(ffilep != NULL, "ffileRead");
    /* verify device status */
    chDbgAssert(ffilep->state >= FLASH_READY, "ffileRead(), #1",
            "invalid state");
    /* verify range is within chip size */
    chDbgAssert((startaddr + n <= ffilep->config->sector_size * ffilep->config->sector_num), "ffileRead(), #2",
            "invalid parameters");

    if (ffileSync(ffilep) != CH_SUCCESS)
        return CH_FAILED;

    /* Read operation in progress. */
    ffilep->state = FLASH_READING;

    if (fseek(ffilep->file, startaddr, SEEK_SET) != 0)
        return CH_FAILED;

    if (fread(buffer, 1, n, ffilep->file) != n)
        return CH_FAILED;

    /* Read operation finished. */
    ffilep->state = FLASH_READY;

    return CH_SUCCESS;
}

/**
 * @brief   Writes data crossing sector boundaries if required.
 *
 * @param[in] ffilep    pointer to the @p FlashFileDriver object
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
bool_t ffileWrite(FlashFileDriver* ffilep, uint32_t startaddr, uint32_t n, const uint8_t* buffer)
{
    chDbgCheck(ffilep != NULL, "ffileWrite");
    /* verify device status */
    chDbgAssert(ffilep->state >= FLASH_READY, "ffileWrite(), #1",
            "invalid state");
    /* verify range is within chip size */
    chDbgAssert((startaddr + n <= ffilep->config->sector_size * ffilep->config->sector_num), "ffileWrite(), #2",
            "invalid parameters");

    if (ffileSync(ffilep) != CH_SUCCESS)
        return CH_FAILED;

    /* Write operation in progress.*/
    ffilep->state = FLASH_WRITING;

    if (fseek(ffilep->file, startaddr, SEEK_SET) != 0)
        return CH_FAILED;

    if (fwrite(buffer, 1, n, ffilep->file) != n)
        return CH_FAILED;

    return CH_SUCCESS;
}

/**
 * @brief   Erases one or more sectors.
 *
 * @param[in] ffilep    pointer to the @p FlashFileDriver object
 * @param[in] startaddr address within to be erased sector
 * @param[in] n         number of bytes to erase
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t ffileErase(FlashFileDriver* ffilep, uint32_t startaddr, uint32_t n)
{
    chDbgCheck(ffilep != NULL, "ffileErase");
    /* verify device status */
    chDbgAssert(ffilep->state >= FLASH_READY, "ffileErase(), #1",
            "invalid state");
    /* verify range is within chip size */
    chDbgAssert((startaddr + n <= ffilep->config->sector_size * ffilep->config->sector_num), "ffileErase(), #2",
            "invalid parameters");

    if (ffileSync(ffilep) != CH_SUCCESS)
        return CH_FAILED;

    /* Erase operation in progress.*/
    ffilep->state = FLASH_ERASING;

    uint32_t first_sector_addr = startaddr - (startaddr % ffilep->config->sector_size);
    uint32_t last_sector_addr = (startaddr + n) - ((startaddr + n) % ffilep->config->sector_size);

    if (fseek(ffilep->file, first_sector_addr, SEEK_SET) != 0)
        return CH_FAILED;

    for (uint32_t addr = first_sector_addr; addr < last_sector_addr; addr += ffilep->config->sector_size)
    {
        static const uint8_t erased = 0xff;
        for (size_t i = 0; i < ffilep->config->sector_size; ++i)
            if (fwrite(&erased, 1, sizeof(erased), ffilep->file) != sizeof(erased))
                return CH_FAILED;
    }

    return CH_SUCCESS;
}

/**
 * @brief   Erases whole chip.
 *
 * @param[in] ffilep    pointer to the @p FlashFileDriver object
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t ffileMassErase(FlashFileDriver* ffilep)
{
    chDbgCheck(ffilep != NULL, "ffileMassErase");
    /* verify device status */
    chDbgAssert(ffilep->state >= FLASH_READY, "ffileMassErase(), #1",
            "invalid state");

    if (ffileSync(ffilep) != CH_SUCCESS)
        return CH_FAILED;

    /* Erase operation in progress.*/
    ffilep->state = FLASH_ERASING;

    for (uint32_t addr = 0; addr < ffilep->config->sector_size * ffilep->config->sector_num; addr += ffilep->config->sector_size)
    {
        if (fseek(ffilep->file, addr, SEEK_SET) != 0)
            return CH_FAILED;

        static const uint8_t erased = 0xff;
        for (size_t i = 0; i < ffilep->config->sector_size; ++i)
            if (fwrite(&erased, 1, sizeof(erased), ffilep->file) != sizeof(erased))
                return CH_FAILED;
    }

    return CH_SUCCESS;
}

/**
 * @brief   Waits for idle condition.
 *
 * @param[in] ffilep    pointer to the @p FlashFileDriver object
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t ffileSync(FlashFileDriver* ffilep)
{
    chDbgCheck(ffilep != NULL, "ffileSync");
    /* verify device status */
    chDbgAssert(ffilep->state >= FLASH_READY, "ffileSync(), #1",
            "invalid state");

    if (ffilep->state == FLASH_READY)
        return CH_SUCCESS;

    if (fflush(ffilep->file) != 0)
        return CH_FAILED;

    ffilep->state = FLASH_READY;

    return CH_SUCCESS;
}

/**
 * @brief   Returns media info.
 *
 * @param[in] ffilep    pointer to the @p FlashFileDriver object
 * @param[out] fdip     pointer to a @p FlashDeviceInfo structure
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t ffileGetInfo(FlashFileDriver* ffilep, FlashDeviceInfo* fdip)
{
    chDbgCheck(ffilep != NULL, "ffileGetInfo");
    /* verify device status */
    chDbgAssert(ffilep->state >= FLASH_READY, "ffileGetInfo(), #1",
            "invalid state");

    fdip->sector_num = ffilep->config->sector_num;
    fdip->sector_size = ffilep->config->sector_size;
    memset(fdip->identification, 0, sizeof(fdip->identification));

    return CH_SUCCESS;
}

#if FLASH_FILE_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
/**
 * @brief   Gains exclusive access to the flash device.
 * @details This function tries to gain ownership to the flash device, if the
 *          device is already being used then the invoking thread is queued.
 * @pre     In order to use this function the option
 *          @p FLASH_FILE_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] ffilep      pointer to the @p FlashFileDriver object
 *
 * @api
 */
void ffileAcquireBus(FlashFileDriver* ffilep)
{
    chDbgCheck(ffilep != NULL, "ffileAcquireBus");

#if CH_USE_MUTEXES
    chMtxLock(&ffilep->mutex);
#elif CH_USE_SEMAPHORES
    chSemWait(&ffilep->semaphore);
#endif
}

/**
 * @brief   Releases exclusive access to the flash device.
 * @pre     In order to use this function the option
 *          @p FLASH_FILE_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] ffilep      pointer to the @p FlashFileDriver object
 *
 * @api
 */
void ffileReleaseBus(FlashFileDriver* ffilep)
{
    chDbgCheck(ffilep != NULL, "ffileReleaseBus");

#if CH_USE_MUTEXES
    (void)ffilep;
    chMtxUnlock();
#elif CH_USE_SEMAPHORES
    chSemSignal(&ffilep->semaphore);
#endif
}
#endif /* FLASH_FILE_USE_MUTUAL_EXCLUSION */

#endif /* HAL_USE_FLASH_FILE */

/** @} */
