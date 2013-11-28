/**************************************************************************//**
  \file pdsWriteData.c

  \brief Periodic data save implementation

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
#include <pdsCrcService.h>
#include <pdsAuxService.h>
#include <leds.h>
#include <stddef.h>
#include <csPersistentMem.h>
#include <pdsDbg.h>
#include <pdsWriteData.h>
#include <taskManager.h>

/******************************************************************************
                   Global variables section
******************************************************************************/
uint8_t            savingIsStarted = SAVE_IS_STOPED;

#ifdef _COMMISSIONING_
HAL_AppTimer_t     pdsEepromSaveServiceTimer;

/******************************************************************************
                   External variables section
******************************************************************************/
/* Configuration Server persistent memory table */
extern const CS_PersistantData_t PROGMEM_DECLARE(csPersistentMemTable[]);

/* Number of records in csPersistentMemTable*/
extern const uint8_t csPersistentItemsAmount;

/* Total size of memory occupied by persistent parameters */
extern const uint16_t csPersistentMemorySize;

/******************************************************************************
                   Prototypes section
******************************************************************************/
static PDS_DataServerState_t pdsCommit(void);
static bool pdsCommitStarted(MEMORY_DESCRIPTOR* descriptor);

/******************************************************************************
                   Implementation section
******************************************************************************/
/******************************************************************************
\brief Start internal PDS timer
\
\param[in] typeInterval - set timer interval. Available intervals are:
                          PDS_LONG_INTERVAL  - 5 minutes
                          PDS_SHORT_INTERVAL - 50 msec
\param[out] callback - callback on timer fired event

******************************************************************************/
void pdsStartPdsTimer(void (*callback)(void), uint8_t typeInterval)
{
  pdsEepromSaveServiceTimer.callback = callback;
  if (PDS_LONG_INTERVAL ==  typeInterval)
  {
#ifdef _POWER_FAILURE_
    uint32_t storingInterval;

    CS_ReadParameter(CS_PDS_STORING_INTERVAL_ID, &storingInterval);
    pdsEepromSaveServiceTimer.interval = storingInterval;
#else
    pdsEepromSaveServiceTimer.interval = STORE_TIMER_TIMEOUT;
#endif /* _POWER_FAILURE_ */
  }
  else
    pdsEepromSaveServiceTimer.interval = SHORT_TIMEOUT;
  pdsEepromSaveServiceTimer.mode = TIMER_ONE_SHOT_MODE;
  HAL_StartAppTimer(&pdsEepromSaveServiceTimer);
}

/*******************************************************************************
\brief Start server work
*******************************************************************************/
void pdsStartPersistServer(void)
{
  pdsStartPdsTimer(pdsOnTimerSave, PDS_LONG_INTERVAL);
}

/******************************************************************************
\brief Commit item value changes (if any) from Configuration Server to persist
\      memory
\
\param[in] descriptor - memory descriptor to store parameter value

\return true - if changes found and commit started; false - otherwise
******************************************************************************/
static bool pdsCommitStarted(MEMORY_DESCRIPTOR* descriptor)
{
  uint8_t data;
  MEMORY_DESCRIPTOR byteAccessDescriptor =
  {
    .data = &data,
    .length = 1
  };

  /* Check for any changes in byte-by-byte order */
  for (uint16_t i = 0; i < descriptor->length; i++)
  {
    byteAccessDescriptor.address = descriptor->address + i;
    pdsRead(&byteAccessDescriptor, pdsDummyCallback);
    /* Start rewrite out of date data in persist memory*/
    if (*byteAccessDescriptor.data != *((descriptor->data) + i))
    {
      pdsWrite(descriptor, pdsOnTimerSave);
      return true;
    }
  }
  return false;
}

/******************************************************************************
\brief Update persistent items values in Configuration Server from persist memory
\
\return Operation result
******************************************************************************/
PDS_DataServerState_t pdsUpdate(void)
{
  PDS_DataServerState_t status;
  MEMORY_DESCRIPTOR descriptor;

  for (uint8_t item = 0; item < csPersistentItemsAmount; item++)
  {
    pdsPrepareMemoryAccess(item, &descriptor);
    status = pdsRead(&descriptor, pdsDummyCallback);
    if (status != PDS_SUCCESS)
      return status;
  }
  return PDS_SUCCESS;
}

