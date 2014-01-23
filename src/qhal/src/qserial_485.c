/*
 * qhal.c
 *
 *  Created on: 22.12.2013
 *      Author: vke
 */

/**
 * @file    qserial_485.c
 * @brief   Serial over 485 Driver code.
 *
 * @addtogroup QSERIAL_485
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
 * Interface implementation.
 */

static size_t write(void* ip, const uint8_t* bp, size_t n)
{
    return chOQWriteTimeout(&((Serial485Driver*)ip)->oqueue, bp,
                          n, TIME_INFINITE);
}

static size_t read(void* ip, uint8_t* bp, size_t n)
{
    return chIQReadTimeout(&((Serial485Driver*)ip)->iqueue, bp,
                         n, TIME_INFINITE);
}

static msg_t put(void* ip, uint8_t b)
{
    return chOQPutTimeout(&((Serial485Driver*)ip)->oqueue, b, TIME_INFINITE);
}

static msg_t get(void* ip)
{
    return chIQGetTimeout(&((Serial485Driver*)ip)->iqueue, TIME_INFINITE);
}

static msg_t putt(void* ip, uint8_t b, systime_t timeout)
{
    return chOQPutTimeout(&((Serial485Driver*)ip)->oqueue, b, timeout);
}

static msg_t gett(void* ip, systime_t timeout)
{
    return chIQGetTimeout(&((Serial485Driver*)ip)->iqueue, timeout);
}

static size_t writet(void* ip, const uint8_t* bp, size_t n, systime_t time)
{
    return chOQWriteTimeout(&((Serial485Driver*)ip)->oqueue, bp, n, time);
}

static size_t readt(void* ip, uint8_t* bp, size_t n, systime_t time)
{
    return chIQReadTimeout(&((Serial485Driver*)ip)->iqueue, bp, n, time);
}

static const struct Serial485DriverVMT vmt =
{
    .write = write,
    .read = read,
    .put = put,
    .get = get,
    .putt = putt,
    .gett = gett,
    .writet = writet,
    .readt = readt,
};

/**
 * @brief   Notification of data inserted into the output queue.
 */
static void onotify(GenericQueue* qp)
{
    chDbgCheck(qp != NULL, "onotify");

    Serial485Driver* sd485p = chQGetLink(qp);

    chDbgCheck(sd485p != NULL, "onotify");

    /* If the driver is not in the appropriate state then transactions
     must not be started. */
    if ((sd485p->config->uartp->state != UART_READY) ||
            (sd485p->state != SD485_READY))
        return;

    size_t lost_rx_bytes = 0;

    /* If low level driver is not transmitting, initiate transfer. */
    if (sd485p->config->uartp->txstate == UART_TX_IDLE)
    {
        /* Fill output buffer */
        size_t length = 0;
        msg_t result;
        while ((result = chOQGetI(&sd485p->oqueue)) >= 0 &&
                length < sizeof(sd485p->uart_ob))
            sd485p->uart_ob[length++] = (uint8_t)result;

        if (length > 0)
        {
            /* Abort rx if pending. */
            lost_rx_bytes = uartStopReceiveI(sd485p->config->uartp);

            /* Set driver enable pad. */
            if (sd485p->config->ssport != NULL)
                palSetPad(sd485p->config->ssport, sd485p->config->sspad);

            /* Continue by doing tx. */
            uartStartSendI(sd485p->config->uartp, length, sd485p->uart_ob);
        }
        else
        {
            chnAddFlagsI(sd485p, CHN_OUTPUT_EMPTY);
        }
    }
}

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
void sd485Init(void)
{
}

/**
 * @brief   Initializes a generic full duplex driver object.
 * @details The HW dependent part of the initialization has to be performed
 *          outside, usually in the hardware initialization code.
 *
 * @param[out] sd485p     pointer to a @p Serial485Driver structure
 *
 * @init
 */
void sd485ObjectInit(Serial485Driver *sd485p)
{
    chDbgCheck(sd485p != NULL, "sd485ObjectInit");

    sd485p->vmt = &vmt;
    chEvtInit(&sd485p->event);
    sd485p->state = SD485_STOP;
    chIQInit(&sd485p->iqueue, sd485p->ib, SERIAL_485_BUFFERS_SIZE, NULL, sd485p);
    chOQInit(&sd485p->oqueue, sd485p->ob, SERIAL_485_BUFFERS_SIZE, onotify, sd485p);
}

/**
 * @brief   Configures and starts the driver.
 *
 * @param[in] sd485p      pointer to a @p Serial485Driver object
 * @param[in] config      the serial over 485 driver configuration
 *
 * @api
 */
void sd485Start(Serial485Driver *sd485p, const Serial485Config *config)
{
    chDbgCheck(sd485p != NULL, "sd485Start");
    chDbgCheck(config != NULL, "sd485Start");

    chSysLock();

    chDbgAssert((sd485p->state == SD485_STOP) || (sd485p->state == SD485_READY),
            "sd485Start(), #1",
            "invalid state");

    sd485p->config = config;
    sd485p->config->uartp->uldp = sd485p;

    /* Ensure that the lower level driver has been started first. */
    chDbgAssert(sd485p->config->uartp->state == UART_READY,
            "sd485Start(), #2",
            "invalid state");

    sd485p->state = SD485_READY;
    uartStartReceiveI(sd485p->config->uartp, sizeof(sd485p->uart_ib),
            sd485p->uart_ib);

    chSysUnlock();
}

