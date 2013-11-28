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

#include "spi_master.h"
#include "spi_serial_flash.h"
#include "spi_serial_fram.h"
#include "driver.h"
#include "fram_mirror.h"
#include "fram_memory_map.h"
#include "settings.h"
#include "dim.h"
#include "sequence.h"
#include "bmm.h"
#include "bmm_profiles.h"
#include "sensors.h"
#include "gloom_manager.h"
#include "log.h"
#include "svinfo_manager.h"
#include "terminal_server.h"
#include "cli.h"
#include "gpssync.h"
#include "settings.h"
#include "detector.h"
#include "indication.h"
#include "modbus_master.h"
#include "meshcom_conn.h"
#include "timer.h"
#include "rtc.h"
#include "hw_clock.h"
#include "status_led.h"
#include "debug.h"
#include "syndication.h"
#include "alarm_clock.h"
#include "backup_register_map.h"
#include "system.h"
#include "random.h"
#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "debug_freertos.h"
#include "can_manager.h"
#include "primestruct.h"
#include "psu_monitoring.h"
#include "aguard.h"

#include "sound.h"

static const uint64_t fram_init_value = 0x5155414E5445433F;

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

const xTaskParameters AlarmClockTaskParameters =
{
    ALARM_CLOCK_Task,                   /* pvTaskCode - the function that implements the task. */
    (signed portCHAR*)"Alarm clock   ", /* pcName       */
    ALARM_CLOCK_TASK_STACK_SIZE_WORDS,  /* usStackDepth */
    NULL,                               /* pvParameters */
    tskIDLE_PRIORITY,                   /* uxPriority */
    ALARM_CLOCK_TaskStack,              /* puxStackBuffer - the array to use as the task stack, as declared above. */
    {   /* xRegions - this task does not use any non-stack data hence all members are zero. */
        /* Base address     Length      Parameters */
        { (void*) __privileged_data_start__, (unsigned long) _Privileged_Data_Region_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { (void*) __task_stack_section_start__, (unsigned long) _Task_Stack_Section_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { ALARM_CLOCK_TaskStack, ALARM_CLOCK_TASK_STACK_SIZE_BYTES, portMPU_REGION_READ_WRITE }
    }
};

const xTaskParameters DETECTOR_TaskParameters =
{
    DETECTOR_Task,                      /* pvTaskCode - the function that implements the task. */
    (signed portCHAR*)"Detector      ", /* pcName       */
    DETECTOR_TASK_STACK_SIZE_WORDS,     /* usStackDepth */
    NULL,                               /* pvParameters */
    tskIDLE_PRIORITY,                   /* uxPriority */
    DETECTOR_TaskStack,                 /* puxStackBuffer - the array to use as the task stack, as declared above. */
    {   /* xRegions - this task does not use any non-stack data hence all members are zero. */
        /* Base address     Length      Parameters */
        { (void*) __privileged_data_start__, (unsigned long) _Privileged_Data_Region_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { (void*) __task_stack_section_start__, (unsigned long) _Task_Stack_Section_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { DETECTOR_TaskStack, DETECTOR_TASK_STACK_SIZE_BYTES, portMPU_REGION_READ_WRITE }
    }
};

const xTaskParameters INDICATION_TaskParameters =
{
    INDICATION_Task,                    /* pvTaskCode - the function that implements the task. */
    (signed portCHAR*)"Indication    ", /* pcName       */
    INDICATION_TASK_STACK_SIZE_WORDS,   /* usStackDepth */
    NULL,                               /* pvParameters */
    tskIDLE_PRIORITY,                   /* uxPriority */
    INDICATION_TaskStack,               /* puxStackBuffer - the array to use as the task stack, as declared above. */
    {   /* xRegions - this task does not use any non-stack data hence all members are zero. */
        /* Base address     Length      Parameters */
        { (void*) __privileged_data_start__, (unsigned long) _Privileged_Data_Region_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { (void*) __task_stack_section_start__, (unsigned long) _Task_Stack_Section_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { INDICATION_TaskStack, INDICATION_TASK_STACK_SIZE_BYTES, portMPU_REGION_READ_WRITE }
    }
};

const xTaskParameters CLI_TaskParameters =
{
    CLI_Task,                           /* pvTaskCode - the function that implements the task. */
    (signed portCHAR*)"Shell         ", /* pcName       */
    CLI_TASK_STACK_SIZE_WORDS,          /* usStackDepth */
    NULL,                               /* pvParameters */
    tskIDLE_PRIORITY,                   /* uxPriority */
    CLI_TaskStack,                      /* puxStackBuffer - the array to use as the task stack, as declared above. */
    {   /* xRegions - this task does not use any non-stack data hence all members are zero. */
        /* Base address     Length      Parameters */
        { (void*) __privileged_data_start__, (unsigned long) _Privileged_Data_Region_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { (void*) __task_stack_section_start__, (unsigned long) _Task_Stack_Section_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { CLI_TaskStack, CLI_TASK_STACK_SIZE_BYTES, portMPU_REGION_READ_WRITE }
    }
};

const xTaskParameters DRIVER_TaskParameters =
{
    DRIVER_Task,                        /* pvTaskCode - the function that implements the task. */
    (signed portCHAR*)"Driver IO     ", /* pcName       */
    DRIVER_TASK_STACK_SIZE_WORDS,       /* usStackDepth */
    NULL,                               /* pvParameters */
    tskIDLE_PRIORITY,                   /* uxPriority */
    DRIVER_TaskStack,                   /* puxStackBuffer - the array to use as the task stack, as declared above. */
    {   /* xRegions - this task does not use any non-stack data hence all members are zero. */
        /* Base address     Length      Parameters */
        { (void*) __privileged_data_start__, (unsigned long) _Privileged_Data_Region_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { (void*) __task_stack_section_start__, (unsigned long) _Task_Stack_Section_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { DRIVER_TaskStack, DRIVER_TASK_STACK_SIZE_BYTES, portMPU_REGION_READ_WRITE }
    }
};

const xTaskParameters SOUND_TaskParameters =
{
    SOUND_Task,                         /* pvTaskCode - the function that implements the task. */
    (signed portCHAR*)"Sound         ", /* pcName       */
    SOUND_TASK_STACK_SIZE_WORDS,        /* usStackDepth */
    NULL,                               /* pvParameters */
    tskIDLE_PRIORITY,                   /* uxPriority */
    SOUND_TaskStack,                    /* puxStackBuffer - the array to use as the task stack, as declared above. */
    {   /* xRegions - this task does not use any non-stack data hence all members are zero. */
        /* Base address     Length      Parameters */
        { (void*) __privileged_data_start__, (unsigned long) _Privileged_Data_Region_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { (void*) __task_stack_section_start__, (unsigned long) _Task_Stack_Section_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { SOUND_TaskStack, SOUND_TASK_STACK_SIZE_BYTES, portMPU_REGION_READ_WRITE }
    }
};


const xTaskParameters TIMER_TaskParameters =
{
    TIMER_Task,                         /* pvTaskCode - the function that implements the task. */
    (signed portCHAR*)"Timer         ", /* pcName       */
    TIMER_TASK_STACK_SIZE_WORDS,        /* usStackDepth */
    NULL,                               /* pvParameters */
    tskIDLE_PRIORITY,                   /* uxPriority */
    TIMER_TaskStack,                    /* puxStackBuffer - the array to use as the task stack, as declared above. */
    {   /* xRegions - this task does not use any non-stack data hence all members are zero. */
        /* Base address     Length      Parameters */
        { (void*) __privileged_data_start__, (unsigned long) _Privileged_Data_Region_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { (void*) __task_stack_section_start__, (unsigned long) _Task_Stack_Section_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { TIMER_TaskStack, TIMER_TASK_STACK_SIZE_BYTES, portMPU_REGION_READ_WRITE }
    }
};

const xTaskParameters SENSORS_TaskParameters =
{
    SENSORS_Task,                       /* pvTaskCode - the function that implements the task. */
    (signed portCHAR*)"Sensors       ", /* pcName       */
    SENSORS_TASK_STACK_SIZE_WORDS,      /* usStackDepth */
    NULL,                               /* pvParameters */
    tskIDLE_PRIORITY,                   /* uxPriority */
    SENSORS_TaskStack,                  /* puxStackBuffer - the array to use as the task stack, as declared above. */
    {   /* xRegions - this task does not use any non-stack data hence all members are zero. */
        /* Base address     Length      Parameters */
        { (void*) __privileged_data_start__, (unsigned long) _Privileged_Data_Region_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { (void*) __task_stack_section_start__, (unsigned long) _Task_Stack_Section_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { SENSORS_TaskStack, SENSORS_TASK_STACK_SIZE_BYTES, portMPU_REGION_READ_WRITE }
    }
};

const xTaskParameters GPSSYNC_TaskParameters =
{
    GPSSYNC_Task,                       /* pvTaskCode - the function that implements the task. */
    (signed portCHAR*)"GPSSync       ", /* pcName       */
    GPSSYNC_TASK_STACK_SIZE_WORDS,      /* usStackDepth */
    NULL,                               /* pvParameters */
    tskIDLE_PRIORITY,                   /* uxPriority */
    GPSSYNC_TaskStack,                  /* puxStackBuffer - the array to use as the task stack, as declared above. */
    {   /* xRegions - this task does not use any non-stack data hence all members are zero. */
        /* Base address     Length      Parameters */
        { (void*) __privileged_data_start__, (unsigned long) _Privileged_Data_Region_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { (void*) __task_stack_section_start__, (unsigned long) _Task_Stack_Section_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { GPSSYNC_TaskStack, GPSSYNC_TASK_STACK_SIZE_BYTES, portMPU_REGION_READ_WRITE }
    }
};

const xTaskParameters GLOOM_MANAGER_TaskParameters =
{
    GLOOM_MANAGER_Task,                     /* pvTaskCode - the function that implements the task. */
    (signed portCHAR*)"Gloom Manager ",     /* pcName       */
    GLOOM_MANAGER_TASK_STACK_SIZE_WORDS,    /* usStackDepth */
    NULL,                                   /* pvParameters */
    tskIDLE_PRIORITY,                       /* uxPriority */
    GLOOM_MANAGER_TaskStack,                /* puxStackBuffer - the array to use as the task stack, as declared above. */
    {   /* xRegions - this task does not use any non-stack data hence all members are zero. */
        /* Base address     Length      Parameters */
        { (void*) __privileged_data_start__, (unsigned long) _Privileged_Data_Region_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { (void*) __task_stack_section_start__, (unsigned long) _Task_Stack_Section_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { GLOOM_MANAGER_TaskStack, GLOOM_MANAGER_TASK_STACK_SIZE_BYTES, portMPU_REGION_READ_WRITE }
    }
};

const xTaskParameters MESHCOM_TaskParameters =
{
    MESHCOM_Task,                       /* pvTaskCode - the function that implements the task. */
    (signed portCHAR*)"Mesh        ",   /* pcName       */
    MESHCOM_TASK_STACK_SIZE_WORDS,      /* usStackDepth */
    NULL,                               /* pvParameters */
    tskIDLE_PRIORITY,                   /* uxPriority */
    MESHCOM_TaskStack,                  /* puxStackBuffer - the array to use as the task stack, as declared above. */
    {   /* xRegions - this task does not use any non-stack data hence all members are zero. */
        /* Base address     Length      Parameters */
        { (void*) __privileged_data_start__, (unsigned long) _Privileged_Data_Region_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { (void*) __task_stack_section_start__, (unsigned long) _Task_Stack_Section_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { MESHCOM_TaskStack, MESHCOM_TASK_STACK_SIZE_BYTES, portMPU_REGION_READ_WRITE }
    }
};

const xTaskParameters SYNDICATION_TaskParameters =
{
    SYNDICATION_Task,                   /* pvTaskCode - the function that implements the task. */
    (signed portCHAR*)"Syndication   ", /* pcName       */
    SYNDICATION_TASK_STACK_SIZE_WORDS,  /* usStackDepth */
    NULL,                               /* pvParameters */
    tskIDLE_PRIORITY,                   /* uxPriority */
    SYNDICATION_TaskStack,              /* puxStackBuffer - the array to use as the task stack, as declared above. */
    {   /* xRegions - this task does not use any non-stack data hence all members are zero. */
        /* Base address     Length      Parameters */
        { (void*) __privileged_data_start__, (unsigned long) _Privileged_Data_Region_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { (void*) __task_stack_section_start__, (unsigned long) _Task_Stack_Section_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { SYNDICATION_TaskStack, SYNDICATION_TASK_STACK_SIZE_BYTES, portMPU_REGION_READ_WRITE }
    }
};

const xTaskParameters DIM_TaskParameters =
{
    DIM_Task,                           /* pvTaskCode - the function that implements the task. */
    (signed portCHAR*)"Dim           ", /* pcName       */
    DIM_TASK_STACK_SIZE_WORDS,          /* usStackDepth */
    NULL,                               /* pvParameters */
    tskIDLE_PRIORITY + 2,               /* uxPriority */
    DIM_TaskStack,                      /* puxStackBuffer - the array to use as the task stack, as declared above. */
    {   /* xRegions - this task does not use any non-stack data hence all members are zero. */
        /* Base address     Length      Parameters */
        { (void*) __privileged_data_start__, (unsigned long) _Privileged_Data_Region_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { (void*) __task_stack_section_start__, (unsigned long) _Task_Stack_Section_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { DIM_TaskStack, DIM_TASK_STACK_SIZE_BYTES, portMPU_REGION_READ_WRITE }
    }
};

const xTaskParameters SEQUENCE_TaskParameters =
{
    SEQUENCE_Task,                      /* pvTaskCode - the function that implements the task. */
    (signed portCHAR*)"Sequence      ", /* pcName       */
    SEQUENCE_TASK_STACK_SIZE_WORDS,     /* usStackDepth */
    NULL,                               /* pvParameters */
    tskIDLE_PRIORITY + 1,               /* uxPriority */
    SEQUENCE_TaskStack,                 /* puxStackBuffer - the array to use as the task stack, as declared above. */
    {   /* xRegions - this task does not use any non-stack data hence all members are zero. */
        /* Base address     Length      Parameters */
        { (void*) __privileged_data_start__, (unsigned long) _Privileged_Data_Region_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { (void*) __task_stack_section_start__, (unsigned long) _Task_Stack_Section_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { SEQUENCE_TaskStack, SEQUENCE_TASK_STACK_SIZE_BYTES, portMPU_REGION_READ_WRITE }
    }
};

const xTaskParameters LOG_TaskParameters =
{
    LOG_Task,                           /* pvTaskCode - the function that implements the task. */
    (signed portCHAR*)"Log           ", /* pcName       */
    LOG_TASK_STACK_SIZE_WORDS,          /* usStackDepth */
    NULL,                               /* pvParameters */
    tskIDLE_PRIORITY,                   /* uxPriority */
    LOG_TaskStack,                      /* puxStackBuffer - the array to use as the task stack, as declared above. */
    {   /* xRegions - this task does not use any non-stack data hence all members are zero. */
        /* Base address     Length      Parameters */
        { (void*) __privileged_data_start__, (unsigned long) _Privileged_Data_Region_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { (void*) __task_stack_section_start__, (unsigned long) _Task_Stack_Section_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { LOG_TaskStack, LOG_TASK_STACK_SIZE_BYTES, portMPU_REGION_READ_WRITE }
    }
};

const xTaskParameters SVINFO_MANAGER_TaskParameters =
{
    SVINFO_MANAGER_Task,                  /* pvTaskCode - the function that implements the task. */
    (signed portCHAR*)"SVInfo Manager",   /* pcName       */
    SVINFO_MANAGER_TASK_STACK_SIZE_WORDS, /* usStackDepth */
    NULL,                                 /* pvParameters */
    tskIDLE_PRIORITY,                     /* uxPriority */
    SVINFO_MANAGER_TaskStack,             /* puxStackBuffer - the array to use as the task stack, as declared above. */
    {   /* xRegions - this task does not use any non-stack data hence all members are zero. */
        /* Base address     Length      Parameters */
        { (void*) __privileged_data_start__, (unsigned long) _Privileged_Data_Region_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { (void*) __task_stack_section_start__, (unsigned long) _Task_Stack_Section_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { SVINFO_MANAGER_TaskStack, SVINFO_MANAGER_TASK_STACK_SIZE_BYTES, portMPU_REGION_READ_WRITE }
    }
};

const xTaskParameters CAN_MANAGER_TaskParameters =
{
    CAN_MANAGER_Task,                     /* pvTaskCode - the function that implements the task. */
    (signed portCHAR*)"CAN Manager   ",   /* pcName       */
    CAN_MANAGER_TASK_STACK_SIZE_WORDS,    /* usStackDepth */
    NULL,                                 /* pvParameters */
    tskIDLE_PRIORITY,                     /* uxPriority */
    CAN_MANAGER_TaskStack,                /* puxStackBuffer - the array to use as the task stack, as declared above. */
    {   /* xRegions - this task does not use any non-stack data hence all members are zero. */
        /* Base address     Length      Parameters */
        { (void*) __privileged_data_start__, (unsigned long) _Privileged_Data_Region_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { (void*) __task_stack_section_start__, (unsigned long) _Task_Stack_Section_Size, portMPU_REGION_PRIVILEGED_READ_WRITE_UNPRIVILEGED_READ_ONLY },
        { CAN_MANAGER_TaskStack, CAN_MANAGER_TASK_STACK_SIZE_BYTES, portMPU_REGION_READ_WRITE }
    }
};


MEMTEST_RESULT_T    ext_fram_test_result = 0;
MEMTEST_RESULT_T    ext_flash_test_result = 0;
xSemaphoreHandle    xInitTaskResetWaitMutex;
__ALIGN_BEGIN USB_OTG_CORE_HANDLE    USB_OTG_dev __ALIGN_END ;

/**
  * @brief  Check if fram is virgin.
  * @param  None
  * @retval - true: fram has not been used before - false: fram has been used before
  */
bool INIT_Check_FRAM_Is_Virgin(void)
{
    // check if first 8 bytes contain init marker
    uint64_t i_temp;
    FRAM_MIRROR_Read64(FRAM_INIT_STRING, &i_temp);
    if (i_temp != fram_init_value)
        return TRUE;
    else
        return FALSE;
}

/**
  * @brief  Write default values to fram.
  * @param  None
  * @retval None
  */
void INIT_Initialize_FRAM(void)
{
    // initialize fram
//    bool dirty_bit;
    uint8_t pBuffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    // 1. erase fram
    for (uint16_t address = 0; address < FRAM_SIZE; address += 8)
    {
        FRAM_Write64(address, pBuffer);
    }
    /*
        // 2. reset dirty bit
        dirty_bit = FALSE;
        FRAM_Write(FRAM_DIRTY_BIT, &dirty_bit, FRAM_MIRROR_SIZE_OF_DIRTY_BIT);
        FRAM_Write(FRAM_DIRTY_BIT + FRAM_ADR_MIRROR_BASE, &dirty_bit, FRAM_MIRROR_SIZE_OF_DIRTY_BIT);

        // 3. reset write counter
        FRAM_Write(FRAM_WRITE_CTR, pBuffer, FRAM_MIRROR_SIZE_OF_WRITE_CTR);
        FRAM_Write(FRAM_WRITE_CTR + FRAM_ADR_MIRROR_BASE, pBuffer, FRAM_MIRROR_SIZE_OF_WRITE_CTR);

        // LOG
        FRAM_MIRROR_Write32(FRAM_LOG_FIRST_ENTRY, pBuffer);  // index of first log entry
        FRAM_MIRROR_Write32(FRAM_LOG_NEXT_ENTRY, pBuffer);  // index of next free log entry
        FRAM_MIRROR_Write32(FRAM_LOG_NEXT_ENTRY_VERIFIED, pBuffer);  // index of first log entry
        FRAM_MIRROR_Write16(FRAM_LOG_OVERFLOW_COUNT, pBuffer);  // number of overflows

        // SETTINGS
        FRAM_MIRROR_Write32(FRAM_SETTINGS_LAYOUT_ID, pBuffer);  // init with 0
    */
    // BMM LED PROFILE
    // write zero profile into fram
    FRAM_MIRROR_Write(FRAM_BMM_LED_PROFILES_BASE, &bmm_led_profiles_flash[0], sizeof(BMM_LED_PROFILE));

    // write the init string at the end of the initialization
    FRAM_MIRROR_Write64(FRAM_INIT_STRING, &fram_init_value);
}

/**
  * @brief  Initialize subsystems and start all tasks.
  * @param  None
  * @retval None
  */
void INIT_Task(void *pvParameters)
{
    xTaskHandle vCreatedTaskHandle;

    xInitTaskResetWaitMutex = xSemaphoreCreateMutex();
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

    // Initialize memory
    PRIMESTRUCT_Init();

    // Initialize Hardware and Peripherals
    HW_CLOCK_Init();
    RTC_SetUnixTime(HW_CLOCK_Get());
    // if USE_USB_OTG_HS is defined, than use USB_OTG_HS_CORE_ID instead of USB_OTG_FS_CORE_ID
    USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_CDC_cb, &USR_cb);

    AGUARD_CreateQueue();
    PSU_MONITORING_Create();
    DETECTOR_Create();
    SYNDICATION_Create();

    // Initialize low level drivers
    DRIVER_LL_Init();

    // Initialize debug output. (Note: UART has to be initialized before)
    DEBUG_Init();

    SPI_SERIAL_FLASH_Init();
    SPI_SERIAL_FRAM_Init();
    while (SPI_SERIAL_FLASH_UnlockFlash() != SPI_SERIAL_FLASH_RESULT_FINISHED) {}  // do not continue if flash could not be unlocked
    while (!SPI_SERIAL_FRAM_UnlockFram()) {}  // do not continue if fram could not be unlocked
    // check if fram has been used or if this is the first time the system is started
    if (INIT_Check_FRAM_Is_Virgin())
    {
//      ext_fram_test_result = MEMTEST_EXT_FRAM();
//      ext_flash_test_result = MEMTEST_EXT_FLASH();
        INIT_Initialize_FRAM();  // the system is started for the first time --> initialize fram
    }

    FRAM_MIRROR_Init();  // the system has already been used sometime before -> load values from fram

    // start log (log must be started early to make sure that other modules can add messages to the log)
    xTaskCreateRestricted(&LOG_TaskParameters, &vCreatedTaskHandle);
    TAG_TASK(vCreatedTaskHandle, LOG_TASK_TAG);
    vTaskDelay(100);  // give log task time to initialize queue

    // todo check why taskYIELD(); does not work

    VERSION_Init();

    // Load and check all settings
    SETTINGS_Init();

    // Initialize higher level drivers
    DRIVER_HL_Init();

    BMM_Init();

    //initialize random number generator
    random_init();

    STATUS_LED_Set(LED_GREEN, LED_STATE_BLINK_FAST, LED_NO_TIMEOUT);

    uint16_t reset_reason;
    uint32_t reset_val1, reset_val2;
    reset_reason = SYSTEM_GetResetReason(&reset_val1, &reset_val2);
    LOG(LOG_ID_SYSTEM_STARTED, reset_reason, reset_val1, reset_val2, NULL);
    LOG(LOG_ID_SYSTEM_STARTED_FW_VERSION, VERSION_MAJOR, VERSION_MINOR, 0, NULL);
    SYSTEM_ResetResetReason();


    /************************************************************************************/
    /* Startup sequence                                                                 */
    /************************************************************************************/
    xTaskCreateRestricted(&DRIVER_TaskParameters, &vCreatedTaskHandle);
    TAG_TASK(vCreatedTaskHandle, DRIVER_TASK_TAG);
    xTaskCreateRestricted(&TIMER_TaskParameters, &vCreatedTaskHandle);
    TAG_TASK(vCreatedTaskHandle, TIMER_TASK_TAG);

    xTaskCreateRestricted(&SOUND_TaskParameters, &vCreatedTaskHandle);
    TAG_TASK(vCreatedTaskHandle, SOUND_TASK_TAG);

    vTaskDelay(100);

    SOUND_PlayMelody(HELLO_MELODY);
    STATUS_LED_Set(LED_GREEN, LED_STATE_BLINK_FAST, LED_NO_TIMEOUT);
    STATUS_LED_Set(LED_SIGNAL, LED_STATE_BLINK_FAST, LED_NO_TIMEOUT);

    vTaskDelay(100);

    xTaskCreateRestricted(&SENSORS_TaskParameters, &vCreatedTaskHandle);
    TAG_TASK(vCreatedTaskHandle, SENSORS_TASK_TAG);
    xTaskCreateRestricted(&GPSSYNC_TaskParameters, &vCreatedTaskHandle);
    TAG_TASK(vCreatedTaskHandle, GPSSYNC_TASK_TAG);

    xTaskCreateRestricted(&GLOOM_MANAGER_TaskParameters, &vCreatedTaskHandle);
    TAG_TASK(vCreatedTaskHandle, GLOOM_MANAGER_TASK_TAG);

    vTaskDelay(100);

    TERMINAL_SERVER_Init();

    xTaskCreateRestricted(&DETECTOR_TaskParameters, &vCreatedTaskHandle);
    TAG_TASK(vCreatedTaskHandle, DETECTOR_TASK_TAG);
    xTaskCreateRestricted(&INDICATION_TaskParameters, &vCreatedTaskHandle);
    TAG_TASK(vCreatedTaskHandle, INDICATION_TASK_TAG);
    xTaskCreateRestricted(&SVINFO_MANAGER_TaskParameters, &vCreatedTaskHandle);
    TAG_TASK(vCreatedTaskHandle, SVINFO_MANAGER_TASK_TAG);
    xTaskCreateRestricted(&CAN_MANAGER_TaskParameters, &vCreatedTaskHandle);
    TAG_TASK(vCreatedTaskHandle, CAN_MANAGER_TASK_TAG);
    xTaskCreateRestricted(&CLI_TaskParameters, &vCreatedTaskHandle);
    TAG_TASK(vCreatedTaskHandle, CLI_TASK_TAG);
    xTaskCreateRestricted(&MESHCOM_TaskParameters, &vCreatedTaskHandle);
    TAG_TASK(vCreatedTaskHandle, MESHCOM_TASK_TAG);
    xTaskCreateRestricted(&SYNDICATION_TaskParameters, &vCreatedTaskHandle);
    TAG_TASK(vCreatedTaskHandle, SYNDICATION_TASK_TAG);


    xTaskCreateRestricted(&AlarmClockTaskParameters, &vCreatedTaskHandle);
    TAG_TASK(vCreatedTaskHandle, ALARM_CLOCK_TASK_TAG);
    xTaskCreateRestricted(&DIM_TaskParameters, &vCreatedTaskHandle);
    TAG_TASK(vCreatedTaskHandle, DIM_TASK_TAG);
    xTaskCreateRestricted(&SEQUENCE_TaskParameters, &vCreatedTaskHandle);
    TAG_TASK(vCreatedTaskHandle, SEQUENCE_TASK_TAG);


    STATUS_LED_Set(LED_SIGNAL, LED_STATE_OFF, LED_NO_TIMEOUT);
    STATUS_LED_Set(LED_GREEN, LED_STATE_ON, LED_NO_TIMEOUT);
    STATUS_LED_Set(LED_RED, LED_STATE_MULTI_3, LED_NO_TIMEOUT);

    while(1)
    {
        // wait for reset command
        if (pdTRUE == xSemaphoreTake(xInitTaskResetWaitMutex, portMAX_DELAY))
            SCB->AIRCR = 0x05fa0004;  // perform system reset
    }
}
