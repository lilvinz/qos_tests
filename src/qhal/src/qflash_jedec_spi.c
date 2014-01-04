/**
 * @file    qflash_jedec_spi.c
 * @brief   FLASH JEDEC over SPI driver code.
 *
 * @addtogroup FLASH_JEDEC_SPI
 * @{
 */

#include "ch.h"
#include "qhal.h"

#if HAL_USE_FLASH_JEDEC_SPI || defined(__DOXYGEN__)

#include "static_assert.h"

/*
 * @todo    - add AAI writing for chips which support it
 *          - add error detection and handling
 *          - add sector level write protection API
 */

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#define FLASH_JEDEC_WREN 0x06
#define FLASH_JEDEC_WRDI 0x04
#define FLASH_JEDEC_RDID 0x9f
#define FLASH_JEDEC_RDSR 0x05
#define FLASH_JEDEC_WRSR 0x01
#define FLASH_JEDEC_FAST_READ 0x0b
#define FLASH_JEDEC_PP 0x02
#define FLASH_JEDEC_MASS_ERASE 0xc7

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   Virtual methods table.
 */
static const struct FlashJedecSPIDriverVMT flash_jedec_spi_vmt =
{
    .read = (bool_t (*)(void*, uint32_t, uint32_t, uint8_t*))fjsRead,
    .write = (bool_t (*)(void*, uint32_t, uint32_t, const uint8_t*))fjsWrite,
    .erase = (bool_t (*)(void*, uint32_t, uint32_t))fjsErase,
    .sync = (bool_t (*)(void*))fjsSync,
    .get_info = (bool_t (*)(void*, NVMDeviceInfo *))fjsGetInfo,
    /* End of mandatory functions. */
#if FLASH_JEDEC_SPI_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
    .acquire = (bool_t (*)(void*))fjsAcquireBus,
    .release = (bool_t (*)(void*))fjsReleaseBus,
#endif
    .write_protect = (bool_t (*)(void*))fjsWriteProtect,
    .write_unprotect = (bool_t (*)(void*))fjsWriteUnprotect,
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

static bool_t flash_jedec_spi_write_enable(FlashJedecSPIDriver* fjsp)
{
    chDbgCheck((fjsp != NULL), "flash_jedec_spi_write_enable");

    spiSelect(fjsp->config->spip);

    // send JEDEC WREN command
    static const uint8_t out[] =
    {
        FLASH_JEDEC_WREN,
    };

    spiSend(fjsp->config->spip, NELEMS(out), out);

    spiUnselect(fjsp->config->spip);

    return CH_SUCCESS;
}

static bool_t flash_jedec_spi_write_disable(FlashJedecSPIDriver* fjsp)
{
    chDbgCheck((fjsp != NULL), "flash_jedec_spi_write_disable");

    spiSelect(fjsp->config->spip);

    // send JEDEC WRDI command
    static const uint8_t out[] =
    {
        FLASH_JEDEC_WRDI,
    };

    spiSend(fjsp->config->spip, NELEMS(out), out);

    spiUnselect(fjsp->config->spip);

    return CH_SUCCESS;
}

static bool_t flash_jedec_spi_page_program(FlashJedecSPIDriver* fjsp,
        uint32_t startaddr, uint32_t n, const uint8_t* buffer)
{
    chDbgCheck(fjsp != NULL, "flash_jedec_spi_page_program");

    if (fjsSync(fjsp) != CH_SUCCESS)
        return CH_FAILED;

    if (flash_jedec_spi_write_enable(fjsp) != CH_SUCCESS)
        return CH_FAILED;

    /* Write operation in progress.*/
    fjsp->state = NVM_WRITING;

    spiSelect(fjsp->config->spip);

    const uint8_t out[] =
    {
        FLASH_JEDEC_PP,
        (startaddr >> 24) & 0xff,
        (startaddr >> 16) & 0xff,
        (startaddr >> 8) & 0xff,
        (startaddr >> 0) & 0xff,
    };

    /* command byte */
    spiSend(fjsp->config->spip, 1, &out[0]);

    /* address bytes */
    spiSend(fjsp->config->spip, fjsp->config->addrbytes_num,
            &out[1 + (4 - fjsp->config->addrbytes_num)]);

    /* data buffer */
    spiSend(fjsp->config->spip, n, buffer);

    spiUnselect(fjsp->config->spip);

    return CH_SUCCESS;
}

static bool_t flash_jedec_spi_sector_erase(FlashJedecSPIDriver* fjsp,
        uint32_t startaddr)
{
    chDbgCheck(fjsp != NULL, "flash_jedec_spi_sector_erase");

    if (fjsSync(fjsp) != CH_SUCCESS)
        return CH_FAILED;

    if (flash_jedec_spi_write_enable(fjsp) != CH_SUCCESS)
        return CH_FAILED;

    /* Erase operation in progress.*/
    fjsp->state = NVM_ERASING;

    spiSelect(fjsp->config->spip);

    const uint8_t out[] =
    {
        fjsp->config->sector_erase, /* Erase command is chip specific. */
        (startaddr >> 24) & 0xff,
        (startaddr >> 16) & 0xff,
        (startaddr >> 8) & 0xff,
        (startaddr >> 0) & 0xff,
        0x00,
    };

    /* command byte */
    spiSend(fjsp->config->spip, 1, &out[0]);

    /* address bytes */
    spiSend(fjsp->config->spip, fjsp->config->addrbytes_num,
            &out[1 + (4 - fjsp->config->addrbytes_num)]);

    spiUnselect(fjsp->config->spip);

    return CH_SUCCESS;
}

static bool_t flash_jedec_spi_page_program_ff(FlashJedecSPIDriver* fjsp,
        uint32_t startaddr)
{
    chDbgCheck(fjsp != NULL, "flash_jedec_spi_page_program_ff");

    if (fjsSync(fjsp) != CH_SUCCESS)
        return CH_FAILED;

    if (flash_jedec_spi_write_enable(fjsp) != CH_SUCCESS)
        return CH_FAILED;

    /* Write operation in progress.*/
    fjsp->state = NVM_WRITING;

    spiSelect(fjsp->config->spip);

    const uint8_t out[] =
    {
        FLASH_JEDEC_PP,
        (startaddr >> 24) & 0xff,
        (startaddr >> 16) & 0xff,
        (startaddr >> 8) & 0xff,
        (startaddr >> 0) & 0xff,
    };

    /* command byte */
    spiSend(fjsp->config->spip, 1, &out[0]);

    /* address bytes */
    spiSend(fjsp->config->spip, fjsp->config->addrbytes_num,
            &out[1 + (4 - fjsp->config->addrbytes_num)]);

    /* data */
    static const uint8_t erased = 0xff;
    for (uint32_t i = 0; i < fjsp->config->page_size; ++i)
        spiSend(fjsp->config->spip, sizeof(erased), &erased);

    spiUnselect(fjsp->config->spip);

    return CH_SUCCESS;
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   FLASH JEDEC over SPI driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void fjsInit(void)
{
}

/**
 * @brief   Initializes an instance.
 *
 * @param[out] fjsp     pointer to the @p FlashJedecSPIDriver object
 *
 * @init
 */
void fjsObjectInit(FlashJedecSPIDriver* fjsp)
{
    fjsp->vmt = &flash_jedec_spi_vmt;
    fjsp->state = NVM_STOP;
    fjsp->config = NULL;
#if FLASH_JEDEC_SPI_USE_MUTUAL_EXCLUSION
#if CH_USE_MUTEXES
    chMtxInit(&fjsp->mutex);
#else
    chSemInit(&fjsp->semaphore, 1);
#endif
#endif /* FLASH_JEDEC_SPI_USE_MUTUAL_EXCLUSION */
}

/**
 * @brief   Configures and activates the FLASH peripheral.
 *
 * @param[in] fjsp      pointer to the @p FlashJedecSPIDriver object
 * @param[in] config    pointer to the @p FlashJedecSPIConfig object.
 *
 * @api
 */
void fjsStart(FlashJedecSPIDriver* fjsp, const FlashJedecSPIConfig* config)
{
    chDbgCheck((fjsp != NULL) && (config != NULL), "fjsStart");
    /* verify device status */
    chDbgAssert((fjsp->state == NVM_STOP) || (fjsp->state == NVM_READY),
            "fjsStart(), #1", "invalid state");

    fjsp->config = config;
    fjsp->state = NVM_READY;
}

/**
 * @brief   Disables the FLASH peripheral.
 *
 * @param[in] fjsp      pointer to the @p FlashJedecSPIDriver object
 *
 * @api
 */
void fjsStop(FlashJedecSPIDriver* fjsp)
{
    chDbgCheck(fjsp != NULL, "fjsStop");
    /* verify device status */
    chDbgAssert((fjsp->state == NVM_STOP) || (fjsp->state == NVM_READY),
            "fjsStop(), #1", "invalid state");

    fjsp->state = NVM_STOP;
}

/**
 * @brief   Reads data crossing sector boundaries if required.
 *
 * @param[in] fjsp      pointer to the @p FlashJedecSPIDriver object
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
bool_t fjsRead(FlashJedecSPIDriver* fjsp, uint32_t startaddr, uint32_t n,
        uint8_t* buffer)
{
    chDbgCheck(fjsp != NULL, "fjsRead");
    /* verify device status */
    chDbgAssert(fjsp->state >= NVM_READY, "fjsRead(), #1",
            "invalid state");
    /* verify range is within chip size */
    chDbgAssert((startaddr + n <= fjsp->config->sector_size * fjsp->config->sector_num),
            "fjsRead(), #2", "invalid parameters");

    if (fjsSync(fjsp) != CH_SUCCESS)
        return CH_FAILED;

    /* Read operation in progress.*/
    fjsp->state = NVM_READING;

    spiSelect(fjsp->config->spip);

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
    spiSend(fjsp->config->spip, 1, &out[0]);

    /* address bytes */
    spiSend(fjsp->config->spip,
            fjsp->config->addrbytes_num + 1,
            &out[1 + (4 - fjsp->config->addrbytes_num)]);

    spiReceive(fjsp->config->spip, n, buffer);

    spiUnselect(fjsp->config->spip);

    /* Read operation finished.*/
    fjsp->state = NVM_READY;

    return CH_SUCCESS;
}

/**
 * @brief   Writes data crossing sector boundaries if required.
 *
 * @param[in] fjsp      pointer to the @p FlashJedecSPIDriver object
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
bool_t fjsWrite(FlashJedecSPIDriver* fjsp, uint32_t startaddr, uint32_t n,
        const uint8_t* buffer)
{
    chDbgCheck(fjsp != NULL, "fjsWrite");
    /* verify device status */
    chDbgAssert(fjsp->state >= NVM_READY, "fjsWrite(), #1",
            "invalid state");
    /* verify range is within chip size */
    chDbgAssert((startaddr + n <= fjsp->config->sector_size * fjsp->config->sector_num),
            "fjsWrite(), #2", "invalid parameters");

    uint32_t written = 0;

    while (written < n)
    {
        uint32_t n_chunk = fjsp->config->page_size - ((startaddr + written) % fjsp->config->page_size);
        if (n_chunk > n - written)
            n_chunk = n - written;

        if (flash_jedec_spi_page_program(fjsp,
                startaddr + written, n_chunk, buffer + written) != CH_SUCCESS)
            return CH_FAILED;

        written += n_chunk;
    }

    return CH_SUCCESS;
}