/******************************************************************************
\brief Commit persistent memory changes (if any) from Configuration Server
\      to persist memory. Process comes iterative way - when some item stored,
\      this function will be called as a callback.
\
\return Operation result
******************************************************************************/
PDS_DataServerState_t pdsCommit(void)
{
  MEMORY_DESCRIPTOR descriptor;
  static uint8_t item = 0;

  while (item < csPersistentItemsAmount)
  {
    pdsPrepareMemoryAccess(item, &descriptor);
    if(pdsCommitStarted(&descriptor))
      return PDS_COMMIT_IN_PROGRESS;

    item++;
  }
  item = 0;
  return PDS_SUCCESS;
}

/******************************************************************************
\brief Timer callback, initiates the commit process.
******************************************************************************/
void pdsOnTimerSave(void)
{
#ifdef USE_LED
  BSP_OnLed(LED_YELLOW);
#endif

  savingIsStarted = SAVE_IS_STARTED;

  if (PDS_SUCCESS == pdsCommit())
  {
    if (PDS_EEPROM_ERROR == pdsWriteCrc())
    {
      pdsStartPdsTimer(pdsOnTimerSave, PDS_SHORT_INTERVAL);
    }
    else
    {
      pdsStartPersistServer();
#ifdef USE_LED
      BSP_OffLed(LED_YELLOW);
#endif
      savingIsStarted = SAVE_IS_STOPED;
    }
  }
}

/******************************************************************************
\brief Locates parameter by it's index in persistent memory table. And prepares
\      memory descriptor to store parameter's value in EEPROM
\
\param[in] index - index of the parameter in persistent memory table
\param[out]descriptor - memory descriptor to store parameter value
******************************************************************************/
void pdsPrepareMemoryAccess(uint8_t index, MEMORY_DESCRIPTOR* descriptor)
{
  CS_PersistantData_t item;

  memcpy_P(&item, &csPersistentMemTable[index], sizeof(CS_PersistantData_t));

  assert(item.value, PDS_PDSPREPAREMEMORYACCESS0);
  descriptor->data = item.value;
  descriptor->address = item.offset;

  /* Calculate item size using offsets */
  if (index == (csPersistentItemsAmount-1))
  {
    descriptor->length = csPersistentMemorySize - item.offset;
  }
  else
  {
    CS_PersistantData_t nextItem;
    memcpy_P(&nextItem, &csPersistentMemTable[index+1], sizeof(CS_PersistantData_t));
    assert(nextItem.value, PDS_PDSPREPAREMEMORYACCESS1);
    descriptor->length = nextItem.offset - item.offset;
  }
}
#endif /* _COMMISSIONING_ */

/******************************************************************************
\brief Write data to persist memory
\
\param[in] descriptor - memory descriptor to store parameter value
\param[out]callback - callback to write-finished event handler
******************************************************************************/
PDS_DataServerState_t pdsWrite(MEMORY_DESCRIPTOR* descriptor, void (*callback)(void))
{
  pdsWaitMemoryFree();
  if (EEPROM_ERROR == WRITE_MEMORY(descriptor, callback))
    return PDS_EEPROM_ERROR;

  if (callback == pdsDummyCallback)
    pdsWaitMemoryFree();

  return PDS_SUCCESS;
}

/******************************************************************************
\brief Read data from persist memory
\
\param[in] descriptor - memory descriptor to read parameter value
\param[out]callback - callback to read-finished event handler
******************************************************************************/
PDS_DataServerState_t pdsRead(MEMORY_DESCRIPTOR* descriptor, void (*callback)(void))
{
  pdsWaitMemoryFree();
  if (EEPROM_ERROR == READ_MEMORY(descriptor, callback))
    return PDS_EEPROM_ERROR;

  if (callback == pdsDummyCallback)
      pdsWaitMemoryFree();

  return PDS_SUCCESS;
}

/******************************************************************************
\brief Wait until memory be ready for transaction
******************************************************************************/
void pdsWaitMemoryFree(void)
{
  while(IS_MEMORY_BUSY())
  {
    SYS_ForceRunTask();
  }
}

/*******************************************************************************
\brief Dummy callback
*******************************************************************************/
void pdsDummyCallback(void)
{}
// eof pdsDataServer.c
