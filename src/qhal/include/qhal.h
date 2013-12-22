/*
 * qhal.h
 *
 *  Created on: 22.12.2013
 *      Author: vke
 */

#ifndef QHAL_H_
#define QHAL_H_

/**
 * @file    qhal.h
 * @brief   Quantec HAL subsystem header.
 *
 * @addtogroup QHAL
 * @{
 */

#include "hal.h"

#include "qhalconf.h"

/* Abstract interfaces.*/

/* Shared headers.*/

/* Layered drivers.*/

/* Complex drivers.*/
#include "qserial_485.h"

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void qhalInit(void);
#ifdef __cplusplus
}
#endif

#endif /* QHAL_H_ */

/** @} */

