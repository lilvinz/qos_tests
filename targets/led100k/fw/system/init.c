/*
 * system_init.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 02.07.2010
 *      Author: CBA
 *
 * Purpose:
 * Initialize system components and start tasks at startup.
 *
 */

#include "stdheader.h"
#include "init.h"

#include "driver.h"
#include "dim.h"
#include "sequence.h"
#include "detector.h"
#include "timer.h"
#include "rtc.h"
#include "status_led.h"
#include "debug.h"
#include "backup_register_map.h"
#include "system.h"
#include "driver.h"
#include "debug_freertos.h"



const uint64_t fram_init_value = 0x5155414E5445433F;

extern unsigned long __privileged_data_start__[];
extern unsigned long _Privileged_Data_Region_Size[];
extern unsigned long __task_stack_section_start__[];
extern unsigned long __task_stack_section_end__[];
extern unsigned long _Task_Stack_Section_Size[];

/* Declare the stack which will be used by the task.
The stack alignment must match its size, so if 128 words are reserved
for the stack then it must be aligned to ( 128 * 4 ) bytes. */
// Attention: Sort stacks by size (greatest first)
TASK_STACK_SECTION portSTACK_TYPE TaskStack[TASK_STACK_SECTION_SIZE_WORDS];

const xTaskParameters DRIVER_TaskParameters =
{
	DRIVER_Task,                 		/* pvTaskCode - the function that implements the task. */
    (signed portCHAR*)"Driver IO     ",	/* pcName       */
    DRIVER_TASK_STACK_SIZE_WORDS,   	/* usStackDepth */
    NULL,                               /* pvParameters */
    tskIDLE_PRIORITY, 					/* uxPriority */
    DRIVER_TaskStack,                 	/* puxStackBuffer - the array to use as the task stack, as declared above. */
    {                                   /* xRegions - this task does not use any non-stack data hence all members are zero. */
        /* Base address     Length      Parameters */
        { (void*) __privileged_data_start__, (unsigned long) _Privileged_Data_Region_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { (void*) __task_stack_section_start__, (unsigned long) _Task_Stack_Section_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { DRIVER_TaskStack, DRIVER_TASK_STACK_SIZE_BYTES, portMPU_REGION_READ_WRITE }
    }
};


const xTaskParameters TIMER_TaskParameters =
{
	TIMER_Task,                    		/* pvTaskCode - the function that implements the task. */
    (signed portCHAR*)"Timer         ",	/* pcName       */
    TIMER_TASK_STACK_SIZE_WORDS,   		/* usStackDepth */
    NULL,                               /* pvParameters */
    tskIDLE_PRIORITY,					/* uxPriority */
    TIMER_TaskStack,                 	/* puxStackBuffer - the array to use as the task stack, as declared above. */
    {                                   /* xRegions - this task does not use any non-stack data hence all members are zero. */
        /* Base address     Length      Parameters */
        { (void*) __privileged_data_start__, (unsigned long) _Privileged_Data_Region_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { (void*) __task_stack_section_start__, (unsigned long) _Task_Stack_Section_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { TIMER_TaskStack, TIMER_TASK_STACK_SIZE_BYTES, portMPU_REGION_READ_WRITE }
    }
};

const xTaskParameters SEQUENCE_TaskParameters =
{
	SEQUENCE_Task,                    	/* pvTaskCode - the function that implements the task. */
    (signed portCHAR*)"Sequence      ",	/* pcName       */
    SEQUENCE_TASK_STACK_SIZE_WORDS,   	/* usStackDepth */
    NULL,                               /* pvParameters */
    tskIDLE_PRIORITY + 2,   			/* uxPriority */
    SEQUENCE_TaskStack,                 /* puxStackBuffer - the array to use as the task stack, as declared above. */
    {                                   /* xRegions - this task does not use any non-stack data hence all members are zero. */
        /* Base address     Length      Parameters */
        { (void*) __privileged_data_start__, (unsigned long) _Privileged_Data_Region_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { (void*) __task_stack_section_start__, (unsigned long) _Task_Stack_Section_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { SEQUENCE_TaskStack, SEQUENCE_TASK_STACK_SIZE_BYTES, portMPU_REGION_READ_WRITE }
    }
};



xSemaphoreHandle	xInitTaskResetWaitMutex;


/**
  * @brief  Initialize subsystems and start all tasks.
  * @param  None
  * @retval None
  */
void INIT_Task(void *pvParameters)
{
    xTaskHandle vCreatedTaskHandle;

    xInitTaskResetWaitMutex	= xSemaphoreCreateMutex();
    DQueueAddToRegistry(xInitTaskResetWaitMutex, "xInitTaskResetWaitMutex");

    assert_param(xInitTaskResetWaitMutex != NULL);

    xSemaphoreTake(xInitTaskResetWaitMutex, portMAX_DELAY);

#if (configUSE_APPLICATION_TASK_TAG == 1)
    /* This task is going to be represented by a voltage scale of 1. */
    vTaskSetApplicationTaskTag(NULL, (void*)1);
#define TAG_TASK(x,y) vTaskSetApplicationTaskTag(x, (void*)y);
#else
#define TAG_TASK(x,y)
#endif


    /************************************************************************************/
    /* Init Modules                                                                     */
    /************************************************************************************/

    // Initialize Hardware and Peripherals
    DRIVER_Init();
    DEBUG_Init();

    vTaskDelay(100);

    VERSION_Init();

    STATUS_LED_Set(LED_GREEN, LED_STATE_BLINK_FAST, LED_NO_TIMEOUT);


    /************************************************************************************/
    /* Startup sequence                                                                 */
    /************************************************************************************/
    xTaskCreateRestricted(&DRIVER_TaskParameters, &vCreatedTaskHandle);
    TAG_TASK(vCreatedTaskHandle, DRIVER_TASK_TAG);
    xTaskCreateRestricted(&TIMER_TaskParameters, &vCreatedTaskHandle);
    TAG_TASK(vCreatedTaskHandle, TIMER_TASK_TAG);
    vTaskDelay(100);

    STATUS_LED_Set(LED_GREEN, LED_STATE_BLINK_FAST, LED_NO_TIMEOUT);

    vTaskDelay(100);

    xTaskCreateRestricted(&SEQUENCE_TaskParameters, &vCreatedTaskHandle);
    TAG_TASK(vCreatedTaskHandle, SEQUENCE_TASK_TAG);

    STATUS_LED_Set(LED_GREEN, LED_STATE_OFF, LED_NO_TIMEOUT);
    STATUS_LED_Set(LED_RED, LED_STATE_OFF, LED_NO_TIMEOUT);

    while(1)
    {
    	// wait for reset command
    	if (pdTRUE == xSemaphoreTake(xInitTaskResetWaitMutex, portMAX_DELAY))
    		SCB->AIRCR = 0x05fa0004;  // perform system reset
    }
}
