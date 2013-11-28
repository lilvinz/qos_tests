
#include "stdheader.h"
#include "log.h"

#include "hash.h"
#include "clock.h"
#include "rtc.h"
#include "debug.h"
#include "spi_serial_flash.h"
#include "spi_serial_fram.h"
#include "fram_memory_map.h"
#include "fram_mirror.h"
#include "flash_memory_map.h"
#include "watchdog.h"
#include "debug_freertos.h"

static bool                b_clear_log;
static bool                b_get_log_entry;
static bool                b_get_log_entry_direction;
static uint8_t             get_entries_number;
static uint32_t            get_entries_start_index;
static uint32_t            log_size;              // actual size of log
static uint32_t            first_entry_index;     // index of first available log entry
static uint32_t            next_entry_index;      // index of next log entry
static LOG_ENTRY_T         *p_get_entries_log_entries;
static xQueueHandle        xQueueLOG;
static xSemaphoreHandle    xLogTaskWaitMutex;
static xSemaphoreHandle    xLogGetLogWaitMutex;

/**
  * @brief  Get number of log entries.
  * @param  None
  * @retval None
  */
uint32_t LOG_GetLogSize(void)
{
    return log_size;
}

/**
  * @brief  Get index of first available log entry.
  * @param  None
  * @retval None
  */
uint32_t LOG_GetFirstEntryIndex(void)
{
//    DPRINT4(DEBUG_LOG, "GET FIRST! first: %u | next: %u | log size:  %u | overflow: %u \n", first_entry_index, next_entry_index, log_size, log_overflow_count);
    return  first_entry_index;
}

/**
  * @brief  Get index of next log entry.
  * @param  None
  * @retval None
  */
uint32_t LOG_GetNextEntryIndex(void)
{
//    DPRINT4(DEBUG_LOG, "GET NEXT! first: %u | next: %u | log size:  %u | overflow: %u \n", first_entry_index, next_entry_index, log_size, log_overflow_count);
    return next_entry_index;
}

/**
  * @brief  Get an log entry from flash memory.
  * @param  - n_entry: index of the log entry in flash memory
  *         - pLogEntry: pointer to a buffer where the selected log entry should be stored
  * @retval boolean: - true: success - false: an error occurred
  */
bool log_entry_get_direct(uint32_t n_entry, LOG_ENTRY_T *pLogEntry)
{
    uint32_t log_address;

    log_address = n_entry * LOG_ENTRY_SIZE;

    while (SPI_SERIAL_FLASH_ReadBuffer(log_address, pLogEntry, LOG_ENTRY_SIZE) != SPI_SERIAL_FLASH_RESULT_FINISHED) {}

//    DPRINT1(DEBUG_LOG, "GET ENTRY DIRECT entry: %u \n", n_entry);

    // if requested entry is higher than available entries, return FALSE,
    // but read entry anyway!
    if (n_entry > log_size)
        return FALSE;

    return TRUE;
}

/**
  * @brief  Get an log entry from flash memory.
  * @param  - n_entry: index of the log entry
  *         - pLogEntry: pointer to a buffer where the selected log entry should be stored
  * @retval boolean: - true: success - false: an error occurred
  */
bool LOG_EntryGet(uint32_t n_entry, LOG_ENTRY_T *pLogEntry)
{
    // check if requested entry is available
//    DPRINT1(DEBUG_LOG, "GET ENTRY entry: %u \n", n_entry);
    if ((n_entry < first_entry_index) || (n_entry >= next_entry_index))
        return FALSE;  // entry is not available

    uint32_t offset;
    offset = n_entry - first_entry_index;

    return log_entry_get_direct(offset, pLogEntry);
}

/**
  * @brief  Get an log entry from flash memory.
  * @param  - n_entry: index of the first log entry which should be returned
  *         - pLogEntry: pointer to a buffer where the selected log entries should be stored
  *         - num: number of log entries which should be read from memory
  *         - dir: direction - true ascending - false: descending
  * @retval boolean: - true: success - false: an error occurred
  */
