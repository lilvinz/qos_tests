/*
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
 * @file    templates/qhalconf.h
 * @brief   QHAL configuration header.
 * @details QHAL configuration file, this file allows to enable or disable the
 *          various device drivers from your application. You may also use
 *          this file in order to override the device drivers default settings.
 *
 * @addtogroup QHAL_CONF
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
 * @brief   Enables the NVM file subsystem.
 */
#if !defined(HAL_USE_NVM_FILE) || defined(__DOXYGEN__)
#define HAL_USE_NVM_FILE            FALSE
#endif

/**
 * @brief   Enables the NVM memory subsystem.
 */
#if !defined(HAL_USE_NVM_MEMORY) || defined(__DOXYGEN__)
#define HAL_USE_NVM_MEMORY          FALSE
#endif

/**
 * @brief   Enables the NVM partition subsystem.
 */
#if !defined(HAL_USE_NVM_PARTITION) || defined(__DOXYGEN__)
#define HAL_USE_NVM_PARTITION       FALSE
#endif

/**
 * @brief   Enables the NVM mirror subsystem.
 */
#if !defined(HAL_USE_NVM_MIRROR) || defined(__DOXYGEN__)
#define HAL_USE_NVM_MIRROR          FALSE
#endif

/**
 * @brief   Enables the NVM flash eeprom emulation subsystem.
 */
#if !defined(HAL_USE_NVM_FEE) || defined(__DOXYGEN__)
#define HAL_USE_NVM_FEE             FALSE
#endif

/**
 * @brief   Enables the internal FLASH subsystem.
 */
#if !defined(HAL_USE_FLASH) || defined(__DOXYGEN__)
#define HAL_USE_FLASH               FALSE
#endif

/**
 * @brief   Enables the LED subsystem.
 */
#if !defined(HAL_USE_LED) || defined(__DOXYGEN__)
#define HAL_USE_LED                 FALSE
#endif

/**
 * @brief   Enables the graphics display ILI9341 subsystem.
 */
#if !defined(HAL_USE_GD_ILI9341) || defined(__DOXYGEN__)
#define HAL_USE_GD_ILI9341          FALSE
#endif

/**
 * @brief   Enables the ms5541 driver.
 */
#if !defined(HAL_USE_MS5541) || defined(__DOXYGEN__)
#define HAL_USE_MS5541              FALSE
#endif

/**
 * @brief   Enables the ms58xx driver.
 */
#if !defined(HAL_USE_MS58XX) || defined(__DOXYGEN__)
#define HAL_USE_MS58XX              FALSE
#endif

/**
 * @brief   Enables the SERIAL VIRTUAL subsystem.
 */
#if !defined(HAL_USE_SERIAL_VIRTUAL) || defined(__DOXYGEN__)
#define HAL_USE_SERIAL_VIRTUAL      TRUE
#endif

/**
 * @brief   Enables the SERIAL FDX subsystem.
 */
#if !defined(HAL_USE_SERIAL_FDX) || defined(__DOXYGEN__)
#define HAL_USE_SERIAL_FDX          TRUE
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
#define SERIAL_485_DEFAULT_BITRATE  38400
#endif

/**
 * @brief   Serial 485 buffers size.
 * @details Configuration parameter, you can change the depth of the queue
 *          buffers depending on the requirements of your application.
 * @note    The default is 64 bytes for both the transmission and receive
 *          buffers.
 */
#if !defined(SERIAL_485_BUFFERS_SIZE) || defined(__DOXYGEN__)
#define SERIAL_485_BUFFERS_SIZE     16
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
#define FLASH_JEDEC_SPI_NICE_WAITING            TRUE
#endif

/**
 * @brief   Enables the @p fjsAcquireBus() and @p fjsReleaseBus() APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(FLASH_JEDEC_SPI_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define FLASH_JEDEC_SPI_USE_MUTUAL_EXCLUSION    TRUE
#endif

/*===========================================================================*/
/* NVM_FILE driver related settings                                          */
/*===========================================================================*/

/**
 * @brief   Enables the @p nvmfileAcquireBus() and @p nvmfileReleaseBus() APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(NVM_FILE_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define NVM_FILE_USE_MUTUAL_EXCLUSION           TRUE
#endif

/*===========================================================================*/
/* NVM_MEMORY driver related settings                                        */
/*===========================================================================*/

/**
 * @brief   Enables the @p nvmmemoryAcquireBus() and
 *          @p nvmmemoryReleaseBus() APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(NVM_MEMORY_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define NVM_MEMORY_USE_MUTUAL_EXCLUSION         TRUE
#endif

/*===========================================================================*/
/* NVM_PARTITION driver related settings                                     */
/*===========================================================================*/

/**
 * @brief   Enables the @p nvmpartAcquireBus() and @p nvmpartReleaseBus() APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(NVM_PARTITION_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define NVM_PARTITION_USE_MUTUAL_EXCLUSION      TRUE
#endif

/*===========================================================================*/
/* NVM_MIRROR driver related settings                                        */
/*===========================================================================*/

/**
 * @brief   Enables the @p fmirrorAcquireBus() and @p fmirrorReleaseBus() APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(NVM_MIRROR_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define NVM_MIRROR_USE_MUTUAL_EXCLUSION         TRUE
#endif

/*===========================================================================*/
/* NVM_FEE driver related settings                                           */
/*===========================================================================*/

/**
 * @brief   Enables the @p nvmfeeAcquireBus() and @p nvmfeeReleaseBus() APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(NVM_FEE_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define NVM_FEE_USE_MUTUAL_EXCLUSION    TRUE
#endif

/**
 * @brief   Sets the number of payload bytes per slot.
 */
#if !defined(NVM_FEE_SLOT_PAYLOAD_SIZE) || defined(__DOXYGEN__)
#define NVM_FEE_SLOT_PAYLOAD_SIZE       8
#endif

/**
 * @brief   Sets the minimum writable unit of the underlying flash device.
 */
#if !defined(NVM_FEE_WRITE_UNIT_SIZE) || defined(__DOXYGEN__)
#define NVM_FEE_WRITE_UNIT_SIZE         2
#endif

/*===========================================================================*/
/* FLASH internal driver related settings                                    */
/*===========================================================================*/

/**
 * @brief   Delays insertions.
 * @details If enabled this options inserts delays into the flash waiting
 *          routines releasing some extra CPU time for the threads with
 *          lower priority, this may slow down the driver a bit however.
 * @note    This does only make sense if code is being executed from RAM.
 */
#if !defined(FLASH_NICE_WAITING) || defined(__DOXYGEN__)
#define FLASH_NICE_WAITING                      FALSE
#endif

/**
 * @brief   Enables the @p flahAcquireBus() and @p flashReleaseBus() APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(FLASH_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define FLASH_USE_MUTUAL_EXCLUSION              FALSE
#endif

/*===========================================================================*/
/* GD_ILI9341 driver related settings                                        */
/*===========================================================================*/

/**
 * @brief   Enables the @p gdili9341AcquireBus() and @p gdili9341ReleaseBus() APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(GD_ILI9341_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define GD_ILI9341_USE_MUTUAL_EXCLUSION         FALSE
#endif

#endif /* _QHALCONF_H_ */

/** @} */
