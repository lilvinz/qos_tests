/*
 * qhal.c
 *
 *  Created on: 22.12.2013
 *      Author: vke
 */

/**
 * @file    serial_485.h
 * @brief   Serial over 485 Driver macros and structures.
 *
 * @addtogroup SERIAL_485
 * @{
 */

#ifndef _QSERIAL_485_H_
#define _QSERIAL_485_H_

#if HAL_USE_SERIAL_485 || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    SERIAL_485 configuration options
 * @{
 */
/**
 * @brief   Stream buffer size.
 * @details Configuration parameter, you can change the depth of the queue
 *          buffers depending on the requirements of your application.
 * @note    The default is 16 bytes for both the transmission and receive
 *          buffers.
 */
#if !defined(SERIAL_485_BUFFERS_SIZE) || defined(__DOXYGEN__)
#define SERIAL_485_BUFFERS_SIZE             16
#endif

/**
 * @brief   Serial buffer size RX.
 * @details Configuration parameter, you can change the depth of the RX
 *          buffer depending on the requirements of your application.
 * @note    The default is 1 byte. Read completes only when buffer has been
 *          filled completely.
 */
#if !defined(SERIAL_485_UART_RX_BUFFER_SIZE) || defined(__DOXYGEN__)
#define SERIAL_485_UART_RX_BUFFER_SIZE      1
#endif

/**
 * @brief   Serial buffer size TX.
 * @details Configuration parameter, you can change the depth of the TX
 *          buffer depending on the requirements of your application.
 * @note    The default is 16 bytes.
 */
#if !defined(SERIAL_485_UART_TX_BUFFER_SIZE) || defined(__DOXYGEN__)
#define SERIAL_485_UART_TX_BUFFER_SIZE      16
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !HAL_USE_UART || !CH_USE_QUEUES || !CH_USE_EVENTS
#error "Serial over 485 Driver requires HAL_USE_UART, CH_USE_QUEUES, "
       "CH_USE_EVENTS"
#endif

/**
 * @name    Serial 485 status flags
 * @{
 */
#define SD485_NO_ERROR           UART_NO_ERROR        /**< @brief No pending conditions.      */
#define SD485_PARITY_ERROR       UART_PARITY_ERROR    /**< @brief Parity error happened.      */
#define SD485_FRAMING_ERROR      UART_FRAMING_ERROR   /**< @brief Framing error happened.     */
#define SD485_OVERRUN_ERROR      UART_OVERRUN_ERROR   /**< @brief Overflow happened.          */
#define SD485_NOISE_ERROR        UART_NOISE_ERROR     /**< @brief Noise on the line.          */
#define SD485_BREAK_DETECTED     UART_BREAK_DETECTED  /**< @brief Break detected.             */
/** @} */

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief Driver state machine possible states.
 */
typedef enum
{
    SD485_UNINIT = 0,                 /**< Not initialized.                   */
    SD485_STOP = 1,                   /**< Stopped.                           */
    SD485_READY = 2                   /**< Ready.                             */
} sd485state_t;

/**
 * @brief   Structure representing a serial over USB driver.
 */
typedef struct Serial485Driver Serial485Driver;

/**
 * @brief   Serial over USB Driver configuration structure.
 * @details An instance of this structure must be passed to @p sduStart()
 *          in order to configure and start the driver operations.
 */
typedef struct
{
    /**
     * @brief   UART driver to use.
     */
    UARTDriver* uartp;
    /**
     * @brief The chip select line port.
     */
    ioportid_t ssport;
    /**
     * @brief The chip select line pad number.
     */
    uint16_t sspad;
} Serial485Config;

/**
 * @brief   @p SerialDriver specific data.
 */
#define _serial_485_driver_data                                               \
    _base_asynchronous_channel_data                                           \
    /* Driver state.*/                                                        \
    sd485state_t state;                                                       \
    /* Input queue.*/                                                         \
    InputQueue iqueue;                                                        \
    /* Output queue.*/                                                        \
    OutputQueue oqueue;                                                       \
    /* Input buffer.*/                                                        \
    uint8_t ib[SERIAL_485_BUFFERS_SIZE];                                      \
    /* Output buffer.*/                                                       \
    uint8_t ob[SERIAL_485_BUFFERS_SIZE];                                      \
    /* End of the mandatory fields.*/                                         \
    /* Current configuration data.*/                                          \
    const Serial485Config* config;                                            \
    /* UART Input buffer.*/                                                   \
    uint8_t uart_ib[SERIAL_485_UART_RX_BUFFER_SIZE];                          \
    /* UART Output buffer.*/                                                  \
    uint8_t uart_ob[SERIAL_485_UART_TX_BUFFER_SIZE];


/**
 * @brief   @p Serial485Driver specific methods.
 */
#define _serial_485_driver_methods                                            \
    _base_asynchronous_channel_methods

/**
 * @extends BaseAsynchronousChannelVMT
 *
 * @brief   @p SerialDriver virtual methods table.
 */
struct Serial485DriverVMT
{
    _serial_485_driver_methods
};

/**
 * @extends BaseAsynchronousChannel
 *
 * @brief   Full duplex serial driver class.
 * @details This class extends @p BaseAsynchronousChannel by adding physical
 *          I/O queues.
 */
struct Serial485Driver
{
    /** @brief Virtual Methods Table.*/
    const struct Serial485DriverVMT* vmt;
    _serial_485_driver_data
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C"
{
#endif
    void sd485Init(void);
    void sd485ObjectInit(Serial485Driver* sdp);
    void sd485Start(Serial485Driver* sd485p, const Serial485Config* config);
    void sd485Stop(Serial485Driver* sd485p);
    void sd485EndOfTx1I(UARTDriver* uartp);
    void sd485EndOfTx2I(UARTDriver* uartp);
    void sd485EndOfRxI(UARTDriver* uartp);
    void sd485ErrorI(UARTDriver* uartp, uartflags_t e);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_SERIAL_485 */

#endif /* _QSERIAL_485_H_ */

/** @} */
