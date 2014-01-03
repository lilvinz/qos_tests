/**
 * @file    STM32/FLASHv2/qflash.c
 * @brief   STM32 low level FLASH driver code.
 *
 * @addtogroup FLASH
 * @{
 */

#include "ch.h"
#include "hal.h"

#if HAL_USE_FLASH_INTERNAL || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief USART1 UART driver identifier.*/
#if STM32_UART_USE_USART1 || defined(__DOXYGEN__)
FlashInternalDriver FLASHD1;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
#if 0
/**
 * @brief   Status bits translation.
 *
 * @param[in] sr        USART SR register value
 *
 * @return  The error flags.
 */
static uartflags_t translate_errors(uint16_t sr)
{
    uartflags_t sts = 0;

    if (sr & USART_SR_ORE)
        sts |= UART_OVERRUN_ERROR;
    if (sr & USART_SR_PE)
        sts |= UART_PARITY_ERROR;
    if (sr & USART_SR_FE)
        sts |= UART_FRAMING_ERROR;
    if (sr & USART_SR_NE)
        sts |= UART_NOISE_ERROR;
    if (sr & USART_SR_LBD)
        sts |= UART_BREAK_DETECTED;
    return sts;
}

/**
 * @brief   USART de-initialization.
 * @details This function must be invoked with interrupts disabled.
 *
 * @param[in] finternalp     pointer to the @p FlashInternalDriver object
 */
static void flash_internal_stop(FlashInternalDriver* finternalp)
{
    /* Stops USART operations.*/
    finternalp->usart->CR1 = 0;
    finternalp->usart->CR2 = 0;
    finternalp->usart->CR3 = 0;
}

/**
 * @brief   USART initialization.
 * @details This function must be invoked with interrupts disabled.
 *
 * @param[in] finternalp     pointer to the @p FlashInternalDriver object
 */
static void flash_internal_start(FlashInternalDriver* finternalp)
{
    FLASH_TypeDef* f = finternalp->flash;

    /* Defensive programming, starting from a clean state.*/
    flash_internal_stop(finternalp);
#if 0
    /* Resetting eventual pending status flags.*/
    (void)u->SR; /* SR reset step 1.*/
    (void)u->DR; /* SR reset step 2.*/
    u->SR = 0;

    /* Note that some bits are enforced because required for correct driver
     operations.*/
    u->CR2 = finternalp->config->cr2 | USART_CR2_LBDIE;
    u->CR3 = finternalp->config->cr3 | USART_CR3_DMAT | USART_CR3_DMAR |
    USART_CR3_EIE;
    if (finternalp->config->txend2_cb == NULL)
    cr1 = USART_CR1_UE | USART_CR1_PEIE | USART_CR1_TE | USART_CR1_RE;
    else
    cr1 = USART_CR1_UE | USART_CR1_PEIE | USART_CR1_TE | USART_CR1_RE |
    USART_CR1_TCIE;
    u->CR1 = finternalp->config->cr1 | cr1;
#endif
}

/**
 * @brief   RX DMA common service routine.
 *
 * @param[in] finternalp     pointer to the @p FlashInternalDriver object
 * @param[in] flags     pre-shifted content of the ISR register
 */
static void flash_internal_lld_serve_rx_end_irq(FlashInternalDriver* finternalp, uint32_t flags)
{
    /* DMA errors handling.*/
#if defined(STM32_UART_DMA_ERROR_HOOK)
    if ((flags & (STM32_DMA_ISR_TEIF | STM32_DMA_ISR_DMEIF)) != 0)
    {
        STM32_UART_DMA_ERROR_HOOK(finternalp);
    }
#else
    (void)flags;
#endif

    if (finternalp->rxstate == UART_RX_IDLE)
    {
        /* Receiver in idle state, a callback is generated, if enabled, for each
         received character and then the driver stays in the same state.*/
        if (finternalp->config->rxchar_cb != NULL)
        finternalp->config->rxchar_cb(finternalp, finternalp->rxbuf);
    }
    else
    {
        /* Receiver in active state, a callback is generated, if enabled, after
         a completed transfer.*/
        dmaStreamDisable(finternalp->dmarx);
        finternalp->rxstate = UART_RX_COMPLETE;
        if (finternalp->config->rxend_cb != NULL)
        finternalp->config->rxend_cb(finternalp);

        /* If the callback didn't explicitly change state then the receiver
         automatically returns to the idle state.*/
        if (finternalp->rxstate == UART_RX_COMPLETE)
        {
            finternalp->rxstate = UART_RX_IDLE;
            set_rx_idle_loop(finternalp);
        }
    }
}

/**
 * @brief   TX DMA common service routine.
 *
 * @param[in] finternalp     pointer to the @p FlashInternalDriver object
 * @param[in] flags     pre-shifted content of the ISR register
 */
