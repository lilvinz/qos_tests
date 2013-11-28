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
#define ALARM_CLOCK_TASK_TAG       20
#define TIMER_TASK_TAG             30
#define DETECTOR_TASK_TAG          40
#define INDICATION_TASK_TAG        50
#define SVINFO_MANAGER_TASK_TAG    60
#define SOUND_TASK_TAG             70
#define SENSORS_TASK_TAG           80
#define GPSSYNC_TASK_TAG           90
#define GLOOM_MANAGER_TASK_TAG     150
#define LOG_TASK_TAG               160
#define MESHCOM_TASK_TAG            170
#define SYNDICATION_TASK_TAG       180
#define DIM_TASK_TAG               190
#define SEQUENCE_TASK_TAG          200
#define CLI_TASK_TAG               210
#define CAN_MANAGER_TASK_TAG       220




// Define the constants used to allocate the task stacks. Note that stack size is defined in words, not bytes.
#define CLI_TASK_STACK_SIZE_WORDS               2048
#define CLI_TASK_STACK_SIZE_BYTES               (CLI_TASK_STACK_SIZE_WORDS * sizeof(portSTACK_TYPE ))
#define ALARM_CLOCK_TASK_STACK_SIZE_WORDS       512
#define ALARM_CLOCK_TASK_STACK_SIZE_BYTES       (ALARM_CLOCK_TASK_STACK_SIZE_WORDS * sizeof(portSTACK_TYPE ))
#define SENSORS_TASK_STACK_SIZE_WORDS           512
#define SENSORS_TASK_STACK_SIZE_BYTES           (SENSORS_TASK_STACK_SIZE_WORDS * sizeof(portSTACK_TYPE ))
#define GLOOM_MANAGER_TASK_STACK_SIZE_WORDS     512
#define GLOOM_MANAGER_TASK_STACK_SIZE_BYTES     (GLOOM_MANAGER_TASK_STACK_SIZE_WORDS * sizeof(portSTACK_TYPE ))
#define MESHCOM_TASK_STACK_SIZE_WORDS               512
#define MESHCOM_TASK_STACK_SIZE_BYTES               (MESHCOM_TASK_STACK_SIZE_WORDS * sizeof(portSTACK_TYPE ))
#define DETECTOR_TASK_STACK_SIZE_WORDS          256
#define DETECTOR_TASK_STACK_SIZE_BYTES          (DETECTOR_TASK_STACK_SIZE_WORDS * sizeof(portSTACK_TYPE ))
#define INDICATION_TASK_STACK_SIZE_WORDS        256
#define INDICATION_TASK_STACK_SIZE_BYTES        (INDICATION_TASK_STACK_SIZE_WORDS * sizeof(portSTACK_TYPE ))
#define DRIVER_TASK_STACK_SIZE_WORDS            256
#define DRIVER_TASK_STACK_SIZE_BYTES            (DRIVER_TASK_STACK_SIZE_WORDS * sizeof(portSTACK_TYPE ))
#define TIMER_TASK_STACK_SIZE_WORDS             256
#define TIMER_TASK_STACK_SIZE_BYTES             (TIMER_TASK_STACK_SIZE_WORDS * sizeof(portSTACK_TYPE ))
#define SOUND_TASK_STACK_SIZE_WORDS             256
#define SOUND_TASK_STACK_SIZE_BYTES             (SOUND_TASK_STACK_SIZE_WORDS * sizeof(portSTACK_TYPE ))
#define GPSSYNC_TASK_STACK_SIZE_WORDS           256
#define GPSSYNC_TASK_STACK_SIZE_BYTES           (GPSSYNC_TASK_STACK_SIZE_WORDS * sizeof(portSTACK_TYPE ))
#define SYNDICATION_TASK_STACK_SIZE_WORDS       256
#define SYNDICATION_TASK_STACK_SIZE_BYTES       (SYNDICATION_TASK_STACK_SIZE_WORDS * sizeof(portSTACK_TYPE ))
#define DIM_TASK_STACK_SIZE_WORDS               256
#define DIM_TASK_STACK_SIZE_BYTES               (DIM_TASK_STACK_SIZE_WORDS * sizeof(portSTACK_TYPE ))
#define SEQUENCE_TASK_STACK_SIZE_WORDS          256
#define SEQUENCE_TASK_STACK_SIZE_BYTES          (SEQUENCE_TASK_STACK_SIZE_WORDS * sizeof(portSTACK_TYPE ))
#define LOG_TASK_STACK_SIZE_WORDS               256
#define LOG_TASK_STACK_SIZE_BYTES               (LOG_TASK_STACK_SIZE_WORDS * sizeof(portSTACK_TYPE ))
#define SVINFO_MANAGER_TASK_STACK_SIZE_WORDS    256
#define SVINFO_MANAGER_TASK_STACK_SIZE_BYTES    (SVINFO_MANAGER_TASK_STACK_SIZE_WORDS * sizeof(portSTACK_TYPE ))
#define CAN_MANAGER_TASK_STACK_SIZE_WORDS       256
#define CAN_MANAGER_TASK_STACK_SIZE_BYTES       (SVINFO_MANAGER_TASK_STACK_SIZE_WORDS * sizeof(portSTACK_TYPE ))



