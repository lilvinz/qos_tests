/**
 * @file    STM32/FLASHv2/qflash.c
 * @brief   STM32 low level FLASH driver header.
 *
 * @addtogroup FLASH
 * @{
 */

#ifndef _FLASH_LLD_H_
#define _FLASH_LLD_H_

#if HAL_USE_FLASH || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

#if 0

/**
  * @brief FLASH Registers
  */
typedef struct
{
  __IO uint32_t ACR;      /*!< FLASH access control register, Address offset: 0x00 */
  __IO uint32_t KEYR;     /*!< FLASH key register,            Address offset: 0x04 */
  __IO uint32_t OPTKEYR;  /*!< FLASH option key register,     Address offset: 0x08 */
  __IO uint32_t SR;       /*!< FLASH status register,         Address offset: 0x0C */
  __IO uint32_t CR;       /*!< FLASH control register,        Address offset: 0x10 */
  __IO uint32_t OPTCR;    /*!< FLASH option control register, Address offset: 0x14 */
} FLASH_TypeDef;


/******************************************************************************/
/*                                                                            */
/*                                    FLASH                                   */
/*                                                                            */
/******************************************************************************/
/*******************  Bits definition for FLASH_ACR register  *****************/
#define FLASH_ACR_LATENCY                    ((uint32_t)0x00000007)
#define FLASH_ACR_LATENCY_0WS                ((uint32_t)0x00000000)
#define FLASH_ACR_LATENCY_1WS                ((uint32_t)0x00000001)
#define FLASH_ACR_LATENCY_2WS                ((uint32_t)0x00000002)
#define FLASH_ACR_LATENCY_3WS                ((uint32_t)0x00000003)
#define FLASH_ACR_LATENCY_4WS                ((uint32_t)0x00000004)
#define FLASH_ACR_LATENCY_5WS                ((uint32_t)0x00000005)
#define FLASH_ACR_LATENCY_6WS                ((uint32_t)0x00000006)
#define FLASH_ACR_LATENCY_7WS                ((uint32_t)0x00000007)

#define FLASH_ACR_PRFTEN                     ((uint32_t)0x00000100)
#define FLASH_ACR_ICEN                       ((uint32_t)0x00000200)
#define FLASH_ACR_DCEN                       ((uint32_t)0x00000400)
#define FLASH_ACR_ICRST                      ((uint32_t)0x00000800)
#define FLASH_ACR_DCRST                      ((uint32_t)0x00001000)
#define FLASH_ACR_BYTE0_ADDRESS              ((uint32_t)0x40023C00)
#define FLASH_ACR_BYTE2_ADDRESS              ((uint32_t)0x40023C03)

/*******************  Bits definition for FLASH_SR register  ******************/
#define FLASH_SR_EOP                         ((uint32_t)0x00000001)
#define FLASH_SR_SOP                         ((uint32_t)0x00000002)
#define FLASH_SR_WRPERR                      ((uint32_t)0x00000010)
#define FLASH_SR_PGAERR                      ((uint32_t)0x00000020)
#define FLASH_SR_PGPERR                      ((uint32_t)0x00000040)
#define FLASH_SR_PGSERR                      ((uint32_t)0x00000080)
#define FLASH_SR_BSY                         ((uint32_t)0x00010000)

/*******************  Bits definition for FLASH_CR register  ******************/
#define FLASH_CR_PG                          ((uint32_t)0x00000001)
#define FLASH_CR_SER                         ((uint32_t)0x00000002)
#define FLASH_CR_MER                         ((uint32_t)0x00000004)
#define FLASH_CR_SNB_0                       ((uint32_t)0x00000008)
#define FLASH_CR_SNB_1                       ((uint32_t)0x00000010)
#define FLASH_CR_SNB_2                       ((uint32_t)0x00000020)
#define FLASH_CR_SNB_3                       ((uint32_t)0x00000040)
#define FLASH_CR_PSIZE_0                     ((uint32_t)0x00000100)
#define FLASH_CR_PSIZE_1                     ((uint32_t)0x00000200)
#define FLASH_CR_STRT                        ((uint32_t)0x00010000)
#define FLASH_CR_EOPIE                       ((uint32_t)0x01000000)
#define FLASH_CR_LOCK                        ((uint32_t)0x80000000)