/**
 * @brief   Erases one or more sectors.
 *
 * @param[in] fjsp      pointer to the @p FlashJedecSPIDriver object
 * @param[in] startaddr address within to be erased sector
 * @param[in] n         number of bytes to erase
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t fjsErase(FlashJedecSPIDriver* fjsp, uint32_t startaddr, uint32_t n)
{
    chDbgCheck(fjsp != NULL, "fjsErase");
    /* verify device status */
    chDbgAssert(fjsp->state >= NVM_READY, "fjsErase(), #1",
            "invalid state");
    /* verify range is within chip size */
    chDbgAssert((startaddr + n <= fjsp->config->sector_size * fjsp->config->sector_num),
            "fjsErase(), #2", "invalid parameters");

    uint32_t first_sector_addr =
            startaddr - (startaddr % fjsp->config->sector_size);
    uint32_t last_sector_addr =
            (startaddr + n) - ((startaddr + n) % fjsp->config->sector_size);

    /* Check if device supports erase command. */
    if (fjsp->config->sector_erase != 0x00)
    {
        for (uint32_t addr = first_sector_addr;
                addr < last_sector_addr;
                addr += fjsp->config->sector_size)
        {
            if (flash_jedec_spi_sector_erase(fjsp, addr) != CH_SUCCESS)
                return CH_FAILED;
        }
    }
    else
    {
        for (uint32_t addr = first_sector_addr;
                addr < last_sector_addr;
                addr += fjsp->config->page_size)
        {
            if (flash_jedec_spi_page_program_ff(fjsp, addr) != CH_SUCCESS)
                return CH_FAILED;
        }
    }

    return CH_SUCCESS;
}

