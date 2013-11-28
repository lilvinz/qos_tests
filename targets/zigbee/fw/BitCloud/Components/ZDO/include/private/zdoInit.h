/**************************************************************************//**
  \file zdoInit.h

  \brief Interface for ZDO initialization routines

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
  History:
    27.10.09 A. Taradov - Created
******************************************************************************/

#ifndef _ZDOINIT_H
#define _ZDOINIT_H

/******************************************************************************
                    Includes section
******************************************************************************/
#include <zdoMem.h>
#include <zdoStartNetwork.h>
#include <zdoManager.h>

/******************************************************************************
                   Prototypes section
******************************************************************************/

/******************************************************************************
  \brief ZDO initialization routine
******************************************************************************/
void zdoInitHandler(void);

#endif // _ZDOINIT_H

// eof zdoInit.h
