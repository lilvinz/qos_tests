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
#include "vcp_uart.h"
#include "spi_serial_flash.h"
#include "status_led.h"
#include "piezo.h"
#include "svinfo.h"
#include "sv_uni_com.h"
#include "mesh.h"
#include "watchdog.h"
#include "vcp_uart.h"
#include "heater.h"


static bool b_erase_flash_section_start = false;
static uint32_t flash_section_start_addr = 0;
static uint32_t flash_section_end_addr = 0;
static xSemaphoreHandle extFlashEraseSectionMutex = NULL;
static xSemaphoreHandle extFlashEraseSectionWaitMutex = NULL;

xSemaphoreHandle xDriverIOPendingSemaphore = NULL;


bool DRIVER_FlashEraseSectionStart(uint32_t start_addr, uint32_t end_addr, xSemaphoreHandle *wait_mutex)
{
    if (xSemaphoreTake(extFlashEraseSectionMutex, 0) != pdTRUE)
        return false;
    if (uxQueueMessagesWaiting(extFlashEraseSectionWaitMutex) > 0)
        return false;
    flash_section_start_addr = start_addr;
    flash_section_end_addr = end_addr;
    b_erase_flash_section_start = true;
    *wait_mutex = extFlashEraseSectionWaitMutex;
    return true;
}

void DRIVER_LL_Init(void)
{
    UART_Init();
    PIEZO_Init();
    ADC_Driver_CreateQueue();  // This may only be called once
    ADC_Driver_Init();
    SV_UNI_COM_Init();
    TODO(This is just proof of concept. Adjust me appropriately.)
    VCP_UART_Init(VCP_UART_CHANNEL_1, 128, 128);
    HEATER_Init();
}

void DRIVER_HL_Init(void)
{
    MESH_Init();
}

void DRIVER_Task(void *pvParameters)
{
    bool b_erasing_flash_section = false;

    portTickType last_tick_timestamp = 0;

    // Semaphore cannot be used before a call to xSemaphoreCreateCounting().
    // The max value to which the semaphore can count shall be 10, and the
    // initial value assigned to the count shall be 0.
    xDriverIOPendingSemaphore = xSemaphoreCreateCounting(255, 10);
    assert_param(xDriverIOPendingSemaphore != NULL);
    DQueueAddToRegistry(xDriverIOPendingSemaphore, "xDriverIOPendingSemaphore");

    extFlashEraseSectionMutex = xSemaphoreCreateMutex();
    assert_param(extFlashEraseSectionMutex != NULL);
    DQueueAddToRegistry(extFlashEraseSectionMutex, "extFlashEraseSectionMutex");

    extFlashEraseSectionWaitMutex = xSemaphoreCreateMutex();
    assert_param(extFlashEraseSectionWaitMutex != NULL);
    DQueueAddToRegistry(extFlashEraseSectionWaitMutex, "extFlashEraseSectionWaitMutex");

    xSemaphoreTake(extFlashEraseSectionWaitMutex, portMAX_DELAY);

    STATUS_LED_Init();

    // The semaphore was created successfully.
    // The semaphore can now be used.
    while (1)
    {
        if (xSemaphoreTake(xDriverIOPendingSemaphore, 10) == pdTRUE)
        {
            UART_Tick();
            VCP_UART_Tick();
#if (SVINFOCOM_IMPLEMENTATION_FINISHED == 1)
            SV_UNI_COM_Tick();
#endif
            MESH_Tick();
        }

        if (xTaskGetTickCount() >= last_tick_timestamp + DRIVER_TICK_INTERVAL_TICKS)
        {
            // 10 ms tick
            last_tick_timestamp = xTaskGetTickCount();

            WATCHDOG_Heartbeat(WATCHDOG_TASK_ID_DRIVER);
            ADC_Driver_Tick();
            HEATER_Tick();

            // TODO[CLEANUP] move the flash erase code to flash driver
            if (b_erase_flash_section_start)
            {
                if (SPI_SERIAL_FLASH_EraseSectionStart(flash_section_start_addr, flash_section_end_addr) == SPI_SERIAL_FLASH_RESULT_FINISHED)
                {
                    b_erase_flash_section_start = false;
                    b_erasing_flash_section = true;
                    // todo[medium] add error handling
                }
            }

            if (b_erasing_flash_section)
            {
                SPI_SERIAL_FLASH_RESULT_T result = SPI_SERIAL_FLASH_EraseSectionTick();
                switch (result)
                {
                case SPI_SERIAL_FLASH_RESULT_BUSY:
                    // do nothing
                    break;
                case SPI_SERIAL_FLASH_RESULT_FINISHED:
                    // finished
                    b_erasing_flash_section = false;
                    xSemaphoreGive(extFlashEraseSectionMutex);
                    xSemaphoreGive(extFlashEraseSectionWaitMutex);
                    break;
                default:
                    // error
                    b_erasing_flash_section = false;
                    break;
                }
            }
        }
    }
}
