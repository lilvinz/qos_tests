/***************************************************************************//**
  \file pdsCrcService.c

  \brief Persistence Data Server implementation

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

#ifdef _COMMISSIONING_
/******************************************************************************
                   Includes section
******************************************************************************/
#include <pdsMemAbstract.h>
#include <pdsCrcService.h>
#include <pdsAuxService.h>
#include <taskManager.h>
#include <csPersistentMem.h>
#include <pdsWriteData.h>

/******************************************************************************
\brief Calculate CRC of persistent data stored in persist memory
\
\param[out] crcStation - pointer to CRC service structure
******************************************************************************/
void pdsCalculateCrc(PDS_ServiceCrc_t *crcStation)
{
  uint8_t tdata = 0;
  MEMORY_DESCRIPTOR descriptor;

  if (NULL == crcStation)
    return;

  crcStation->crc = 0;
  descriptor.address = SYSTEM_BASE_EEPROM_ADDRESS;
  descriptor.length = 1;
  while (descriptor.address < csPersistentMemorySize)
  {
    descriptor.data = &tdata;

    pdsWaitMemoryFree();

    if (EEPROM_ERROR == READ_MEMORY(&descriptor, NULL))
    {
      crcStation->eepromState = PDS_EEPROM_ERROR;
      return;
    }

    pdsWaitMemoryFree();

    crcStation->crc += tdata;
    descriptor.address++;
  }
  crcStation->eepromState = PDS_SUCCESS;
}

/******************************************************************************
\brief Read CRC of stored data from persist memory
\
\param[out] crcStation - pointer to CRC service structure
******************************************************************************/
void pdsReadCrc(PDS_ServiceCrc_t *crcStation)
{
  MEMORY_DESCRIPTOR descriptor;

  if (NULL == crcStation)
    return;

  descriptor.address = csPersistentMemorySize;
  descriptor.length = 1;
  while (descriptor.address < USER_BASE_EEPROM_ADDRESS)
  {
    descriptor.data = &(crcStation->crc);

    pdsWaitMemoryFree();

    if (EEPROM_ERROR == READ_MEMORY(&descriptor, NULL))
    {
      crcStation->eepromState = PDS_EEPROM_ERROR;
      return;
    }

    pdsWaitMemoryFree();

    if (0xFF != crcStation->crc)
    {
      crcStation->position = descriptor.address;
      crcStation->eepromState = PDS_SUCCESS;
      return;
    }
    descriptor.address++;
  }
  crcStation->position = csPersistentMemorySize;
  crcStation->eepromState = PDS_SUCCESS;
}

/******************************************************************************
\brief Clears whole CRC area in persist memory
\
\return operation result
******************************************************************************/
PDS_DataServerState_t pdsClearCrcArea(void)
{
  uint8_t value = 0xFF;
  MEMORY_DESCRIPTOR descriptor;
  uint8_t status;

  descriptor.data = &value;
  descriptor.address = csPersistentMemorySize;
  descriptor.length = 1;

  for (uint8_t i = 0; i < PDS_CRC_AREA; i++)
  {
    status = pdsWrite(&descriptor, pdsDummyCallback);
    if (status != PDS_SUCCESS)
      return status;

    descriptor.address++;
  }
  return PDS_SUCCESS;
}

/******************************************************************************
\brief Write CRC of stored data to persist memory. The ring buffer used
\      to increase persist memory life cycle
\
\param[out] crcStation - pointer to CRC service structure
******************************************************************************/
PDS_DataServerState_t pdsWriteCrc(void)
{
  PDS_ServiceCrc_t crcRead, crcCalc;
  MEMORY_DESCRIPTOR descriptor;
  uint8_t datadelete = 0xFF;
  PDS_DataServerState_t status;

  pdsReadCrc(&crcRead);
  if (PDS_EEPROM_ERROR == crcRead.eepromState)
    return PDS_EEPROM_ERROR;

  pdsCalculateCrc(&crcCalc);
  if (PDS_EEPROM_ERROR == crcCalc.eepromState)
    return PDS_EEPROM_ERROR;

  if (0 == (uint8_t)(crcCalc.crc + crcRead.crc))
    return PDS_SUCCESS;

  crcCalc.crc = 0 - crcCalc.crc;
  descriptor.address = crcRead.position;
  descriptor.data = &datadelete;
  descriptor.length = 1;

  status = pdsWrite(&descriptor, pdsDummyCallback);
  if (status != PDS_SUCCESS)
    return status;

  if (++descriptor.address >= USER_BASE_EEPROM_ADDRESS)
    descriptor.address = csPersistentMemorySize;
  descriptor.data = &(crcCalc.crc);

  status = pdsWrite(&descriptor, pdsDummyCallback);
  if (status != PDS_SUCCESS)
    return status;

  return PDS_SUCCESS;
}

/******************************************************************************
\brief Check if any valid data exists in persist memory
\
\return operation result
******************************************************************************/
PDS_DataServerState_t pdsCheckPersistMemory(void)
{
  PDS_ServiceCrc_t crcRead, crcCalc;

  pdsCalculateCrc(&crcCalc);
  if (PDS_EEPROM_ERROR == crcCalc.eepromState)
    return PDS_EEPROM_ERROR;

  pdsReadCrc(&crcRead);
  if (PDS_EEPROM_ERROR == crcRead.eepromState)
    return PDS_EEPROM_ERROR;

  if (0x00 == (uint8_t)(crcCalc.crc + crcRead.crc))
    return PDS_SUCCESS;

  return PDS_CRC_ERROR;
}

#ifdef __DBG_PDS__
  uint8_t  eepromCopy[0x400u];

  void pdsDbgReadAllEeprom(void)
  {
    PDS_DataServerState_t status;
    MEMORY_DESCRIPTOR descriptor;

    descriptor.address = 0;
    descriptor.data = eepromCopy;
    descriptor.length = 0x400u;

    status = pdsRead(&descriptor, pdsDummyCallback);
    if (status != PDS_SUCCESS)
      return;
  }
#endif
#endif /* _COMMISSIONING_ */
// eof pdsDataServer.c
