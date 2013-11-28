/*
 * system_init.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 02.07.2010
 *      Author: DHA
 *
 * Purpose:
 * Initialize system components at startup.
 *
 */

#ifndef INIT_H_
#define INIT_H_

#include "memtest.h"

#define DRIVER_TASK_TAG            10
#define TIMER_TASK_TAG             30
#define SEQUENCE_TASK_TAG          200

// Define the constants used to allocate the task stacks. Note that stack size is defined in words, not bytes.
#define DRIVER_TASK_STACK_SIZE_WORDS   		    512
#define DRIVER_TASK_STACK_SIZE_BYTES   		    (DRIVER_TASK_STACK_SIZE_WORDS * sizeof(portSTACK_TYPE ))
#define TIMER_TASK_STACK_SIZE_WORDS             512
#define TIMER_TASK_STACK_SIZE_BYTES             (TIMER_TASK_STACK_SIZE_WORDS * sizeof(portSTACK_TYPE ))
#define SEQUENCE_TASK_STACK_SIZE_WORDS   	    512
#define SEQUENCE_TASK_STACK_SIZE_BYTES   	    (SEQUENCE_TASK_STACK_SIZE_WORDS * sizeof(portSTACK_TYPE ))

#define TASK_STACK_SECTION_SIZE_WORDS	(32768U / 4)  // 32k, _Task_Stack_Section_Size

#define DRIVER_TaskStack		    TaskStack
#define TIMER_TaskStack			    DRIVER_TaskStack + DRIVER_TASK_STACK_SIZE_WORDS
#define SEQUENCE_TaskStack		    TIMER_TaskStack + TIMER_TASK_STACK_SIZE_WORDS

#define INIT_RAM_SIZE_96K 0x18000
#define TASK_STACK_SECTION __attribute__((section("task_stack_section")))

extern MEMTEST_RESULT_T ext_fram_test_result;
extern MEMTEST_RESULT_T ext_flash_test_result;
extern xSemaphoreHandle	xInitTaskResetWaitMutex;

void INIT_Task(void *pvParameters);

#endif /* SYSTEM_INIT_H_ */
