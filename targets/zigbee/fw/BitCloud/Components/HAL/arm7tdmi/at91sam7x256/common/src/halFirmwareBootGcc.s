/**************************************************************************//**
\file  halFirmwareBootGcc.s

\brief Initialization file.

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


	/* Initialization file */

	.extern main
	.extern exit
	.extern halLowLevelInit
	.extern custom_pabt_handler
	.extern custom_dabt_handler
	.extern custom_undf_handler
	.extern halAvarRegState

	.text
	.code 32

	.align  0

	.extern __stack_end__
	.extern __bss_beg__
	.extern __bss_end__
	.extern __data_beg__
	.extern __data_end__
	.extern __data+beg_src__

	.global _start
	.global _entry_point
	.global endless_loop
	.global _startup_code

	/* Stack Sizes */
    .set  UND_STACK_SIZE, 0x00000020
    .set  ABT_STACK_SIZE, 0x00000020
    .set  FIQ_STACK_SIZE, 0x00000090
    .set  IRQ_STACK_SIZE, 0X00000400
    .set  SVC_STACK_SIZE, 0x00000400

	/* Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs */
    .set  MODE_USR, 0x10            /* User Mode */
    .set  MODE_FIQ, 0x11            /* FIQ Mode */
    .set  MODE_IRQ, 0x12            /* IRQ Mode */
    .set  MODE_SVC, 0x13            /* Supervisor Mode */
    .set  MODE_ABT, 0x17            /* Abort Mode */
    .set  MODE_UND, 0x1B            /* Undefined Mode */
    .set  MODE_SYS, 0x1F            /* System Mode */

    .equ  I_BIT, 0x80               /* when I bit is set, IRQ is disabled */
    .equ  F_BIT, 0x40               /* when F bit is set, FIQ is disabled */
    .equ  THUMB_BIT, 0x20           /* when THUMB bit is set, THUMB is enabled */

    .set  AT91C_BASE_AIC, 0xFFFFF000

_start:
	/* Dummy access to the .vector section so it does not get optimized */
    ldr     r0, =resetVector

	/* Setup a stack for each mode - note that this only sets up a usable stack
	for system/user, SWI and IRQ modes.   Also each mode is setup with
	interrupts initially disabled. */
    ldr   r0, .LC6
    msr   CPSR_c, #MODE_UND|I_BIT|F_BIT /* Undefined Instruction Mode */
    mov   sp, r0
    sub   r0, r0, #UND_STACK_SIZE
    msr   CPSR_c, #MODE_ABT|I_BIT|F_BIT /* Abort Mode */
    mov   sp, r0
    sub   r0, r0, #ABT_STACK_SIZE
    msr   CPSR_c, #MODE_FIQ|I_BIT|F_BIT /* FIQ Mode */
    mov   sp, r0
    sub   r0, r0, #FIQ_STACK_SIZE
    msr   CPSR_c, #MODE_IRQ|I_BIT|F_BIT /* IRQ Mode */
    mov   sp, r0
    sub   r0, r0, #IRQ_STACK_SIZE
    msr   CPSR_c, #MODE_SVC             /* Supervisor Mode */
    mov   sp, r0

    bl		halLowLevelInit

	/* Clear BSS. */

	mov     a2, #0			/* Fill value */
	mov		fp, a2			/* Null frame pointer */
	mov		r7, a2			/* Null frame pointer for Thumb */

	ldr		r1, .LC1		/* Start of memory block */
	ldr		r3, .LC2		/* End of memory block */
	subs	r3, r3, r1      /* Length of block */
	beq		.end_clear_loop
	mov		r2, #0

.clear_loop:
	strb	r2, [r1], #1
	subs	r3, r3, #1
	bgt		.clear_loop

.end_clear_loop:

	/* Initialise data. */

	ldr		r1, .LC3		/* Start of memory block */
	ldr		r2, .LC4		/* End of memory block */
	ldr		r3, .LC5
	subs	r3, r3, r1		/* Length of block */
	beq		.end_set_loop

.set_loop:
	ldrb	r4, [r2], #1
	strb	r4, [r1], #1
	subs	r3, r3, #1
	bgt		.set_loop

.end_set_loop:

	mov		r0, #0          /* no arguments  */
	mov		r1, #0          /* no argv either */

    ldr lr, =main
	bx	lr