/**
 * @brief   Erases whole chip.
 *
 * @param[in] fjsp      pointer to the @p FlashJedecSPIDriver object
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t fjsMassErase(FlashJedecSPIDriver* fjsp)
{
    chDbgCheck(fjsp != NULL, "fjsMassErase");
    /* verify device status */
    chDbgAssert(fjsp->state >= NVM_READY, "fjsMassErase(), #1",
            "invalid state");

    /* Check if device supports erase command. */
    if (fjsp->config->sector_erase != 0x00)
    {
        if (fjsSync(fjsp) != CH_SUCCESS)
            return CH_FAILED;

        if (flash_jedec_spi_write_enable(fjsp) != CH_SUCCESS)
            return CH_FAILED;

        /* Erase operation in progress.*/
        fjsp->state = NVM_ERASING;

        spiSelect(fjsp->config->spip);

        static const uint8_t out[] =
        {
            FLASH_JEDEC_MASS_ERASE,
        };

        /* command byte */
        spiSend(fjsp->config->spip, NELEMS(out), out);

        spiUnselect(fjsp->config->spip);
    }
    else
    {
        return fjsErase(fjsp, 0,
                fjsp->config->sector_size * fjsp->config->sector_num);
    }

    return CH_SUCCESS;
}

/**
 * @brief   Waits for idle condition.
 *
 * @param[in] fjsp      pointer to the @p FlashJedecSPIDriver object
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t fjsSync(FlashJedecSPIDriver* fjsp)
{
    chDbgCheck(fjsp != NULL, "fjsSync");
    /* verify device status */
    chDbgAssert(fjsp->state >= NVM_READY, "fjsSync(), #1",
            "invalid state");

    if (fjsp->state == NVM_READY)
        return CH_SUCCESS;

    spiSelect(fjsp->config->spip);

    static const uint8_t out[] =
    {
        FLASH_JEDEC_RDSR,
    };

    spiSend(fjsp->config->spip, NELEMS(out), out);

    uint8_t in;
    for (uint8_t i = 0; i < 16; ++i)
    {
        spiReceive(fjsp->config->spip, sizeof(in), &in);
        if ((in & 0x01) == 0x00)
            break;
    }

    /* Looks like it is a long wait.*/
    while ((in & 0x01) != 0x00)
    {
#ifdef FLASH_JEDEC_SPI_NICE_WAITING
        /* Trying to be nice with the other threads.*/
        chThdSleep(1);
#endif
        spiReceive(fjsp->config->spip, sizeof(in), &in);
    }

    spiUnselect(fjsp->config->spip);

    fjsp->state = NVM_READY;

    return CH_SUCCESS;
}

