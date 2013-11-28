/**
 ******************************************************************************
 * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c
 * @author  MCD Application Team
 * @version V3.4.0
 * @date    10/15/2010
 * @brief   Main Interrupt Service Routines.
 *          This file provides template for all exceptions handler and
 *          peripherals interrupt service routine.
 ******************************************************************************
 * @copy
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
 */

/* Includes ------------------------------------------------------------------*/

#include "stdheader.h"
#include "stm32f2xx_it.h"

#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include "dcc_stdio.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------------*/
#define MemMangFaultAddressReg     ( ( volatile unsigned long *) 0xE000ED34 )
#define MemMangFaultReg            ( ( volatile unsigned long *) 0xE000ED28 )
#define MemManagFaultRegClearBit   (unsigned long) 0x00000080
#define IACCVIOL                   (unsigned long) 0x0000001
#define MMARVALID                  (unsigned long) 0x00000080
#define DACCVIOL                   (unsigned long) 0x0000002
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/
typedef struct
{
    uint32_t r0, r1, r2, r3, r12;
    uint32_t pc;
    uint32_t psr;
    uint32_t lr;
}SP_FRAME_T;

struct stackframe_s
{
    void * handler;
    uint32_t control;
    SP_FRAME_T msp;
    SP_FRAME_T psp;
} StackFrame;


/* Extern function prototypes ------------------------------------------------------*/


/* Extern variables -----------------------------------------------------------------*/
extern unsigned long __SRAM_segment_start__[];
extern unsigned long __SRAM_segment_end__[];




typedef struct tskTaskControlBlock
{
    volatile portSTACK_TYPE *pxTopOfStack;      /*< Points to the location of the last item placed on the tasks stack.  THIS MUST BE THE FIRST MEMBER OF THE STRUCT. */

    #if ( portUSING_MPU_WRAPPERS == 1 )
        xMPU_SETTINGS xMPUSettings;             /*< The MPU settings are defined as part of the port layer.  THIS MUST BE THE SECOND MEMBER OF THE STRUCT. */
    #endif

    xListItem               xGenericListItem;   /*< List item used to place the TCB in ready and blocked queues. */
    xListItem               xEventListItem;     /*< List item used to place the TCB in event lists. */
    unsigned portBASE_TYPE  uxPriority;         /*< The priority of the task where 0 is the lowest priority. */
    portSTACK_TYPE          *pxStack;           /*< Points to the start of the stack. */
    signed char             pcTaskName[ configMAX_TASK_NAME_LEN ];/*< Descriptive name given to the task when created.  Facilitates debugging only. */

    #if ( portSTACK_GROWTH > 0 )
        portSTACK_TYPE *pxEndOfStack;           /*< Used for stack overflow checking on architectures where the stack grows up from low memory. */
    #endif

    #if ( portCRITICAL_NESTING_IN_TCB == 1 )
        unsigned portBASE_TYPE uxCriticalNesting;
    #endif

    #if ( configUSE_TRACE_FACILITY == 1 )
        unsigned portBASE_TYPE  uxTCBNumber;    /*< This is used for tracing the scheduler and making debugging easier only. */
    #endif

    #if ( configUSE_MUTEXES == 1 )
        unsigned portBASE_TYPE uxBasePriority;  /*< The priority last assigned to the task - used by the priority inheritance mechanism. */
    #endif

    #if ( configUSE_APPLICATION_TASK_TAG == 1 )
        pdTASK_HOOK_CODE pxTaskTag;
    #endif

    #if ( configGENERATE_RUN_TIME_STATS == 1 )
        unsigned long ulRunTimeCounter;     /*< Used for calculating how much CPU time each task is utilising. */
    #endif

} tskTCB;
extern tskTCB * volatile pxCurrentTCB;

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
 * @brief  This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void)
{
    dbg_write_str("NMI_Handler");
}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
void HardFault_Tail(uint32_t * MSP, uint32_t * PSP, uint32_t CONTROL)
{
    dbg_write_str("HardFault_Handler");

    portTickType ticks = xTaskGetTickCountFromISR();
    dbg_write_str("TickCount:");
    dbg_write_u32(((uint32_t*)&ticks), 2);

    //IWDG_ReloadCounter();

    dbg_write_str("Stack Trace:");

    StackFrame.handler = HardFault_Handler;
    StackFrame.control = CONTROL;

    if (MSP > __SRAM_segment_start__ && MSP <= (__SRAM_segment_end__ - (8 * 4)))
    {
        StackFrame.msp.r0  = MSP[0];
        StackFrame.msp.r1  = MSP[1];
        StackFrame.msp.r2  = MSP[2];
        StackFrame.msp.r3  = MSP[3];
        StackFrame.msp.r12 = MSP[4];
        StackFrame.msp.lr  = MSP[5];
        StackFrame.msp.pc  = MSP[6];
        StackFrame.msp.psr = MSP[7];

        dbg_write_str("MSP:");
        dbg_write_str("R0:      R1:      R2:      R3:      R12:     LR:      PC:      PSR:");
        dbg_write_u32(MSP, 8);
    }

    if (PSP > __SRAM_segment_start__ && PSP <= (__SRAM_segment_end__ - (8 * 4)))
    {
        StackFrame.psp.r0  = PSP[0];
        StackFrame.psp.r1  = PSP[1];
        StackFrame.psp.r2  = PSP[2];
        StackFrame.psp.r3  = PSP[3];
        StackFrame.psp.r12 = PSP[4];
        StackFrame.psp.lr  = PSP[5];
        StackFrame.psp.pc  = PSP[6];
        StackFrame.psp.psr = PSP[7];

        dbg_write_str("PSP:");
        dbg_write_str("R0:      R1:      R2:      R3:      R12:     LR:      PC:      PSR:");
        dbg_write_u32(PSP, 8);
    }
}

