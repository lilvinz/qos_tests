/**
 * @file    qnvm_partition.c
 * @brief   NVM partition driver code.
 *
 * @addtogroup NVM_PARTITION
 * @{
 */

#include "ch.h"
#include "qhal.h"

#include "static_assert.h"

#include <string.h>

#if HAL_USE_NVM_PARTITION || defined(__DOXYGEN__)

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
static const struct NVMPartitionDriverVMT nvm_partition_vmt =
{
    .read = (bool_t (*)(void*, uint32_t, uint32_t, uint8_t*))nvmpartRead,
    .write = (bool_t (*)(void*, uint32_t, uint32_t, const uint8_t*))nvmpartWrite,
    .erase = (bool_t (*)(void*, uint32_t, uint32_t))nvmpartErase,
    .sync = (bool_t (*)(void*))nvmpartSync,
#if NVM_PARTITION_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
    .acquire = (bool_t (*)(void*))nvmpartAcquireBus,
    .release = (bool_t (*)(void*))nvmpartReleaseBus,
#endif
    .get_info = (bool_t (*)(void*, NVMDeviceInfo *))nvmpartGetInfo,
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   NVM partition driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void nvmpartInit(void)
{
}

/**
 * @brief   Initializes an instance.
 *
 * @param[out] nvmpartp   pointer to the @p NVMPartitionDriver object
 *
 * @init
 */
void nvmpartObjectInit(NVMPartitionDriver* nvmpartp)
{
    nvmpartp->vmt = &nvm_partition_vmt;
    nvmpartp->state = NVM_STOP;
    nvmpartp->config = NULL;
#if NVM_PARTITION_USE_MUTUAL_EXCLUSION
#if CH_USE_MUTEXES
    chMtxInit(&nvmpartp->mutex);
#else
    chSemInit(&nvmpartp->semaphore, 1);
#endif
#endif /* NVM_PARTITION_USE_MUTUAL_EXCLUSION */
}

/**
 * @brief   Configures and activates the NVM partition.
 *
 * @param[in] nvmpartp      pointer to the @p NVMPartitionDriver object
 * @param[in] config        pointer to the @p NVMPartitionConfig object.
 *
 * @api
 */
void nvmpartStart(NVMPartitionDriver* nvmpartp,
        const NVMPartitionConfig* config)
{
    chDbgCheck((nvmpartp != NULL) && (config != NULL), "nvmpartStart");
    /* verify device status */
    chDbgAssert((nvmpartp->state == NVM_STOP) || (nvmpartp->state == NVM_READY),
            "nvmpartStart(), #1", "invalid state");

    nvmpartp->config = config;
    nvmGetInfo(nvmpartp->config->nvmp, &nvmpartp->llnvmdi);
    nvmpartp->state = NVM_READY;
}

/**
 * @brief   Disables the NVM partition.
 *
 * @param[in] nvmpartp      pointer to the @p NVMPartitionDriver object
 *
 * @api
 */
void nvmpartStop(NVMPartitionDriver* nvmpartp)
{
    chDbgCheck(nvmpartp != NULL, "nvmpartStop");
    /* verify device status */
    chDbgAssert((nvmpartp->state == NVM_STOP) || (nvmpartp->state == NVM_READY),
            "nvmpartStop(), #1", "invalid state");

    nvmpartp->state = NVM_STOP;
}

/**
 * @brief   Reads data crossing sector boundaries if required.
 *
 * @param[in] nvmpartp      pointer to the @p NVMPartitionDriver object
 * @param[in] startaddr     address to start reading from
 * @param[in] n             number of bytes to read
 * @param[in] buffer        pointer to data buffer
 *
 * @return                  The operation status.
 * @retval CH_SUCCESS       the operation succeeded.
 * @retval CH_FAILED        the operation failed.
 *
 * @api
 */
bool_t nvmpartRead(NVMPartitionDriver* nvmpartp, uint32_t startaddr,
        uint32_t n, uint8_t* buffer)
{
    chDbgCheck(nvmpartp != NULL, "nvmpartRead");
    /* verify device status */
    chDbgAssert(nvmpartp->state >= NVM_READY, "nvmpartRead(), #1",
            "invalid state");
    /* verify range is within partition size */
    chDbgAssert(
            (startaddr + n <= nvmpartp->llnvmdi.sector_size * nvmpartp->config->sector_num),
            "nvmpartRead(), #2", "invalid parameters");

    nvmpartp->state = NVM_READING;

    bool_t result = nvmRead(nvmpartp->config->nvmp,
            nvmpartp->llnvmdi.sector_size * nvmpartp->config->sector_offset + startaddr,
            n, buffer);

    nvmpartp->state = NVM_READY;

    return result;
}

/**
 * @brief   Writes data crossing sector boundaries if required.
 *
 * @param[in] nvmpartp      pointer to the @p NVMPartitionDriver object
 * @param[in] startaddr     address to start writing to
 * @param[in] n             number of bytes to write
 * @param[in] buffer        pointer to data buffer
 *
 * @return                  The operation status.
 * @retval CH_SUCCESS       the operation succeeded.
 * @retval CH_FAILED        the operation failed.
 *
 * @api
 */
bool_t nvmpartWrite(NVMPartitionDriver* nvmpartp, uint32_t startaddr,
       uint32_t n, const uint8_t* buffer)
{
    chDbgCheck(nvmpartp != NULL, "nvmpartWrite");
    /* verify device status */
    chDbgAssert(nvmpartp->state >= NVM_READY, "nvmpartWrite(), #1",
            "invalid state");
    /* verify range is within partition size */
    chDbgAssert(
            (startaddr + n <= nvmpartp->llnvmdi.sector_size * nvmpartp->config->sector_num),
            "nvmpartWrite(), #2", "invalid parameters");

    nvmpartp->state = NVM_WRITING;

    return nvmWrite(nvmpartp->config->nvmp,
            nvmpartp->llnvmdi.sector_size * nvmpartp->config->sector_offset + startaddr,
            n, buffer);
}

/**
 * @brief   Erases one or more sectors.
 *
 * @param[in] nvmpartp      pointer to the @p NVMPartitionDriver object
 * @param[in] startaddr     address within to be erased sector
 * @param[in] n             number of bytes to erase
 *
 * @return                  The operation status.
 * @retval CH_SUCCESS       the operation succeeded.
 * @retval CH_FAILED        the operation failed.
 *
 * @api
 */
bool_t nvmpartErase(NVMPartitionDriver* nvmpartp, uint32_t startaddr,
        uint32_t n)
{
    chDbgCheck(nvmpartp != NULL, "nvmpartErase");
    /* verify device status */
    chDbgAssert(nvmpartp->state >= NVM_READY, "nvmpartErase(), #1",
            "invalid state");
    /* verify range is within partition size */
    chDbgAssert((startaddr + n <= nvmpartp->llnvmdi.sector_size * nvmpartp->config->sector_num),
            "nvmpartRead(), #2", "invalid parameters");

    nvmpartp->state = NVM_ERASING;

    return nvmErase(nvmpartp->config->nvmp,
            nvmpartp->llnvmdi.sector_size * nvmpartp->config->sector_offset + startaddr,
            n);
}

/**
 * @brief   Waits for idle condition.
 *
 * @param[in] nvmpartp      pointer to the @p NVMPartitionDriver object
 *
 * @return                  The operation status.
 * @retval CH_SUCCESS       the operation succeeded.
 * @retval CH_FAILED        the operation failed.
 *
 * @api
 */
bool_t nvmpartSync(NVMPartitionDriver* nvmpartp)
{
    chDbgCheck(nvmpartp != NULL, "nvmpartSync");
    /* verify device status */
    chDbgAssert(nvmpartp->state >= NVM_READY, "nvmpartSync(), #1",
            "invalid state");

    bool_t result = nvmSync(nvmpartp->config->nvmp);

    nvmpartp->state = NVM_READY;

    return result;
}

/**
 * @brief   Returns media info.
 *
 * @param[in] nvmpartp      pointer to the @p NVMPartitionDriver object
 * @param[out] nvmdip         pointer to a @p NVMDeviceInfo structure
 *
 * @return                  The operation status.
 * @retval CH_SUCCESS       the operation succeeded.
 * @retval CH_FAILED        the operation failed.
 *
 * @api
 */
bool_t nvmpartGetInfo(NVMPartitionDriver* nvmpartp, NVMDeviceInfo* nvmdip)
{
    chDbgCheck(nvmpartp != NULL, "nvmpartGetInfo");
    /* verify device status */
    chDbgAssert(nvmpartp->state >= NVM_READY, "nvmpartGetInfo(), #1",
            "invalid state");

    nvmdip->sector_num = nvmpartp->config->sector_num;
    nvmdip->sector_size = nvmpartp->llnvmdi.sector_size;
    memcpy(nvmdip->identification, nvmpartp->llnvmdi.identification,
            sizeof(nvmdip->identification));

    return CH_SUCCESS;
}

#if NVM_PARTITION_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
/**
 * @brief   Gains exclusive access to the nvm partition device.
 * @details This function tries to gain ownership to the nvm partition device,
 *          if the device is already being used then the invoking thread
 *          is queued.
 * @pre     In order to use this function the option
 *          @p NVM_PARTITION_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] nvmpartp      pointer to the @p NVMPartitionDriver object
 *
 * @api
 */
void nvmpartAcquireBus(NVMPartitionDriver* nvmpartp)
{
    chDbgCheck(nvmpartp != NULL, "nvmpartAcquireBus");

#if CH_USE_MUTEXES
    chMtxLock(&nvmpartp->mutex);
#elif CH_USE_SEMAPHORES
    chSemWait(&nvmpartp->semaphore);
#endif

    /* Lock the underlying device as well */
    nvmAcquire(nvmpartp->config->nvmp);
}

/**
 * @brief   Releases exclusive access to the nvm partition device.
 * @pre     In order to use this function the option
 *          @p NVM_PARTITION_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] nvmpartp      pointer to the @p NVMPartitionDriver object
 *
 * @api
 */
void nvmpartReleaseBus(NVMPartitionDriver* nvmpartp)
{
    chDbgCheck(nvmpartp != NULL, "nvmpartReleaseBus");

#if CH_USE_MUTEXES
    (void)nvmpartp;
    chMtxUnlock();
#elif CH_USE_SEMAPHORES
    chSemSignal(&nvmpartp->semaphore);
#endif

    /* Release the underlying device as well */
    nvmRelease(nvmpartp->config->nvmp);
}
#endif /* NVM_PARTITION_USE_MUTUAL_EXCLUSION */

#endif /* HAL_USE_NVM_PARTITION */

/** @} */