bool LOG_GetLogEntry(uint32_t *n_entry, LOG_ENTRY_T *pLogEntry, uint8_t *num, bool dir)
{
    // check if requested entry is available
//    DPRINT1(DEBUG_LOG, "GET ENTRY entry: %u \n", *n_entry);
    /*    if ((n_entry < first_entry_index) || (n_entry >= next_entry_index))
        {
            // entry is not available
            return FALSE;
        }
    */
    get_entries_start_index     = *n_entry;
    get_entries_number          = *num;
    p_get_entries_log_entries   = pLogEntry;
    b_get_log_entry_direction   = dir;
    b_get_log_entry             = TRUE;
    xSemaphoreGive(xLogTaskWaitMutex);
    xSemaphoreTake(xLogGetLogWaitMutex, portMAX_DELAY);
    *n_entry    = get_entries_start_index;
    *num        = get_entries_number;

    if (get_entries_number)
        return TRUE;
    return FALSE;
}

/**
  * @brief  Load log entries from flash.
  * @param  None
  * @retval None
  */
void LOG_CollectLogEntries(void)
{
    uint8_t     num_entries_read    = 0;
    uint32_t    act_index           = get_entries_start_index;
    while(get_entries_number)
    {
        //
        // entry is available
        if (LOG_EntryGet(act_index, &p_get_entries_log_entries[num_entries_read]))
        {
            // entry loaded
            if (num_entries_read == 0)
                get_entries_start_index = act_index;
            num_entries_read++;
        }
        if (b_get_log_entry_direction)
            act_index++;  // increment index
        else
            act_index--;  // decrement index
        get_entries_number--;
    }

    get_entries_number = num_entries_read;
}

/**
  * @brief  Get index of next log entry.
  * @param  None
  * @retval None
  */
void LOG_CalcEntryIndexes(LOG_DATA_STRUCT_T *p_log_ds)
{
    next_entry_index = p_log_ds->overflow_count * LOG_MAX_ENTRIES + p_log_ds->next_entry;
    first_entry_index = next_entry_index - log_size;
}

/**
  * @brief  Calculate number of log entries.
  * @param  None
  * @retval None
  */
void LOG_CalcLogSize(LOG_DATA_STRUCT_T *p_log_ds)
{
    if (p_log_ds->first_entry <= p_log_ds->next_entry)
        log_size = p_log_ds->next_entry - p_log_ds->first_entry;
    else
        log_size = LOG_MAX_ENTRIES - p_log_ds->first_entry + p_log_ds->next_entry;
}

/**
  * @brief  Set flag to clear log.
  * @param  None
  * @retval None
  */
void LOG_Clear(void)
{
    b_clear_log = TRUE;
    xSemaphoreGive(xLogTaskWaitMutex);
}

/**
  * @brief  Clear log.
  * @param  None
  * @retval None
  */
void LOG_Clear_intern(LOG_DATA_STRUCT_T *p_log_ds)
{
    b_clear_log = FALSE;

    p_log_ds->first_entry     = 0;
    p_log_ds->next_entry      = 0;
    p_log_ds->overflow_count  = 0;

    while(uxQueueMessagesWaiting(xQueueLOG) > 0)
    {
        LOG_ENTRY_T log_entry;
        xQueueReceive(xQueueLOG, &log_entry, 0);
    }

    FRAM_MIRROR_Write32(FRAM_LOG_FIRST_ENTRY, &p_log_ds->first_entry);
    FRAM_MIRROR_Write32(FRAM_LOG_NEXT_ENTRY, &p_log_ds->next_entry);
    FRAM_MIRROR_Write16(FRAM_LOG_OVERFLOW_COUNT, &p_log_ds->overflow_count);

    // delete first block
    while (SPI_SERIAL_FLASH_EraseSector64k(FLASH_LOG_START_ADDRESS) != SPI_SERIAL_FLASH_RESULT_FINISHED) {}

    FRAM_MIRROR_Write32(FRAM_LOG_NEXT_ENTRY_VERIFIED, &p_log_ds->next_entry);
    LOG(LOG_ID_LOG_STATE_STATUS_CLEARED, 0, 0, 0, NULL);
}

bool LOG_LogEvent(uint16_t event_id, uint16_t argument_1, uint32_t argument_2, uint32_t argument_3, LOG_DUPVAR_T * pDupVar)
{
    // pDupVar offers duplicate detection if not NULL
    if (pDupVar != NULL)
    {
        LOG_DUPVAR_T hash;
        hash = (LOG_DUPVAR_T)hash_rj_hash(event_id);

        if (*pDupVar == hash)
            return FALSE;     // previous message was the same so ignore and simply return.

        *pDupVar = hash;
    }

    LOG_ENTRY_T log_entry;
    log_entry.timestamp     = RTC_GetUnixTime();
    log_entry.event_id      = event_id;
    log_entry.argument_1    = argument_1;
    log_entry.argument_2    = argument_2;
    log_entry.argument_3    = argument_3;

    DPRINT5(DEBUG_LOG, "LOG: TS %u EID %u ARG1 %u ARG2 %u ARG3 %u\n", log_entry.timestamp, log_entry.event_id, log_entry.argument_1, log_entry.argument_2, log_entry.argument_3);

    if (pdPASS != xQueueSend(xQueueLOG, &log_entry, 0)) //portMAX_DELAY);
        asm("nop");
    if (pdPASS != xSemaphoreGive(xLogTaskWaitMutex))
        asm("nop");

    return TRUE;
}