void HardFault_Handler(void)
{
    HardFault_Tail((uint32_t*)__get_MSP(),(uint32_t*) __get_PSP(), __get_CONTROL());

    while (1)
    {
    }
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Tail(uint32_t MSP)
{
    vu32 flagstatus;
    char *TaskName;
    vu32 mmareg;

    IWDG_ReloadCounter();
#if 0
    uint32_t *pMSP = (uint32_t *)MSP;

    StackFrame.handler = MemManage_Handler;
    StackFrame.r0  = pMSP[0];
    StackFrame.r1  = pMSP[1];
    StackFrame.r2  = pMSP[2];
    StackFrame.r3  = pMSP[3];
    StackFrame.r12 = pMSP[4];
    StackFrame.lr  = pMSP[5];
    StackFrame.pc  = pMSP[6];
    StackFrame.psr = pMSP[7];

    dbg_write_str("MemManage_Handler");

    portTickType ticks;
    ticks = xTaskGetTickCountFromISR();
    dbg_write_str("TickCount:");
    dbg_write_u32(((uint32_t*)&ticks), 2);

    dbg_write_str("Stack Trace:");
    dbg_write_str("MSP:");
    dbg_write_str("R0:      R1:      R2:      R3:      R12:     LR:      PC:      PSR:");
    dbg_write_u32(((uint32_t*)MSP), 8);
#endif
    /*read the memory management fault address register to get the address of the offending code*/
    mmareg = *MemMangFaultAddressReg;

    /*read the Memory Management Fault Register*/
    flagstatus = *MemMangFaultReg;
    /*Clear the Memory Management Fault Register to not confuse fault handler in the next interrupt*/
    *MemMangFaultReg |= MemManagFaultRegClearBit;

    /*Get the name of the running task*/
    TaskName = (char*)pxCurrentTCB->pcTaskName;

    dbg_write_str("MMgfault in:");
    dbg_write_str(TaskName);

    /*Get the mode of the task generating the fault*/
    dbg_write_str("Actual mode is:");
    if (__get_CONTROL() == 1)
        dbg_write_str("user");
    else
        dbg_write_str("privileged");

    /*Check the cause of the fault by testing the fault status register bits*/
    /*check the IACCVIOL to see if it is an instruction violation*/
    if ((flagstatus & IACCVIOL) != ((u32) 0))
        dbg_write_str("Instruction violation @");

    /*check the DACCVIOL to see if it is a data access violation*/
    else if ((flagstatus & DACCVIOL) != (u32) 0)
        dbg_write_str("data access violation @");

    /*if the MMARVALID is set so we can get the address of the offending code*/
    if ((flagstatus & MMARVALID) != (u32) 0)
        dbg_write_u32((const long unsigned int*)&mmareg, 1);
}

void MemManage_Handler(void)
{
    MemManage_Tail(__get_MSP());

    while (1)
    {
    }
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Tail(uint32_t MSP)
{
    portTickType ticks;
    ticks = xTaskGetTickCountFromISR();

    IWDG_ReloadCounter();
#if 0
    uint32_t *pMSP = (uint32_t *)MSP;

    StackFrame.handler = BusFault_Handler;
    StackFrame.r0  = pMSP[0];
    StackFrame.r1  = pMSP[1];
    StackFrame.r2  = pMSP[2];
    StackFrame.r3  = pMSP[3];
    StackFrame.r12 = pMSP[4];
    StackFrame.lr  = pMSP[5];
    StackFrame.pc  = pMSP[6];
    StackFrame.psr = pMSP[7];
#endif
    dbg_write_str("BusFault_Handler");
    dbg_write_str("TickCount:");
    dbg_write_u32(((uint32_t*)&ticks), 2);

    dbg_write_str("Stack Trace:");
    dbg_write_str("MSP:");
    dbg_write_str("R0:      R1:      R2:      R3:      R12:     LR:      PC:      PSR:");
    dbg_write_u32(((uint32_t*)MSP), 8);

}

void BusFault_Handler(void)
{
    BusFault_Tail(__get_MSP());
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Tail(uint32_t MSP)
{
    portTickType ticks;
    ticks = xTaskGetTickCountFromISR();

    IWDG_ReloadCounter();

#if 0
    uint32_t *pMSP = (uint32_t *)MSP;

    StackFrame.handler = UsageFault_Handler;
    StackFrame.r0  = pMSP[0];
    StackFrame.r1  = pMSP[1];
    StackFrame.r2  = pMSP[2];
    StackFrame.r3  = pMSP[3];
    StackFrame.r12 = pMSP[4];
    StackFrame.lr  = pMSP[5];
    StackFrame.pc  = pMSP[6];
    StackFrame.psr = pMSP[7];
#endif
    dbg_write_str("UsageFault_Handler");
    dbg_write_str("TickCount:");
    dbg_write_u32(((uint32_t*)&ticks), 2);

    dbg_write_str("Stack Trace:");
    dbg_write_str("MSP:");
    dbg_write_str("R0:      R1:      R2:      R3:      R12:     LR:      PC:      PSR:");
    dbg_write_u32(((uint32_t*)MSP), 8);
}

void UsageFault_Handler(void)
{
    UsageFault_Tail(__get_MSP());
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles SVCall exception.
 * @param  None
 * @retval None
 */
/*
 * Used by FreeRTOS
 *
 void SVC_Handler(void)
 {
 }
 */

/**
 * @brief  This function handles Debug Monitor exception.
 * @param  None
 * @retval None
 */
void DebugMon_Handler(void)
{
}

/**
 * @brief  This function handles PendSV_Handler exception.
 * @param  None
 * @retval None
 */
/*
 * Used by FreeRTOS
 *
 void PendSV_Handler(void)
 {
 //Call the FreeRTOS function here
 }
 */

/**
 * @brief  This function handles SysTick Handler.
 * @param  None
 * @retval None
 */
/*
 * Used by FreeRTOS
 *
 void SysTick_Handler(void)
 {
 //Call the FreeRTOS function here
 }
 */

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
 * @brief  This function handles PPP interrupt request.
 * @param  None
 * @retval None
 */
/*void PPP_IRQHandler(void)
 {
 }*/

/**
 * @}
 */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