static void flash_internal_lld_serve_tx_end_irq(FlashInternalDriver* finternalp, uint32_t flags)
{
    /* DMA errors handling.*/
#if defined(STM32_UART_DMA_ERROR_HOOK)
    if ((flags & (STM32_DMA_ISR_TEIF | STM32_DMA_ISR_DMEIF)) != 0)
    {
        STM32_UART_DMA_ERROR_HOOK(finternalp);
    }
#else
    (void)flags;
#endif

    dmaStreamDisable(finternalp->dmatx);

    /* A callback is generated, if enabled, after a completed transfer.*/
    finternalp->txstate = UART_TX_COMPLETE;
    if (finternalp->config->txend1_cb != NULL)
    finternalp->config->txend1_cb(finternalp);

    /* If the callback didn't explicitly change state then the transmitter
     automatically returns to the idle state.*/
    if (finternalp->txstate == UART_TX_COMPLETE)
    finternalp->txstate = UART_TX_IDLE;
}

/**
 * @brief   USART common service routine.
 *
 * @param[in] finternalp     pointer to the @p FlashInternalDriver object
 */
static void serve_usart_irq(FlashInternalDriver* finternalp)
{
    uint16_t sr;
    USART_TypeDef *u = finternalp->usart;

    sr = u->SR; /* SR reset step 1.*/
    (void)u->DR; /* SR reset step 2.*/
    if (sr & (USART_SR_LBD | USART_SR_ORE | USART_SR_NE |
                    USART_SR_FE | USART_SR_PE))
    {
        u->SR = ~USART_SR_LBD;
        if (finternalp->config->rxerr_cb != NULL)
        finternalp->config->rxerr_cb(finternalp, translate_errors(sr));
    }
    if (sr & USART_SR_TC)
    {
        u->SR = ~USART_SR_TC;

        /* End of transmission, a callback is generated.*/
        if (finternalp->config->txend2_cb != NULL)
        finternalp->config->txend2_cb(finternalp);
    }
}
#endif
/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if STM32_UART_USE_USART1 || defined(__DOXYGEN__)
#if !defined(STM32_USART1_HANDLER)
#error "STM32_USART1_HANDLER not defined"
#endif
/**
 * @brief   USART1 IRQ handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(STM32_USART1_HANDLER) {

  CH_IRQ_PROLOGUE();

  serve_usart_irq(&UARTD1);

  CH_IRQ_EPILOGUE();
}
#endif /* STM32_UART_USE_USART1 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level UART driver initialization.
 *
 * @notapi
 */
void flash_internal_lld_init(void)
{
#if 0
#if STM32_UART_USE_USART1
    uartObjectInit(&UARTD1);
    UARTD1.usart = USART1;
    UARTD1.dmamode = STM32_DMA_CR_DMEIE | STM32_DMA_CR_TEIE;
    UARTD1.dmarx = STM32_DMA_STREAM(STM32_UART_USART1_RX_DMA_STREAM);
    UARTD1.dmatx = STM32_DMA_STREAM(STM32_UART_USART1_TX_DMA_STREAM);
#endif
#endif
}

/**
 * @brief   Configures and activates the UART peripheral.
 *
 * @param[in] finternalp     pointer to the @p FlashInternalDriver object
 *
 * @notapi
 */
void flash_internal_lld_start(FlashInternalDriver* finternalp)
{
#if 0
    if (finternalp->state == UART_STOP)
    {
#if STM32_UART_USE_USART1
        if (&UARTD1 == finternalp)
        {
            bool_t b;
            b = dmaStreamAllocate(finternalp->dmarx,
                    STM32_UART_USART1_IRQ_PRIORITY,
                    (stm32_dmaisr_t)flash_internal_lld_serve_rx_end_irq,
                    (void *)finternalp);
            chDbgAssert(!b, "flash_internal_lld_start(), #1", "stream already allocated");
            b = dmaStreamAllocate(finternalp->dmatx,
                    STM32_UART_USART1_IRQ_PRIORITY,
                    (stm32_dmaisr_t)flash_internal_lld_serve_tx_end_irq,
                    (void *)finternalp);
            chDbgAssert(!b, "flash_internal_lld_start(), #2", "stream already allocated");
            rccEnableUSART1(FALSE);
            nvicEnableVector(STM32_USART1_NUMBER,
                    CORTEX_PRIORITY_MASK(STM32_UART_USART1_IRQ_PRIORITY));
            finternalp->dmamode |= STM32_DMA_CR_CHSEL(USART1_RX_DMA_CHANNEL) |
            STM32_DMA_CR_PL(STM32_UART_USART1_DMA_PRIORITY);
        }
#endif

        /* Static DMA setup, the transfer size depends on the USART settings,
         it is 16 bits if M=1 and PCE=0 else it is 8 bits.*/
        if ((finternalp->config->cr1 & (USART_CR1_M | USART_CR1_PCE)) == USART_CR1_M)
            finternalp->dmamode |= STM32_DMA_CR_PSIZE_HWORD | STM32_DMA_CR_MSIZE_HWORD;
        dmaStreamSetPeripheral(finternalp->dmarx, &finternalp->usart->DR);
        dmaStreamSetPeripheral(finternalp->dmatx, &finternalp->usart->DR);
        finternalp->rxbuf = 0;
    }

    finternalp->rxstate = UART_RX_IDLE;
    finternalp->txstate = UART_TX_IDLE;
    usart_start(finternalp);
#endif
}