/**
 * @brief   Stops the driver.
 * @details Any thread waiting on the driver's queues will be awakened with
 *          the message @p Q_RESET.
 *
 * @param[in] sd485p      pointer to a @p Serial485Driver object
 *
 * @api
 */
void sd485Stop(Serial485Driver *sd485p)
{
    chDbgCheck(sd485p != NULL, "sd485Stop");

    chSysLock();

    chDbgAssert((sd485p->state == SD485_STOP) || (sd485p->state == SD485_READY),
            "sd485Stop(), #1",
            "invalid state");

    /* Ensure that the lower level driver is still running. */
    chDbgAssert(sd485p->config->uartp->state == UART_READY,
            "sd485Stop(), #2",
            "invalid state");

    uartStopReceiveI(sd485p->config->uartp);

    /* Driver in stopped state. */
    sd485p->config->uartp->uldp = NULL;
    sd485p->state = SD485_STOP;

    /* Queues reset in order to signal the driver stop to the application. */
    chnAddFlagsI(sd485p, CHN_DISCONNECTED);
    chIQResetI(&sd485p->iqueue);
    chOQResetI(&sd485p->oqueue);
    chSchRescheduleS();

    chSysUnlock();
}

/**
 * @brief   Default data sent callback
 * @details The application must use this function as callback for the lower
 *          level UART driver to signal end of transfer
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 */
void sd485EndOfTx1(UARTDriver* uartp)
{
    chDbgCheck(uartp != NULL, "sd485EndOfTx1I");

    Serial485Driver* sd485p = (Serial485Driver*)uartp->uldp;
    if (sd485p == NULL)
        return;

    chSysLockFromIsr();

    size_t length = 0;
    msg_t result;
    while ((result = chOQGetI(&sd485p->oqueue)) >= 0 &&
            length < sizeof(sd485p->uart_ob))
        sd485p->uart_ob[length++] = (uint8_t)result;

    if (length > 0)
    {
        /* Continue by doing tx. */
        uartStartSendI(sd485p->config->uartp, length, sd485p->uart_ob);
    }
    else
    {
        chnAddFlagsI(sd485p, CHN_OUTPUT_EMPTY);
    }

    chSysUnlockFromIsr();
}

/**
 * @brief   Default data physically sent callback
 * @details The application must use this function as callback for the lower
 *          level UART driver to signal physical end of transfer
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 */
void sd485EndOfTx2(UARTDriver* uartp)
{
    chDbgCheck(uartp != NULL, "sd485EndOfTx2I");

    Serial485Driver* sd485p = (Serial485Driver*)uartp->uldp;
    if (sd485p == NULL)
        return;

    chSysLockFromIsr();

    /* Clear driver enable pad. */
    if (sd485p->config->ssport != NULL)
        palClearPad(sd485p->config->ssport, sd485p->config->sspad);

    /* Restart receiving but check if its not running already.
     This happens once on startup for unknown reason. */
    if (sd485p->config->uartp->rxstate == UART_RX_IDLE)
        uartStartReceiveI(sd485p->config->uartp, sizeof(sd485p->uart_ib),
                sd485p->uart_ib);

    chnAddFlagsI(sd485p, CHN_TRANSMISSION_END);

    chSysUnlockFromIsr();
}

/**
 * @brief   Default data received callback
 * @details The application must use this function as callback for the lower
 *          level UART driver to signal data receive complete
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 */
void sd485EndOfRx(UARTDriver* uartp)
{
    chDbgCheck(uartp != NULL, "sd485EndOfRxI");

    Serial485Driver* sd485p = (Serial485Driver*)uartp->uldp;
    if (sd485p == NULL)
        return;

    chSysLockFromIsr();

    /* If queue was empty before, set flag. */
    if (chIQIsEmptyI(&sd485p->iqueue) == TRUE)
        chnAddFlagsI(sd485p, CHN_INPUT_AVAILABLE);

    /* Consume received data. */
    for (size_t i = 0; i < sizeof(sd485p->uart_ib); ++i)
    {
        if (chIQPutI(&sd485p->iqueue, sd485p->uart_ib[i]) == Q_FULL)
        {
            chnAddFlagsI(sd485p, SD485_OVERRUN_ERROR);
            break;
        }
    }

    /* Restart receiving. */
    uartStartReceiveI(sd485p->config->uartp, sizeof(sd485p->uart_ib),
            sd485p->uart_ib);

    chSysUnlockFromIsr();
}

/**
 * @brief   Default error callback
 * @details The application must use this function as callback for the lower
 *          level UART driver to signal error
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 */
void sd485Error(UARTDriver* uartp, uartflags_t e)
{
    chDbgCheck(uartp != NULL, "sd485ErrorI");

    Serial485Driver* sd485p = (Serial485Driver*)uartp->uldp;
    if (sd485p == NULL)
        return;

    chSysLockFromIsr();

    chnAddFlagsI(sd485p, e);

    chSysUnlockFromIsr();
}

#endif /* HAL_USE_SERIAL_485 */

/** @} */
