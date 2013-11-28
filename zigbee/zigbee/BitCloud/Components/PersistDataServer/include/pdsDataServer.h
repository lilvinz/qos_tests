/***************************************************************************//**
  \file pdsDataServer.h

  \brief The header file describes the Persistence Data Server interface

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
  History:
    28/09/07 A. Khromykh - Created
    01/11/10 A. Razinkov - Modified
*****************************************************************************/

#ifndef _PERSISTDATASERVER_H
#define _PERSISTDATASERVER_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <configServer.h>

/******************************************************************************
                   Types section
******************************************************************************/
//! PDS state result after a command call
typedef enum
{
  PDS_SUCCESS,                    //!< - Command completed successfully
  PDS_EEPROM_ERROR,               //!< - EEPROM error occurred
  PDS_CRC_ERROR,                  //!< - Wrong CRC
  PDS_COMMIT_IN_PROGRESS          //!< - Commit to external memory started
} PDS_DataServerState_t;

#ifdef _COMMISSIONING_
//! \cond internal
/***************************************************************************//**
\brief Reads all parameters from EEPROM to Config Server memory.

Reads all parameters from EEPROM to Config Server memory and also checks CRC of
EEPROM stack parameters. In case of wrong CRC ConfigServer should reset EEPROM stack
parameters to default values.
*******************************************************************************/
void PDS_Init(void);
//! \endcond

/*****************************************************************************//**
\brief Writes default values of parameters to EEPROM

\return - PDS state as a result of setting parameters to default state
*******************************************************************************/
PDS_DataServerState_t PDS_SetToDefault(void);

/***************************************************************************//**
\brief Must be called from the ::APL_TaskHandler() function only.\n
The function forces writing data from ConfigServer to EEPROM. After returning from the function
all ConfigServer data is stored in EEPROM.
*******************************************************************************/
void PDS_FlushData(void);

/***************************************************************************//**
\brief Must be called from the ::APL_TaskHandler() function only.\n
Stops Persist Data Server.
*******************************************************************************/
void PDS_Stop(void);
#endif /* _COMMISSIONING_ */

/*****************************************************************************//**
\brief Must be called from the ::APL_TaskHandler() function only.\n
Reads data from the user area of EEPROM.

\param[in] offset - data offset
\param[in] data - pointer to user data area
\param[in] length - data length
\param[in] callback - pointer to a callback function; if callback is NULL, then data will be read syncronously

\return - PDS state as a result of data reading operation
*******************************************************************************/
PDS_DataServerState_t PDS_ReadUserData(uint16_t offset, uint8_t *data, uint16_t length, void (*callback)(void));

/****************************************************************************//**
\brief Must be called only from ::APL_TaskHandler() function.\n
Writes data to user area of EEPROM.

\param[in] offset - data offset
\param[in] data - pointer to user data area
\param[in] length - data length
\param[in] callback - pointer to a callback function; if callback is NULL, then data will be written syncronously

\return - PDS state as a result of data writing operation
*******************************************************************************/
PDS_DataServerState_t PDS_WriteUserData(uint16_t offset, uint8_t *data, uint16_t length, void (*callback)(void));

#endif  //#ifndef _PERSISTDATASERVER_H

