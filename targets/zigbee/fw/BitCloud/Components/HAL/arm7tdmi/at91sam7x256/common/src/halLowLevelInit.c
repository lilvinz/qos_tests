/**************************************************************************//**
\file  halLowLevelInit.c

\brief Implementation of low level initialization.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008 , Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    5/12/07 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                   Includes section
******************************************************************************/
#include <AT91SAM7X256.h>
#include <dbgPort.h>
#include <types.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
//#define HAL_ENABLE_AIC_DEBUG_MODE

#ifdef RF
  #define PLL_MULTIPLIER       95
  #define PLL_DIVIDER          1
#else
  #if defined(HAL_8MHz)
    #define PLL_MULTIPLIER     95
    #define PLL_DIVIDER        8
  #elif defined(HAL_18d432MHz)
    #define PLL_MULTIPLIER     25
    #define PLL_DIVIDER        5
  #endif
#endif

#define RESET_ERSTL             0x01
#define RESET_KEY               0xA5

#define FLASH_USEC_CYCLE_NUM    75ul

#define BOOTLOADER_USED         1
#undef HAL_DISABLE_WDT

/* After reset clock chip needs 240 us for exit from sleep
   or about 150 us (practice 143 us) for setup clock if it is in RX_ON_NOCLK state.
   1 nop is 24 us min (internal RC has 22..42 kHz) */
#define DELAY_FOR_RF_EXTCLCK_STABILIZE    NOP; \
                                          NOP; \
                                          NOP; \
                                          NOP; \
                                          NOP; \
                                          NOP; \
                                          NOP; \
                                          NOP; \
                                          NOP; \
                                          NOP; \
                                          NOP



void halRemapVectorTable(void)
{
    AT91PS_MC   pMC = AT91C_BASE_MC;
    uint32_t         *reset_rom = 0;
    uint32_t         *reset_ram = (uint32_t *) AT91C_ISRAM;
    uint32_t         temp;

    temp = *reset_ram;
    *reset_ram = 0xDEADBEEF;
    if (*reset_rom != *reset_ram)
    {
        // remap
        pMC->MC_RCR = AT91C_MC_RCB;
    }
    else
    {
        *reset_ram = temp;
    }
}


/***************************************************************************//*
 \brief This function performs very low level HW initialization
        this function can be use a Stack, depending the compilation
        optimization mode
******************************************************************************/
void halLowLevelInit(void)
{
  uint8_t i = 0;

#if (BOOTLOADER_USED == 1)

  //halRemapVectorTable();

  // set reset vektor to bootloaders reset vector
  // to ensure bootloader is running first if reset occures.
  *((uint32_t*)AT91C_ISRAM) = *((uint32_t *)AT91C_IFLASH);

#endif

  /* EFC Init. Maximum Operating Frequency 48000000(Hz) > 30 MHz. Read operation is 2 cycle.
     Master clock is 48000000 Hz. It's approximately 50 cycle for 1 us or about 75 cycle in 1.5 us */
  AT91C_BASE_MC->MC_FMR = (AT91C_MC_FMCN & (FLASH_USEC_CYCLE_NUM << 16)) | AT91C_MC_FWS_1FWS;

#ifdef HAL_DISABLE_WDT
  /* Watchdog Disable */
  AT91C_BASE_WDTC->WDTC_WDMR = AT91C_WDTC_WDDIS;
#endif

  /* enable clock on port A */
  AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_PIOA);

#ifdef RF
  /* setting to low level on rf sleep pin */
  AT91C_BASE_PIOA->PIO_OER = AT91C_PIO_PA15;
  AT91C_BASE_PIOA->PIO_PER = AT91C_PIO_PA15;
  AT91C_BASE_PIOA->PIO_CODR = AT91C_PIO_PA15;
  DELAY_FOR_RF_EXTCLCK_STABILIZE;
#endif

#ifdef RF
  /* Set MCK at 48 000 000 Hz. XIN 2 MHz. */
  /* 1 Disabling and bypassing the Main Oscillator: */
  AT91C_BASE_PMC->PMC_MOR = AT91C_CKGR_OSCBYPASS;
  /* 2 Checking the Main Oscillator Frequency (Optional) */
  /* 3 Setting PLL and divider: */
  /* divider is bypassed. = 1.
     multiplier is (95+1). Fout = (1 * 96) = 96 MHz.
     for 96 MHz the error is 0%.
     AT91C_CKGR_PLLCOUNT was counted ATMEL utilite.
     USB source clock is PLL clock output divided by 2. */
  AT91C_BASE_PMC->PMC_PLLR = ((AT91C_CKGR_DIV & PLL_DIVIDER) |
                             (AT91C_CKGR_PLLCOUNT & (28 << 8)) |
                             (AT91C_CKGR_MUL & (PLL_MULTIPLIER << 16))) |
                             AT91C_CKGR_USBDIV_1;
  // Wait the startup time
  while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_LOCK));
  /* 4. Selection of Master Clock and Processor Clock */
  /* select the PLL clock divided by 2 */
  AT91C_BASE_PMC->PMC_MCKR = AT91C_PMC_PRES_CLK_2 ;
  while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY));
  AT91C_BASE_PMC->PMC_MCKR |= AT91C_PMC_CSS_PLL_CLK ;
  while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY));
