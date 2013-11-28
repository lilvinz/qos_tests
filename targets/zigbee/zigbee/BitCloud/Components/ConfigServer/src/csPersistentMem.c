/******************************************************************************
  \file csPersistentMem.c

  \brief
    Configuration Server persistent memory to store in EEPROM

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2010 , Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
    28.10.10 A. Razinkov - Created.
******************************************************************************/

/******************************************************************************
                    Includes section
******************************************************************************/
#include <csPersistentMem.h>
#include <macenvMem.h>
#ifndef _MAC2_
#include <apsConfigServer.h>
#include <nwkConfigServer.h>
#endif
#include <csSIB.h>
#include <csBuffers.h>
#ifndef _MAC2_
#include <zdoZib.h>
#endif

/******************************************************************************
                   Defines section
******************************************************************************/
/* Macros used to define records in csPersistentMemTable table*/
#define PERSISTREC(id, address) {address, RECORD_OFFSET(CS_PersistentDataOffsets_t, id)}
#define RECORD_OFFSET(struct_type, field_name) (offsetof(struct_type, field_name) + SYSTEM_BASE_EEPROM_ADDRESS)

#ifdef _COMMISSIONING_
#define PERSISTENT_ITEMS_AMOUNT (ARRAY_SIZE(csPersistentMemTable))
#define PERSISTENT_MEMORY_SIZE  (sizeof(CS_PersistentDataOffsets_t))
#else
#define PERSISTENT_ITEMS_AMOUNT 0U
#define PERSISTENT_MEMORY_SIZE  0U
#endif /* _COMMISSIONING_ */

/******************************************************************************
                    Types section
******************************************************************************/
/*
 * Persistence module used to save some of ConfigServer items to EEPROM.
 * Items to save enumerated in csPersistentMemTable, stored in FLASH.
 * This table consists of CS_PersistantData_t records.
 * Every record have 2 values: address (in RAM) of item to save and the offset - address in
 * EEPROM, were item will be placed.
 * Size of area occupied by item calculated by difference from offset of next item.
 *
 * Macros PERSISTREC(id, address) is used to mount records in csPersistentMemTable.
 * It automatically calculates the offsets for each item with the help of RECORD_OFFSET macros.
 * When all macroses gets expanded, code looks like this:
 *
 * typedef struct {
 *    uint8_t id1,
 *    uint16_t id2,
 *    ExtAddr_t id3
 * } CS_PersistentDataOffsets_t ;
 * csPersistentMemTable =
 * {
 *   address1, __field_offset(CS_PersistentDataOffsets_t, id1),
 *   address2, __field_offset(CS_PersistentDataOffsets_t, id2),
 *   address3, __field_offset(CS_PersistentDataOffsets_t, id3),
 * };
 *
 * Please pay attention: records in csPersistentMemTable must be in the same order and under
 * the same features as defined in CS_PersistentDataOffsets_t offsets structure.
 */

/* CS_PersistentDataOffsets_t structure used to automatically calculate
 * offsets for csPersistentMemTable.
 * Variables of this structure never instantiated. */