#define TASK_STACK_SECTION_SIZE_WORDS   (32768U / 4)  // 32k, _Task_Stack_Section_Size

#define CLI_TaskStack               TaskStack
#define ALARM_CLOCK_TaskStack       CLI_TaskStack + CLI_TASK_STACK_SIZE_WORDS
#define SENSORS_TaskStack           ALARM_CLOCK_TaskStack + ALARM_CLOCK_TASK_STACK_SIZE_WORDS
#define GLOOM_MANAGER_TaskStack     SENSORS_TaskStack + SENSORS_TASK_STACK_SIZE_WORDS
#define MESHCOM_TaskStack           GLOOM_MANAGER_TaskStack + GLOOM_MANAGER_TASK_STACK_SIZE_WORDS
#define DETECTOR_TaskStack          MESHCOM_TaskStack + MESHCOM_TASK_STACK_SIZE_WORDS
#define INDICATION_TaskStack        DETECTOR_TaskStack + DETECTOR_TASK_STACK_SIZE_WORDS
#define DRIVER_TaskStack            INDICATION_TaskStack + INDICATION_TASK_STACK_SIZE_WORDS
#define TIMER_TaskStack             DRIVER_TaskStack + DRIVER_TASK_STACK_SIZE_WORDS
#define SOUND_TaskStack             TIMER_TaskStack + TIMER_TASK_STACK_SIZE_WORDS
#define GPSSYNC_TaskStack           SOUND_TaskStack + SOUND_TASK_STACK_SIZE_WORDS
#define SYNDICATION_TaskStack       GPSSYNC_TaskStack + GPSSYNC_TASK_STACK_SIZE_WORDS
#define DIM_TaskStack               SYNDICATION_TaskStack + SYNDICATION_TASK_STACK_SIZE_WORDS
#define SEQUENCE_TaskStack          DIM_TaskStack + DIM_TASK_STACK_SIZE_WORDS
#define LOG_TaskStack               SEQUENCE_TaskStack + SEQUENCE_TASK_STACK_SIZE_WORDS
#define SVINFO_MANAGER_TaskStack    LOG_TaskStack + LOG_TASK_STACK_SIZE_WORDS
#define CAN_MANAGER_TaskStack       SVINFO_MANAGER_TaskStack + SVINFO_MANAGER_TASK_STACK_SIZE_WORDS

#define INIT_RAM_SIZE_96K 0x18000
#define TASK_STACK_SECTION __attribute__((section("task_stack_section")))

extern MEMTEST_RESULT_T ext_fram_test_result;
extern MEMTEST_RESULT_T ext_flash_test_result;
extern xSemaphoreHandle xInitTaskResetWaitMutex;

void INIT_Task(void *pvParameters);

#endif /* SYSTEM_INIT_H_ */
