/******************************************************************************
  \file csDbg.h

  \brief
    Persistent Data Server debug info

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2010 , Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
    29.10.10 A. Razinkov - Created.
******************************************************************************/

#ifndef _PDSDBG_H_
#define _PDSDBG_H_

/******************************************************************************
                    Includes section
******************************************************************************/
#include <dbg.h>

/******************************************************************************
                    Types section
******************************************************************************/

typedef enum
{
  PDS_PDSPREPAREMEMORYACCESS0,
  PDS_PDSPREPAREMEMORYACCESS1
} PDS_DbgCodeId_t;

#endif /* CSDBG_H_ */
