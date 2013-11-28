/*
 * modbus_master.h
 *
 *  Created on: 02.08.2010
 *      Author: Carsten Bartels
 */

#ifndef MODBUS_MASTER_H_
#define MODBUS_MASTER_H_

#include "settings.h"
#include "modbus_rtu.h"

#define MODBUS_MASTER_SLAVE_ADDRESS     1

typedef struct
{
    struct
    {
        uint32_t requests;
        uint32_t responses;
    } io;
    struct
    {
        uint32_t timeouts;
        uint32_t exceptions;
    } error;
} MODBUS_MASTER_STATS_T;

extern MODBUS_MASTER_STATS_T modbusMasterStats;

void MODBUS_MASTER_Init();
void MODBUS_MASTER_Tick();
void MODBUS_MASTER_Response_Receiver(MODBUS_RTU_PACKET_T *packet);
void MODBUS_MASTER_SettingsUpdated(SETTINGS_GROUP_T group_id);

#endif /* MODBUS_MASTER_H_ */