/*******************  Bits definition for FLASH_OPTCR register  ***************/
#define FLASH_OPTCR_OPTLOCK                  ((uint32_t)0x00000001)
#define FLASH_OPTCR_OPTSTRT                  ((uint32_t)0x00000002)
#define FLASH_OPTCR_BOR_LEV_0                ((uint32_t)0x00000004)
#define FLASH_OPTCR_BOR_LEV_1                ((uint32_t)0x00000008)
#define FLASH_OPTCR_BOR_LEV                  ((uint32_t)0x0000000C)
#define FLASH_OPTCR_WDG_SW                   ((uint32_t)0x00000020)
#define FLASH_OPTCR_nRST_STOP                ((uint32_t)0x00000040)
#define FLASH_OPTCR_nRST_STDBY               ((uint32_t)0x00000080)
#define FLASH_OPTCR_RDP_0                    ((uint32_t)0x00000100)
#define FLASH_OPTCR_RDP_1                    ((uint32_t)0x00000200)
#define FLASH_OPTCR_RDP_2                    ((uint32_t)0x00000400)
#define FLASH_OPTCR_RDP_3                    ((uint32_t)0x00000800)
#define FLASH_OPTCR_RDP_4                    ((uint32_t)0x00001000)
#define FLASH_OPTCR_RDP_5                    ((uint32_t)0x00002000)
#define FLASH_OPTCR_RDP_6                    ((uint32_t)0x00004000)
#define FLASH_OPTCR_RDP_7                    ((uint32_t)0x00008000)
#define FLASH_OPTCR_nWRP_0                   ((uint32_t)0x00010000)
#define FLASH_OPTCR_nWRP_1                   ((uint32_t)0x00020000)
#define FLASH_OPTCR_nWRP_2                   ((uint32_t)0x00040000)
#define FLASH_OPTCR_nWRP_3                   ((uint32_t)0x00080000)
#define FLASH_OPTCR_nWRP_4                   ((uint32_t)0x00100000)
#define FLASH_OPTCR_nWRP_5                   ((uint32_t)0x00200000)
#define FLASH_OPTCR_nWRP_6                   ((uint32_t)0x00400000)
#define FLASH_OPTCR_nWRP_7                   ((uint32_t)0x00800000)
#define FLASH_OPTCR_nWRP_8                   ((uint32_t)0x01000000)
#define FLASH_OPTCR_nWRP_9                   ((uint32_t)0x02000000)
#define FLASH_OPTCR_nWRP_10                  ((uint32_t)0x04000000)
#define FLASH_OPTCR_nWRP_11                  ((uint32_t)0x08000000)

#endif

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Flash internal driver configuration structure.
 */
typedef struct
{
    /**
     * @brief Pointer to flash peripheral base.
     */
    FLASH_TypeDef* flashp;
} FLASHConfig;

/**
 * @brief   @p FLASHDriver specific methods.
 */
#define _flash_driver_methods                                                 \
    _base_nvm_device_methods

/**
 * @extends BaseFlashDeviceVMT
 *
 * @brief   @p FLASHDriver virtual methods table.
 */
struct FLASHDriverVMT
{
    _flash_driver_methods
};

/**
 * @extends BaseFlashDevice
 *
 * @brief   Structure representing a FLASH driver.
 */
typedef struct
{
    /**
    * @brief Virtual Methods Table.
    */
    const struct FLASHDriverVMT* vmt;
    _base_nvm_device_data
    /**
    * @brief Current configuration data.
    */
    const FLASHConfig* config;
    /**
     * @brief Pointer to the FLASH registers block.
     */
    FLASH_TypeDef* flash;
#if FLASH_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
#if CH_USE_MUTEXES || defined(__DOXYGEN__)
    /**
     * @brief Mutex protecting the device.
     */
    Mutex mutex;
#elif CH_USE_SEMAPHORES
    Semaphore semaphore;
#endif
#endif /* FLASH_USE_MUTUAL_EXCLUSION */
} FLASHDriver;

typedef struct
{
    uint32_t origin;
    uint32_t size;
} FLASHSectorInfo;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern FLASHDriver FLASHD;

#ifdef __cplusplus
extern "C" {
#endif
    void flash_lld_init(void);
    void flash_lld_start(FLASHDriver* flashp);
    void flash_lld_stop(FLASHDriver* flashp);
    void flash_lld_read(FLASHDriver* flashp, uint32_t startaddr, uint32_t n, uint8_t* buffer);
    void flash_lld_write(FLASHDriver* flashp, uint32_t startaddr, uint32_t n, const uint8_t* buffer);
    void flash_lld_erase(FLASHDriver* flashp, uint32_t startaddr, uint32_t n);
    void flash_lld_masserase(FLASHDriver* flashp);
    void flash_lld_sync(FLASHDriver* flashp);
    void flash_lld_get_info(FLASHDriver* flashp, NVMDeviceInfo* nvmdip);
    void flash_lld_write_unprotect(FLASHDriver* flashp);
    void flash_lld_write_protect(FLASHDriver* flashp);
    bool_t flash_lld_addr_to_sector(uint32_t addr, FLASHSectorInfo* sinfo);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_FLASH */

#endif /* _FLASH_LLD_H_ */

/** @} */
