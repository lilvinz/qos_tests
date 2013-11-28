/**************************************************************************//**
\file  dbgPort.c

\brief Implementation of debug interface

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    17/12/07 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/
/******************************************************************************
                   Includes section
******************************************************************************/
#include <halW1.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
//#define DEBUG_SOFT
#define MESSAGE_ATOMIC_DELAY   20000
#define MESSAGE_DELAY          20
#define STACK_VIEW_MATRIX      7
#define STACK_WORD_VIEW        20

/******************************************************************************
                   Types section
******************************************************************************/
typedef union
{
  struct {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t r12;
    uint32_t r13;
    uint32_t r14;
    uint32_t spsr;
  };
  uint32_t avarRegState[16];
} HalAvarRegState_t;

/******************************************************************************
                   Global variables section
******************************************************************************/
HalAvarRegState_t halAvarRegState;

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
 Configure DBGU
 Parameters:
   none
 Returns:
   none
******************************************************************************/
void configure_dbgu(void)
{
  /* Reset and disable receiver */
  AT91C_BASE_DBGU->DBGU_CR = AT91C_US_RSTRX | AT91C_US_RSTTX;

  /* Disable interrupts */
  AT91C_BASE_DBGU->DBGU_IDR = 0xFFFFFFFF;

  /* Configure PIOs for DBGU */
  AT91C_BASE_PIOA->PIO_PDR = AT91C_PA27_DRXD | AT91C_PA28_DTXD;
  AT91C_BASE_PIOA->PIO_ASR = AT91C_PA27_DRXD | AT91C_PA28_DTXD;

  /* === Configure serial link === */
  /* Define the baud rate divisor register [BRGR = MCK / (115200 * 16)] */
  AT91C_BASE_DBGU->DBGU_BRGR = 26;
  /* Define the USART mode */
  AT91C_BASE_DBGU->DBGU_MR = AT91C_US_PAR_NONE | AT91C_US_CHMODE_NORMAL;

  /* Disable the RX and TX PDC transfer requests */
  AT91C_BASE_DBGU->DBGU_PTCR = AT91C_PDC_RXTDIS;
  AT91C_BASE_DBGU->DBGU_PTCR = AT91C_PDC_TXTDIS;

  /* Enable transmitter */
  AT91C_BASE_DBGU->DBGU_CR = AT91C_US_TXEN;
}

/******************************************************************************
 This function is used to send a string through the DBGU channel
 (Very low level debugging)
 Parameters:
   buffer - char string
 Returns:
   none
******************************************************************************/
void dbgu_print_ascii(const char *buffer)
{
  while (*buffer != '\0')
  {
    while (!(AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_TXRDY));
      AT91C_BASE_DBGU->DBGU_THR = (*buffer++ & 0x1FF);
  }
}

/******************************************************************************
 This function is used to print a 32-bit value in hex
 Parameters:
   value - parameter for dbgu translation
 Returns:
   none
******************************************************************************/
void dbgu_print_hex8(uint32_t value)
{
  char c = 0;
  char shift = sizeof(uint32_t) * 8;

  dbgu_print_ascii("0x\0");
  do {
    shift -= 4;
    while (!(AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_TXRDY));
      c = ((value >> shift) & 0xF);
    if (c > 9)
      AT91C_BASE_DBGU->DBGU_THR = (('A' + (c - 10)) & 0x1FF);
    else
      AT91C_BASE_DBGU->DBGU_THR = (('0' + c) & 0x1FF);
  } while (shift != 0);
}

/******************************************************************************
 Delay between messages
 Parameters:
   none
 Returns:
   none
******************************************************************************/
static void halDelayBetweenMessages(void)
{
  uint8_t itr;

  for (itr = 0; itr < MESSAGE_DELAY; itr++)
  {
    halDelayUs(MESSAGE_ATOMIC_DELAY);
    halDelayUs(MESSAGE_ATOMIC_DELAY);
    halDelayUs(MESSAGE_ATOMIC_DELAY);
    halDelayUs(MESSAGE_ATOMIC_DELAY);
    halDelayUs(MESSAGE_ATOMIC_DELAY);
  }
}

