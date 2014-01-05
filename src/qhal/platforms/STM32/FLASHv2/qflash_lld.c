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

/* Missing register bit definitions from ST's headers */
#define FLASH_SR_OPERR                       ((uint32_t)0x00000002)
#define FLASH_CR_ERRIE                       ((uint32_t)0x02000000)

/**
 * @name    FLASH status flags
 * @{
 */
/**< @brief No pending conditions. */
#define FLASH_NO_ERROR                  0
/**< @brief Write protection error happened. */
#define FLASH_PRG_WRITEPROTECT_ERROR    4
/**< @brief Programming alignment error happened. */
#define FLASH_PRG_ALIGNMENT_ERROR       8
/**< @brief Programming parallelism error happened. */
#define FLASH_PRG_PARALLELISM_ERROR     16
/**< @brief Programming sequence error happened. */
#define FLASH_PRG_SEQUENCE_ERROR        32
/** @} */

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

static const struct
{
    uint32_t size;
    uint16_t sector_bits;
} flash_sector_infos[] =
{
    [0] =
    {
        .size = 16 * 1024,
        .sector_bits = 0x0000,
    },
    [1] =
    {
        .size = 16 * 1024,
        .sector_bits = 0x0008,
    },
    [2] =
    {
        .size = 16 * 1024,
        .sector_bits = 0x0010,
    },
    [3] =
    {
        .size = 16 * 1024,
        .sector_bits = 0x0018,
    },
    [4] =
    {
        .size = 64 * 1024,
        .sector_bits = 0x0020,
    },
    [5] =
    {
        .size = 128 * 1024,
        .sector_bits = 0x0028,
    },
    [6] =
    {
        .size = 128 * 1024,
        .sector_bits = 0x0030,
    },
    [7] =
    {
        .size = 128 * 1024,
        .sector_bits = 0x0038,
    },
    [8] =
    {
        .size = 128 * 1024,
        .sector_bits = 0x0040,
    },
    [9] =
    {
        .size = 128 * 1024,
        .sector_bits = 0x0048,
    },
    [10] =
    {
        .size = 128 * 1024,
        .sector_bits = 0x0050,
    },
    [11] =
    {
        .size = 128 * 1024,
        .sector_bits = 0x0058,
    },
    [12] =
    {
        .size = 16 * 1024,
        .sector_bits = 0x0080,
    },
    [13] =
    {
        .size = 16 * 1024,
        .sector_bits = 0x0088,
    },
    [14] =
    {
        .size = 16 * 1024,
        .sector_bits = 0x0090,
    },
    [15] =
    {
        .size = 16 * 1024,
        .sector_bits = 0x0098,
    },
    [16] =
    {
        .size = 64 * 1024,
        .sector_bits = 0x00a0,
    },
    [17] =
    {
        .size = 128 * 1024,
        .sector_bits = 0x00a8,
    },
    [18] =
    {
        .size = 128 * 1024,
        .sector_bits = 0x00b0,
    },
    [19] =
    {
        .size = 128 * 1024,
        .sector_bits = 0x00b8,
    },
    [20] =
    {
        .size = 128 * 1024,
        .sector_bits = 0x00c0,
    },
    [21] =
    {
        .size = 128 * 1024,
        .sector_bits = 0x00c8,
    },
    [22] =
    {
        .size = 128 * 1024,
        .sector_bits = 0x00d0,
    },
    [23] =
    {
        .size = 128 * 1024,
        .sector_bits = 0x00d8,
    },
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
static void flash_lld_cr_lock(FLASHDriver* flashp)
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
static void flash_lld_cr_unlock(FLASHDriver* flashp)
{
    static const uint32_t FLASH_KEY1 = 0x45670123;
    static const uint32_t FLASH_KEY2 = 0xCDEF89AB;

    if ((flashp->flash->CR & FLASH_CR_LOCK) != 0)
    {
        flashp->flash->KEYR = FLASH_KEY1;
        flashp->flash->KEYR = FLASH_KEY2;
    }
}

#if 0
/**
 * @brief   Locks access to FLASH OPTCR peripheral register.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 *
 * @notapi
 */
static void flash_lld_optcr_lock(FLASHDriver* flashp)
{
    flashp->flash->OPTCR |= FLASH_OPTCR_OPTLOCK;
}

/**
 * @brief   Unlocks access to FLASH OPTCR peripheral register.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 *
 * @notapi
 */
static void flash_lld_optcr_unlock(FLASHDriver* flashp)
{
    static const uint32_t FLASH_OPT_KEY1 = 0x08192A3B;
    static const uint32_t FLASH_OPT_KEY2 = 0x4C5D6E7F;

    if ((flashp->flash->OPTCR & FLASH_OPTCR_OPTLOCK) != 0)
    {
        flashp->flash->KEYR = FLASH_OPT_KEY1;
        flashp->flash->KEYR = FLASH_OPT_KEY2;
    }
}
#endif

/**
 * @brief   Returns permitted programming size depending on supply voltage.
 *
 * @notapi
 */
static __inline__ flash_program_size_e flash_lld_get_psize(void)
{
#if STM32_VDD >= 270
    return PSIZE_4;
#elif STM32_VDD >= 210
    return PSIZE_2;
#else
    return PSIZE_1;
#endif
}

/**
 * @brief   Programs a 64bit doubleword of data to flash memory
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 * @param[in] addr      absolute address
 * @param[in] data      data to program
 *
 * @notapi
 */
static void flash_lld_program_64(FLASHDriver* flashp, uint32_t addr, uint64_t data)
{
    flash_lld_sync(flashp);

    flash_lld_cr_unlock(flashp);

    /* Set psize. */
    flashp->flash->CR &= ~(FLASH_CR_PSIZE_1 | FLASH_CR_PSIZE_0);
    flashp->flash->CR |= PSIZE_8;

    /* Set operation to perform. */
    flashp->flash->CR &= ~(FLASH_CR_MER | FLASH_CR_SER | FLASH_CR_PG);
    flashp->flash->CR |= FLASH_CR_PG;

    flash_lld_cr_lock(flashp);

    *(__O uint64_t*)addr = data;
}

/**
 * @brief   Programs a 32bit word of data to flash memory
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 * @param[in] addr      absolute address
 * @param[in] data      data to program
 *
 * @notapi
 */
static void flash_lld_program_32(FLASHDriver* flashp, uint32_t addr, uint32_t data)
{
    flash_lld_sync(flashp);

    flash_lld_cr_unlock(flashp);

    /* Set psize. */
    flashp->flash->CR &= ~(FLASH_CR_PSIZE_1 | FLASH_CR_PSIZE_0);
    flashp->flash->CR |= PSIZE_4;

    /* Set operation to perform. */
    flashp->flash->CR &= ~(FLASH_CR_MER | FLASH_CR_SER | FLASH_CR_PG);
    flashp->flash->CR |= FLASH_CR_PG;

    flash_lld_cr_lock(flashp);

    *(__O uint32_t*)addr = data;
}

/**
 * @brief   Programs a 16bit halfword of data to flash memory
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 * @param[in] addr      absolute address
 * @param[in] data      data to program
 *
 * @notapi
 */
static void flash_lld_program_16(FLASHDriver* flashp, uint32_t addr, uint16_t data)
{
    flash_lld_sync(flashp);

    flash_lld_cr_unlock(flashp);

    /* Set psize. */
    flashp->flash->CR &= ~(FLASH_CR_PSIZE_1 | FLASH_CR_PSIZE_0);
    flashp->flash->CR |= PSIZE_2;

    /* Set operation to perform. */
    flashp->flash->CR &= ~(FLASH_CR_MER | FLASH_CR_SER | FLASH_CR_PG);
    flashp->flash->CR |= FLASH_CR_PG;

    flash_lld_cr_lock(flashp);

    *(__O uint16_t*)addr = data;
}

/**
 * @brief   Programs a 8bit byte of data to flash memory
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 * @param[in] addr      absolute address
 * @param[in] data      data to program
 *
 * @notapi
 */
static void flash_lld_program_8(FLASHDriver* flashp, uint32_t addr, uint8_t data)
{
    flash_lld_sync(flashp);

    flash_lld_cr_unlock(flashp);

    /* Set psize. */
    flashp->flash->CR &= ~(FLASH_CR_PSIZE_1 | FLASH_CR_PSIZE_0);
    flashp->flash->CR |= PSIZE_1;

    /* Set operation to perform. */
    flashp->flash->CR &= ~(FLASH_CR_MER | FLASH_CR_SER | FLASH_CR_PG);
    flashp->flash->CR |= FLASH_CR_PG;

    flash_lld_cr_lock(flashp);

    *(__O uint8_t*)addr = data;
}

/**
 * @brief   FLASH common service routine.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 */
static void serve_flash_irq(FLASHDriver* flashp)
{
    uint32_t sr;
    FLASH_TypeDef *f = flashp->flash;

    sr = f->SR;
    f->SR = FLASH_SR_PGSERR | FLASH_SR_PGPERR
            | FLASH_SR_PGAERR | FLASH_SR_WRPERR | FLASH_SR_OPERR
            | FLASH_SR_EOP;
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

            /* Clear all possibly pending flags. */
            flashp->flash->SR = FLASH_SR_PGSERR | FLASH_SR_PGPERR
                    | FLASH_SR_PGAERR | FLASH_SR_WRPERR | FLASH_SR_OPERR
                    | FLASH_SR_EOP;

            flash_lld_cr_unlock(flashp);
            flashp->flash->CR = FLASH_CR_EOPIE | FLASH_CR_ERRIE;
            flash_lld_cr_lock(flashp);

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
            flash_lld_cr_unlock(flashp);
            flashp->flash->CR = 0;
            flash_lld_cr_lock(flashp);

            /* Clear all possibly pending flags. */
            flashp->flash->SR = FLASH_SR_PGSERR | FLASH_SR_PGPERR
                    | FLASH_SR_PGAERR | FLASH_SR_WRPERR | FLASH_SR_SOP
                    | FLASH_SR_EOP;

            nvicDisableVector(STM32_FLASH_NUMBER);
            return;
        }
    }
}

