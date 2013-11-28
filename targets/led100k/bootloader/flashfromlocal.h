#ifndef FLASHFROMLOCAL_H
#define FLASHFROMLOCAL_H


#ifdef STM32F10X_LD_VL
  #define FLASH_PAGE_SIZE    ((uint16_t)0x400)
#elif defined STM32F10X_LD
  #define FLASH_PAGE_SIZE    ((uint16_t)0x400)
#elif defined STM32F10X_MD_VL
  #define FLASH_PAGE_SIZE    ((uint16_t)0x400)
#elif defined STM32F10X_MD
  #define FLASH_PAGE_SIZE    ((uint16_t)0x400)
#elif defined STM32F10X_HD

  #define FLASH_PAGE_SIZE    ((uint16_t)0x800)
#elif defined STM32F10X_CL
  #define FLASH_PAGE_SIZE    ((uint16_t)0x800)
#elif defined STM32F10X_XL
  #define FLASH_PAGE_SIZE    ((uint16_t)0x800)
#endif /* STM32F10X_LD_VL */

#define BLOCK_SIZE      256

#define FFL_ERROR_WAIT_TIME_MS  20

#define FFL_MAX_ERROR_COUNT     20

#define FFL_MAX_RETRIES         4

#define EraseTimeout          ((uint32_t)0x000B0000)

bool FFL_ResetRetryCounter(void);
bool FFL_GetRetryCounter(uint8_t *pdata);
bool FFL_DecrementRetryCounter(void);
bool FFL_Start(void);

#endif
