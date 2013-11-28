/*
 * fram_memory_map.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 30.06.2010
 *      Author: DHA
 *
 * Purpose:
 * This file is used to organize the fram.
 * The base address of every variable stored in the fram should be defined in this file.
 * The size of each variable should be documented as a comment after the base address.
 *
 */

#ifndef FRAM_MEMORY_MAP_H_
#define FRAM_MEMORY_MAP_H_

#define FRAM_SIZE   0x8000  // size of fram

// the first 256 bytes are reserved for special purposes
#define FRAM_INIT_STRING                0x000  // 8 bytes (QUANTEC)

#define FRAM_WRITE_CTR                  0x008  // 8 bytes, needed for mirroring
#define FRAM_DIRTY_BIT                  0x010  // 1 byte, needed for mirroring

#define FRAM_SETTINGS_LAYOUT_ID                 0x020  // 4 bytes
#define FRAM_FW_UPDATE_RETRIES                  0x024  // 1 byte
#define FRAM_ZIGBEE_FW_UPDATE_RETRIES           0x025  // 1 byte
#define FRAM_TIME_METER_SYSTEM                  0x026  // 4 byte
#define FRAM_TIME_METER_NETWORK                 0x02A  // 4 byte
#define FRAM_TIME_METER_LED_WHITE               0x02E  // 4 byte
#define FRAM_TIME_METER_LED_RED_1               0x032  // 4 byte
#define FRAM_TIME_METER_LED_RED_2               0x036  // 4 byte
#define FRAM_TIME_METER_LED_RED_3               0x03A  // 4 byte
#define FRAM_TIME_METER_LED_RED_4               0x03E  // 4 byte
#define FRAM_TIME_METER_LED_RED_C               0x042  // 4 byte
#define FRAM_TIME_METER_COMPENSATED_LED_WHITE   0x046  // 4 byte
#define FRAM_TIME_METER_COMPENSATED_LED_RED_1   0x04A  // 4 byte
#define FRAM_TIME_METER_COMPENSATED_LED_RED_2   0x04E  // 4 byte
#define FRAM_TIME_METER_COMPENSATED_LED_RED_3   0x052  // 4 byte
#define FRAM_TIME_METER_COMPENSATED_LED_RED_4   0x056  // 4 byte
#define FRAM_TIME_METER_COMPENSATED_LED_RED_C   0x05A  // 4 byte

#define FRAM_LOG_FIRST_ENTRY            0x100  // 4 bytes
#define FRAM_LOG_NEXT_ENTRY             0x104  // 4 bytes
#define FRAM_LOG_NEXT_ENTRY_VERIFIED    0x108  // 4 bytes
#define FRAM_LOG_OVERFLOW_COUNT         0x10C  // 2 bytes

#define FRAM_SETTINGS_BASE              0x200  // SETTINGS_COUNT * 4 bytes

#define FRAM_CALIBRATION_LED_BASE       0x500  // LED CALIBRATION, sizeof(AGUARD_LED_CALIBRATION_T) * ADC_LED_CH_INDEX_COUNT

#define FRAM_BMM_LED_PROFILES_BASE      0x600  // size of BMM_LED_PROFILE

#define FRAM_ADR_MIRROR_BASE            FRAM_SIZE / 2  // base address of mirror section
#endif
/* FRAM_MEMORY_MAP_H_ */
