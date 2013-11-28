/******************************************************************************
  \file csMem.c

  \brief
    BitCloud information bases memory

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2010 , Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
    21.10.10 A. Razinkov - Created.
******************************************************************************/

/******************************************************************************
                    Includes section
******************************************************************************/
#include <csBuffers.h>
#include <macenvMem.h>
#ifndef _MAC2_
#include <zdoZib.h>
#endif
#include <csSIB.h>
#include <csDefaults.h>
#include <configServer.h>

/******************************************************************************
                    Global variables section
******************************************************************************/

/* BitCloud information bases allocation */
PIB_t csPIB;
#ifndef _MAC2_
NIB_t csNIB;
ZIB_t csZIB;
AIB_t csAIB;
#endif
SIB_t csSIB;

/* BitCloud memory buffers allocation */
CS_StackBuffers_t stackBuffers;

/* FLASH memory allocation to store read-only parameters */

/* Default information bases item values */
#ifndef _MAC2_
NIB_t PROGMEM_DECLARE(defaultNIB) =
{
#if defined _SECURITY_
  .securityIB = {
    .securityLevel = CS_SECURITY_LEVEL,
    .secureAllFrames = CS_SECURITY_ALL_FRAMES,
    .keyAmount = CS_NWK_SECURITY_KEYS_AMOUNT,
    .key = stackBuffers.csNwkSecKeys,
    .counter = stackBuffers.csNwkSecCounters
  },
#endif
  .deviceType = CS_DEVICE_TYPE,
  .addrAlloc = CS_ADDRESS_ASSIGNMENT_METHOD,
  .symLink = true,
  .uniqueAddr = CS_NWK_UNIQUE_ADDR,
  .stackProfile = CS_STACK_PROFILE,
  .protocolVersion = CS_PROTOCOL_VERSION,
  .maxDepth = CS_MAX_NETWORK_DEPTH,
  .maxNeighborRouteCost = CS_MAX_NEIGHBOR_ROUTE_COST,
  .channelPage = CS_CHANNEL_PAGE,
  .logicalChannel = 0,
  .networkAddress = CCPU_TO_LE16(CS_NWK_ADDR),
  .panId = CCPU_TO_LE16(CS_NWK_PANID),
  .extendedPanId = CCPU_TO_LE64(CS_NWK_EXT_PANID), /* uses as joined-to-network flag */
  .maxRouters = CS_MAX_CHILDREN_ROUTER_AMOUNT,
  .maxEndDevices = CS_MAX_CHILDREN_AMOUNT - CS_MAX_CHILDREN_ROUTER_AMOUNT
};

ZIB_t PROGMEM_DECLARE(defaultZIB) =
{
  .permitJoinDuration = CS_PERMIT_DURATION,
  .channelMask = CS_CHANNEL_MASK,
  .zdpResponseTimeout = CS_ZDP_RESPONSE_TIMEOUT,
  .scanDuration = CS_SCAN_DURATION,
};

AIB_t PROGMEM_DECLARE(defaultAIB) =
{
  .nonMemberRadius = APS_AIB_NONMEMBER_RADIUS_DEFAULT_VALUE, /* See ZigBee spec r19, Table 2.24. */
#ifdef _SECURITY_
  .trustCenterAddress = CCPU_TO_LE64(CS_APS_TRUST_CENTER_ADDRESS),
#endif /* _SECURITY_ */
};
#endif /* !_MAC2_*/

