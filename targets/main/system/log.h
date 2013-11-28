#ifndef LOG_H
#define LOG_H

#include "local_texts_log.h"


// Erase whole EEPROM if out of memory
#define LOG_CFG_AUTOERASE       0

#define LOG_QUEUE_SIZE          24

#define LOG_MEMORY_SIZE         (FLASH_LOG_END_ADDRESS - FLASH_LOG_START_ADDRESS + 1)
#define LOG_SECTOR_SIZE         SPI_SERIAL_FLASH_SECTOR_SIZE_64K
#define LOG_ENTRY_SIZE          sizeof(LOG_ENTRY_T)

#define LOG_SECTOR_COUNT        (LOG_MEMORY_SIZE / LOG_SECTOR_SIZE)
#define LOG_ENTRIES_PER_SECTOR  (LOG_SECTOR_SIZE / LOG_ENTRY_SIZE)
#define LOG_MAX_ENTRIES         (LOG_MEMORY_SIZE / LOG_ENTRY_SIZE)

#define LOG_TASK_DELAY_TIME 100

//Note: Using 3/4 of an 32 megabit eeprom and 16 Bytes for each log entry

//Argument
//an argument is some sort of value or an internal state

#define LOG_STATE_WAIT_TIMEOUT  (10000)  // wait seconds to erase log

extern uint16_t log_eep_overflow_count;  // number of overflows

typedef struct log_entry_s
{
    uint32_t timestamp;
    uint16_t event_id;
    uint16_t argument_1;
    uint32_t argument_2;
    uint32_t argument_3;
} LOG_ENTRY_T;

typedef struct
{
    uint32_t    first_entry;       // index of first log entry
    uint32_t    next_entry;        // index of next free log entry
    uint16_t    overflow_count;    // number of overflows
} LOG_DATA_STRUCT_T;

typedef uint32_t LOG_DUPVAR_T;


void LOG_Clear(void);
uint32_t LOG_GetLogSize(void);
bool LOG_GetLogEntry(uint32_t *n_entry, LOG_ENTRY_T *pLogEntry, uint8_t *num, bool dir);
bool LOG_LogEvent(uint16_t event_id, uint16_t argument_1, uint32_t argument_2, uint32_t argument_3, LOG_DUPVAR_T * pDupVar);
#define LOG(event_id, argument_1, argument_2, argument_3, dupvar) LOG_LogEvent(event_id, argument_1, argument_2, argument_3, dupvar)
void LOG_Task(void *pvParameters);
uint32_t LOG_GetFirstEntryIndex(void);
uint32_t LOG_GetNextEntryIndex(void);

#endif
