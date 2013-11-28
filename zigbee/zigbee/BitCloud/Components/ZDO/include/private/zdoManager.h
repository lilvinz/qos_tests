/**************************************************************************//**
  \file zdoManager.h

  \brief ZDO internal management functions interface

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
  History:
    28.10.09 A. Taradov - Created
******************************************************************************/
#ifndef _ZDOMANAGER_H
#define _ZDOMANAGER_H

/******************************************************************************
                    Includes section
******************************************************************************/
#include <zdoMem.h>

/******************************************************************************
                    Types section
******************************************************************************/
typedef enum _ZdoModuleId_t
{
  ZDO_START_NETWORK_MODULE_ID       = (1 << 0),
  ZDO_RESET_NETWORK_MODULE_ID       = (1 << 1),
  ZDO_LEAVE_NETWORK_MODULE_ID       = (1 << 2),
  ZDO_BINDING_MANAGER_MODULE_ID     = (1 << 3),
  ZDO_DISCOVERY_MANAGER_MODULE_ID   = (1 << 4),
  ZDO_NODE_MANAGER_MODULE_ID        = (1 << 5),
  ZDO_NWK_MANAGER_MODULE_ID         = (1 << 6),
  ZDO_POWER_MANAGER_MODULE_ID       = (1 << 7),
  ZDO_SECURITY_MANAGER_MODULE_ID    = (1 << 8),
  ZDO_ZDP_FRAME_PROCESSOR_MODULE_ID = (1 << 9),
  ZDO_ZDP_MANAGER_MODULE_ID         = (1 << 10),
  ZDO_NETWORK_STATUS_MODULE_ID      = (1 << 11),
  ZDO_TC_SWAPOUT_MODULE_ID          = (1 << 12),
} ZdoModuleId_t;

typedef enum _ZdoModuleSet_t
{
  ZDO_INITIAL_MODULE_SET        = 0,
  ZDO_ALL_DISABLED_MODULE_SET   = 0,
  ZDO_READY_MODULE_SET          = ZDO_START_NETWORK_MODULE_ID | ZDO_RESET_NETWORK_MODULE_ID |
                                  ZDO_POWER_MANAGER_MODULE_ID | ZDO_TC_SWAPOUT_MODULE_ID,
  ZDO_START_NETWORK_MODULE_SET  = ZDO_ZDP_MANAGER_MODULE_ID,
  ZDO_IN_NETWORK_MODULE_SET     = ZDO_BINDING_MANAGER_MODULE_ID | ZDO_DISCOVERY_MANAGER_MODULE_ID |
                                  ZDO_NODE_MANAGER_MODULE_ID | ZDO_NWK_MANAGER_MODULE_ID |
                                  ZDO_POWER_MANAGER_MODULE_ID | ZDO_SECURITY_MANAGER_MODULE_ID |
                                  ZDO_ZDP_FRAME_PROCESSOR_MODULE_ID | ZDO_ZDP_MANAGER_MODULE_ID |
                                  ZDO_NETWORK_STATUS_MODULE_ID | ZDO_RESET_NETWORK_MODULE_ID |
                                  ZDO_LEAVE_NETWORK_MODULE_ID | ZDO_TC_SWAPOUT_MODULE_ID,
  ZDO_REJOIN_NETWORK_MODULE_SET = ZDO_ZDP_MANAGER_MODULE_ID,
  ZDO_LEAVING_MODULE_SET        = ZDO_LEAVE_NETWORK_MODULE_ID | ZDO_ZDP_MANAGER_MODULE_ID,
  ZDO_TC_SWAPOUT_MODULE_SET     = ZDO_ZDP_MANAGER_MODULE_ID,
  ZDO_RESET_NETWORK_MODULE_SET  = 0,
} ZdoModuleSet_t;

/******************************************************************************
                    Prototypes section
******************************************************************************/
void zdoUpdateRandSeed(void);
/******************************************************************************
                    Implementations section
******************************************************************************/

/******************************************************************************
  Enable certain modules
  Parameters:
    modules - modules to enable
  Return:
    none
******************************************************************************/
inline static void zdoEnable(uint16_t modules)
{
  zdoMem.enabledModules = modules;
}

/******************************************************************************
  Check if certain module is enabled
  Parameters:
    module - module id to check
  Return:
    true  - module is enabled
    false - module is disabled
******************************************************************************/
inline static bool zdoEnabled(uint16_t module)
{
  return (zdoMem.enabledModules & module);
}

/******************************************************************************
  Set 'Out of Network' flag for correct state restore after power failure.
  Parameters:
    none
  Return:
    none
******************************************************************************/
inline static void zdoSetOutOfNetworkFlag(void)
{
#if defined(_POWER_FAILURE_)
  ExtPanId_t nwkExtPanId = NWK_NO_EXT_PANID;
  NWK_SetExtPanId(&nwkExtPanId);

  zdoMem.zdoStartNetwork.powerFailureControl.networkOperationsAccepted = 0U;
#endif
}

#endif // _ZDOMANAGER_H

// eof zdoManager.h
