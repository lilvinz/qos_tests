/*
 * can_cmd_i.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 30.07.2012
 *      Author: DHA
 */

#ifndef CAN_CMD_I_H_
#define CAN_CMD_I_H_

#include "version.h"
#include "can_stm32_f2xx.h"
#include "config.h"
#include "clock.h"

#define COMMAND_QN_READ_MEMORY_RESPONSE_MSG_SIZE    8
#define COMMAND_QN_WRITE_MEMORY_MSG_SIZE            8
#define COMMAND_QN_READ_MEMORY_MSG_SIZE             4

#define CAN_CMD_I_READ_SLAVE_MEMORY_TIMEOUT_MS 100

#define CAN_CMD_I_HIGHEST_SLAVE_ID_ALLOWED  7

typedef enum
{
    CAN_CMD_I_RESULT_OK = 1,
    CAN_CMD_I_RESULT_WAITING = 0,
    CAN_CMD_I_RESULT_BLOCKED = -1,
    CAN_CMD_I_RESULT_ERROR = -2,
    CAN_CMD_I_RESULT_TIMEOUT = -3,
} CAN_CMD_I_RESULT_T;

typedef struct
{
    bool        b_received;
    uint32_t    *p_data;
} SLAVE_DATA_LIST_ITEM_T;

typedef struct
{
    uint32_t addr;
    uint32_t value;
} PACK CAN_CMD_I_MEMACC_T;

// Structure of IDs:
// Bits 0 to 7: command
// Bits 8 to 10: id of device:
// id 0: High priority, master
// id 1: first slave
// id 2: second slave
#define CAN_CMD_I_ID_FILTER_MASK    0x700  // bits 8 to 10 are to identify device

typedef enum
{
    CAN_CMD_I_COMMAND_ST_IAP_GET = 0x00,  // ST IAP command GET
    CAN_CMD_I_COMMAND_ST_IAP_GET_VERSION_READ_PROTECTION_STATUS = 0x01,  // ST IAP command GET VERSION READ PROTECTION STATUS
    CAN_CMD_I_COMMAND_ST_IAP_GET_ID = 0x02,  // ST IAP command GET ID
    CAN_CMD_I_COMMAND_ST_IAP_SPEED = 0x03,  // ST IAP command SPEED
    CAN_CMD_I_COMMAND_ST_IAP_READ_MEMORY = 0x11,  // ST IAP command READ MEMORY
    CAN_CMD_I_COMMAND_ST_IAP_GO = 0x21,  // ST IAP command GO
    CAN_CMD_I_COMMAND_ST_IAP_WRITE_MEMORY = 0x31,  // ST IAP command WRITE MEMORY
    CAN_CMD_I_COMMAND_ST_IAP_EXTENDED_ERASE = 0x44,  // ST IAP command EXTENDED ERASE
    CAN_CMD_I_COMMAND_ST_IAP_ERASE = 0x43,  // ST IAP command ERASE
    CAN_CMD_I_COMMAND_ST_IAP_WRITE_PROTECT = 0x63,  // ST IAP command WRITE PROTECT
    CAN_CMD_I_COMMAND_ST_IAP_WRITE_UNPROTECT = 0x73,  // ST IAP command WRITE UNPROTECT
    CAN_CMD_I_COMMAND_ST_IAP_READOUT_PROTECT = 0x82,  // ST IAP command READOUT PROTECT
    CAN_CMD_I_COMMAND_ST_IAP_READOUT_UNPROTECT = 0x92,  // ST IAP command READOUT UNPROTECT
    CAN_CMD_I_COMMAND_QN_WRITE_MEMORY = 0xA0,  // QN command WRITE MEMORY
    CAN_CMD_I_COMMAND_QN_WRITE_MEMORY_RESPONSE = 0xA1,  // QN command WRITE MEMORY RESPONSE
    CAN_CMD_I_COMMAND_QN_READ_MEMORY = 0xB0,  // QN command READ MEMORY
    CAN_CMD_I_COMMAND_QN_READ_MEMORY_RESPONSE = 0xB1,  // QN command READ MEMORY RESPONSE
    CAN_CMD_I_COMMAND_QN_RESTART_SLAVE = 0xC0,  // QN command RESTART SLAVE
    CAN_CMD_I_COMMAND_QN_SLAVE_IN_BOOTLOADER = 0xC1,  // QN command SLAVE IN BOOTLOADER RESPONSE
} CAN_CMD_I_COMMAND_T;

#ifdef HWP_CAN_MASTER
typedef struct
{
    CanRxMsg message;
//    CLOCK_MS_T timestamp;
} CAN_CMD_I_CAN_MSG_DATA_STRUCT_T;
#endif

bool CAN_CMD_I_Init(void);
void CAN_CMD_Tick(void);
void CAN_CMD_ExecuteCommand(CanRxMsg RxMessage);
CAN_CMD_I_RESULT_T CAN_CMD_I_ReadMemory (uint32_t address);
CAN_CMD_I_RESULT_T CAN_CMD_I_WriteMemory (uint32_t address, uint32_t data);
CAN_CMD_I_RESULT_T CAN_CMD_I_SendCommand(uint8_t slave_id, uint8_t command);
#ifdef HWP_CAN_MASTER
CAN_CMD_I_RESULT_T CAN_CMD_I_ReadSlaveMemoryBlocking (uint8_t slave_id, uint32_t address, uint32_t *pData);
CAN_CMD_I_RESULT_T CAN_CMD_I_ReadSlaveMemoryNonBlocking (uint8_t slave_id, uint32_t address, uint32_t *pData, CAN_CMD_I_RESULT_T *p_notifier);
CAN_CMD_I_RESULT_T CAN_CMD_I_WriteSlaveMemory (uint8_t slave_id, uint32_t address, uint32_t data);
CAN_CMD_I_RESULT_T CAN_CMD_I_ReadAllSlavesMemory (uint32_t address, uint8_t num_slaves, SLAVE_DATA_LIST_ITEM_T *pData);
#endif

#endif /* CAN_CMD_I_H_ */