endless_loop:
	b               endless_loop


	.align 0

	.LC1:
	.word   __bss_beg__
	.LC2:
	.word   __bss_end__
	.LC3:
	.word   __data_beg__
	.LC4:
	.word   __data_beg_src__
	.LC5:
	.word   __data_end__
	.LC6:
	.word	__stack_end__

.section .entry_point,"ax"
		.code 32
		.align 0

_entry_point:
	b 	_start


	/* Setup vector table.  Note that undf, pabt, dabt, fiq just execute
	a null loop. */

.section .vector,"ax"
         .code 32
         .align 0

_vector_table:
resetVector:
	b     _start						/* reset - _start			*/
undefVector:
	ldr   pc, _undf						/* undefined - _undf		*/
swiVector:
	ldr   pc, _swi						/* SWI - _swi				*/
prefetchAbortVector:
	ldr   pc, _pabt						/* program abort - _pabt	*/
dataAbortVector:
	ldr   pc, _dabt						/* data abort - _dabt		*/
reservedVector:
	nop									/* reserved					*/
irqVector:
	ldr   pc, _irq      				/* IRQ - _irq       		*/
fiqVector:
	ldr   pc, _fiq						/* FIQ - _fiq				*/


_undf:  .word __undf                    /* undefined				*/
_swi:   .word swi_handler				/* SWI						*/
_pabt:  .word __pabt                    /* program abort			*/
_dabt:  .word __dabt                    /* data abort				*/
_fiq:   .word __fiq                     /* FIQ						*/
_irq:   .word __irq                     /* IRQ						*/

__undf: b  undf_handler                 /* undefined				*/
__pabt: b  pabt_handler                 /* program abort			*/
__dabt: b  dabt_handler                 /* data abort				*/
__fiq:  b  fiq_handler                  /* FIQ						*/
__irq:  b  irq_handler                  /* IRQ						*/

/*------------------------------------------------------------------------------
 *- Function             : fiq_handler
 *- Treatments           : FIQ Interrupt Handler.
 *- Called Functions     :
 *------------------------------------------------------------------------------*/
