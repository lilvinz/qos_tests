/**************************************************************************//**
\file   ofdExtMemory.h

\brief  The public API of external flash driver.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    31/07/09 A. Khromykh - Created
*******************************************************************************/

#ifndef _OFDEXTMEMORY_H
#define _OFDEXTMEMORY_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <types.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
/** \brief Size of memory block for internal ofd algorithms */
#define OFD_BLOCK_FOR_CHECK_CRC           64
#define OFD_BLOCK_SIZE                    OFD_BLOCK_FOR_CHECK_CRC

/******************************************************************************
                   Types section
******************************************************************************/
/** \brief Status messages for upper component about ofd state */
typedef enum
{
  OFD_STATUS_SUCCESS = 0,
  OFD_STATUS_SERIAL_BUSY,
  OFD_STATUS_UNKNOWN_EXTERNAL_FLASH_TYPE,
  OFD_STATUS_INCORRECT_API_PARAMETER,
  OFD_STATUS_INCORRECT_EEPROM_PARAMETER,
  OFD_SERIAL_INTERFACE_BUSY
} OFD_Status_t ;

/** \brief Number of position for image in the external flash */
typedef enum
{
  OFD_POSITION_1,
  OFD_POSITION_2,
  OFD_POSITION_MAX
} OFD_Position_t;

/** \brief Source type which was been initiator of image saving. */
typedef enum
{
  OFD_IMAGE_WAS_SAVED_FROM_MCU,
  OFD_IMAGE_WAS_WRITTEN_THROUGH_API,
} OFD_ImageSource_t;

/** \brief Parameters for access to external memory  \n
 offset - offset from start address  \n
 data - pointer to mcu ram area with\for data for\from external memory  \n
 length - size of mcu ram area with\for data. */
typedef struct
{
  uint32_t  offset;
  uint8_t  *data;
  uint32_t  length;
} OFD_MemoryAccessParam_t;

/** \brief image crc */
typedef uint8_t OFD_Crc_t;

/** \brief Image information. Crc and image type. */
typedef struct
{
  OFD_ImageSource_t type;
  OFD_Crc_t          crc;
} OFD_ImageInfo_t;

/** \brief callback methods for OFD API. \n
 ATENTION!!!!  Callback functions must not call public OFD API directly.
 */
typedef void (* OFD_Callback_t)(OFD_Status_t);
typedef void (* OFD_InfoCallback_t)(OFD_Status_t, OFD_ImageInfo_t *);

/******************************************************************************
                   Prototypes section
******************************************************************************/
/**************************************************************************//**
\brief Opens serial interface and checks memory type.

\param[in]
  cb - pointer to callback
******************************************************************************/
void OFD_Open(OFD_Callback_t cb);

/**************************************************************************//**
\brief Closes serial interface.
******************************************************************************/
void OFD_Close(void);

/**************************************************************************//**
\brief Erases image in the external memory.

\param[in]
  pos - image position in the external memory
\param[in]
  cb - pointer to callback
******************************************************************************/
void OFD_EraseImage(OFD_Position_t pos, OFD_Callback_t cb);

/**************************************************************************//**
\brief Writes data to the external memory.

\param[in]
  pos - image position for new data
\param[in]
  accessParam - pointer to the access structure
\param[in]
  cb - pointer to callback
******************************************************************************/
void OFD_Write(OFD_Position_t pos, OFD_MemoryAccessParam_t *accessParam, OFD_Callback_t cb);

/**************************************************************************//**
\brief Flushes data from internal buffer, checks image crc and saves it to
the external memory.

\param[in]
  pos - image position for new data
\param[in]
  countBuff - pointer to the memory for internal data (memory size must be OFD_BLOCK_FOR_CHECK_CRC)
\param[in]
  cb - pointer to callback
******************************************************************************/
void OFD_FlushAndCheckCrc(OFD_Position_t pos, uint8_t *countBuff, OFD_InfoCallback_t cb);

/**************************************************************************//**
\brief Saves current mcu flash and eeprom to the external memory, checks crc for its
and set command for bootloader.

\param[in]
  whereToSave - image position for current mcu flash and eeprom
\param[in]
  from        - new image position
\param[in]
  copyBuff - pointer to the memory for internal data (memory size must be OFD_BLOCK_FOR_CHECK_CRC)
\param[in]
  cb - pointer to callback
******************************************************************************/
void OFD_SwitchToNewImage(OFD_Position_t whereToSave, OFD_Position_t from, uint8_t *copyBuff, OFD_Callback_t cb);

/**************************************************************************//**
\brief Sets command for bootloader.

\param[in]
  from        - image position
\param[in]
  cb - pointer to callback
******************************************************************************/
void OFD_ChangeImage(OFD_Position_t from, OFD_Callback_t cb);

/**************************************************************************//**
\brief Reads image informations.

\param[in]
  pos - image position
\param[in]
  cb - pointer to callback
******************************************************************************/
void OFD_ReadImageInfo(OFD_Position_t pos, OFD_InfoCallback_t cb);

#endif /* _OFDEXTMEMORY_H */