/**
 * @brief   Reads data from flash peripheral.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 * @param[in] startaddr relative address to start of flash
 * @param[in] n         number of bytes to read
 * @param[out] buffer   pointer to receiving data buffer
 *
 * @notapi
 */
void flash_lld_read(FLASHDriver* flashp, uint32_t startaddr, uint32_t n, uint8_t* buffer)
{
    memcpy(buffer, (uint8_t*)(0x08000000 + startaddr), n);
}

/**
 * @brief   Writes data to flash peripheral.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 * @param[in] startaddr relative address to start of flash
 * @param[in] n         number of bytes to read
 * @param[in] buffer    pointer to data buffer
 *
 * @notapi
 */
void flash_lld_write(FLASHDriver* flashp, uint32_t startaddr, uint32_t n, const uint8_t* buffer)
{
    uint32_t addr = 0x08000000 + startaddr;
    uint32_t remaining = n;
    while (remaining > 0)
    {
        switch (flash_lld_get_psize())
        {
        case PSIZE_8:
            if (addr % 8 == 0 && remaining >= 8)
            {
                flash_lld_program_64(flashp, addr, *(uint64_t*)addr);
                addr += 8;
                remaining -= 8;
                break;
            }
        case PSIZE_4:
            if (addr % 4 == 0 && remaining >= 4)
            {
                flash_lld_program_32(flashp, addr, *(uint32_t*)addr);
                addr += 4;
                remaining -= 4;
                break;
            }
        case PSIZE_2:
            if (addr % 2 == 0 && remaining >= 2)
            {
                flash_lld_program_16(flashp, addr, *(uint16_t*)addr);
                addr += 2;
                remaining -= 2;
                break;
            }
        case PSIZE_1:
            if (remaining >= 1)
            {
                flash_lld_program_8(flashp, addr, *(uint8_t*)addr);
                addr += 1;
                remaining -= 1;
                break;
            }
        }
    }
}

