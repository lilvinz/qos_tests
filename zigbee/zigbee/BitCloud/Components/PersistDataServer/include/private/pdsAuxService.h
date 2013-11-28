/***************************************************************************//**
  \file pdsAuxService.h

  \brief The header file describes the auxiliary service of Persistence Data Server

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal

  History:
    22/01/08 A. Khromykh - Created
    01/11/10 A. Razinkov - Modified
*****************************************************************************/

#ifndef _PDSAUXSERVICE_H
#define _PDSAUXSERVICE_H

/******************************************************************************
                   Includes section
******************************************************************************/
#ifndef _MAC2_
  #include <aps.h>
  #include <nwk.h>
  #include <appFramework.h>
#else  // _MAC2_
  #include <appTimer.h>
#endif // _MAC2_

/******************************************************************************
                   Define(s) section
******************************************************************************/
/* define for eeprom write highlight */
//#define USE_LED

// crc area
#define PDS_CRC_AREA                  10
// user area
#define USER_BASE_EEPROM_ADDRESS      (csPersistentMemorySize + PDS_CRC_AREA)

#define PDS_CS_PARAMETER             0
#define PDS_CS_MEMORY                1
#define PDS_STOP_SAVE                2

#define EEPROM_BUSY                  -2
#define EEPROM_ERROR                 -1

#define STORE_TIMER_TIMEOUT  300000ul  // 5 minutes
#define SHORT_TIMEOUT        50ul      // 50 msec
#define PDS_LONG_INTERVAL            0
#define PDS_SHORT_INTERVAL           1
#define SAVE_IS_STARTED              1
#define SAVE_IS_STOPED               0

#define MAX_CS_PDS_VARIABLE_SIZE     16

/******************************************************************************
                   Types section
******************************************************************************/
// crc property
typedef struct
{
  uint8_t crc;
  uint16_t position;
  PDS_DataServerState_t eepromState;
} PDS_ServiceCrc_t;

/******************************************************************************
                   External global variables section
******************************************************************************/
extern HAL_AppTimer_t     pdsEepromSaveServiceTimer;
extern uint8_t            savingIsStarted;
extern const uint8_t      csPersistentItemsAmount;
extern const uint16_t     csPersistentMemorySize;

#endif /*_PDSAUXSERVICE_H*/