#ifdef _COMMISSIONING_
BEGIN_PACK
typedef struct PACK
{
  ExtAddr_t CS_UID_ID;
  int8_t CS_RF_TX_POWER_ID;
#ifndef _MAC2_
  ExtPanId_t CS_EXT_PANID_ID;
  uint32_t CS_CHANNEL_MASK_ID;
  uint8_t CS_CHANNEL_PAGE_ID;
  DeviceType_t CS_DEVICE_TYPE_ID;
  bool CS_RX_ON_WHEN_IDLE_ID;
  bool CS_COMPLEX_DESCRIPTOR_AVAILABLE_ID;
  bool CS_USER_DESCRIPTOR_AVAILABLE_ID;
  UserDescriptor_t CS_ZDP_USER_DESCRIPTOR_ID;
  PanId_t CS_NWK_PANID_ID;
  bool CS_NWK_PREDEFINED_PANID_ID;
  ShortAddr_t CS_NWK_ADDR_ID;
  bool CS_NWK_UNIQUE_ADDR_ID;
  bool CS_DTR_WAKEUP_ID;
#if defined(_POWER_FAILURE_)
  ShortAddr_t CS_NWK_PARENT_ADDR_ID;
  uint8_t CS_NWK_DEPTH_ID;
  ExtPanId_t CS_NWK_EXT_PANID_ID;
  uint8_t CS_NWK_LOGICAL_CHANNEL_ID;
  uint8_t CS_POWER_FAILURE_ID;
#endif /* _POWER_FAILURE_ */
#if defined(_SECURITY_)
  ExtAddr_t CS_APS_TRUST_CENTER_ADDRESS_ID;
  uint8_t CS_ZDO_SECURITY_STATUS_ID;
#endif /* _SECURITY_ */
#if defined(_POWER_FAILURE_)
  uint8_t CS_NEIB_TABLE_ID[CS_NEIB_TABLE_SIZE * sizeof(Neib_t)];
  #if defined(_BINDING_) && (CS_APS_BINDING_TABLE_SIZE > 0)
    ApsBindingEntry_t CS_APS_BINDING_TABLE_ID[CS_APS_BINDING_TABLE_SIZE];
  #endif /* _BINDING_ */
  #if defined(_SECURITY_)
    NWK_SecurityIB_t CS_NWK_SECURITY_IB_ID;
    NWK_SecurityKey_t CS_NWK_SECURITY_KEYS_ID[CS_NWK_SECURITY_KEYS_AMOUNT];
  #endif /* _SECURITY_ */
#endif /* _POWER_FAILURE_ */
#if defined (_GROUP_TABLE_)
  uint8_t CS_GROUP_TABLE_ID[(CS_GROUP_TABLE_SIZE * sizeof(NWK_GroupTableEntry_t))];
#endif /* _GROUP_TABLE_ */
#if defined(_SECURITY_)
  #if defined(_TC_PERMISSION_TABLE_) && CS_MAX_TC_ALLOWED_DEVICES_AMOUNT > 0
    uint8_t CS_TC_PERMISSION_TABLE_ID[sizeof(ExtAddr_t) * CS_MAX_TC_ALLOWED_DEVICES_AMOUNT];
  #endif
  #if defined (_HIGH_SECURITY_) && CS_APS_KEY_PAIR_DESCRIPTORS_AMOUNT > 0
    uint8_t CS_APS_KEY_PAIR_DESCRIPTORS_ID[(CS_APS_KEY_PAIR_DESCRIPTORS_AMOUNT * sizeof(ApsKeyPairDescriptor_t))];
  #endif
#endif /* _SECURITY_ */
#endif /* _MAC2_ */
} CS_PersistentDataOffsets_t;
END_PACK
/******************************************************************************
                    External variables section
******************************************************************************/
extern SIB_t csSIB;
extern NIB_t csNIB;
extern ZIB_t csZIB;
extern PIB_t csPIB;
extern CS_StackBuffers_t stackBuffers;
#ifdef _HIGH_SECURITY_
extern CS_ReadOnlyItems_t PROGMEM_DECLARE(csReadOnlyItems);
#endif

/******************************************************************************
                    Global variables section
******************************************************************************/
/* Table of Configuration Server persistent parameters, stored in FLASH.
 * This table is used by Persist Data Server, which performs parameters storing
 * and backuping from persistent memory.
 * */
