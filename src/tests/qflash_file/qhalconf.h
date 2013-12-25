/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    templates/halconf.h
 * @brief   HAL configuration header.
 * @details HAL configuration file, this file allows to enable or disable the
 *          various device drivers from your application. You may also use
 *          this file in order to override the device drivers default settings.
 *
 * @addtogroup HAL_CONF
 * @{
 */

#ifndef _QHALCONF_H_
#define _QHALCONF_H_

/**
 * @brief   Enables the SERIAL 485 subsystem.
 */
#if !defined(HAL_USE_SERIAL_485) || defined(__DOXYGEN__)
#define HAL_USE_SERIAL_485          FALSE
#endif

/**
 * @brief   Enables the FLASH_JEDEC_SPI subsystem.
 */
#if !defined(HAL_USE_FLASH_JEDEC_SPI) || defined(__DOXYGEN__)
#define HAL_USE_FLASH_JEDEC_SPI     FALSE
#endif

/**
 * @brief   Enables the FLASH file subsystem.
 */
#if !defined(HAL_USE_FLASH_FILE) || defined(__DOXYGEN__)
#define HAL_USE_FLASH_FILE          TRUE
#endif

/**
 * @brief   Enables the FLASH partition subsystem.
 */
#if !defined(HAL_USE_FLASH_PARTITION) || defined(__DOXYGEN__)
#define HAL_USE_FLASH_PARTITION     TRUE
#endif

/*===========================================================================*/
/* SERIAL_485 driver related settings.                                       */
/*===========================================================================*/

/**
 * @brief   Default bit rate.
 * @details Configuration parameter, this is the baud rate selected for the
 *          default configuration.
 */
#if !defined(SERIAL_485_DEFAULT_BITRATE) || defined(__DOXYGEN__)
#define SERIAL_485_DEFAULT_BITRATE      38400
#endif

/**
 * @brief   Serial buffers size.
 * @details Configuration parameter, you can change the depth of the queue
 *          buffers depending on the requirements of your application.
 * @note    The default is 16 bytes for both the transmission and receive
 *          buffers.
 */
#if !defined(SERIAL_485_BUFFERS_SIZE) || defined(__DOXYGEN__)
#define SERIAL_485_BUFFERS_SIZE         16
#endif

/**
 * @brief   Serial buffer size RX.
 * @details Configuration parameter, you can change the depth of the RX
 *          buffer depending on the requirements of your application.
 * @note    The default is 1 byte. Read completes only when buffer has been
 *          filled completely.
 */
#if !defined(SERIAL_485_UART_RX_BUFFER_SIZE) || defined(__DOXYGEN__)
#define SERIAL_485_UART_RX_BUFFER_SIZE     1
#endif

/**
 * @brief   Serial buffer size TX.
 * @details Configuration parameter, you can change the depth of the TX
 *          buffer depending on the requirements of your application.
 * @note    The default is 16 bytes.
 */
#if !defined(SERIAL_485_UART_TX_BUFFER_SIZE) || defined(__DOXYGEN__)
#define SERIAL_485_UART_TX_BUFFER_SIZE     16
#endif

/**
 * @brief   Uart driver upper pointer
 * @details Required pointer to upper level driver
 * @note    This is required for 485 driver to work
 */
#if defined(HAL_USE_SERIAL_485)
#if !defined(UART_DRIVER_EXT_FIELDS)
#define UART_DRIVER_EXT_FIELDS void* uldp;
#endif
#endif

/*===========================================================================*/
/* FLASH_JEDEC_SPI driver related settings                                   */
/*===========================================================================*/

/**
 * @brief   Delays insertions.
 * @details If enabled this options inserts delays into the flash waiting
 *          routines releasing some extra CPU time for the threads with
 *          lower priority, this may slow down the driver a bit however.
 *          This option is recommended also if the SPI driver does not
 *          use a DMA channel and heavily loads the CPU.
 */
#if !defined(FLASH_JEDEC_SPI_NICE_WAITING) || defined(__DOXYGEN__)
#define FLASH_JEDEC_SPI_NICE_WAITING             TRUE
#endif

/**
 * @brief   Enables the @p fjsAcquireBus() and @p fjsReleaseBus() APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(FLASH_JEDEC_SPI_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define FLASH_JEDEC_SPI_USE_MUTUAL_EXCLUSION     TRUE
#endif

/*===========================================================================*/
/* FLASH_FILE driver related settings                                        */
/*===========================================================================*/

/**
 * @brief   Enables the @p ffileAcquireBus() and @p ffileReleaseBus() APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(FLASH_FILE_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define FLASH_FILE_USE_MUTUAL_EXCLUSION          FALSE
#endif

/*===========================================================================*/
/* FLASH_PARTITION driver related settings                                   */
/*===========================================================================*/

/**
 * @brief   Enables the @p fpartAcquireBus() and @p fpartReleaseBus() APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(FLASH_PARTITION_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define FLASH_PARTITION_USE_MUTUAL_EXCLUSION     TRUE
#endif

#endif /* _QHALCONF_H_ */

/** @} */
