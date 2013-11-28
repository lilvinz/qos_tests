/**************************************************************************//**
\file   ofdMemoryDriver.h

\brief  The implementation interface of external flash.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    31/07/09 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/
#ifndef _OFDMEMORYDRIVER_H
#define _OFDMEMORYDRIVER_H

#ifdef _OTAU_
/******************************************************************************
                   Includes section
******************************************************************************/
#include <gpio.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#if defined(AT25F2048)

  #if !defined(ATMEGA1281) && !defined(ATMEGA128RFA1)
    #error 'at25f2048 is supported only for atmega1281/128rfa1.'
  #endif

  // flash instruction format (see at25f2048 manual)
  #define WREN          0x06
  #define WRDI          0x04
  #define RDSR          0x05
  #define WRSR          0x01
  #define READ          0x03
  #define PROGRAM       0x02
  #define SECTOR_ERASE  0x52
  #define CHIP_ERASE    0x62
  #define RDID          0x15

  // status register bits
  #define RDY         0x01
  #define WEN         0x02
  #define BP0         0x04
  #define BP1         0x08
  #define WPEN        0x80

  // Atmel ID
  #define OFD_MANUFACTURER_ID   0x1F
  // at25f2048
  #define OFD_DEVICE_ID         0x63

  #define PAGE_SIZE                 256

  // flash sectors
  #define SECTOR_ONE             0x00000000ul
  #define SECTOR_TWO             0x00010000ul
  #define SECTOR_THREE           0x00020000ul
  #define SECTOR_FOUR            0x00030000ul
  // 2 Mbits
  #define OFD_FLASH_SIZE         262144
  // image consists mcu flash - bootloader size + eeprom size. 128k - 4k + 4k
  #define OFD_IMAGE_SIZE                                  131072
  #define OFD_FLASH_START_ADDRESS                              0
  #define OFD_IMAGE1_START_ADDRESS       OFD_FLASH_START_ADDRESS
  #define OFD_IMAGE2_START_ADDRESS       (OFD_FLASH_START_ADDRESS + OFD_IMAGE_SIZE)

  #define OFD_MCU_FLASH_SIZE                       OFD_IMAGE_SIZE
  #define OFD_MCU_EEPROM_SIZE                                4096
  #define OFD_EEPROM_OFFSET_WITHIN_IMAGE   (OFD_MCU_FLASH_SIZE - OFD_MCU_EEPROM_SIZE)
  // 1 byte (action for bootloader), 2 bytes (images crc), 1 byte table of image types.
  #define OFD_SERVICE_INFO_SIZE                                 4

#elif defined(AT45DB041)

  #if !defined(ATMEGA1281) && !defined(ATMEGA128RFA1) && !defined(ATXMEGA256A3) && !defined(ATXMEGA256D3)
    #error 'at45db041 is supported only for atmega1281/128rfa1, atxmega256a/d3.'
  #endif

  // flash instruction format (see at45db041 manual)
  #define WRITE_BUF1    0x84
  #define WRITE_BUF2    0x87
  #define RDSR          0xD7
  #define READ          0xE8
  #define PROGRAM_BUF1  0x88
  #define PROGRAM_BUF2  0x89
  #define BLOCK_ERASE   0x50

  // status register bits
  #define RDY         0x80
  // unchanged bit mask within status register xx0111xx
  #define STATUS_UNCHANGED_BIT_MASK      0x3C
  #define STATUS_UNCHANGED_BITS          0x1C

  #define PAGE_SIZE                  264
  // block addresses
  #define FIRST_HALF_BLOCK_NUMBER    0
  #define SECOND_HALF_BLOCK_NUMBER   128

  // 4 Mbits
  #define OFD_FLASH_SIZE         540672

  #if defined(ATMEGA1281) || defined(ATMEGA128RFA1)
    // image consists mcu flash - bootloader size + eeprom size. 128k - 4k + 4k
    // image uses 497 pages. 1 block = 8 pages
    #define OFD_USED_BLOCKS_AMOUNT                              63
    #define OFD_IMAGE_SIZE                                  131072
    #define OFD_MCU_FLASH_SIZE                      OFD_IMAGE_SIZE
    #define OFD_EEPROM_OFFSET_WITHIN_IMAGE   (OFD_MCU_FLASH_SIZE - OFD_MCU_EEPROM_SIZE)
    #define OFD_LOAD_NO_COMMAND_TO_NVM
  #elif  defined(ATXMEGA256A3) || defined(ATXMEGA256D3)
    // image consists mcu flash + eeprom size. 256k + 4k
    // image uses 1009 pages. 1 block = 8 pages
    #define OFD_USED_BLOCKS_AMOUNT                             128
    #define OFD_IMAGE_SIZE                                  266240
    #define OFD_MCU_FLASH_SIZE                              262144
    #define OFD_EEPROM_OFFSET_WITHIN_IMAGE       OFD_MCU_FLASH_SIZE
    #define OFD_LOAD_NO_COMMAND_TO_NVM      NVM.CMD = NVM_CMD_NO_OPERATION_gc
  #endif

  #define OFD_FLASH_START_ADDRESS                              0
  #define OFD_IMAGE1_START_ADDRESS       OFD_FLASH_START_ADDRESS
  #define OFD_IMAGE2_START_ADDRESS                        270336

  #define OFD_MCU_EEPROM_SIZE                                4096
  // 1 byte (action for bootloader), 2 bytes (images crc), 1 byte table of image types.
  #define OFD_SERVICE_INFO_SIZE                                 4

