/**************************************************************************//**
  \file apsAIB.h

  \brief Interface of APS information base.

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
   History:
    2010-10-19 Max Gekk - Created.
   Last change:
    $Id: apsAIB.h 17470 2011-06-10 11:31:00Z mgekk $
 ******************************************************************************/
/**//**
 *
 *  The APS information base comprises the attributes required to manage the APS
 * layer of a device. The attributes of the AIB are listed in ZigBee spec r19,
 * Table 2.24, page 61. The securityrelated AIB attributes are described in
 * sub-clause 4.4.10, page 489.
 **/
#if !defined _APS_AIB_H
#define _APS_AIB_H

/******************************************************************************
                               Includes section
 ******************************************************************************/
#include <nwk.h>
#include <configServer.h>

/******************************************************************************
                    Definitions section
******************************************************************************/
#define APS_AIB_NONMEMBER_RADIUS_DEFAULT_VALUE 2

/******************************************************************************
                                Types section
 ******************************************************************************/
/**//**
 * \struct AIB_t apsAIB.h "aps.h"
 *
 * \brief Attributes in APS Information Base.
 **/
typedef struct
{
  /** The value to be used for the NonmemberRadius parameter when using
   * NWK layer multicast. Valid range: 0 - 7, default value - 2. */
  NwkRadius_t nonMemberRadius;
#if defined _SECURITY_
  /** \ref Endian "[LE]" Identifies the address of the device's Trust Center. */
  ExtAddr_t  trustCenterAddress;
#endif /* _SECURITY_ */
#ifdef _APS_FRAGMENTATION_
  uint8_t fragWindowSize;
#ifdef _CERTIFICATION_
  uint8_t fragDropMask;
#endif /* _CERTIFICATION_  */
#endif /* _APS_FRAGMENTATION_ */
} AIB_t;

/******************************************************************************
                           Global variables section
 ******************************************************************************/
extern AIB_t csAIB; /*!< APS Information Base */

/******************************************************************************
                              Prototypes section
 ******************************************************************************/
#ifdef _SECURITY_
/**************************************************************************//**
  \brief Check wether supplied address is a Trust Center address

  \param[in] addr - pointer to required address

  \return true, if required address is TC address, false - otherwise
******************************************************************************/
INLINE bool APS_IsTcAddress(const ExtAddr_t *addr)
{
  return 0 == memcmp(addr, &csAIB.trustCenterAddress, sizeof(ExtAddr_t));
}

/**************************************************************************//**
  \brief Get pointer to Trust Center address

  \return pointer to Trust Center extended address
******************************************************************************/
INLINE ExtAddr_t* APS_GetTrustCenterAddress(void)
{
  return &csAIB.trustCenterAddress;
}

/**************************************************************************//**
  \brief Set Trust Center address

  \param[in] addr - pointer to address to set

  \return none
******************************************************************************/
INLINE void APS_SetTrustCenterAddress(const ExtAddr_t *addr)
{
  memcpy(&csAIB.trustCenterAddress, addr, sizeof(ExtAddr_t));
}

#endif /* _SECURITY_ */

#endif /* _APS_AIB_H */
/** eof apsAIB.h */

