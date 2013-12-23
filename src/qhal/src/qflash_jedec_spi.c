/**
 * @file    qflash_jedec_spi.c
 * @brief   FLASH JEDEC over SPI driver code.
 *
 * @addtogroup FLASH_JEDEC_SPI
 * @{
 */

#include "ch.h"
#include "qhal.h"

#include "static_assert.h"

#if HAL_USE_FLASH_JEDEC_SPI || defined(__DOXYGEN__)

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
    .erase = (bool_t (*)(void*, uint32_t))fjsErase,
    .sync = (bool_t (*)(void*))fjsSync,
    .get_info = (bool_t (*)(void*, FlashDeviceInfo *))fjsGetInfo,
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

static bool_t flash_jedec_spi_write_enable(FlashJedecSPIDriver* fjsp)
{
    chDbgCheck((fjsp != NULL), "flash_jedec_spi_write_enable");

    if (fjsp->state != FLASH_READY)
        return CH_FAILED;

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

    if (fjsp->state != FLASH_READY)
        return CH_FAILED;

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
 * @param[out] fjsp         pointer to the @p FlashJedecSPIDriver object
 *
 * @init
 */
void fjsObjectInit(FlashJedecSPIDriver* fjsp)
{
    fjsp->vmt = &flash_jedec_spi_vmt;
    fjsp->state = FLASH_STOP;
    fjsp->config = NULL;
    fjsp->block_addresses = FALSE;
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
    chDbgAssert((fjsp->state == FLASH_STOP) || (fjsp->state == FLASH_READY),
            "fjsStart(), #1", "invalid state");

    fjsp->config = config;
    fjsp->state = FLASH_READY;
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
    chDbgAssert((fjsp->state == FLASH_STOP) || (fjsp->state == FLASH_READY),
            "fjsStop(), #1", "invalid state");

    spiStop(fjsp->config->spip);
    fjsp->state = FLASH_STOP;
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
bool_t fjsRead(FlashJedecSPIDriver* fjsp, uint32_t startaddr, uint32_t n, uint8_t *buffer)
{
    chDbgCheck(fjsp != NULL, "fjsStartSequentialRead");
    chDbgAssert(fjsp->state >= FLASH_READY, "fjsStartSequentialRead(), #1",
            "invalid state");
    chDbgAssert((startaddr + n <= fjsp->config->sector_size * fjsp->config->sector_num), "fjsStartSequentialRead(), #2",
            "invalid parameters");

    if (fjsSync(fjsp) != CH_SUCCESS)
        return CH_FAILED;

    /* Read operation in progress.*/
    fjsp->state = FLASH_READING;

    spiSelect(fjsp->config->spip);

    // send JEDEC FAST_READ command
    uint8_t out[] =
    {
        FLASH_JEDEC_FAST_READ,
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

    spiSend(fjsp->config->spip, 1, &out[5]); /* dummy byte for fast read */

    spiReceive(fjsp->config->spip, n, buffer);

    spiUnselect(fjsp->config->spip);

    if (flash_jedec_spi_write_disable(fjsp) != CH_SUCCESS)
        return CH_FAILED;

    return CH_SUCCESS;
}

/**
 * @brief   Write data within a single sector.
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
bool_t fjsWrite(FlashJedecSPIDriver* fjsp, uint32_t startaddr, uint32_t n, const uint8_t* buffer)
{
    chDbgCheck(fjsp != NULL, "fjsWrite");
    chDbgAssert(fjsp->state >= FLASH_READY, "fjsWrite(), #1",
            "invalid state");
    chDbgAssert((startaddr + n <= fjsp->config->sector_size * fjsp->config->sector_num), "fjsWrite(), #2",
            "invalid parameters");
    chDbgAssert((n <= fjsp->config->sector_size), "fjsWrite(), #3",
            "invalid parameters");

    if (fjsSync(fjsp) != CH_SUCCESS)
        return CH_FAILED;

    if (flash_jedec_spi_write_enable(fjsp) != CH_SUCCESS)
        return CH_FAILED;

    /* Write operation in progress.*/
    fjsp->state = FLASH_WRITING;

    spiSelect(fjsp->config->spip);

    // send JEDEC PP command
    uint8_t out[] =
    {
        FLASH_JEDEC_PP,
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

    spiSend(fjsp->config->spip, n, buffer);

    spiUnselect(fjsp->config->spip);

    if (flash_jedec_spi_write_disable(fjsp) != CH_SUCCESS)
        return CH_FAILED;

    return CH_SUCCESS;
}

/**
 * @brief   Erases a single sector.
 *
 * @param[in] fjsp      pointer to the @p FlashJedecSPIDriver object
 * @param[in] startaddr address within to be erased sector
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t fjsErase(FlashJedecSPIDriver* fjsp, uint32_t startaddr)
{
    chDbgCheck(fjsp != NULL, "fjsErase");
    chDbgAssert(fjsp->state >= FLASH_READY, "fjsErase(), #1",
            "invalid state");
    chDbgAssert((startaddr < fjsp->config->sector_size * fjsp->config->sector_num), "flash_jedec_spi_erase(), #1",
            "invalid parameters");

    /* Check if device supports erase command. */
    if (fjsp->config->sector_erase == 0x00)
        return CH_SUCCESS;

    if (fjsSync(fjsp) != CH_SUCCESS)
        return CH_FAILED;

    if (flash_jedec_spi_write_enable(fjsp) != CH_SUCCESS)
        return CH_FAILED;

    /* Erase operation in progress.*/
    fjsp->state = FLASH_ERASING;

    spiSelect(fjsp->config->spip);

    // send JEDEC ERASE command
    uint8_t out[] =
    {
        fjsp->config->sector_erase,
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

    if (flash_jedec_spi_write_disable(fjsp) != CH_SUCCESS)
        return CH_FAILED;

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
    chDbgAssert(fjsp->state >= FLASH_READY, "fjsSync(), #1",
            "invalid state");

    if (fjsp->state == FLASH_READY)
        return CH_SUCCESS;

    spiSelect(fjsp->config->spip);

    // send JEDEC RDSR command
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
    while ((in & 0x01) == 0x00)
    {
#ifdef FLASH_JEDEC_SPI_NICE_WAITING
        /* Trying to be nice with the other threads.*/
        chThdSleep(1);
#endif
        spiReceive(fjsp->config->spip, sizeof(in), &in);
    }

    spiUnselect(fjsp->config->spip);

    fjsp->state = FLASH_READY;

    return CH_SUCCESS;
}

/**
 * @brief   Returns media info.
 *
 * @param[in] fjsp      pointer to the @p FlashJedecSPIDriver object
 * @param[out] fdip     pointer to a @p FlashDeviceInfo structure
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @api
 */
bool_t fjsGetInfo(FlashJedecSPIDriver* fjsp, FlashDeviceInfo* fdip)
{
    chDbgCheck(fjsp != NULL, "fjsGetInfo");
    chDbgAssert(fjsp->state >= FLASH_READY, "fjsGetInfo(), #1",
            "invalid state");

    fdip->sector_num = fjsp->config->sector_num;
    fdip->sector_size = fjsp->config->sector_size;

    spiSelect(fjsp->config->spip);

    // send JEDEC RDID command
    static const uint8_t out[] =
    {
        0x9f,
    };
    spiSend(fjsp->config->spip, NELEMS(out), out);

    // skip JEDEC continuation id
    fdip->identification[0] = 0x7f;
    while (fdip->identification[0] == 0x7f)
        spiReceive(fjsp->config->spip, 1, fdip->identification);

    spiReceive(fjsp->config->spip, NELEMS(fdip->identification) - 1, fdip->identification + 1);

    spiUnselect(fjsp->config->spip);

    return CH_SUCCESS;
}

#endif /* HAL_USE_FLASH_JEDEC_SPI */

/** @} */
