/***************************************************************************//**
  \file pdsCrcService.h

  \brief The header file describes the interface of crc counting

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

#ifndef _PDSCRCSERVICE_H
#define _PDSCRCSERVICE_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <pdsDataServer.h>
#include <pdsAuxService.h>

#ifdef _COMMISSIONING_
/******************************************************************************
                   Prototypes section
******************************************************************************/
/******************************************************************************
\brief Calculate CRC of persistent data stored in persist memory
\
\param[out] crcStation - pointer to CRC service structure
******************************************************************************/
void pdsCalculateCrc(PDS_ServiceCrc_t *crcStation);

/******************************************************************************
\brief Read CRC of stored data from persist memory
\
\param[out] crcStation - pointer to CRC service structure
******************************************************************************/
void pdsReadCrc(PDS_ServiceCrc_t *crcStation);

/******************************************************************************
\brief Write CRC of stored data to persist memory. The ring buffer used
\      to increase persist memory life cycle
\
\param[out] crcStation - pointer to CRC service structure
******************************************************************************/
PDS_DataServerState_t pdsWriteCrc(void);

/******************************************************************************
\brief Clears whole CRC area in persist memory
\
\return operation result
******************************************************************************/
PDS_DataServerState_t pdsClearCrcArea(void);

/******************************************************************************
\brief Check if any valid data exists in persist memory
\
\return operation result
******************************************************************************/
PDS_DataServerState_t pdsCheckPersistMemory(void);

#ifdef __DBG_PDS__
  void pdsDbgReadAllEeprom(void);
#endif

#endif /* _COMMISSIONING_ */
#endif /*_PDSCRCSERVICE_H*/
