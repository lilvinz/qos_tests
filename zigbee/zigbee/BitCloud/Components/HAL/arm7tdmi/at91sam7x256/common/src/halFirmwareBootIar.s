/**************************************************************************//**
\file  halFirmwareBootIar.s

\brief Initialization file.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    5/12/07 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

;------------------------------------------------------------------------------
; Include your AT91 Library files
;------------------------------------------------------------------------------
#define __ASSEMBLY__
#include "AT91SAM7X256.h"
;------------------------------------------------------------------------------

#define TOP_OF_MEMORY    (AT91C_ISRAM + AT91C_ISRAM_SIZE)
#define IRQ_STACK_SIZE   (3*8*4)
     ; 3 words to be saved per interrupt priority level

; Mode, correspords to bits 0-5 in CPSR
MODE_BITS DEFINE  0x1F    ; Bit mask for mode bits in CPSR
USR_MODE  DEFINE  0x10    ; User mode
FIQ_MODE  DEFINE  0x11    ; Fast Interrupt Request mode
IRQ_MODE  DEFINE  0x12    ; Interrupt Request mode
SVC_MODE  DEFINE  0x13    ; Supervisor mode
ABT_MODE  DEFINE  0x17    ; Abort mode
UND_MODE  DEFINE  0x1B    ; Undefined Instruction mode
SYS_MODE  DEFINE  0x1F    ; System mode

I_BIT     DEFINE  0x80           ; when I bit is set, IRQ is disabled
F_BIT     DEFINE  0x40           ; when F bit is set, FIQ is disabled
THUMB_BIT DEFINE  0x20           ; when THUMB bit is set, THUMB is enabled

;------------------------------------------------------------------------------
; ?RESET
; Reset Vector.
; Normally, segment INTVEC is linked at address 0.
; For debugging purposes, INTVEC may be placed at other addresses.
; A debugger that honors the entry point will start the
; program in a normal way even if INTVEC is not at address 0.
;------------------------------------------------------------------------------
        SECTION .intvec:CODE:NOROOT(2)
        PUBLIC  __vector
        PUBLIC  __iar_program_start

        ARM
__vector:
        b  __iar_program_start             ;; Reset
        ldr  pc, _undf                     ;; Undefined instructions
        ldr  pc, _swi                      ;; Software interrupt (SWI/SVC)
        ldr  pc, _pabt                     ;; Prefetch abort
        ldr  pc, _dabt                     ;; Data abort
        nop                                ;; RESERVED
        ldr  pc, _irq                      ;; IRQ
        ldr  pc, _fiq                      ;; FIQ

        DC32  __iar_program_start
        _undf:  DC32 __undf                    ;;/* undefined				*/
        _swi:   DC32 swi_handler               ;;/* SWI                     */
        _pabt:  DC32 __pabt                    ;;/* program abort			*/
        _dabt:  DC32 __dabt                    ;;/* data abort				*/
        _fiq:   DC32 __fiq                     ;;/* FIQ						*/
        _irq:   DC32 __irq                     ;;/* IRQ						*/

        __undf: b  undf_handler                 ;;/* undefined				*/
        __pabt: b  pabt_handler                 ;;/* program abort			*/
        __dabt: b  dabt_handler                 ;;/* data abort				*/
        __fiq:  b  fiq_handler                  ;;/* FIQ						*/
        __irq:  b  irq_handler                  ;;/* IRQ						*/

;------------------------------------------------------------------------------
;- Manage exception: The exception must be ensure in ARM mode
;------------------------------------------------------------------------------
        SECTION text:CODE:NOROOT(2)
        ARM
;------------------------------------------------------------------------------
;- Function             : IRQ_Handler_Entry
;- Treatments           : IRQ Controller Interrupt Handler.
;- Called Functions     : AIC_IVR[interrupt]
;------------------------------------------------------------------------------
;/*------------------------------------------------------------------------------
; *- Function             : fiq_handler
; *- Treatments           : FIQ Interrupt Handler.
; *- Called Functions     :
; *------------------------------------------------------------------------------*/
fiq_handler:
;    /* Adjust LR_irq and save in FIQ stack*/
        sub      lr, lr, #4
        stmfd    sp!, {lr}

;    /*- Save r0 and SPSR in FIQ stack */
        mrs      r14, SPSR
        stmfd    sp!, {r0,r14}

