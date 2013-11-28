/*
 * driver.c
 *
 *  Created on: 26.05.2010
 *      Author: Administrator
 */

#include "stdheader.h"
#include "driver.h"

#include "debug_freertos.h"
#include "uart.h"
#include "adc.h"
#include "uart.h"
#include "status_led.h"
#include "watchdog.h"
#include "can_cmd_i.h"

xSemaphoreHandle xDriverIOPendingSemaphore = NULL;

void DRIVER_Init(void)
{
    UART_Init();
    ADC_Driver_Init();
}

void DRIVER_Task(void *pvParameters)
{
    // Semaphore cannot be used before a call to xSemaphoreCreateCounting().
    // The max value to which the semaphore can count shall be 10, and the
    // initial value assigned to the count shall be 0.
    xDriverIOPendingSemaphore = xSemaphoreCreateCounting( 255, 10 );
    assert_param(xDriverIOPendingSemaphore != NULL);
    DQueueAddToRegistry(xDriverIOPendingSemaphore, "xDriverIOPendingSemaphore");
    // The semaphore was created successfully.
    // The semaphore can now be used.

    STATUS_LED_Init();

    CAN_CMD_I_Init();

    while (1)
    {
        xSemaphoreTake(xDriverIOPendingSemaphore, 10);
        {
            WATCHDOG_Heartbeat(WATCHDOG_TASK_ID_DRIVER);
            UART_Tick();
            ADC_Tick();
        }
    }
}