/******************************************************************************
 Send stack through dbgu
 Parameters:
   stackPointer - pointer to end of stack
 Returns:
   none
******************************************************************************/
static void halStackView(uint32_t *stackPointer)
{
  uint8_t itr, itm;

  dbgu_print_ascii(" Stack memory dump: \n\r");

  for (itr = 0, itm = 1; itr < STACK_WORD_VIEW; itr++, itm++)
  {
    dbgu_print_hex8(*stackPointer++);
    if (STACK_VIEW_MATRIX > itm)
    {
      dbgu_print_ascii(" ");
    }
    else
    {
      itm = 0;
      dbgu_print_ascii("\n\r");
    }
  }
  dbgu_print_ascii("\n\r");
}

/******************************************************************************
 Send cpu register through dbgu
 Parameters:
   none
 Returns:
   none
******************************************************************************/
static void halCommRegisterView(void)
{
  uint8_t itr;

  dbgu_print_ascii(" CPU registers r0-r12: \n\r");

  for (itr = 0; itr < 13; itr++)
  {
    dbgu_print_hex8(halAvarRegState.avarRegState[itr]);
    if (0x03 == (itr & 0x03)) // every fourth
      dbgu_print_ascii("\n\r");
    else
      dbgu_print_ascii(" ");
  }
  dbgu_print_ascii("\n\r");
}

/******************************************************************************
 Send sp, lr and spsr through dbgu
 Parameters:
   none
 Returns:
   none
******************************************************************************/
static void halSpecRegisterView(void)
{
  dbgu_print_ascii(" Stack pointer before exception. SP = ");
  dbgu_print_hex8(halAvarRegState.r13);
  dbgu_print_ascii("\n\r");

  dbgu_print_ascii(" Data abort link register. LR = ");
  dbgu_print_hex8(halAvarRegState.r14);
  dbgu_print_ascii("\n\r");

  dbgu_print_ascii(" Status register before exception. CPSR = ");
  dbgu_print_hex8(halAvarRegState.spsr);
  dbgu_print_ascii("\n\r");
}

/******************************************************************************
 Parse memory controller state
 Parameters:
   abortAddress - abort access address
   abortStatus - memory controller status
 Returns:
   none
******************************************************************************/
static void parseAbortState(uint32_t abortAddress, uint32_t abortStatus)
{
  dbgu_print_hex8(abortAddress);
  dbgu_print_ascii("\n\r");

  if (abortStatus & (1ul << 16))
    dbgu_print_ascii(" EMAC is source of the abort status. \n\r");

  if (abortStatus & (1ul << 17))
    dbgu_print_ascii(" PDS is source of the abort status. \n\r");

  if (abortStatus & (1ul << 18))
    dbgu_print_ascii(" ARM core is source of the abort status. \n\r");

  switch (abortStatus & AT91C_MC_ABTSZ)
  {
    case AT91C_MC_ABTSZ_BYTE:
        dbgu_print_ascii(" Byte size (8-bit) abort access to");
      break;
    case AT91C_MC_ABTSZ_HWORD:
        dbgu_print_ascii(" Half-word size (16-bit) abort access to");
      break;
    case AT91C_MC_ABTSZ_WORD:
        dbgu_print_ascii(" Word size (32-bit) abort access to");
      break;
    default:
        dbgu_print_ascii(" Undefined size of abort access to");
      break;
  }

  if (abortStatus & AT91C_MC_MISADD)
    dbgu_print_ascii(" misaligned address. \n\r");

  if (abortStatus & AT91C_MC_UNDADD)
    dbgu_print_ascii(" undefined address. \n\r");

  switch (abortStatus & AT91C_MC_ABTTYP)
  {
    case AT91C_MC_ABTTYP_DATAR:
        dbgu_print_ascii(" Data read abort access. \n\r");
      break;
    case AT91C_MC_ABTTYP_DATAW:
        dbgu_print_ascii(" Data write abort access. \n\r");
      break;
    case AT91C_MC_ABTTYP_FETCH:
        dbgu_print_ascii(" Code fetch abort access. \n\r");
      break;
    default:
        dbgu_print_ascii(" Undefined type of abort access. \n\r");
      break;
  }
}

/******************************************************************************
 Prefetch Abort Handler
 Parameters:
   prevSP - pointer to stack of the previous regime
 Returns:
   none
******************************************************************************/
void custom_pabt_handler(uint32_t *prevSP)
{
  uint32_t abortAddress = AT91C_BASE_MC->MC_AASR;
  uint32_t abortStatus  = AT91C_BASE_MC->MC_ASR;

  configure_dbgu();

  while (1)
  {
    dbgu_print_ascii(" Prefetch abort at ");

    parseAbortState(abortAddress, abortStatus);
    halSpecRegisterView();

    dbgu_print_ascii(" Abort instruction has LR-4 address. \n\r");
    halCommRegisterView();
    halStackView(prevSP);
    dbgu_print_ascii("********************************************* \n\r");
    // delay for reading of message
    halDelayBetweenMessages();
  }
}