/**
  * @brief  Write a log entry to flash memory.
  * @param  *pLogEntry: pointer to the log entry which should be written to the flash memory
  * @retval None
  */
void LOG_Entry_Add(LOG_ENTRY_T *pLogEntry, LOG_DATA_STRUCT_T *p_log_ds)
{
    uint32_t log_next_free_address;

    if ((p_log_ds->next_entry % LOG_ENTRIES_PER_SECTOR) == 0)
    {
        log_next_free_address = p_log_ds->next_entry * LOG_ENTRY_SIZE;
        while (SPI_SERIAL_FLASH_EraseSector64k(log_next_free_address) != SPI_SERIAL_FLASH_RESULT_FINISHED) {}  // delete next sector
    }

    log_next_free_address = p_log_ds->next_entry * LOG_ENTRY_SIZE;
    p_log_ds->next_entry++;
    FRAM_MIRROR_Write32(FRAM_LOG_NEXT_ENTRY, &p_log_ds->next_entry);
    // write data
    while (SPI_SERIAL_FLASH_WriteBuffer(log_next_free_address, pLogEntry, LOG_ENTRY_SIZE) != SPI_SERIAL_FLASH_RESULT_FINISHED) {}

    if (p_log_ds->next_entry >= LOG_MAX_ENTRIES)
    {
        // overflow
        p_log_ds->next_entry = 0;
        FRAM_MIRROR_Write32(FRAM_LOG_NEXT_ENTRY, &p_log_ds->next_entry);
        p_log_ds->overflow_count++;
        FRAM_MIRROR_Write16(FRAM_LOG_OVERFLOW_COUNT, &p_log_ds->overflow_count);
    }

    if (p_log_ds->next_entry == p_log_ds->first_entry)
    {
        if (p_log_ds->first_entry >= LOG_MAX_ENTRIES - LOG_ENTRIES_PER_SECTOR)
            p_log_ds->first_entry = 0;
        else
            p_log_ds->first_entry += LOG_ENTRIES_PER_SECTOR;
        FRAM_MIRROR_Write32(FRAM_LOG_FIRST_ENTRY, &p_log_ds->first_entry);
    }
    FRAM_MIRROR_Write32(FRAM_LOG_NEXT_ENTRY_VERIFIED, &p_log_ds->next_entry);
}

