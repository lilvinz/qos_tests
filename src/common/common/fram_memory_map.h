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

#define FRAM_SIZE   0x2000  // size of fram, must be <= partition size in board_cfg.h

// ************************************************************
// **** ATTENTION:                                         ****
// **** Check and if necessary adjust all preprocessor     ****
// **** macros used for size checking if a new section     ****
// **** or variable is added to the fram!                  ****
// ************************************************************

#define FRAM_SETTINGS_LAYOUT_ID                 0x000  // 4 bytes
#define FRAM_FW_UPDATE_RETRIES                  0x004  // 1 byte
#define FRAM_MESH_FW_UPDATE_RETRIES             0x005  // 1 byte
#define FRAM_TIME_METER_SYSTEM                  0x006  // 4 byte
#define FRAM_TIME_METER_NETWORK                 0x00A  // 4 byte
#define FRAM_TIME_METER_LED_WHITE               0x00E  // 4 byte
#define FRAM_TIME_METER_LED_RED_1               0x012  // 4 byte
#define FRAM_TIME_METER_LED_RED_2               0x016  // 4 byte
#define FRAM_TIME_METER_LED_RED_3               0x01A  // 4 byte
#define FRAM_TIME_METER_LED_RED_4               0x01E  // 4 byte
#define FRAM_TIME_METER_LED_RED_C               0x022  // 4 byte
#define FRAM_TIME_METER_COMPENSATED_LED_WHITE   0x026  // 4 byte
#define FRAM_TIME_METER_COMPENSATED_LED_RED_1   0x02A  // 4 byte
#define FRAM_TIME_METER_COMPENSATED_LED_RED_2   0x02E  // 4 byte
#define FRAM_TIME_METER_COMPENSATED_LED_RED_3   0x032  // 4 byte
#define FRAM_TIME_METER_COMPENSATED_LED_RED_4   0x036  // 4 byte
#define FRAM_TIME_METER_COMPENSATED_LED_RED_C   0x03A  // 4 byte
#define FRAM_TIME_METER_LED_RED                 0x050  // 4 byte
#define FRAM_TIME_METER_LED_IR                  0x054  // 4 byte
#define FRAM_TIME_METER_COMPENSATED_LED_RED     0x058  // 4 byte
#define FRAM_TIME_METER_COMPENSATED_LED_IR      0x05C  // 4 byte

#define FRAM_PSU_SERIAL                         0x03E  // 4 byte

// Memory used to store led fail information
#define FRAM_LED_FAIL_FLAGS_START       0x042  // Start of section for led fail flags
#define FRAM_LED_FAIL_FLAGS_BM_MAIN     0x042  // Fail flags for led stages on main board
#define FRAM_LED_FAIL_FLAGS_BM_SLAVE_1  0x044  // Fail flags for led stages on slave board 1
#define FRAM_LED_FAIL_FLAGS_BM_SLAVE_2  0x046  // Fail flags for led stages on slave board 2
#define FRAM_LED_FAIL_FLAGS_NEXT_FREE   0x048  // Please leave enough space for expansion

// Memory used for log
#define FRAM_LOG_FIRST_ENTRY            0x0e0  // 4 bytes
#define FRAM_LOG_NEXT_ENTRY             0x0e4  // 4 bytes
#define FRAM_LOG_NEXT_ENTRY_VERIFIED    0x0e8  // 4 bytes
#define FRAM_LOG_OVERFLOW_COUNT         0x0eC  // 2 bytes

#define FRAM_SETTINGS_BASE              0x1e0  // SETTINGS_COUNT * 4 bytes

#define FRAM_CALIBRATION_LED_BASE       0x4e0  // LED CALIBRATION, sizeof(AGUARD_LED_CALIBRATION_T) * ADC_LED_CH_INDEX_COUNT

#define FRAM_BMM_LED_PROFILES_BASE      0x5e0  // size of BMM_LED_PROFILE

#define FRAM_SCRATCHPAD_ORG             (FRAM_SIZE - FRAM_SCRATCHPAD_SIZE) // used for factory test
#define FRAM_SCRATCHPAD_SIZE            512

#endif
/* FRAM_MEMORY_MAP_H_ */
