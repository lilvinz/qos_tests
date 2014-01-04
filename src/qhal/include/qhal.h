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

#include "qhal_lld.h"

/* Abstract interfaces.*/
#include "qio_nvm.h"

/* Shared headers.*/

/* Layered drivers.*/
#include "qflash.h"

/* Complex drivers.*/
#include "qserial_485.h"
#include "qflash_jedec_spi.h"
#include "qnvm_partition.h"
#include "qnvm_file.h"
#include "qnvm_mirror.h"

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