void LOG_Init(LOG_DATA_STRUCT_T *p_log_ds)
{
    uint32_t    log_next_entry_verified;
    b_clear_log             = FALSE;
    b_get_log_entry         = FALSE;
    log_size                = 0;
    first_entry_index       = 0;
    next_entry_index        = 0;
    get_entries_number      = 0;
    get_entries_start_index = 0;

    FRAM_MIRROR_Read32(FRAM_LOG_FIRST_ENTRY, &p_log_ds->first_entry);  // index of first log entry
    FRAM_MIRROR_Read32(FRAM_LOG_NEXT_ENTRY, &p_log_ds->next_entry);  // index of next free log entry
    FRAM_MIRROR_Read32(FRAM_LOG_NEXT_ENTRY_VERIFIED, &log_next_entry_verified);  // index of next free log entry
    FRAM_MIRROR_Read16(FRAM_LOG_OVERFLOW_COUNT, &p_log_ds->overflow_count);  // number of overflows

    // check validity of log
    if (p_log_ds->next_entry != log_next_entry_verified)
    {
        // error, maybe there was a power loss while a log entry was written
        if (p_log_ds->next_entry == log_next_entry_verified + 1)
        {
            // this error may occur if adding a new log entry could not be finished
            // change all bytes of the last log entry to 0 to mark it as invalid (flash memory)
            uint8_t     pClearBuffer[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            uint32_t    log_address;
            bool        b_restored_overflow_ctr = FALSE;

            log_address = (p_log_ds->next_entry - 1) * LOG_ENTRY_SIZE;
            while (SPI_SERIAL_FLASH_WriteBuffer(log_address, pClearBuffer, LOG_ENTRY_SIZE) != SPI_SERIAL_FLASH_RESULT_FINISHED) {}

            if (p_log_ds->next_entry >= LOG_MAX_ENTRIES)
            {
                // overflow
                p_log_ds->next_entry = 0;
                FRAM_MIRROR_Write32(FRAM_LOG_NEXT_ENTRY, &p_log_ds->next_entry);
                p_log_ds->overflow_count++;
                FRAM_MIRROR_Write16(FRAM_LOG_OVERFLOW_COUNT, &p_log_ds->overflow_count);
                b_restored_overflow_ctr = TRUE;
            }

            if (p_log_ds->next_entry == p_log_ds->first_entry)
            {
                // an error occurred during log overflow
                if (p_log_ds->first_entry >= LOG_MAX_ENTRIES - LOG_ENTRIES_PER_SECTOR)
                    p_log_ds->first_entry = 0;
                else
                    p_log_ds->first_entry += LOG_ENTRIES_PER_SECTOR;

                if (p_log_ds->overflow_count == 0)
                {
                    // log_overflow_count should not be 0 after an overflow
                    if (!b_restored_overflow_ctr)
                    {
                        p_log_ds->overflow_count++;
                        FRAM_MIRROR_Write16(FRAM_LOG_OVERFLOW_COUNT, &p_log_ds->overflow_count);
                    }
                }
                FRAM_MIRROR_Write32(FRAM_LOG_FIRST_ENTRY, &p_log_ds->first_entry);
            }

            if ((p_log_ds->next_entry % LOG_ENTRIES_PER_SECTOR) == 0)
                while (SPI_SERIAL_FLASH_EraseSector64k(p_log_ds->next_entry) != SPI_SERIAL_FLASH_RESULT_FINISHED) {} // delete next sector

            FRAM_MIRROR_Write32(FRAM_LOG_NEXT_ENTRY_VERIFIED, &p_log_ds->next_entry);

            // log restored!
            LOG(LOG_ID_LOG_RESTORED_LOG, 0, 0, 0, NULL);
        }
        else
            b_clear_log = TRUE;  // this error should not be possible --> erase log
    }
}

void LOG_Task(void *pvParameters)
{
    LOG_DATA_STRUCT_T log_ds;

    xQueueLOG           = xQueueCreate(LOG_QUEUE_SIZE, sizeof(LOG_ENTRY_T));
    DQueueAddToRegistry(xQueueLOG, "xQueueLOG");
    xLogTaskWaitMutex   = xSemaphoreCreateMutex();
    DQueueAddToRegistry(xLogTaskWaitMutex, "xLogTaskWaitMutex");
    xLogGetLogWaitMutex = xSemaphoreCreateMutex();
    DQueueAddToRegistry(xLogGetLogWaitMutex, "xLogGetLogWaitMutex");

    assert_param(xQueueLOG != NULL);
    assert_param(xLogTaskWaitMutex != NULL);
    assert_param(xLogGetLogWaitMutex != NULL);

    xSemaphoreTake(xLogGetLogWaitMutex, portMAX_DELAY);

    LOG_Init(&log_ds);

    LOG_CalcLogSize(&log_ds);
    LOG_CalcEntryIndexes(&log_ds);

    while(1)
    {
        WATCHDOG_Heartbeat(WATCHDOG_TASK_ID_LOG);  // send heartbeat

        if (b_clear_log)
            LOG_Clear_intern(&log_ds);  // clear log, this should be done first to avoid missing erase commands from init function

        if (pdTRUE == xSemaphoreTake(xLogTaskWaitMutex, WATCHDOG_TASK_WAIT_TIME_MS))  // wait only WATCHDOG_TASK_WAIT_TIME_MS to send heartbeat
        {
            while(uxQueueMessagesWaiting(xQueueLOG) > 0)
            {
                LOG_ENTRY_T log_entry;
                xQueueReceive(xQueueLOG, &log_entry, 0);
                LOG_Entry_Add(&log_entry, &log_ds);
                LOG_CalcLogSize(&log_ds);
                LOG_CalcEntryIndexes(&log_ds);
            }

            if (b_get_log_entry)
            {
                b_get_log_entry = FALSE;
                // read log entry from memory
                LOG_CollectLogEntries();
                xSemaphoreGive(xLogGetLogWaitMutex);
            }
        }
    }
}