SIB_t PROGMEM_DECLARE(defaultSIB) =
{
  /* MAC parameters */
  .csMacTransactionTime = CS_MAC_TRANSACTION_TIME,
  .csRfTxPower = CS_RF_TX_POWER,
#ifdef AT86RF212
  .csLbtMode = CS_LBT_MODE,
#endif //AT86RF212
#ifndef _MAC2_

  /* NWK parameters */
  .csNwkPanid = CCPU_TO_LE16(CS_NWK_PANID),
  .csNwkUseMulticast = CS_NWK_USE_MULTICAST,
  .csNwkMaxLinkStatusFailures = CS_NWK_MAX_LINK_STATUS_FAILURES,
  .csNwkEndDeviceMaxFailures = CS_NWK_END_DEVICE_MAX_FAILURES,
  .csNwkLogicalChannel = 0,
  .csNwkPredefinedPanid = CS_NWK_PREDEFINED_PANID,
#ifdef _NWK_CONCENTRATOR_
  .csNwkConcentratorDiscoveryTime = NWK_CONCENTRATOR_DISCOVERY_TIME,
#endif

  /* ZDO parameters */
  .csEndDeviceSleepPeriod = CS_END_DEVICE_SLEEP_PERIOD,
  .csFfdSleepPeriod = CS_FFD_SLEEP_PERIOD,
  .csRxOnWhenIdle = (CS_DEVICE_TYPE == DEVICE_TYPE_END_DEVICE) ? CS_RX_ON_WHEN_IDLE : true,
  .csComplexDescriptorAvailable = CS_COMPLEX_DESCRIPTOR_AVAILABLE,
  .csUserDescriptorAvailable = CS_USER_DESCRIPTOR_AVAILABLE,
  .csIndirectPollRate = CS_INDIRECT_POLL_RATE,
  .csZdoJoinAttempts = CS_ZDO_JOIN_ATTEMPTS,
  .csZdoJoinInterval = CS_ZDO_JOIN_INTERVAL,
  .csExtPANID = CCPU_TO_LE64(CS_EXT_PANID), /* user-defined PAN ID */
#ifdef _TC_SWAPOUT_
  .csZdoTcKeepAliveInterval = CS_ZDO_TC_KEEP_ALIVE_INTERVAL,
#endif // _TC_SWAPOUT_

  /* APS parameters */
  .csApsMaxFrameRetries = CS_APS_MAX_FRAME_RETRIES,
#ifdef _APS_FRAGMENTATION_
  .csApsMaxTransmissionWindowSize = CS_APS_MAX_TRANSMISSION_WINDOW_SIZE,
#endif /* _APS_FRAGMENTATION_ */

  /* PDS parameters*/
  .csDtrWakeup = CS_DTR_WAKEUP,

  /* ZCL parameters */
#ifdef _ZCL_
  .csZclAttributeReportTimerInterval = CS_ZCL_ATTRIBUTE_REPORT_TIMER_INTERVAL,
  #ifdef _OTAU_
  .csOtauDefaultServerAddress = CS_ZCL_OTAU_DEFAULT_UPGRADE_SERVER_IEEE_ADDRESS,
  .csOtauServerDiscoveryPeriod = CS_ZCL_OTAU_SERVER_DISCOVERY_PERIOD,
  #endif /* _OTAU_ */
#endif /* _ZCL_ */

    /* Other parameters */
#ifdef _COMMISSIONING_
  .csPdsStoringInterval = CS_PDS_STORING_INTERVAL,
  #ifdef _POWER_FAILURE_
  .csPowerFailure = CS_POWER_FAILURE,
  #endif /* _POWER_FAILURE_ */
#endif /* _COMMISSIONING_ */

    /* Security parameters */
#ifdef _SECURITY_
  .csSecurityOn = CS_SECURITY_ON,
  .csZdoSecurityStatus = CS_ZDO_SECURITY_STATUS,
#endif /* _SECURITY_ */
#endif /* _MAC2_ */
};

#ifdef _SECURITY_
uint8_t defaultKey[SECURITY_KEY_SIZE] = CS_NETWORK_KEY;
#endif /* _SECURITY_ */