;   /* Read the AIC Fast Interrupt Vector register to clear the interrupt */
        ldr		 r14, =AT91C_BASE_AIC
        ldr      r0 , [r14, #0x00000104]
        str      r14, [r14, #0x00000104]

;    /*- Save scratch/used registers and LR in FIQ Stack */
        stmfd    sp!, {r1-r7, r12, r14}

;    /*- Branch to the routine pointed by the AIC_FVR */
        mov      r14, pc
        bx       r0

;    /*- Restore scratch/used registers and LR from FIQ Stack */
        ldmia    sp!, {r1-r7, r12, r14}

;    /*- Restore SPSR_irq and r0 from FIQ stack */
        ldmia    sp!, {r0,r14}
        msr      SPSR_cxsf, r14

;    /*- Restore adjusted  LR_irq from FIQ stack */
        ldmia    sp!, {lr}

;    /* Return from Fiq interrupt */
		movs	 pc, lr

;/*------------------------------------------------------------------------------
; *- Function             : irq_handler
; *- Treatments           : IRQ Controller Interrupt Handler.
; *- Called Functions     : AIC_IVR[interrupt]
; *------------------------------------------------------------------------------*/
irq_handler:
;/*- Manage Exception Entry */
;/*- Adjust and save LR_irq in IRQ stack */
        sub      lr, lr, #4
        stmfd    sp!, {lr}
;/*- Save r0 and SPSR in IRQ stack */
        mrs      r14, SPSR
        stmfd    sp!, {r0,r14}

;/*- Write in the IVR to support Protect Mode */
;/*- No effect in Normal Mode */
;/*- De-assert the NIRQ and clear the source in Protect Mode */
        ldr      r14, =AT91C_BASE_AIC
        ldr      r0 , [r14, #0x00000100]
        str      r14, [r14, #0x00000100]

;/*- Save scratch/used registers and LR in User Stack */
        stmfd    sp!, {r1-r3, r12, r14}

;/*- Branch to the routine pointed by the AIC_IVR */
        mov      r14, pc
        bx       r0

;/*- Restore scratch/used registers and LR from User Stack */
        ldmia    sp!, {r1-r3, r12, r14}

;/*- Mark the End of Interrupt on the AIC */
        ldr      r14, =AT91C_BASE_AIC
        str      r14, [r14, #0x00000130]

;/*- Restore SPSR_irq and r0 from IRQ stack */
        ldmia    sp!, {r0,r14}
        msr      SPSR_cxsf, r14

;/*- Restore adjusted  LR_irq from IRQ stack directly in the PC */
        ldmia    sp!, {pc}^

swi_handler:
        b        swi_handler      ;/* Software Interrupt */

undf_handler:                     ;/* undefined command */
;/*- Save r0-r12, lr and SPSR in stack */
        stmdb    sp!, {r0-r12, lr}
        mrs      r0, SPSR
        stmdb    sp!, {r0}

;/* load handler pointer to handler register */
        ldr      r9, =custom_undf_handler

;/* copy registers from stack to data structure */
        bl       reg_struct_fill

;/*- Restore SPSR */
        ldmia    sp!, {r0}
        msr      SPSR_cxsf, r0

;/*- Restore r0-r12 and LR_abt from stack */
        ldmia    sp!, {r0-r12, lr}

;/* Return from undefined command interrupt */
		movs	 pc, lr

pabt_handler:                       ;/* program abort */
;/*- Save r0-r12, lr and SPSR in stack */
        stmdb    sp!, {r0-r12, lr}
        mrs      r0, SPSR
        stmdb    sp!, {r0}

;/* load handler pointer to handler register */
        ldr      r9, =custom_pabt_handler

;/* copy registers from stack to data structure */
        bl       reg_struct_fill

;/*- Restore SPSR */
        ldmia    sp!, {r0}
        msr      SPSR_cxsf, r0

;/*- Restore r0-r12 and LR_abt from stack */
        ldmia    sp!, {r0-r12, lr}

;/* Adjust LR and return from perfetch abort interrupt */
        sub      lr, lr, #4
		movs	 pc, lr

dabt_handler:                       ;/* data abort */
;/*- Save r0-r12, lr and SPSR in stack */
        stmdb    sp!, {r0-r12, lr}
        mrs      r0, SPSR
        stmdb    sp!, {r0}

;/* load handler pointer to handler register */
        ldr      r9, =custom_dabt_handler

;/* copy registers from stack to data structure */
        bl       reg_struct_fill

;/*- Restore SPSR */
        ldmia    sp!, {r0}
        msr      SPSR_cxsf, r0

;/*- Restore r0-r12 and LR_abt from stack */
        ldmia    sp!, {r0-r12, lr}

;/* Adjust LR and return from perfetch abort interrupt */
        sub      lr, lr, #8
		movs	 pc, lr

reg_struct_fill:
;/*	Save registers to halAvarRegState  */
        mov      r1, sp

;/*  save lr to r4  */
        ldr      r4, [r1], #4
        mov      r3, #13

;/*  load address structure to r5 */
        ldr      r5, =halAvarRegState

cycle_label:
;/* load r0-r12 from stack stack to r2 */
        ldr      r2, [r1], #4

;/* load r2 to halAvarRegState struct */
        str      r2, [r5], #4
        subs     r3, r3,   #1  ;/* r3-- */
        bne      cycle_label

;/* save structure address for saving stack pointer */
        mov      r6, r5
        add      r5, r5,   #4  ;/* addr++ */

;/* load link register from stack to r2 */
        ldr      r2, [r1], #4

;/* load link register from r2 to struct*/
        str      r2, [r5], #4

;/* load SPSR to struct */
        str      r4, [r5], #4

;/* read stack pointer previous regim */
        mrs      r7, CPSR
        mrs      r8, SPSR

;/* disable interrupt and turn off thumb */
        orr	     r8, r8, #(I_BIT | F_BIT)
        bic	     r8, r8, #THUMB_BIT
        msr      CPSR_cxsf, r8
        mov      r0, sp
        msr      CPSR_cxsf, r7

;/* load stack pointer to struct */
        str      r0, [r6]

;/*- Branch to the routine handler */
        mov      r14, pc
        bx       r9

;/* return to exception handler */
        mov      pc, lr

    ARM      ; Always ARM mode after exeption

;------------------------------------------------------------------------------
; ?INIT
; Program entry.
;------------------------------------------------------------------------------

    SECTION FIQ_STACK:DATA:NOROOT(3)
    SECTION IRQ_STACK:DATA:NOROOT(3)
    SECTION SVC_STACK:DATA:NOROOT(3)
    SECTION ABT_STACK:DATA:NOROOT(3)
    SECTION UND_STACK:DATA:NOROOT(3)
    SECTION CSTACK:DATA:NOROOT(3)
    SECTION text:CODE:NOROOT(2)
    REQUIRE __vector
    EXTERN  ?main
    PUBLIC  __iar_program_start
    EXTERN  halLowLevelInit
    EXTERN custom_pabt_handler
    EXTERN custom_dabt_handler
    EXTERN custom_undf_handler
    EXTERN halAvarRegState

__iar_program_start:
;------------------------------------------------------------------------------
;- Low level Init is performed in a C function: AT91F_LowLevelInit
;- Init Stack Pointer to a valid memory area before calling AT91F_LowLevelInit
;------------------------------------------------------------------------------

;- Retrieve end of RAM address

                ldr     r13,=TOP_OF_MEMORY          ;- Temporary stack in internal RAM for Low Level Init execution
                ldr     r0,=halLowLevelInit
                mov     lr, pc
                bx      r0                          ;- Branch on C function (with interworking)

; Initialize the stack pointers.
; The pattern below can be used for any of the exception stacks:
; FIQ, IRQ, SVC, ABT, UND, SYS.
; The USR mode uses the same stack as SYS.
; The stack segments must be defined in the linker command file,
; and be declared above.

                mrs     r0,cpsr                             ; Original PSR value
                bic     r0,r0,#MODE_BITS                    ; Clear the mode bits
                orr     r0,r0,#UND_MODE                     ; Set UND mode bits
                msr     cpsr_c,r0                           ; Change the mode
                ldr     sp,=SFE(UND_STACK)                  ; End of UND_STACK

                bic     r0,r0,#MODE_BITS                    ; Clear the mode bits
                orr     r0,r0,#ABT_MODE                     ; Set ABT mode bits
                msr     cpsr_c,r0                           ; Change the mode
                ldr     sp,=SFE(ABT_STACK)                  ; End of ABT_STACK

                bic     r0,r0,#MODE_BITS                    ; Clear the mode bits
                orr     r0,r0,#FIQ_MODE                     ; Set FIQ mode bits
                msr     cpsr_c,r0                           ; Change the mode
                ldr     sp,=SFE(FIQ_STACK)                  ; End of FIQ_STACK

                bic     r0,r0,#MODE_BITS                    ; Clear the mode bits
                orr     r0,r0,#IRQ_MODE                     ; Set IRQ mode bits
                msr     cpsr_c,r0                           ; Change the mode
                ldr     sp,=SFE(IRQ_STACK)                  ; End of IRQ_STACK

                bic     r0,r0,#MODE_BITS  |I_BIT|F_BIT      ; Clear the mode bits
                orr     r0,r0,#SVC_MODE                     ; Set SVC mode bits
                msr     cpsr_c,r0                           ; Change the mode
                ldr     sp,=SFE(SVC_STACK)                  ; End of SVC_STACK

#ifdef __ARMVFP__
; Enable the VFP coprocessor.
                mov     r0, #0x40000000                 ; Set EN bit in VFP
                fmxr    fpexc, r0                       ; FPEXC, clear others.

; Disable underflow exceptions by setting flush to zero mode.
; For full IEEE 754 underflow compliance this code should be removed
; and the appropriate exception handler installed.
                mov     r0, #0x01000000	                ; Set FZ bit in VFP
                fmxr    fpscr, r0                       ; FPSCR, clear others.
#endif

; Add more initialization here


; Continue to ?main for more IAR specific system startup

                ldr     r0,=?main
                bx      r0

    END         ;- Terminates the assembly of the last module in a file
