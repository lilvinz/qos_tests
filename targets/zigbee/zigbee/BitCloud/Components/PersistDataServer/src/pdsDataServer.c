/***************************************************************************//**
  \file pdsDataServer.c

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
/******************************************************************************
                   Includes section
******************************************************************************/
#include <pdsMemAbstract.h>
#include <appTimer.h>
#include <pdsDataServer.h>
#include <pdsCrcService.h>
#include <pdsAuxService.h>
#include <leds.h>
#include <pdsWriteData.h>
#include <csPersistentMem.h>
#include <taskManager.h>

#ifdef _COMMISSIONING_
/******************************************************************************
                   Prototypes section
******************************************************************************/

/******************************************************************************
                   External variables section
******************************************************************************/
/* Number of records in csPersistentMemTable*/
extern const uint8_t csPersistentItemsAmount;

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
\brief Persist Data Server initialization procedure
******************************************************************************/
void PDS_Init(void)
{
  PDS_DataServerState_t status;

#ifdef USE_LED
  BSP_OpenLeds();
#endif

  /* Check if any valid data exists in persist memory */
  status = pdsCheckPersistMemory();

  /* Valid data exists - try to update from persist memory */
  if (PDS_SUCCESS == status)
  {
    if (PDS_EEPROM_ERROR != pdsUpdate())
    {
      pdsStartPersistServer();
    }
  }

  else if (PDS_EEPROM_ERROR != status)
  {
    pdsStartPersistServer();
  }
}

/******************************************************************************
\brief Set the default system area values and store persistent data to persist
      memory

\return operation result
******************************************************************************/
PDS_DataServerState_t PDS_SetToDefault(void)
{
  MEMORY_DESCRIPTOR descriptor;
  PDS_DataServerState_t status;

  CS_PdsDefaultValue();
  /* Store all persistent parameters */
  for (uint8_t index = 0; index < csPersistentItemsAmount; index++)
  {
    pdsPrepareMemoryAccess(index, &descriptor);
    status = pdsWrite(&descriptor, pdsDummyCallback);
    if (status != PDS_SUCCESS)
      return status;
  }

  /* Reset CRC area and rewrite CRC*/
  status = pdsClearCrcArea();
  if (status != PDS_SUCCESS)
    return status;
  status = pdsWriteCrc();
  if (status != PDS_SUCCESS)
    return status;

  return PDS_SUCCESS;
}

/******************************************************************************
\brief On-demand data storing in persist memory
******************************************************************************/
void PDS_FlushData(void)
{
  if (SAVE_IS_STOPED == savingIsStarted)
  {
    /* Force timer to finish - store data immediately */
    HAL_StopAppTimer(&pdsEepromSaveServiceTimer);
    pdsOnTimerSave();
  }

  while (SAVE_IS_STARTED == savingIsStarted)
  {
    SYS_ForceRunTask();
  }
}

/***************************************************************************//**
\brief Must be called only from ::APL_TaskHandler() function.\n
       Stops persist data server.
*******************************************************************************/
void PDS_Stop(void)
{
  while (SAVE_IS_STARTED == savingIsStarted)
  {
    SYS_ForceRunTask();
  }

  HAL_StopAppTimer(&pdsEepromSaveServiceTimer);
}
#endif /* _COMMISSIONING_ */

/******************************************************************************
\brief Read data from user area in persist memory

\param[in] offset - data offset in persist memory
\param[in] length - data lenght
\param[out] data - pointer to user data area in RAM
\param[out] callback - callback to read-finished event handler

\return operation result
******************************************************************************/
PDS_DataServerState_t PDS_ReadUserData(uint16_t offset, uint8_t *data, uint16_t length, void (*callback)(void))
{
  MEMORY_DESCRIPTOR descriptor;
  PDS_DataServerState_t status;

  descriptor.address = USER_BASE_EEPROM_ADDRESS + offset;
  descriptor.length = length;
  descriptor.data = data;

  while(SAVE_IS_STARTED == savingIsStarted)
  {
    SYS_ForceRunTask();
  }

  if (NULL == callback)
    callback = pdsDummyCallback;

  status = pdsRead(&descriptor, callback);
  if (status != PDS_SUCCESS)
    return status;

  return PDS_SUCCESS;
}

/******************************************************************************
\brief Write data to user area in persist memory
\
\param[in] offset - data offset in persist memory
\param[in] length - data lenght
\param[out] data - pointer to user data area
\param[out] callback - callback to read-finished event handler

\return operation result
******************************************************************************/
PDS_DataServerState_t PDS_WriteUserData(uint16_t offset, uint8_t *data, uint16_t length, void (*callback)(void))
{
  MEMORY_DESCRIPTOR descriptor;
  PDS_DataServerState_t status;

  descriptor.address = USER_BASE_EEPROM_ADDRESS + offset;
  descriptor.length = length;
  descriptor.data = data;

  while(SAVE_IS_STARTED == savingIsStarted)
  {
    SYS_ForceRunTask();
  }

  if (NULL == callback)
    callback = pdsDummyCallback;

  status = pdsWrite(&descriptor, callback);
  if (status != PDS_SUCCESS)
    return status;

  return PDS_SUCCESS;
}
// eof pdsDataServer.c