PROGMEM_DECLARE(const CS_PersistantData_t csPersistentMemTable[]) =
{
  PERSISTREC(CS_UID_ID,                          &csPIB.macAttr.extAddr),
  PERSISTREC(CS_RF_TX_POWER_ID,                  &csSIB.csRfTxPower),
#ifndef _MAC2_
  PERSISTREC(CS_EXT_PANID_ID,                    &csSIB.csExtPANID),
  PERSISTREC(CS_CHANNEL_MASK_ID,                 &csZIB.channelMask),
  PERSISTREC(CS_CHANNEL_PAGE_ID,                 &csNIB.channelPage),
  PERSISTREC(CS_DEVICE_TYPE_ID,                  &csNIB.deviceType),
  PERSISTREC(CS_RX_ON_WHEN_IDLE_ID,              &csSIB.csRxOnWhenIdle),
  PERSISTREC(CS_COMPLEX_DESCRIPTOR_AVAILABLE_ID, &csSIB.csComplexDescriptorAvailable),
  PERSISTREC(CS_USER_DESCRIPTOR_AVAILABLE_ID,    &csSIB.csUserDescriptorAvailable),
  PERSISTREC(CS_ZDP_USER_DESCRIPTOR_ID,          &csSIB.csUserDescriptor),
  PERSISTREC(CS_NWK_PANID_ID,                    &csSIB.csNwkPanid),
  PERSISTREC(CS_NWK_PREDEFINED_PANID_ID,         &csSIB.csNwkPredefinedPanid),
  PERSISTREC(CS_NWK_ADDR_ID,                     &csNIB.networkAddress),
  PERSISTREC(CS_NWK_UNIQUE_ADDR_ID,              &csNIB.uniqueAddr),
  PERSISTREC(CS_DTR_WAKEUP_ID,                   &csSIB.csDtrWakeup),
#if defined(_POWER_FAILURE_)
  PERSISTREC(CS_NWK_PARENT_ADDR_ID,              &csNIB.parentNetworkAddress),
  PERSISTREC(CS_NWK_DEPTH_ID,                    &csNIB.depth),
  PERSISTREC(CS_NWK_EXT_PANID_ID,                &csNIB.extendedPanId),
  PERSISTREC(CS_NWK_LOGICAL_CHANNEL_ID,          &csSIB.csNwkLogicalChannel),
  PERSISTREC(CS_POWER_FAILURE_ID,                &csSIB.csPowerFailure),
#endif /* _POWER_FAILURE_ */
#if defined(_SECURITY_)
  PERSISTREC(CS_APS_TRUST_CENTER_ADDRESS_ID,     &csAIB.trustCenterAddress),
  PERSISTREC(CS_ZDO_SECURITY_STATUS_ID,          &csSIB.csZdoSecurityStatus),
#endif /* _SECURITY_ */

#if defined(_POWER_FAILURE_)
  PERSISTREC(CS_NEIB_TABLE_ID,                              &stackBuffers.csNeibTable),
  #if defined(_BINDING_) && (CS_APS_BINDING_TABLE_SIZE > 0)
  PERSISTREC(CS_APS_BINDING_TABLE_ID,                       &stackBuffers.csApsBindingTable),
  #endif /* _BINDING_ */
  #if defined(_SECURITY_)
  PERSISTREC(CS_NWK_SECURITY_IB_ID,                         &csNIB.securityIB),
  PERSISTREC(CS_NWK_SECURITY_KEYS_ID,                       &stackBuffers.csNwkSecKeys),
  #endif /* _SECURITY_ */
#endif /* _POWER_FAILURE_ */
#if defined (_GROUP_TABLE_)
  PERSISTREC(CS_GROUP_TABLE_ID,                             &stackBuffers.csGroupTable),
#endif /* _GROUP_TABLE_ */
#if defined(_SECURITY_)
  #if defined(_TC_PERMISSION_TABLE_) && CS_MAX_TC_ALLOWED_DEVICES_AMOUNT > 0
  PERSISTREC(CS_TC_PERMISSION_TABLE_ID,                     &stackBuffers.csTcDevicePermissionTable),
  #endif /* _TC_PERMISSION_TABLE_ */
  #if defined(_HIGH_SECURITY_) && CS_APS_KEY_PAIR_DESCRIPTORS_AMOUNT > 0
  PERSISTREC(CS_APS_KEY_PAIR_DESCRIPTORS_ID,                &stackBuffers.csApsKeyPairDescriptors),
  #endif /* _HIGH_SECURITY_ */
#endif /* _SECURITY_ */
#endif /* _MAC2_ */
};
#endif /* _COMMISSIONING_ */

/* Number of records in csPersistentMemTable*/
const uint8_t csPersistentItemsAmount = PERSISTENT_ITEMS_AMOUNT;

/* Total size of memory occupied by persistent parameters */
const uint16_t csPersistentMemorySize = PERSISTENT_MEMORY_SIZE + SYSTEM_BASE_EEPROM_ADDRESS;

/* eof csPersistentMem.c */