/**
 * @brief   Returns media info.
 *
 * @param[in] fjsp      pointer to the @p FlashJedecSPIDriver object
 * @param[out] nvmdip     pointer to a @p NVMDeviceInfo structure
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t fjsGetInfo(FlashJedecSPIDriver* fjsp, NVMDeviceInfo* nvmdip)
{
    chDbgCheck(fjsp != NULL, "fjsGetInfo");
    /* verify device status */
    chDbgAssert(fjsp->state >= NVM_READY, "fjsGetInfo(), #1",
            "invalid state");

    nvmdip->sector_num = fjsp->config->sector_num;
    nvmdip->sector_size = fjsp->config->sector_size;

    spiSelect(fjsp->config->spip);

    static const uint8_t out[] =
    {
        FLASH_JEDEC_RDID,
    };
    spiSend(fjsp->config->spip, NELEMS(out), out);

    // skip JEDEC continuation id
    nvmdip->identification[0] = 0x7f;
    while (nvmdip->identification[0] == 0x7f)
        spiReceive(fjsp->config->spip, 1, nvmdip->identification);

    spiReceive(fjsp->config->spip, NELEMS(nvmdip->identification) - 1,
            nvmdip->identification + 1);

    spiUnselect(fjsp->config->spip);

    return CH_SUCCESS;
}