fiq_handler:
	/* Adjust LR_irq and save in FIQ stack*/
		sub		 lr, lr, #4
		stmfd    sp!, {lr}

    /*- Save r0 and SPSR in FIQ stack */
        mrs      r14, SPSR
        stmfd    sp!, {r0,r14}

    /* Read the AIC Fast Interrupt Vector register to clear the interrupt */
        ldr		 r14, =AT91C_BASE_AIC
        ldr      r0 , [r14, #0x00000104]
        str      r14, [r14, #0x00000104]

    /*- Save scratch/used registers and LR in FIQ Stack */
        stmfd    sp!, {r1-r7, r12, r14}

    /*- Branch to the routine pointed by the AIC_FVR */
        mov      r14, pc
        bx       r0

    /*- Restore scratch/used registers and LR from FIQ Stack */
        ldmia    sp!, {r1-r7, r12, r14}

    /*- Restore SPSR_irq and r0 from FIQ stack */
        ldmia    sp!, {r0,r14}
        msr      SPSR_cxsf, r14

    /*- Restore adjusted  LR_irq from FIQ stack */
        ldmia    sp!, {lr}

    /* Return from Fiq interrupt */
		movs	 pc, lr

/*------------------------------------------------------------------------------
 *- Function             : irq_handler
 *- Treatments           : IRQ Controller Interrupt Handler.
 *- Called Functions     : AIC_IVR[interrupt]
 *------------------------------------------------------------------------------*/
irq_handler:
/*- Manage Exception Entry */
/*- Adjust and save LR_irq in IRQ stack */
        sub      lr, lr, #4
        stmfd    sp!, {lr}
/*- Save r0 and SPSR in IRQ stack */
        mrs      r14, SPSR
        stmfd    sp!, {r0,r14}

/*- Write in the IVR to support Protect Mode */
/*- No effect in Normal Mode */
/*- De-assert the NIRQ and clear the source in Protect Mode */
        ldr      r14, =AT91C_BASE_AIC
        ldr      r0 , [r14, #0x00000100]
        str      r14, [r14, #0x00000100]

/*- Save scratch/used registers and LR in User Stack */
        stmfd    sp!, {r1-r3, r12, r14}

/*- Branch to the routine pointed by the AIC_IVR */
        mov      r14, pc
        bx       r0

/*- Restore scratch/used registers and LR from User Stack */
        ldmia    sp!, {r1-r3, r12, r14}

/*- Mark the End of Interrupt on the AIC */
        ldr      r14, =AT91C_BASE_AIC
        str      r14, [r14, #0x00000130]

/*- Restore SPSR_irq and r0 from IRQ stack */
        ldmia    sp!, {r0,r14}
        msr      SPSR_cxsf, r14

/*- Restore adjusted  LR_irq from IRQ stack directly in the PC */
        ldmia    sp!, {pc}^


swi_handler:
        b        swi_handler      /* Software Interrupt */

undf_handler:                     /* undefined command */
/*- Save r0-r12, lr and SPSR in stack */
        stmdb    sp!, {r0-r12, lr}
        mrs      r0, SPSR
        stmdb    sp!, {r0}

/* load handler pointer to handler register */
        ldr      r9, =custom_undf_handler

/* copy registers from stack to data structure */
        bl       reg_struct_fill

/*- Restore SPSR */
        ldmia    sp!, {r0}
        msr      SPSR, r0

/*- Restore r0-r12 and LR_abt from stack */
        ldmia    sp!, {r0-r12, lr}

/* Return from undefined command interrupt */
		movs	 pc, lr

pabt_handler:                       /* program abort */
/*- Save r0-r12, lr and SPSR in stack */
        stmdb    sp!, {r0-r12, lr}
        mrs      r0, SPSR
        stmdb    sp!, {r0}

/* load handler pointer to handler register */
        ldr      r9, =custom_pabt_handler

/* copy registers from stack to data structure */
        bl       reg_struct_fill

/*- Restore SPSR */
        ldmia    sp!, {r0}
        msr      SPSR, r0

/*- Restore r0-r12 and LR_abt from stack */
        ldmia    sp!, {r0-r12, lr}

/* Adjust LR and return from perfetch abort interrupt */
        sub      lr, lr, #4
		movs	 pc, lr

dabt_handler:                       /* data abort */
/*- Save r0-r12, lr and SPSR in stack */
        stmdb    sp!, {r0-r12, lr}
        mrs      r0, SPSR
        stmdb    sp!, {r0}

/* load handler pointer to handler register */
        ldr      r9, =custom_dabt_handler

/* copy registers from stack to data structure */
        bl       reg_struct_fill

/*- Restore SPSR */
        ldmia    sp!, {r0}
        msr      SPSR, r0

/*- Restore r0-r12 and LR_abt from stack */
        ldmia    sp!, {r0-r12, lr}

/* Adjust LR and return from perfetch abort interrupt */
        sub      lr, lr, #8
		movs	 pc, lr

reg_struct_fill:
/*	Save registers to halAvarRegState  */
        mov      r1, sp

/*  save lr to r4  */
        ldr      r4, [r1], #4
        mov      r3, #13

/*  load address structure to r5 */
        ldr      r5, =halAvarRegState

cycle_label:
/* load r0-r12 from stack stack to r2 */
        ldr      r2, [r1], #4

/* load r2 to halAvarRegState struct */
        str      r2, [r5], #4
        subs     r3, r3,   #1  /* r3-- */
        bne      cycle_label

/* save structure address for saving stack pointer */
        mov      r6, r5
        add      r5, r5,   #4  /* addr++ */

/* load link register from stack to r2 */
        ldr      r2, [r1], #4

/* load link register from r2 to struct*/
        str      r2, [r5], #4

/* load SPSR to struct */
        str      r4, [r5], #4

/* read stack pointer previous regim */
        mrs      r7, CPSR
        mrs      r8, SPSR

/* disable interrupt and turn off thumb */
        orr	     r8, r8, #(I_BIT | F_BIT)
        bic	     r8, r8, #THUMB_BIT
        msr      CPSR, r8
        mov      r0, sp
        msr      CPSR, r7

/* load stack pointer to struct */
        str      r0, [r6]

/*- Branch to the routine handler */
        mov      r14, pc
        bx       r9

/* return to exception handler */
        mov      pc, lr