/**
 * @brief   Deactivates the UART peripheral.
 *
 * @param[in] finternalp     pointer to the @p FlashInternalDriver object
 *
 * @notapi
 */
void flash_internal_lld_stop(FlashInternalDriver* finternalp)
{
#if 0
    if (finternalp->state == UART_READY)
    {
        usart_stop(finternalp);
        dmaStreamRelease(finternalp->dmarx);
        dmaStreamRelease(finternalp->dmatx);

#if STM32_UART_USE_USART1
        if (&UARTD1 == finternalp)
        {
            nvicDisableVector(STM32_USART1_NUMBER);
            rccDisableUSART1(FALSE);
            return;
        }
#endif
    }
#endif
}

/**
 * @brief   Starts a transmission on the UART peripheral.
 * @note    The buffers are organized as uint8_t arrays for data sizes below
 *          or equal to 8 bits else it is organized as uint16_t arrays.
 *
 * @param[in] finternalp     pointer to the @p FlashInternalDriver object
 * @param[in] n         number of data frames to send
 * @param[in] txbuf     the pointer to the transmit buffer
 *
 * @notapi
 */
void flash_internal_lld_start_send(FlashInternalDriver* finternalp, size_t n, const void* txbuf)
{
#if 0
    /* TX DMA channel preparation and start.*/
    dmaStreamSetMemory0(finternalp->dmatx, txbuf);
    dmaStreamSetTransactionSize(finternalp->dmatx, n);
    dmaStreamSetMode(finternalp->dmatx, finternalp->dmamode | STM32_DMA_CR_DIR_M2P |
            STM32_DMA_CR_MINC | STM32_DMA_CR_TCIE);
    dmaStreamEnable(finternalp->dmatx);
#endif
}

/**
 * @brief   Stops any ongoing transmission.
 * @note    Stopping a transmission also suppresses the transmission callbacks.
 *
 * @param[in] finternalp     pointer to the @p FlashInternalDriver object
 *
 * @return              The number of data frames not transmitted by the
 *                      stopped transmit operation.
 *
 * @notapi
 */
size_t flash_internal_lld_stop_send(FlashInternalDriver* finternalp)
{
#if 0
    dmaStreamDisable(finternalp->dmatx);
    return dmaStreamGetTransactionSize(finternalp->dmatx);
#endif
    return 0;
}

/**
 * @brief   Starts a receive operation on the UART peripheral.
 * @note    The buffers are organized as uint8_t arrays for data sizes below
 *          or equal to 8 bits else it is organized as uint16_t arrays.
 *
 * @param[in] finternalp     pointer to the @p FlashInternalDriver object
 * @param[in] n         number of data frames to send
 * @param[out] rxbuf    the pointer to the receive buffer
 *
 * @notapi
 */
void flash_internal_lld_start_receive(FlashInternalDriver* finternalp, size_t n, void* rxbuf)
{
#if 0
    /* Stopping previous activity (idle state).*/
    dmaStreamDisable(finternalp->dmarx);

    /* RX DMA channel preparation and start.*/
    dmaStreamSetMemory0(finternalp->dmarx, rxbuf);
    dmaStreamSetTransactionSize(finternalp->dmarx, n);
    dmaStreamSetMode(finternalp->dmarx, finternalp->dmamode | STM32_DMA_CR_DIR_P2M |
            STM32_DMA_CR_MINC | STM32_DMA_CR_TCIE);
    dmaStreamEnable(finternalp->dmarx);
#endif
}

/**
 * @brief   Stops any ongoing receive operation.
 * @note    Stopping a receive operation also suppresses the receive callbacks.
 *
 * @param[in] finternalp    pointer to the @p FlashInternalDriver object
 *
 * @return                  The number of data frames not received by the
 *                          stopped receive operation.
 *
 * @notapi
 */
size_t flash_internal_lld_stop_receive(FlashInternalDriver* finternalp)
{
    size_t n;

    dmaStreamDisable(finternalp->dmarx);
    n = dmaStreamGetTransactionSize(finternalp->dmarx);
    set_rx_idle_loop(finternalp);
    return n;
}

#endif /* HAL_USE_FLASH_INTERNAL */

/** @} */
