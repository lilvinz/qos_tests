/**************************************************************************//**
  \file tcCommonPrivat.h

  \brief Trust Centre privat common definnitions header file.

  \author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

  \internal
    History:
      2007.12 - ALuzhetsky created.
******************************************************************************/
#ifndef _TCCOMMONPRIVATH
#define _TCCOMMONPRIVATH

/******************************************************************************
                             Includes section.
******************************************************************************/
#include <configServer.h>
#include <nwk.h>

/******************************************************************************
                             Definitions section.
******************************************************************************/
#define FIELD_OFFSET(struct_type, field_name) (((uint8_t*) &((struct_type *)(NULL))->field_name) - (uint8_t*)NULL)
#define GET_STRUCT_BY_FIELD_POINTER(struct_type, field_name, field_pointer) ((struct_type *) (((uint8_t *) field_pointer) - FIELD_OFFSET(struct_type, field_name)))

#endif // _TCCOMMONPRIVATH
// eof tcCommonPrivat.h
