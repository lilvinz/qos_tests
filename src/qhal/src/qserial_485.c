/**
 * @file    qserial_485.c
 * @brief   Serial driver code.
 *
 * @addtogroup SERIAL_485
 * @{
 */

#include "ch.h"
#include "qhal.h"

#if HAL_USE_SERIAL_485 || defined(__DOXYGEN__)

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

/*
 * Interface implementation, the following functions just invoke the equivalent
 * queue-level function or macro.
 */

static size_t write(void *ip, const uint8_t *bp, size_t n) {

  return chOQWriteTimeout(&((Serial485Driver *)ip)->oqueue, bp,
                          n, TIME_INFINITE);
}

static size_t read(void *ip, uint8_t *bp, size_t n) {

  return chIQReadTimeout(&((Serial485Driver *)ip)->iqueue, bp,
                         n, TIME_INFINITE);
}

static msg_t put(void *ip, uint8_t b) {

  return chOQPutTimeout(&((Serial485Driver *)ip)->oqueue, b, TIME_INFINITE);
}

static msg_t get(void *ip) {

  return chIQGetTimeout(&((Serial485Driver *)ip)->iqueue, TIME_INFINITE);
}

static msg_t putt(void *ip, uint8_t b, systime_t timeout) {

  return chOQPutTimeout(&((Serial485Driver *)ip)->oqueue, b, timeout);
}

static msg_t gett(void *ip, systime_t timeout) {

  return chIQGetTimeout(&((Serial485Driver *)ip)->iqueue, timeout);
}

static size_t writet(void *ip, const uint8_t *bp, size_t n, systime_t time) {

  return chOQWriteTimeout(&((Serial485Driver *)ip)->oqueue, bp, n, time);
}

static size_t readt(void *ip, uint8_t *bp, size_t n, systime_t time) {

  return chIQReadTimeout(&((Serial485Driver *)ip)->iqueue, bp, n, time);
}

static const struct Serial485DriverVMT vmt = {
  write, read, put, get,
  putt, gett, writet, readt
};

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Serial Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void sd485Init(void) {

  sd485_lld_init();
}

/**
 * @brief   Initializes a generic full duplex driver object.
 * @details The HW dependent part of the initialization has to be performed
 *          outside, usually in the hardware initialization code.
 *
 * @param[out] sd485p   pointer to a @p Serial485Driver structure
 * @param[in] inotify   pointer to a callback function that is invoked when
 *                      some data is read from the Queue. The value can be
 *                      @p NULL.
 * @param[in] onotify   pointer to a callback function that is invoked when
 *                      some data is written in the Queue. The value can be
 *                      @p NULL.
 *
 * @init
 */
void sd485ObjectInit(Serial485Driver *sd485p, qnotify_t inotify, qnotify_t onotify) {

  sd485p->vmt = &vmt;
  chEvtInit(&sd485p->event);
  sd485p->state = SD485_STOP;
  chIQInit(&sd485p->iqueue, sd485p->ib, SERIAL_BUFFERS_SIZE, inotify, sd485p);
  chOQInit(&sd485p->oqueue, sd485p->ob, SERIAL_BUFFERS_SIZE, onotify, sd485p);
}

/**
 * @brief   Configures and starts the driver.
 *
 * @param[in] sd485p    pointer to a @p Serial485Driver object
 * @param[in] config    the architecture-dependent serial driver configuration.
 *                      If this parameter is set to @p NULL then a default
 *                      configuration is used.
 *
 * @api
 */
void sd485Start(Serial485Driver *sd485p, const Serial485Config *config) {

  chDbgCheck(sd485p != NULL, "sd485485Start");

  chSysLock();
  chDbgAssert((sd485p->state == SD485_STOP) || (sd485p->state == SD485_READY),
              "sd485485Start(), #1",
              "invalid state");
  sd485p->config = config;
  sd485_lld_start(sd485p, config);
  sd485p->state = SD485_READY;
  chnAddFlagsI(sd485p, CHN_CONNECTED);
  chSysUnlock();
}

/**
 * @brief   Stops the driver.
 * @details Any thread waiting on the driver's queues will be awakened with
 *          the message @p Q_RESET.
 *
 * @param[in] sd485p    pointer to a @p Serial485Driver object
 *
 * @api
 */
void sd485Stop(Serial485Driver *sd485p) {

  chDbgCheck(sd485p != NULL, "sd485485Stop");

  chSysLock();
  chDbgAssert((sd485p->state == SD485_STOP) || (sd485p->state == SD485_READY),
              "sd485485Stop(), #1",
              "invalid state");
  chnAddFlagsI(sd485p, CHN_DISCONNECTED);
  sd485_lld_stop(sd485p);
  sd485p->state = SD485_STOP;
  chOQResetI(&sd485p->oqueue);
  chIQResetI(&sd485p->iqueue);
  chSchRescheduleS();
  chSysUnlock();
}

/**
 * @brief   Handles incoming data.
 * @details This function must be called from the input interrupt service
 *          routine in order to enqueue incoming data and generate the
 *          related events.
 * @note    The incoming data event is only generated when the input queue
 *          becomes non-empty.
 * @note    In order to gain some performance it is suggested to not use
 *          this function directly but copy this code directly into the
 *          interrupt service routine.
 *
 * @param[in] sd485p    pointer to a @p Serial485Driver structure
 * @param[in] b         the byte to be written in the driver's Input Queue
 *
 * @iclass
 */
void sd485IncomingDataI(Serial485Driver *sd485p, uint8_t b) {

  chDbgCheckClassI();
  chDbgCheck(sd485p != NULL, "sd485IncomingDataI");

  if (chIQIsEmptyI(&sd485p->iqueue))
    chnAddFlagsI(sd485p, CHN_INPUT_AVAILABLE);
  if (chIQPutI(&sd485p->iqueue, b) < Q_OK)
    chnAddFlagsI(sd485p, SD485_OVERRUN_ERROR);
}

/**
 * @brief   Handles outgoing data.
 * @details Must be called from the output interrupt service routine in order
 *          to get the next byte to be transmitted.
 * @note    In order to gain some performance it is suggested to not use
 *          this function directly but copy this code directly into the
 *          interrupt service routine.
 *
 * @param[in] sd485p    pointer to a @p Serial485Driver structure
 * @return              The byte value read from the driver's output queue.
 * @retval Q_EMPTY      if the queue is empty (the lower driver usually
 *                      disables the interrupt source when this happens).
 *
 * @iclass
 */
msg_t sd485RequestDataI(Serial485Driver *sd485p) {
  msg_t  b;

  chDbgCheckClassI();
  chDbgCheck(sd485p != NULL, "sd485RequestDataI");

  b = chOQGetI(&sd485p->oqueue);
  if (b < Q_OK)
    chnAddFlagsI(sd485p, CHN_OUTPUT_EMPTY);
  return b;
}

#endif /* HAL_USE_SERIAL_485 */

/** @} */
