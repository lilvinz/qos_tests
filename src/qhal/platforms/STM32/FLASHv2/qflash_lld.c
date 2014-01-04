/**
 * @file    STM32/FLASHv2/qflash.c
 * @brief   STM32 low level FLASH driver code.
 *
 * @addtogroup FLASH
 * @{
 */

#include "ch.h"
#include "qhal.h"

#if HAL_USE_FLASH || defined(__DOXYGEN__)

#include "static_assert.h"

#include <string.h>

/*
 * @todo    - add error detection and handling
 *          - add option bytes programming
 *          - add readout protection programming and clearing
 */

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

static const uint16_t RDP_KEY = 0x00A5;
static const uint32_t FLASH_OPT_KEY1 = 0x08192A3B;
static const uint32_t FLASH_OPT_KEY2 = 0x4C5D6E7F;

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*
 * @brief FLASH driver identifier.
 */
FLASHDriver FLASHD;

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

typedef enum
{
    PSIZE_1 = (uint32_t)0,
    PSIZE_2 = FLASH_CR_PSIZE_0,
    PSIZE_4 = FLASH_CR_PSIZE_1,
    PSIZE_8 = FLASH_CR_PSIZE_1 | FLASH_CR_PSIZE_0,
} flash_program_size_e;

static const uint32_t flash_sector_sizes[] =
{
    [0] = 16 * 1024,
    [1] = 16 * 1024,
    [2] = 16 * 1024,
    [3] = 16 * 1024,
    [4] = 64 * 1024,
    [5] = 128 * 1024,
    [6] = 128 * 1024,
    [7] = 128 * 1024,
    [8] = 128 * 1024,
    [9] = 128 * 1024,
    [10] = 128 * 1024,
    [11] = 128 * 1024,
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Locks access to FLASH CR peripheral register.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 *
 * @notapi
 */
void flash_lld_cr_lock(FLASHDriver* flashp)
{
    flashp->flash->CR |= FLASH_CR_LOCK;
}

/**
 * @brief   Unlocks access to FLASH CR peripheral register.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 *
 * @notapi
 */
void flash_lld_cr_unlock(FLASHDriver* flashp)
{
    static const uint32_t FLASH_KEY1 = 0x45670123;
    static const uint32_t FLASH_KEY2 = 0xCDEF89AB;

    if ((flashp->flash->CR & FLASH_CR_LOCK) != 0)
    {
        flashp->flash->KEYR = FLASH_KEY1;
        flashp->flash->KEYR = FLASH_KEY2;
    }
}

/**
 * @brief   Unlocks access to FLASH CR peripheral register.
 *
 * @notapi
 */
static flash_program_size_e flash_lld_get_psize(void)
{
    if (STM32_VDD >= 270)
        return PSIZE_4;
    if (STM32_VDD >= 210)
        return PSIZE_2;
    return PSIZE_1;
}

/**
 * @brief   FLASH common service routine.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 */
static void serve_flash_irq(FLASHDriver* flashp)
{
#if 0
    uint16_t sr;
    USART_TypeDef *u = flashp->usart;

    sr = u->SR; /* SR reset step 1.*/
    (void)u->DR; /* SR reset step 2.*/
    if (sr & (USART_SR_LBD | USART_SR_ORE | USART_SR_NE |
                    USART_SR_FE | USART_SR_PE))
    {
        u->SR = ~USART_SR_LBD;
        if (flashp->config->rxerr_cb != NULL)
        flashp->config->rxerr_cb(flashp, translate_errors(sr));
    }
    if (sr & USART_SR_TC)
    {
        u->SR = ~USART_SR_TC;

        /* End of transmission, a callback is generated.*/
        if (flashp->config->txend2_cb != NULL)
        flashp->config->txend2_cb(flashp);
    }
#endif
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/**
 * @brief   FLASH IRQ handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(STM32_FLASH_HANDLER)
{
    CH_IRQ_PROLOGUE();

    serve_flash_irq(&FLASHD);

    CH_IRQ_EPILOGUE();
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level UART driver initialization.
 *
 * @notapi
 */
void flash_lld_init(void)
{
    flashObjectInit(&FLASHD);
    FLASHD.flash = FLASH;
}

/**
 * @brief   Configures and activates the FLASH peripheral.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 *
 * @notapi
 */
void flash_lld_start(FLASHDriver* flashp)
{
    if (flashp->state == NVM_STOP)
    {
        if (flashp == &FLASHD)
        {
            nvicEnableVector(STM32_FLASH_NUMBER,
                    CORTEX_PRIORITY_MASK(STM32_FLASH_IRQ_PRIORITY));
            return;
        }
    }
}

/**
 * @brief   Deactivates the FLASH peripheral.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 *
 * @notapi
 */
void flash_lld_stop(FLASHDriver* flashp)
{
    if (flashp->state == NVM_READY)
    {
        if (flashp == &FLASHD)
        {
            nvicDisableVector(STM32_FLASH_NUMBER);
            return;
        }
    }
}

/**
 * @brief   Waits for FLASH peripheral to become idle.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 *
 * @notapi
 */
void flash_lld_read(FLASHDriver* flashp, uint32_t startaddr, uint32_t n, uint8_t* buffer)
{
    memcpy(buffer, (uint8_t*)0x08000000 + startaddr, n);
}

/**
 * @brief   Waits for FLASH peripheral to become idle.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 *
 * @notapi
 */
void flash_lld_write(FLASHDriver* flashp, uint32_t startaddr, uint32_t n, const uint8_t* buffer)
{

}

/**
 * @brief   Waits for FLASH peripheral to become idle.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 *
 * @notapi
 */
void flash_lld_erase(FLASHDriver* flashp, uint32_t startaddr, uint32_t n)
{
    flash_program_size_e psize = flash_lld_get_psize();

    flashp->flash->CR &= FLASH_CR_PSIZE_1 | FLASH_CR_PSIZE_0;
    flashp->flash->CR |= psize;
    flashp->flash->CR |= FLASH_CR_MER;
    flashp->flash->CR |= FLASH_CR_STRT;

    flash_lld_sync(flashp);

    /* when the erase operation is completed, disable the MER Bit */
    flashp->flash->CR &= (~FLASH_CR_MER);
}

/**
 * @brief   Waits for FLASH peripheral to become idle.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 *
 * @notapi
 */
void flash_lld_masserase(FLASHDriver* flashp)
{
    flash_program_size_e psize = flash_lld_get_psize();

    flashp->flash->CR &= FLASH_CR_PSIZE_1 | FLASH_CR_PSIZE_0;
    flashp->flash->CR |= psize;
    flashp->flash->CR |= FLASH_CR_MER;
    flashp->flash->CR |= FLASH_CR_STRT;

    flash_lld_sync(flashp);

    /* when the erase operation is completed, disable the MER Bit */
    flashp->flash->CR &= (~FLASH_CR_MER);
}

/**
 * @brief   Waits for FLASH peripheral to become idle.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 *
 * @notapi
 */
void flash_lld_sync(FLASHDriver* flashp)
{
    while (flashp->flash->SR & FLASH_SR_BSY)
    {
#ifdef FLASH_NICE_WAITING
        /* Trying to be nice with the other threads.*/
        chThdSleep(1);
#endif
    }
}

/**
 * @brief   Returns chip information.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 *
 * @notapi
 */
void flash_lld_get_info(FLASHDriver* flashp, NVMDeviceInfo* nvmdip)
{
}

/**
 * @brief   Disables whole chip write protection.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 *
 * @notapi
 */
void flash_lld_write_unprotect(FLASHDriver* flashp)
{
}

/**
 * @brief   Enables whole chip write protection.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 *
 * @notapi
 */
void flash_lld_write_protect(FLASHDriver* flashp)
{
}

/**
 * @brief   Converts address to sector.
 *
 * @param[in] addr      address to convert to sector
 * @param[out] sinfo    pointer to variable receiving sector info or NULL
 *
 * @return              The operation status.
 * @retval CH_SUCCESS   the operation succeeded.
 * @retval CH_FAILED    the operation failed.
 *
 * @notapi
 */
bool_t flash_lld_addr_to_sector(uint32_t addr, FLASHSectorInfo* sinfo)
{
    FLASHSectorInfo sector =
    {
        .origin = 0,
        .size = 0,
    };

    for (uint8_t i = 0; i < NELEMS(flash_sector_sizes); ++i)
    {
        sector.size = flash_sector_sizes[i];
        if (addr >= sector.origin
                && addr <= sector.origin + sector.size)
        {
            if (sinfo != NULL)
            {
                sinfo->origin = sector.origin;
                sinfo->size = sector.size;
            }
            return CH_SUCCESS;
        }
        sector.origin += sector.size;
    }
    return CH_FAILED;
}

#endif /* HAL_USE_FLASH */

/** @} */
