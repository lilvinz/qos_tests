/**************************************************************************//**
  \file  halDiagnostic.h

  \brief Implementation of diagnostics defines.

  \author
      Atmel Corporation: http://www.atmel.com \n
      Support email: avr@atmel.com

    Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
    Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      20/05/09 D. Kasyanov - Created
 ******************************************************************************/

#ifndef _HALDIAGNOSTIC_H
#define _HALDIAGNOSTIC_H

#include <halDbg.h>

#if defined (MEASURE)
    #define BEGIN_MEASURE
    #define END_MEASURE(code)
#else
    #define BEGIN_MEASURE
    #define END_MEASURE(code)
#endif


#endif /* _HALDIAGNOSTIC_H */

