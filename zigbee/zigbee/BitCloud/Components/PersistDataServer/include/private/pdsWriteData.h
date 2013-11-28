/******************************************************************************
  \file csDbg.h

  \brief
    Persistent Periodic data save implementation header

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2010 , Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
    29.10.10 A. Razinkov - Created.
******************************************************************************/

#ifndef _PDSWRITEDATA_H_
#define _PDSWRITEDATA_H_

/******************************************************************************
                    Includes section
******************************************************************************/
#include <pdsMemAbstract.h>

#ifdef _COMMISSIONING_
/******************************************************************************
                    Functions prototypes section
******************************************************************************/
/******************************************************************************
\brief Update persistent items values in Configuration Server from persist memory
\
\return Operation result
******************************************************************************/
PDS_DataServerState_t pdsUpdate(void);

/******************************************************************************
\brief Locates parameter by it's index in persistent memory table. And prepares
\      memory descriptor to store parameter's value in EEPROM
\
\param[in] index - index of the parameter in persistent memory table
\param[out]descriptor - memory descriptor to store parameter value
******************************************************************************/
void pdsPrepareMemoryAccess(uint8_t index, MEMORY_DESCRIPTOR* descriptor);

/******************************************************************************
\brief Timer callback, initiates the commit process.
******************************************************************************/
void pdsOnTimerSave(void);

/*******************************************************************************
\brief Start server work
*******************************************************************************/
void pdsStartPersistServer(void);
#endif /* _COMMISSIONING_ */

/******************************************************************************
\brief Writes data to persist memory
\
\param[in] descriptor - memory descriptor to store parameter value
\param[out]callback - callback to write-finidhed event handler
******************************************************************************/
PDS_DataServerState_t pdsWrite(MEMORY_DESCRIPTOR* descriptor, void (*callback)(void));

/******************************************************************************
\brief Read data from persist memory
\
\param[in] descriptor - memory descriptor to read parameter value
\param[out]callback - callback to read-finished event handler
******************************************************************************/
PDS_DataServerState_t pdsRead(MEMORY_DESCRIPTOR* descriptor, void (*callback)(void));

/******************************************************************************
\brief Wait until memory be ready for transaction
******************************************************************************/
void pdsWaitMemoryFree(void);

/*******************************************************************************
\brief Dummy callback
*******************************************************************************/
void pdsDummyCallback(void);

#endif /* _PDSWRITEDATA_H_ */