#else
  #error 'Unknown memory type.'
#endif

#define OFD_START_EEPROM_SREC_ADDRESS     0x810000
#define OFD_LITTLE_TO_BIG_ENDIAN(A)  ((((uint32_t)A & 0xFFul) << 24)   \
                                   | (((uint32_t)A & 0xFF00ul) << 8)   \
                                   | (((uint32_t)A >> 8) & 0xFF00ul)   \
                                   | (((uint32_t)A >> 24) & 0xFFul))


typedef struct
{
  union
  {
    uint32_t   flashOffset;
    uint16_t   eepromOffset;
  };
  uint8_t  *data;
  uint32_t  length;
} OfdInternalMemoryAccessParam_t;

// image type table
typedef uint8_t OfdImageTable_t;

/******************************************************************************
                   Prototypes section
******************************************************************************/
/**************************************************************************//**
\brief Starts image erasing in the external memory.
******************************************************************************/
void ofdEraseImage(void);

/**************************************************************************//**
\brief Writes data to the external memory.
******************************************************************************/
void ofdWriteData(void);

/**************************************************************************//**
\brief Checks image crc.
******************************************************************************/
void ofdCheckCrc(void);

/**************************************************************************//**
\brief Starts saving internal flash.
******************************************************************************/
void ofdSaveCurrentFlashImage(void);

/**************************************************************************//**
\brief Reads image crc from internal eeprom.
******************************************************************************/
void ofdReadCrc(void);

/**************************************************************************//**
\brief Counts crc current memory area. CRC-8. Polynom 0x31    x^8 + x^5 + x^4 + 1.

\param[in]
  crc - first crc state
\param[in]
  pcBlock - pointer to the memory for crc counting
\param[in]
  length - memory size

\return
  current area crc
******************************************************************************/
uint8_t ofdCrc(uint8_t crc, uint8_t *pcBlock, uint8_t length);

/**************************************************************************//**
\brief Finds storage space.
******************************************************************************/
void ofdFindStorageSpace(void);

/**************************************************************************//**
\brief Sets action for internal bootloader.
******************************************************************************/
void ofdSetActionForBootloader(void);

/**************************************************************************//**
\brief Flushs memory buffer to flash.
******************************************************************************/
void ofdFlushData(void);

/******************************************************************************
                 Inline static functions prototypes section.
******************************************************************************/
// Macros for the EXT_MEM_CS pin manipulation.
#if defined(ATMEGA1281)

HAL_ASSIGN_PIN(EXT_MEM_CS, F, 3);

#elif defined(ATMEGA128RFA1)

HAL_ASSIGN_PIN(EXT_MEM_CS, G, 5);

#elif defined(ATXMEGA256A3) || defined(ATXMEGA256D3)

HAL_ASSIGN_PIN(EXT_MEM_CS, D, 4);

#endif

#endif // _OTAU_

#endif /* _OFDMEMORYDRIVER_H */
