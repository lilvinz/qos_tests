/*
 * qhal.c
 *
 *  Created on: 22.12.2013
 *      Author: vke
 */

/**
 * @file    qhal.c
 * @brief   Quantec HAL subsystem code.
 *
 * @addtogroup QHAL
 * @{
 */

#include "qhal.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Quantec HAL initialization.
 * @details This function invokes the low level initialization code for
 *          additional drivers. It has to be invoked from boardInit().
 *
 * @init
 */
void qhalInit(void)
{
#if HAL_USE_SERIAL_485 || defined(__DOXYGEN__)
    sd485Init();
#endif
#if HAL_USE_FLASH_JEDEC_SPI || defined(__DOXYGEN__)
    fjsInit();
#endif
}

/** @} */
