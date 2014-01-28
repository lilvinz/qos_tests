/**
 * @file    STM32/USARTv1/qserial_485_lld.c
 * @brief   STM32 low level serial 485 driver code.
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

/** @brief USART1 serial driver identifier.*/
#if STM32_SERIAL_485_USE_USART1 || defined(__DOXYGEN__)
Serial485Driver SD4851;
#endif

/** @brief USART2 serial driver identifier.*/
#if STM32_SERIAL_485_USE_USART2 || defined(__DOXYGEN__)
Serial485Driver SD4852;
#endif

/** @brief USART3 serial driver identifier.*/
#if STM32_SERIAL_485_USE_USART3 || defined(__DOXYGEN__)
Serial485Driver SD4853;
#endif

/** @brief UART4 serial driver identifier.*/
#if STM32_SERIAL_485_USE_UART4 || defined(__DOXYGEN__)
Serial485Driver SD4854;
#endif

/** @brief UART5 serial driver identifier.*/
#if STM32_SERIAL_485_USE_UART5 || defined(__DOXYGEN__)
Serial485Driver SD4855;
#endif

/** @brief USART6 serial driver identifier.*/
#if STM32_SERIAL_485_USE_USART6 || defined(__DOXYGEN__)
Serial485Driver SD4856;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/** @brief Driver default configuration.*/
static const Serial485Config default_config =
{
  SERIAL_DEFAULT_BITRATE,
  0,
  USART_CR2_STOP1_BITS | USART_CR2_LINEN,
  0,
  0,
  0,
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   USART initialization.
 * @details This function must be invoked with interrupts disabled.
 *
 * @param[in] sd485p    pointer to a @p Serial485Driver object
 * @param[in] config    the architecture-dependent serial driver configuration
 */
static void usart_init(Serial485Driver *sd485p, const Serial485Config *config) {
  USART_TypeDef *u = sd485p->usart;

  /* Baud rate setting.*/
#if STM32_HAS_USART6
  if ((sd485p->usart == USART1) || (sd485p->usart == USART6))
#else
  if (sd485p->usart == USART1)
#endif
    u->BRR = STM32_PCLK2 / config->speed;
  else
    u->BRR = STM32_PCLK1 / config->speed;

  /* Note that some bits are enforced.*/
  u->CR2 = config->cr2 | USART_CR2_LBDIE;
  u->CR3 = config->cr3 | USART_CR3_EIE;
  u->CR1 = config->cr1 | USART_CR1_UE | USART_CR1_PEIE |
                         USART_CR1_RXNEIE | USART_CR1_TE |
                         USART_CR1_RE;
  u->SR = 0;
  (void)u->SR;  /* SR reset step 1.*/
  (void)u->DR;  /* SR reset step 2.*/

  /* Clear driver enable pad. */
  if (sd485p->config->ssport != NULL)
    palClearPad(sd485p->config->ssport, sd485p->config->sspad);
}

/**
 * @brief   USART de-initialization.
 * @details This function must be invoked with interrupts disabled.
 *
 * @param[in] u         pointer to an USART I/O block
 */
static void usart_deinit(USART_TypeDef *u) {

  u->CR1 = 0;
  u->CR2 = 0;
  u->CR3 = 0;
}

/**
 * @brief   Error handling routine.
 *
 * @param[in] sd485p    pointer to a @p Serial485Driver object
 * @param[in] sr        USART SR register value
 */
static void set_error(Serial485Driver *sd485p, uint16_t sr) {
  flagsmask_t sts = 0;

  if (sr & USART_SR_ORE)
    sts |= SD485_OVERRUN_ERROR;
  if (sr & USART_SR_PE)
    sts |= SD485_PARITY_ERROR;
  if (sr & USART_SR_FE)
    sts |= SD485_FRAMING_ERROR;
  if (sr & USART_SR_NE)
    sts |= SD485_NOISE_ERROR;
  chnAddFlagsI(sd485p, sts);
}

/**
 * @brief   Common IRQ handler.
 *
 * @param[in] sd485p    communication channel associated to the USART
 */
static void serve_interrupt(Serial485Driver *sd485p) {
  USART_TypeDef *u = sd485p->usart;
  uint16_t cr1 = u->CR1;
  uint16_t sr = u->SR;

  /* Special case, LIN break detection.*/
  if (sr & USART_SR_LBD) {
    chSysLockFromIsr();
    chnAddFlagsI(sd485p, SD485_BREAK_DETECTED);
    chSysUnlockFromIsr();
    u->SR = ~USART_SR_LBD;
  }

  /* Data available.*/
  chSysLockFromIsr();
  while (sr & USART_SR_RXNE) {
    /* Error condition detection.*/
    if (sr & (USART_SR_ORE | USART_SR_NE | USART_SR_FE  | USART_SR_PE))
      set_error(sd485p, sr);
    sd485IncomingDataI(sd485p, u->DR);
    sr = u->SR;
  }
  chSysUnlockFromIsr();

  /* Transmission buffer empty.*/
  if ((cr1 & USART_CR1_TXEIE) && (sr & USART_SR_TXE)) {
    msg_t b;
    chSysLockFromIsr();
    b = chOQGetI(&sd485p->oqueue);
    if (b < Q_OK) {
      chnAddFlagsI(sd485p, CHN_OUTPUT_EMPTY);
      u->CR1 = (cr1 & ~USART_CR1_TXEIE) | USART_CR1_TCIE;
    }
    else
    {
      /* Set driver enable pad. */
      if (sd485p->config->ssport != NULL)
        palSetPad(sd485p->config->ssport, sd485p->config->sspad);
      /* Disable RX. */
      u->CR1 = cr1 & ~USART_CR1_RE;
      u->DR = b;
    }
    chSysUnlockFromIsr();
  }

  /* Physical transmission end.*/
  if (sr & USART_SR_TC) {
    chSysLockFromIsr();
    chnAddFlagsI(sd485p, CHN_TRANSMISSION_END);
    chSysUnlockFromIsr();
    u->CR1 = cr1 & ~(USART_CR1_TXEIE | USART_CR1_TCIE);
    u->SR = ~USART_SR_TC;
    /* Clear driver enable pad. */
    if (sd485p->config->ssport != NULL)
      palClearPad(sd485p->config->ssport, sd485p->config->sspad);
    /* Enable RX. */
    u->CR1 = cr1 | USART_CR1_RE;
  }
}

#if STM32_SERIAL_485_USE_USART1 || defined(__DOXYGEN__)
static void notify1(GenericQueue *qp) {

  (void)qp;
  USART1->CR1 |= USART_CR1_TXEIE;
}
#endif

#if STM32_SERIAL_485_USE_USART2 || defined(__DOXYGEN__)
static void notify2(GenericQueue *qp) {

  (void)qp;
  USART2->CR1 |= USART_CR1_TXEIE;
}
#endif

#if STM32_SERIAL_485_USE_USART3 || defined(__DOXYGEN__)
static void notify3(GenericQueue *qp) {

  (void)qp;
  USART3->CR1 |= USART_CR1_TXEIE;
}
#endif

#if STM32_SERIAL_485_USE_UART4 || defined(__DOXYGEN__)
static void notify4(GenericQueue *qp) {

  (void)qp;
  UART4->CR1 |= USART_CR1_TXEIE;
}
#endif

#if STM32_SERIAL_485_USE_UART5 || defined(__DOXYGEN__)
static void notify5(GenericQueue *qp) {

  (void)qp;
  UART5->CR1 |= USART_CR1_TXEIE;
}
#endif

#if STM32_SERIAL_485_USE_USART6 || defined(__DOXYGEN__)
static void notify6(GenericQueue *qp) {

  (void)qp;
  USART6->CR1 |= USART_CR1_TXEIE;
}
#endif

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if STM32_SERIAL_485_USE_USART1 || defined(__DOXYGEN__)
#if !defined(STM32_USART1_HANDLER)
#error "STM32_USART1_HANDLER not defined"
#endif
/**
 * @brief   USART1 interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(STM32_USART1_HANDLER) {

  CH_IRQ_PROLOGUE();

  serve_interrupt(&SD4851);

  CH_IRQ_EPILOGUE();
}
#endif

#if STM32_SERIAL_485_USE_USART2 || defined(__DOXYGEN__)
#if !defined(STM32_USART2_HANDLER)
#error "STM32_USART2_HANDLER not defined"
#endif
/**
 * @brief   USART2 interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(STM32_USART2_HANDLER) {

  CH_IRQ_PROLOGUE();

  serve_interrupt(&SD4852);

  CH_IRQ_EPILOGUE();
}
#endif

#if STM32_SERIAL_485_USE_USART3 || defined(__DOXYGEN__)
#if !defined(STM32_USART3_HANDLER)
#error "STM32_USART3_HANDLER not defined"
#endif
/**
 * @brief   USART3 interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(STM32_USART3_HANDLER) {

  CH_IRQ_PROLOGUE();

  serve_interrupt(&SD4853);

  CH_IRQ_EPILOGUE();
}
#endif

#if STM32_SERIAL_485_USE_UART4 || defined(__DOXYGEN__)
#if !defined(STM32_UART4_HANDLER)
#error "STM32_UART4_HANDLER not defined"
#endif
/**
 * @brief   UART4 interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(STM32_UART4_HANDLER) {

  CH_IRQ_PROLOGUE();

  serve_interrupt(&SD4854);

  CH_IRQ_EPILOGUE();
}
#endif

#if STM32_SERIAL_485_USE_UART5 || defined(__DOXYGEN__)
#if !defined(STM32_UART5_HANDLER)
#error "STM32_UART5_HANDLER not defined"
#endif
/**
 * @brief   UART5 interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(STM32_UART5_HANDLER) {

  CH_IRQ_PROLOGUE();

  serve_interrupt(&SD4855);

  CH_IRQ_EPILOGUE();
}
#endif

#if STM32_SERIAL_485_USE_USART6 || defined(__DOXYGEN__)
#if !defined(STM32_USART6_HANDLER)
#error "STM32_USART6_HANDLER not defined"
#endif
/**
 * @brief   USART1 interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(STM32_USART6_HANDLER) {

  CH_IRQ_PROLOGUE();

  serve_interrupt(&SD4856);

  CH_IRQ_EPILOGUE();
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level serial driver initialization.
 *
 * @notapi
 */
void sd485_lld_init(void) {

#if STM32_SERIAL_485_USE_USART1
  sd485ObjectInit(&SD4851, NULL, notify1);
  SD4851.usart = USART1;
#endif

#if STM32_SERIAL_485_USE_USART2
  sd485ObjectInit(&SD4852, NULL, notify2);
  SD4852.usart = USART2;
#endif

#if STM32_SERIAL_485_USE_USART3
  sd485ObjectInit(&SD4853, NULL, notify3);
  SD4853.usart = USART3;
#endif

#if STM32_SERIAL_485_USE_UART4
  sd485ObjectInit(&SD4854, NULL, notify4);
  SD4854.usart = UART4;
#endif

#if STM32_SERIAL_485_USE_UART5
  sd485ObjectInit(&SD4855, NULL, notify5);
  SD4855.usart = UART5;
#endif

#if STM32_SERIAL_485_USE_USART6
  sd485ObjectInit(&SD4856, NULL, notify6);
  SD4856.usart = USART6;
#endif
}

/**
 * @brief   Low level serial driver configuration and (re)start.
 *
 * @param[in] sd485p    pointer to a @p Serial485Driver object
 * @param[in] config    the architecture-dependent serial driver configuration.
 *                      If this parameter is set to @p NULL then a default
 *                      configuration is used.
 *
 * @notapi
 */
void sd485_lld_start(Serial485Driver *sd485p, const Serial485Config *config) {

  if (config == NULL)
    config = &default_config;

  if (sd485p->state == SD485_STOP) {
#if STM32_SERIAL_485_USE_USART1
    if (&SD4851 == sd485p) {
      rccEnableUSART1(FALSE);
      nvicEnableVector(STM32_USART1_NUMBER,
                       CORTEX_PRIORITY_MASK(STM32_SERIAL_485_USART1_PRIORITY));
    }
#endif
#if STM32_SERIAL_485_USE_USART2
    if (&SD4852 == sd485p) {
      rccEnableUSART2(FALSE);
      nvicEnableVector(STM32_USART2_NUMBER,
                       CORTEX_PRIORITY_MASK(STM32_SERIAL_485_USART2_PRIORITY));
    }
#endif
#if STM32_SERIAL_485_USE_USART3
    if (&SD4853 == sd485p) {
      rccEnableUSART3(FALSE);
      nvicEnableVector(STM32_USART3_NUMBER,
                       CORTEX_PRIORITY_MASK(STM32_SERIAL_485_USART3_PRIORITY));
    }
#endif
#if STM32_SERIAL_485_USE_UART4
    if (&SD4854 == sd485p) {
      rccEnableUART4(FALSE);
      nvicEnableVector(STM32_UART4_NUMBER,
                       CORTEX_PRIORITY_MASK(STM32_SERIAL_485_UART4_PRIORITY));
    }
#endif
#if STM32_SERIAL_485_USE_UART5
    if (&SD4855 == sd485p) {
      rccEnableUART5(FALSE);
      nvicEnableVector(STM32_UART5_NUMBER,
                       CORTEX_PRIORITY_MASK(STM32_SERIAL_485_UART5_PRIORITY));
    }
#endif
#if STM32_SERIAL_485_USE_USART6
    if (&SD4856 == sd485p) {
      rccEnableUSART6(FALSE);
      nvicEnableVector(STM32_USART6_NUMBER,
                       CORTEX_PRIORITY_MASK(STM32_SERIAL_485_USART6_PRIORITY));
    }
#endif
  }
  usart_init(sd485p, config);
}

/**
 * @brief   Low level serial driver stop.
 * @details De-initializes the USART, stops the associated clock, resets the
 *          interrupt vector.
 *
 * @param[in] sd485p    pointer to a @p Serial485Driver object
 *
 * @notapi
 */
void sd485_lld_stop(Serial485Driver *sd485p) {

  if (sd485p->state == SD485_READY) {
    usart_deinit(sd485p->usart);
#if STM32_SERIAL_485_USE_USART1
    if (&SD4851 == sd485p) {
      rccDisableUSART1(FALSE);
      nvicDisableVector(STM32_USART1_NUMBER);
      return;
    }
#endif
#if STM32_SERIAL_485_USE_USART2
    if (&SD4852 == sd485p) {
      rccDisableUSART2(FALSE);
      nvicDisableVector(STM32_USART2_NUMBER);
      return;
    }
#endif
#if STM32_SERIAL_485_USE_USART3
    if (&SD4853 == sd485p) {
      rccDisableUSART3(FALSE);
      nvicDisableVector(STM32_USART3_NUMBER);
      return;
    }
#endif
#if STM32_SERIAL_485_USE_UART4
    if (&SD4854 == sd485p) {
      rccDisableUART4(FALSE);
      nvicDisableVector(STM32_UART4_NUMBER);
      return;
    }
#endif
#if STM32_SERIAL_485_USE_UART5
    if (&SD4855 == sd485p) {
      rccDisableUART5(FALSE);
      nvicDisableVector(STM32_UART5_NUMBER);
      return;
    }
#endif
#if STM32_SERIAL_485_USE_USART6
    if (&SD4856 == sd485p) {
      rccDisableUSART6(FALSE);
      nvicDisableVector(STM32_USART6_NUMBER);
      return;
    }
#endif
  }
}

#endif /* HAL_USE_SERIAL_485 */

/** @} */
