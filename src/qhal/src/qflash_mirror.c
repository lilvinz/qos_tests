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

/*
 * @brief   Functional description
 *          Memory partitioning is:
 *          - header (at least 1 sector)
 *          - mirror a
 *          - mirror b (same size as mirror a)
 *          Header is being used to store the current state
 *          where a state entry can be:
 *          - unused
 *          - dirty a
 *          - dirty b
 *          - synced
 *          Mirror a and mirror b must be of the same size.
 *          The flow of operation is:
 *          Startup / Recovery rollback:
 *              - state synced:
 *                  Do nothing.
 *              - state dirty a:
 *                  Copy mirror b to mirror a erasing pages as required.
 *                  Set state to synced.
 *                  Execute sync of lower level driver.
 *              - state dirty b:
 *                  Copy mirror a to mirror b erasing pages as required.
 *                  Set state to synced.
 *                  Execute sync of lower level driver.
 *          Sync:
 *              - state synced:
 *                  Execute sync of lower level driver.
 *              - state dirty a:
 *                  State is changed to dirty b.
 *                  Execute sync of lower level driver.
 *                  Copy mirror a to mirror b erasing pages as required.
 *                  State is changed to synced.
 *                  Execute sync of lower level driver.
 *              - state dirty b:
 *                  Invalid state!
 *          Read:
 *              - state synced:
 *              - state dirty a:
 *                  Read data from mirror a.
 *              - state dirty b:
 *                  Invalid state!
 *          Write / Erase:
 *              - state synced:
 *                  State is being set to dirty a.
 *                  Execute sync of lower level driver.
 *                  Write(s) and / or erase(s) are being executed on mirror a.
 *              - state dirty a:
 *                  Write(s) and / or erase(s) are being executed on mirror a.
 *              - state dirty b:
 *                  Invalid state!
 *
 * @todo
 */

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*
 * @brief   State values have to be chosen so that they can be represented on
 *          any flash device including devices with the following
 *          limitations:
 *          - Writes are not allowed to cells which contain low bits.
 *          - Smallest write operation is 16bits
 *
 *          The header section is being filled by an array of state entries.
 *          On startup, the last used entry is taken into account. Further
 *          updates are being written to unused entries until the array has
 *          been filled. At that point the header is being erased and the first
 *          entry is being used.
 */
static const uint64_t flash_mirror_state_mark_table[] =
{
    0xffffffffffffffff,
    0x0000ffffffffffff,
    0x00000000ffffffff,
    0x000000000000ffff,
};
STATIC_ASSERT(NELEMS(flash_mirror_state_mark_table) == STATE_DIRTY_COUNT);

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

static bool_t flash_mirror_state_init(FlashMirrorDriver* fmirrorp)
{
    chDbgCheck((fmirrorp != NULL), "flash_mirror_state_init");

    const uint32_t header_orig = 0;
    const uint32_t header_size = fmirrorp->config->sectors_header_num * fmirrorp->llfdi.sector_size;

    FlashMirrorState new_state = STATE_INVALID;
    uint32_t new_state_addr;

    uint64_t state_mark;

    for (new_state_addr = header_orig;
            new_state_addr < header_orig + header_size;
            new_state_addr += sizeof(state_mark))
    {
        bool_t result = flashRead(fmirrorp->config->flashp,
                new_state_addr,
                sizeof(new_state_addr),
                (uint8_t*)&state_mark);
        if (result != CH_SUCCESS)
            return result;

        if (state_mark == flash_mirror_state_mark_table[STATE_SYNCED])
            new_state = STATE_SYNCED;
        else if (state_mark == flash_mirror_state_mark_table[STATE_DIRTY_A])
            new_state = STATE_DIRTY_A;
        else if (state_mark == flash_mirror_state_mark_table[STATE_DIRTY_B])
            new_state = STATE_DIRTY_B;
        else if (state_mark == flash_mirror_state_mark_table[STATE_INVALID])
            goto end;
    }

end:
    fmirrorp->mirror_state = new_state;
    fmirrorp->mirror_state_addr = new_state_addr;

    return CH_SUCCESS;
}

static bool_t flash_mirror_state_update(FlashMirrorDriver* fmirrorp, FlashMirrorState new_state)
{
    chDbgCheck((fmirrorp != NULL), "flash_mirror_state_write");

    if (new_state == fmirrorp->mirror_state)
        return CH_SUCCESS;

    const uint32_t header_orig = 0;
    const uint32_t header_size = fmirrorp->config->sectors_header_num * fmirrorp->llfdi.sector_size;

    uint32_t new_state_addr = fmirrorp->mirror_state_addr;
    uint64_t new_state_mark = flash_mirror_state_mark_table[new_state];

    /* Advance state entry pointer if last state was synced */
    if (fmirrorp->mirror_state == STATE_SYNCED)
        new_state_addr += sizeof(new_state_mark);

    /* Detect wrap around and erase header if necessary. */
    if (new_state_addr >= header_orig + header_size)
    {
        new_state_addr = 0;
        bool_t result = flashErase(fmirrorp->config->flashp, header_orig, fmirrorp->config->sectors_header_num * fmirrorp->llfdi.sector_size);
        if (result != CH_SUCCESS)
            return result;
    }

    /* Write updated state entry. */
    {
        bool_t result = flashWrite(fmirrorp->config->flashp, new_state_addr, sizeof(new_state_mark), (uint8_t*)&new_state_mark);
        if (result != CH_SUCCESS)
            return result;
    }

    /* Sync lower level driver. */
    {
        bool_t result = flashSync(fmirrorp->config->flashp);
        if (result != CH_SUCCESS)
            return result;
    }

    fmirrorp->mirror_state = new_state;
    fmirrorp->mirror_state_addr = new_state_addr;

    return CH_SUCCESS;
}

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

    flash_mirror_state_init(fmirrorp);

    switch (fmirrorp->mirror_state)
    {
    case STATE_DIRTY_A:
        /* Copy mirror b to mirror a erasing pages as required. */

        /* Set state to synced. */
        flash_mirror_state_update(fmirrorp, STATE_SYNCED);
        /* Execute sync of lower level driver. */
        flashSync(fmirrorp->config->flashp);
        break;
    case STATE_INVALID:
        /* Invalid state (all header invalid) assumes mirror b to be dirty. */
    case STATE_DIRTY_B:
        /* Copy mirror a to mirror b erasing pages as required. */

        /* Set state to synced. */
        flash_mirror_state_update(fmirrorp, STATE_SYNCED);
        /* Execute sync of lower level driver. */
        flashSync(fmirrorp->config->flashp);
        break;
    case STATE_SYNCED:
    default:
        break;
    }

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