/******************************************************************************
 Data Abort Handler
 Parameters:
   prevSP - pointer to stack of the previous regime
 Returns:
   none
******************************************************************************/
void custom_dabt_handler(uint32_t *prevSP)
{
  uint32_t abortAddress = AT91C_BASE_MC->MC_AASR;
  uint32_t abortStatus  = AT91C_BASE_MC->MC_ASR;

  configure_dbgu();

  while (1)
  {
    dbgu_print_ascii(" Data abort at ");

    parseAbortState(abortAddress, abortStatus);
    halSpecRegisterView();

    dbgu_print_ascii(" Abort instruction has LR-8 address. \n\r");
    halCommRegisterView();
    halStackView(prevSP);
    dbgu_print_ascii("********************************************* \n\r");
    // delay for reading of message
    halDelayBetweenMessages();
  }
}

/******************************************************************************
 Undefined command Handler
 Parameters:
   prevSP - pointer to stack of the previous regime
 Returns:
   none
******************************************************************************/
void custom_undf_handler(uint32_t *prevSP)
{
  uint32_t abortAddress = AT91C_BASE_MC->MC_AASR;
  uint32_t abortStatus  = AT91C_BASE_MC->MC_ASR;

  configure_dbgu();

  while (1)
  {
    dbgu_print_ascii(" Undefined command at ");

    parseAbortState(abortAddress, abortStatus);
    halSpecRegisterView();

    dbgu_print_ascii(" Undefined instruction has LR address. \n\r");
    halCommRegisterView();
    halStackView(prevSP);
    dbgu_print_ascii("********************************************* \n\r");
    // delay for reading of message
    halDelayBetweenMessages();
  }
}

/******************************************************************************
  Function Name       : default_spurious_handler
  Object              : default handler for spurious interrupt
*******************************************************************************/
#if defined(__GNUC__)
  void default_spurious_handler(void)
#elif defined(__ICCARM__)
  void default_spurious_handler(void)
#endif
{
#ifdef DEBUG_SOFT
  configure_dbgu();

  while(1)
  {
    dbgu_print_ascii(" Spurious Interrupt. \n\r");
    // delay for reading of message
    halDelayBetweenMessages();
  }
#else
  AT91C_BASE_AIC->AIC_EOICR = 0;
#endif
}

/******************************************************************************
  Function Name       : default_fiq_handler
  Object              : default handler for fast interrupt
*******************************************************************************/
#if defined(__GNUC__)
  void default_fiq_handler(void)
#elif defined(__ICCARM__)
  __fiq __arm void default_fiq_handler(void)
#endif
{
  configure_dbgu();

  while(1)
  {
    dbgu_print_ascii(" Unexpected FIQ Interrupt\n\r ");
    // delay for reading of message
    halDelayBetweenMessages();
  }
}

/******************************************************************************
  Function Name       : default_irq_handler
  Object              : default handler for irq
*******************************************************************************/
#if defined(__GNUC__)
  void default_irq_handler(void)
#elif defined(__ICCARM__)
  __irq __arm void default_irq_handler(void)
#endif
{
  uint32_t addr = AT91C_BASE_AIC->AIC_ISR;
  configure_dbgu();

  while (1)
  {
    dbgu_print_ascii(" Unexpected IRQ Interrupt \n\r");
    dbgu_print_ascii(" AIC_ISR = ");
    dbgu_print_hex8(addr);
    dbgu_print_ascii("\n\r");
    // delay for reading of message
    halDelayBetweenMessages();
  }
}

/******************************************************************************
 This function is used to send a uint8_t buffer through the DBGU channel
 (Very low level debugging)
 Parameters:
   buffer - pointer to buffer
   size - size buffer in bytes
 Returns:
   none
******************************************************************************/
void dbgu_print_buff(uint8_t *buffer, uint8_t size)
{
  while (size != 0x00)
  {
    while (!(AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_TXRDY));
      AT91C_BASE_DBGU->DBGU_THR = (*buffer++ & 0x1FF);
    size--;
  }
}
