/**************************************************************************//**
\file   atomic.c

\brief  Implementation of atomic sections.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    12/12/07 A. Khromykh - Created
******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                   Includes section
******************************************************************************/
#include <atomic.h>
#include <types.h>

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
 Disables global interrupt. Enter atomic section.
******************************************************************************/
void halStartAtomic(atomic_t volatile *pAtomic)
{
#if defined(__GNUC__)
  asm volatile (
                "STMDB SP!, {R1}      \n\t" /* Push R0. */
                "MRS   R1, CPSR       \n\t" /* Get CPSR.*/
                "STR   R1, [R0]       \n\t" /* Copy pAtomic to R1 */
                "ORR   R1, R1, #0xC0  \n\t" /* Disable IRQ, FIQ. */
                "MSR   CPSR, R1       \n\t" /* Write back modified value.*/
                "LDMIA SP!, {R1}" );  /* Pop R0.*/
#elif defined(__ICCARM__)
  *pAtomic = __get_interrupt_state();
  __disable_interrupt();
#else
#error 'Compiler not supported.'
#endif
  (void)pAtomic;
}

/******************************************************************************
 Exit atomic section
******************************************************************************/
void halEndAtomic(atomic_t volatile *pAtomic)
{
#if defined(__GNUC__)
  asm volatile (
                "STMDB SP!, {R1}      \n\t" /* Push R1.*/
                "STMDB SP!, {R2}      \n\t" /* Push R2.*/

                "LDR   R1, [R0]       \n\t" /* Copy pAtomic to R1*/

                "MRS   R2, CPSR       \n\t" /* Get CPSR.*/
                "BIC   R2, R2, #0xC0  \n\t" /* Clear IRQ, FIQ bits.*/
                "ORR   R1, R1, R2     \n\t" /* Enable IRQ, FIQ.*/

                "MSR   CPSR, R1       \n\t"   /* Write back modified value. */

                "LDMIA SP!, {R2}      \n\t"  /* Pop R2.*/
                "LDMIA SP!, {R1}" );   /* Pop R1.*/
#elif defined(__ICCARM__)
  __set_interrupt_state(*pAtomic);
#else
#error 'Compiler not supported.'
#endif
  (void)pAtomic;
}