/**
 * @brief   Erases one flash sector.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 * @param[in] startaddr relative address to start of flash
 *
 * @notapi
 */
void flash_lld_erase(FLASHDriver* flashp, uint32_t startaddr)
{
    flash_lld_cr_unlock(flashp);

    /* Set psize. */
    flash_program_size_e psize = flash_lld_get_psize();
    flashp->flash->CR &= ~(FLASH_CR_PSIZE_1 | FLASH_CR_PSIZE_0);
    flashp->flash->CR |= psize;

    /* Set sector. */
    FLASHSectorInfo info;
    flash_lld_addr_to_sector(startaddr, &info);
    flashp->flash->CR &= ~(FLASH_CR_SNB_3 | FLASH_CR_SNB_2 | FLASH_CR_SNB_1 |
            FLASH_CR_SNB_0);
    flashp->flash->CR |= flash_sector_infos[info.sector].sector_bits;

    /* Set operation to perform. */
    flashp->flash->CR &= ~(FLASH_CR_MER | FLASH_CR_SER | FLASH_CR_PG);
    flashp->flash->CR |= FLASH_CR_SER;

    /* Start operation. */
    flashp->flash->CR |= FLASH_CR_STRT;

    flash_lld_cr_lock(flashp);
}

/**
 * @brief   Erases all flash sectors.
 *
 * @param[in] flashp    pointer to the @p FLASHDriver object
 *
 * @notapi
 */
void flash_lld_masserase(FLASHDriver* flashp)
{
    flash_lld_cr_unlock(flashp);

    /* Set psize. */
    flash_program_size_e psize = flash_lld_get_psize();
    flashp->flash->CR &= ~(FLASH_CR_PSIZE_1 | FLASH_CR_PSIZE_0);
    flashp->flash->CR |= psize;

    /* Set operation to perform. */
    flashp->flash->CR &= ~(FLASH_CR_MER | FLASH_CR_SER | FLASH_CR_PG);
    flashp->flash->CR |= FLASH_CR_MER;

    /* Start operation. */
    flashp->flash->CR |= FLASH_CR_STRT;

    flash_lld_cr_lock(flashp);
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
    FLASHSectorInfo info =
    {
        .sector = 0,
        .origin = 0,
        .size = 0,
    };

    for (uint8_t i = 0; i < NELEMS(flash_sector_infos); ++i)
    {
        info.size = flash_sector_infos[i].size;
        if (addr >= info.origin
                && addr <= info.origin + info.size)
        {
            if (sinfo != NULL)
            {
                sinfo->sector = i;
                sinfo->origin = info.origin;
                sinfo->size = info.size;
            }
            return CH_SUCCESS;
        }
        info.origin += info.size;

        /* Test against total flash size. */
        if (info.origin >= (uint32_t)(*((__I uint16_t*)(0x1fff7a22))) * 1024)
            return CH_FAILED;
    }
    return CH_FAILED;
}

#endif /* HAL_USE_FLASH */

/** @} */
