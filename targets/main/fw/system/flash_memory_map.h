/*
 * flash_memory_map.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 17.08.2010
 *      Author: DHA
 *
 * Purpose:
 * This file is used to organize the flash.
 * The base address of every variable stored in the flash should be defined in this file.
 * The size of each variable should be documented as a comment after the base address.
 *
 */

#ifndef FLASH_MEMORY_MAP_H_
#define FLASH_MEMORY_MAP_H_

// ************************************************************
// **** ATTENTION:                                         ****
// **** Check and if necessary adjust all preprocessor     ****
// **** macros used for size checking if a new section     ****
// **** or variable is added to the flash!                  ****
// ************************************************************

#define FLASH_LOG_START_ADDRESS         0x00000L
#define FLASH_LOG_SIZE                  0x600000  // 6 MB
#define FLASH_LOG_END_ADDRESS           (FLASH_LOG_START_ADDRESS + FLASH_LOG_SIZE - 1)

#define FLASH_MAIN_FW_BASE              0x600000
#define FLASH_MAIN_FW_SIZE              0x100000  // 1 MB
#define FLASH_MAIN_FW_END               (FLASH_MAIN_FW_BASE + FLASH_MAIN_FW_SIZE - 1)

#define FLASH_MESH_FW_BASE          0x700000
#define FLASH_MESH_FW_SIZE          0x40000  // 256 kbytes
#define FLASH_MESH_FW_END           (FLASH_MESH_FW_BASE + FLASH_MESH_FW_SIZE - 1)

#define FLASH_COREDUMP_START_ADDRESS    0x7C0000
#define FLASH_COREDUMP_SIZE             0x40000  // 256 kbytes
#define FLASH_COREDUMP_END_ADDRESS      (FLASH_COREDUMP_START_ADDRESS + FLASH_COREDUMP_SIZE - 1)

#endif
/* FLASH_MEMORY_MAP_H_ */
