/**
 * @file    qserial_485.h
 * @brief   Serial 485 driver macros and structures.
 *
 * @addtogroup SERIAL_485
 * @{
 */

#ifndef _QSERIAL_485_H_
#define _QSERIAL_485_H_

#if HAL_USE_SERIAL_485 || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    Serial status flags
 * @{
 */
#define SD485_PARITY_ERROR      32  /**< @brief Parity error happened.      */
#define SD485_FRAMING_ERROR     64  /**< @brief Framing error happened.     */
#define SD485_OVERRUN_ERROR     128 /**< @brief Overflow happened.          */
#define SD485_NOISE_ERROR       256 /**< @brief Noise on the line.          */
#define SD485_BREAK_DETECTED    512 /**< @brief Break detected.             */
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Serial 485 configuration options
 * @{
 */
/**
 * @brief   Default bit rate.
 * @details Configuration parameter, this is the baud rate selected for the
 *          default configuration.
 */
#if !defined(SERIAL_485_DEFAULT_BITRATE) || defined(__DOXYGEN__)
#define SERIAL_485_DEFAULT_BITRATE      38400
#endif

/**
 * @brief   Serial 485 buffers size.
 * @details Configuration parameter, you can change the depth of the queue
 *          buffers depending on the requirements of your application.
 * @note    The default is 16 bytes for both the transmission and receive
 *          buffers.
 */
#if !defined(SERIAL_485_BUFFERS_SIZE) || defined(__DOXYGEN__)
#define SERIAL_485_BUFFERS_SIZE         16
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !CH_USE_QUEUES && !CH_USE_EVENTS
#error "Serial 485 Driver requires CH_USE_QUEUES and CH_USE_EVENTS"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief Driver state machine possible states.
 */
typedef enum {
  SD485_UNINIT = 0,                 /**< Not initialized.                   */
  SD485_STOP = 1,                   /**< Stopped.                           */
  SD485_READY = 2                   /**< Ready.                             */
} sd485state_t;

/**
 * @brief   Structure representing a serial driver.
 */
typedef struct Serial485Driver Serial485Driver;

#include "qserial_485_lld.h"

/**
 * @brief   @p Serial485Driver specific methods.
 */
#define _serial_485_driver_methods                                            \
  _base_asynchronous_channel_methods

/**
 * @extends BaseAsynchronousChannelVMT
 *
 * @brief   @p Serial485Driver virtual methods table.
 */
struct Serial485DriverVMT {
  _serial_485_driver_methods
};

/**
 * @extends BaseAsynchronousChannel
 *
 * @brief   Full duplex serial driver class.
 * @details This class extends @p BaseAsynchronousChannel by adding physical
 *          I/O queues.
 */
struct Serial485Driver {
  /** @brief Virtual Methods Table.*/
  const struct Serial485DriverVMT *vmt;
  _serial_485_driver_data
  const Serial485Config *config;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @name    Macro Functions
 * @{
 */
/**
 * @brief   Direct output check on a @p Serial485Driver.
 * @note    This function bypasses the indirect access to the channel and
 *          checks directly the output queue. This is faster but cannot
 *          be used to check different channels implementations.
 *
 * @deprecated
 *
 * @api
 */
#define sd485PutWouldBlock(sdp) chOQIsFullI(&(sdp)->oqueue)

/**
 * @brief   Direct input check on a @p Serial485Driver.
 * @note    This function bypasses the indirect access to the channel and
 *          checks directly the input queue. This is faster but cannot
 *          be used to check different channels implementations.
 *
 * @deprecated
 *
 * @api
 */
#define sd485GetWouldBlock(sdp) chIQIsEmptyI(&(sdp)->iqueue)

/**
 * @brief   Direct write to a @p Serial485Driver.
 * @note    This function bypasses the indirect access to the channel and
 *          writes directly on the output queue. This is faster but cannot
 *          be used to write to different channels implementations.
 *
 * @see     chnPutTimeout()
 *
 * @api
 */
#define sd485Put(sdp, b) chOQPut(&(sdp)->oqueue, b)

/**
 * @brief   Direct write to a @p Serial485Driver with timeout specification.
 * @note    This function bypasses the indirect access to the channel and
 *          writes directly on the output queue. This is faster but cannot
 *          be used to write to different channels implementations.
 *
 * @see     chnPutTimeout()
 *
 * @api
 */
#define sd485PutTimeout(sdp, b, t) chOQPutTimeout(&(sdp)->oqueue, b, t)

/**
 * @brief   Direct read from a @p Serial485Driver.
 * @note    This function bypasses the indirect access to the channel and
 *          reads directly from the input queue. This is faster but cannot
 *          be used to read from different channels implementations.
 *
 * @see     chnGetTimeout()
 *
 * @api
 */
#define sd485Get(sdp) chIQGet(&(sdp)->iqueue)

/**
 * @brief   Direct read from a @p Serial485Driver with timeout specification.
 * @note    This function bypasses the indirect access to the channel and
 *          reads directly from the input queue. This is faster but cannot
 *          be used to read from different channels implementations.
 *
 * @see     chnGetTimeout()
 *
 * @api
 */
#define sd485GetTimeout(sdp, t) chIQGetTimeout(&(sdp)->iqueue, t)

/**
 * @brief   Direct blocking write to a @p Serial485Driver.
 * @note    This function bypasses the indirect access to the channel and
 *          writes directly to the output queue. This is faster but cannot
 *          be used to write from different channels implementations.
 *
 * @see     chnWrite()
 *
 * @api
 */
#define sd485Write(sdp, b, n)                                                 \
  chOQWriteTimeout(&(sdp)->oqueue, b, n, TIME_INFINITE)

/**
 * @brief   Direct blocking write to a @p Serial485Driver with timeout
 *          specification.
 * @note    This function bypasses the indirect access to the channel and
 *          writes directly to the output queue. This is faster but cannot
 *          be used to write to different channels implementations.
 *
 * @see     chnWriteTimeout()
 *
 * @api
 */
#define sd485WriteTimeout(sdp, b, n, t)                                       \
  chOQWriteTimeout(&(sdp)->oqueue, b, n, t)

/**
 * @brief   Direct non-blocking write to a @p Serial485Driver.
 * @note    This function bypasses the indirect access to the channel and
 *          writes directly to the output queue. This is faster but cannot
 *          be used to write to different channels implementations.
 *
 * @see     chnWriteTimeout()
 *
 * @api
 */
#define sd485AsynchronousWrite(sdp, b, n)                                     \
  chOQWriteTimeout(&(sdp)->oqueue, b, n, TIME_IMMEDIATE)

/**
 * @brief   Direct blocking read from a @p Serial485Driver.
 * @note    This function bypasses the indirect access to the channel and
 *          reads directly from the input queue. This is faster but cannot
 *          be used to read from different channels implementations.
 *
 * @see     chnRead()
 *
 * @api
 */
#define sd485Read(sdp, b, n)                                                  \
  chIQReadTimeout(&(sdp)->iqueue, b, n, TIME_INFINITE)

/**
 * @brief   Direct blocking read from a @p Serial485Driver with timeout
 *          specification.
 * @note    This function bypasses the indirect access to the channel and
 *          reads directly from the input queue. This is faster but cannot
 *          be used to read from different channels implementations.
 *
 * @see     chnReadTimeout()
 *
 * @api
 */
#define sd485ReadTimeout(sdp, b, n, t)                                        \
  chIQReadTimeout(&(sdp)->iqueue, b, n, t)

/**
 * @brief   Direct non-blocking read from a @p Serial485Driver.
 * @note    This function bypasses the indirect access to the channel and
 *          reads directly from the input queue. This is faster but cannot
 *          be used to read from different channels implementations.
 *
 * @see     chnReadTimeout()
 *
 * @api
 */
#define sd485AsynchronousRead(sdp, b, n)                                      \
  chIQReadTimeout(&(sdp)->iqueue, b, n, TIME_IMMEDIATE)
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void sd485Init(void);
  void sd485ObjectInit(Serial485Driver *sdp, qnotify_t inotify, qnotify_t onotify);
  void sd485Start(Serial485Driver *sdp, const Serial485Config *config);
  void sd485Stop(Serial485Driver *sdp);
  void sd485IncomingDataI(Serial485Driver *sdp, uint8_t b);
  msg_t sd485RequestDataI(Serial485Driver *sdp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_SERIAL_485 */

#endif /* _QSERIAL_H_ */

/** @} */