CS_ReadOnlyItems_t PROGMEM_DECLARE(csReadOnlyItems) =
{
  /* MAC parameters */
  .csMacFrameRxBufferSize = CS_MAC_FRAME_RX_BUFFER_SIZE,
#ifdef _MAC_BAN_NODE_
  .csMacBanTableSize = CS_BAN_TABLE_SIZE,
#endif /* _MAC_BAN_NODE_ */

  /* NWK parameters */
#ifndef _MAC2_
  .csNwkRouteCacheSize = CS_ROUTE_CACHE_SIZE,
  .csNeibTableSize = CS_NEIB_TABLE_SIZE,
  .csMacPanDescriptorAmount = CS_NEIB_TABLE_SIZE,
  .csMaxChildrenAmount = CS_MAX_CHILDREN_AMOUNT,
  .csMaxChildrenRouterAmount = CS_MAX_CHILDREN_ROUTER_AMOUNT,
  .csMaxNetworkDepth = CS_MAX_NETWORK_DEPTH,
  .csAddressAssignmentMethod = CS_ADDRESS_ASSIGNMENT_METHOD,
  .csNwkBuffersAmount = CS_NWK_BUFFERS_AMOUNT,
  .csJoinIndObjAmount = CS_JOIN_IND_OBJ_AMOUNT,
  .csRouteTableSize = CS_ROUTE_TABLE_SIZE,
  .csAddressMapTableSize = CS_ADDRESS_MAP_TABLE_SIZE,
  .csRouteDiscoveryTableSize = CS_ROUTE_DISCOVERY_TABLE_SIZE,
  .csNwkBttSize = CS_NWK_BTT_SIZE,
#ifdef _GROUP_TABLE_
  .csGroupTableSize = CS_GROUP_TABLE_SIZE,
#endif /* _GROUP_TABLE_ */

  /* APS parameters */
  .csApsDataReqBuffersAmount = CS_APS_DATA_REQ_BUFFERS_AMOUNT,
  .csApsAckFrameBuffesAmount = CS_APS_ACK_FRAME_BUFFERS_AMOUNT,
  .csDuplicateRejectionTableSize = CS_DUPLICATE_REJECTION_TABLE_SIZE,
#ifdef _BINDING_
  .csApsBindingTableSize = CS_APS_BINDING_TABLE_SIZE,
#endif /* _BINDING_ */
#ifdef _APS_FRAGMENTATION_
  .csApsMaxBlocksAmount = CS_APS_MAX_BLOCKS_AMOUNT,
  .csApsBlockSize = CS_APS_BLOCK_SIZE,
#endif /* _APS_FRAGMENTATION_ */

#ifdef _ZCL_
  .csZclMemoryBuffersAmount = CS_ZCL_MEMORY_BUFFERS_AMOUNT,
  #ifdef _OTAU_
    .csOtauDiscoveredServerAmount = CS_ZCL_OTAU_DISCOVERED_SERVER_AMOUNT,
    .csOtauClientSessionAmount = CS_ZCL_OTAU_CLIENT_SESSION_AMOUNT,
  #endif /* _OTAU_ */
#endif /* _ZCL_ */

#ifdef _SECURITY_
  .csApsSecurityTimeoutPeriod = CS_APS_SECURITY_TIMEOUT_PERIOD,
#ifdef _TRUST_CENTRE_
  .csMaxTcAuthenticProcessAmount = CS_MAX_TC_AUTHENTIC_PROCESS_AMOUNT,
  #ifdef _TC_PERMISSION_TABLE_
  .csMaxTcAllowedDevicesAmount = CS_MAX_TC_ALLOWED_DEVICES_AMOUNT,
  #endif /* _TC_PERMISSION_TABLE_ */
#endif /* _TRUST_CENTRE_ */
#ifdef _HIGH_SECURITY_
  #ifdef _TRUST_CENTRE_
  .csMaxTcEstablishKeyAmount = CS_MAX_TC_ESTABLISH_KEY_AMOUNT,
  #endif /* _TRUST_CENTRE_ */
  .csApsKeyPairDescriptorsAmount = CS_APS_KEY_PAIR_DESCRIPTORS_AMOUNT,
#endif /* _HIGH_SECURITY_ */
#endif /* _SECURITY_ */
  /* Other parameters */
  .csStackVersion = CS_STACK_VERSION,
  .csManufacturerCode = CCPU_TO_LE16(CS_MANUFACTURER_CODE),
#endif /* !_MAC2_*/
};

/*
 * \brief Configuration Server items information table stored in FLASH.
 *        Table record contains informaion about corresponding item displacement and size.
 *        Records are automatically sorted by item location and type
 *        (atomic parameter or memory region).
 */
#define SEPARATOR(id) {{NULL}, 0},
#define DUMMY_MEMORY(id) {{NULL}, 0},
#define FLASH_PARAMETER(id, addr) {{(const void FLASH_PTR*)&(addr)}, sizeof(addr)},
#define RAM_PARAMETER(id, addr) {{(const void FLASH_PTR*)&(addr)}, sizeof(addr)},
#define MEMORY_REGION(id, addr) {{(const void FLASH_PTR*)&(addr)}, 0},
CS_MemoryItem_t PROGMEM_DECLARE(csMemoryItems[]) =
{
  #include "csParamTable.h"
};
#undef SEPARATOR
#undef DUMMY_MEMORY
#undef FLASH_PARAMETER
#undef RAM_PARAMETER
#undef MEMORY_REGION

/******************************************************************************
                    Implementation section
******************************************************************************/

/******************************************************************************
\brief Set Configuration Server memory to its default state
******************************************************************************/
void csSetToDefault(void)
{
  /* Restore information bases default values from FLASH */
  memset(&csPIB, 0x00, sizeof(PIB_t));
  csPIB.macAttr.extAddr = CCPU_TO_LE64(CS_UID);
  csPIB.macAttr.maxFrameTransmissionTime = CS_MAX_FRAME_TRANSMISSION_TIME;

#ifndef _MAC2_
  memcpy_P(&csNIB, &defaultNIB, sizeof(NIB_t));
  memcpy_P(&csZIB, &defaultZIB, sizeof(ZIB_t));
  memcpy_P(&csAIB, &defaultAIB, sizeof(AIB_t));
#endif
  memcpy_P(&csSIB, &defaultSIB, sizeof(SIB_t));
  memset(&stackBuffers, 0x00, sizeof(CS_StackBuffers_t));
}

/* eof csMem.c */