#else
  #if defined(HAL_8MHz)
    /* Set MCK at 48 000 000 Hz. Crystal 8 MHz. */
    /* 1 Enabling the Main Oscillator: */
    /* Typically startup time 5 ms for 8 MHz crystal.
       Slow clock = 1/32768 = 30.51 us approximately.
       Start up time = 8 * 19 / SCK = 56 * 30.51 = 5 ms */
    AT91C_BASE_PMC->PMC_MOR = ((AT91C_CKGR_OSCOUNT & (0x13 << 8)) | AT91C_CKGR_MOSCEN);
    // Wait the startup time
    while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MOSCS));
    /* 2 Checking the Main Oscillator Frequency (Optional) */
    /* 3 Setting PLL and divider: */
    /* divider is 8. (8 / 8) = 1.
       multiplier is (95+1). Fout = (1 * 96) = 96 MHz.
       for 96 MHz the error is 0%.
       AT91C_CKGR_PLLCOUNT was counted ATMEL utilite */
    AT91C_BASE_PMC->PMC_PLLR = ((AT91C_CKGR_DIV & PLL_DIVIDER) |
                               (AT91C_CKGR_PLLCOUNT & (28 << 8)) |
                               (AT91C_CKGR_MUL & (PLL_MULTIPLIER << 16))) |
                               AT91C_CKGR_USBDIV_1;;
    // Wait the startup time
    while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_LOCK));
    /* 4. Selection of Master Clock and Processor Clock */
    /* select the PLL clock divided by 2 */
    AT91C_BASE_PMC->PMC_MCKR = AT91C_PMC_PRES_CLK_2 ;
    while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY));
    AT91C_BASE_PMC->PMC_MCKR |= AT91C_PMC_CSS_PLL_CLK ;
    while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY));
  #elif defined(HAL_18d432MHz)

  // Startup time of main oscillator (in number of slow clock ticks).
  #define BOARD_OSCOUNT           (AT91C_CKGR_OSCOUNT & (0x40 << 8))
  // USB PLL divisor value to obtain a 48MHz clock.
  #define BOARD_USBDIV            AT91C_CKGR_USBDIV_1
  // PLL frequency range.
  #define BOARD_CKGR_PLL          AT91C_CKGR_OUT_0
  // PLL startup time (in number of slow clock ticks).
  #define BOARD_PLLCOUNT          (16 << 8)
  // PLL MUL value.
  #define BOARD_MUL               (AT91C_CKGR_MUL & (72 << 16))
  // PLL DIV value.
  #define BOARD_DIV               (AT91C_CKGR_DIV & 14)
  // Master clock prescaler value.
  #define BOARD_PRESCALER         AT91C_PMC_PRES_CLK_2

  /* Initialize main oscillator
   ****************************/
  AT91C_BASE_PMC->PMC_MOR = BOARD_OSCOUNT | AT91C_CKGR_MOSCEN;
  while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MOSCS));

  /* Initialize PLL at 96MHz (96.109) and USB clock to 48MHz */
  AT91C_BASE_PMC->PMC_PLLR = BOARD_USBDIV | BOARD_CKGR_PLL | BOARD_PLLCOUNT
                           | BOARD_MUL | BOARD_DIV;
  while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_LOCK));

  /* Wait for the master clock if it was already initialized */
  while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY));

  /* Switch to fast clock
   **********************/
  /* Switch to slow clock + prescaler */
  AT91C_BASE_PMC->PMC_MCKR = BOARD_PRESCALER;
  while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY));

  /* Switch to fast clock + prescaler */
  AT91C_BASE_PMC->PMC_MCKR |= AT91C_PMC_CSS_PLL_CLK;
  while (!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY));

  #endif
#endif

  /* Reset AIC: assign default handler for each interrupt source */
  AT91C_BASE_AIC->AIC_SVR[0] = (int)default_fiq_handler ;
  for (i = 1; i < 31; i++) {
    AT91C_BASE_AIC->AIC_SVR[i] = (int)default_irq_handler ;
  }
  AT91C_BASE_AIC->AIC_SPU = (int)default_spurious_handler;
  // Perform 8 IT acknowledge (write any value in EOICR)
  for (i = 0; i < 8 ; i++) {
    AT91C_BASE_AIC->AIC_EOICR = 0;
  }

#ifdef HAL_ENABLE_AIC_DEBUG_MODE
  // Enable the Debug mode
  AT91C_BASE_AIC->AIC_DCR = AT91C_AIC_DCR_PROT;
#endif

  /* Enable hardware reset on RST pin. */
  /* The external reset is asserted during a time of 2^(RESET_ERSTL+1) slow clock cycles. */
  AT91C_BASE_RSTC->RSTC_RMR = AT91C_RSTC_URSTEN | (RESET_ERSTL << 8) | (RESET_KEY << 24);

}