/**
 * @brief   Write unprotects the whole chip.
 *
 * @param[in] fjsp      pointer to the @p FlashJedecSPIDriver object
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t fjsWriteUnprotect(FlashJedecSPIDriver* fjsp)
{
    chDbgCheck(fjsp != NULL, "fjsWriteUnprotect");
    /* verify device status */
    chDbgAssert(fjsp->state >= NVM_READY, "fjsWriteUnprotect(), #1",
            "invalid state");

    if (flash_jedec_spi_write_enable(fjsp) != CH_SUCCESS)
        return CH_FAILED;

    spiSelect(fjsp->config->spip);

    static const uint8_t out[] =
    {
        FLASH_JEDEC_WRSR,
        0x00,
    };

    /* command byte */
    spiSend(fjsp->config->spip, NELEMS(out), out);

    /* address bytes */
    spiSend(fjsp->config->spip, fjsp->config->addrbytes_num,
            &out[1 + (4 - fjsp->config->addrbytes_num)]);

    spiUnselect(fjsp->config->spip);

    if (flash_jedec_spi_write_disable(fjsp) != CH_SUCCESS)
        return CH_FAILED;

    return CH_SUCCESS;
}

/**
 * @brief   Write protects the whole chip.
 *
 * @param[in] fjsp      pointer to the @p FlashJedecSPIDriver object
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t fjsWriteProtect(FlashJedecSPIDriver* fjsp)
{
    chDbgCheck(fjsp != NULL, "fjsWriteProtect");
    /* verify device status */
    chDbgAssert(fjsp->state >= NVM_READY, "fjsWriteProtect(), #1",
            "invalid state");

    if (flash_jedec_spi_write_enable(fjsp) != CH_SUCCESS)
        return CH_FAILED;

    spiSelect(fjsp->config->spip);

    static const uint8_t out[] =
    {
        FLASH_JEDEC_WRSR,
        0x3c, /* set BP0 ... BP3 */
    };

    /* command byte */
    spiSend(fjsp->config->spip, NELEMS(out), out);

    /* address bytes */
    spiSend(fjsp->config->spip, fjsp->config->addrbytes_num,
            &out[1 + (4 - fjsp->config->addrbytes_num)]);

    spiUnselect(fjsp->config->spip);

    if (flash_jedec_spi_write_disable(fjsp) != CH_SUCCESS)
        return CH_FAILED;

    return CH_SUCCESS;
}

#if FLASH_JEDEC_SPI_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
/**
 * @brief   Gains exclusive access to the flash device.
 * @details This function tries to gain ownership to the flash device, if the
 *          device is already being used then the invoking thread is queued.
 * @pre     In order to use this function the option
 *          @p FLASH_JEDEC_SPI_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] fjsp      pointer to the @p FlashJedecSPIDriver object
 *
 * @api
 */
void fjsAcquireBus(FlashJedecSPIDriver* fjsp)
{
    chDbgCheck(fjsp != NULL, "fjsAcquireBus");

#if CH_USE_MUTEXES
    chMtxLock(&fjsp->mutex);
#elif CH_USE_SEMAPHORES
    chSemWait(&fjsp->semaphore);
#endif

#if SPI_USE_MUTUAL_EXCLUSION
    /* Acquire the underlying device as well */
    spiAcquireBus(fjsp->config->spip);
#endif /* SPI_USE_MUTUAL_EXCLUSION */
}

/**
 * @brief   Releases exclusive access to the flash device.
 * @pre     In order to use this function the option
 *          @p FLASH_JEDEC_SPI_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] fjsp      pointer to the @p FlashJedecSPIDriver object
 *
 * @api
 */
void fjsReleaseBus(FlashJedecSPIDriver* fjsp)
{
    chDbgCheck(fjsp != NULL, "fjsReleaseBus");

#if CH_USE_MUTEXES
    (void)fjsp;
    chMtxUnlock();
#elif CH_USE_SEMAPHORES
    chSemSignal(&fjsp->semaphore);
#endif

#if SPI_USE_MUTUAL_EXCLUSION
    /* Release the underlying device as well */
    spiReleaseBus(fjsp->config->spip);
#endif /* SPI_USE_MUTUAL_EXCLUSION */
}
#endif /* FLASH_JEDEC_SPI_USE_MUTUAL_EXCLUSION */

#endif /* HAL_USE_FLASH_JEDEC_SPI */

/** @} */
