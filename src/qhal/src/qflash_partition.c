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

    return flashRead(fpartp->config->flashp,
            fpartp->llfdi.sector_size * fpartp->config->sector_offset + startaddr,
            n,
            buffer);
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

    return flashSync(fpartp->config->flashp);
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

#endif /* HAL_USE_FLASH_PARTITION */

/** @} */
