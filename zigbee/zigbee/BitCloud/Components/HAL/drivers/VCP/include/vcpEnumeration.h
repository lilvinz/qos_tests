/****************************************************************************//**
  \file vcpEnumeration.h

  \brief Declaration of enumeration process command.

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
  History:
    12/09/08 A. Khromykh - Created
*******************************************************************************/
#ifndef _VCPENUMERATION_H
#define _VCPENUMERATION_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <types.h>

/******************************************************************************
                   Prototypes section
******************************************************************************/
/******************************************************************************
Standard usb request handler

Parameters:
  data - pointer to host's request
******************************************************************************/
void runtimeRequestHandler(uint8_t *data);

#if defined(AT91SAM7X256) || defined(AT91SAM3S4C)
/******************************************************************************
send zero-length packet through control pipe
******************************************************************************/
void sendZLP(void);
#endif

#endif /* _VCPENUMERATION_H */
