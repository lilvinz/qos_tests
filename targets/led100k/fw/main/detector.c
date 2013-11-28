#include "stdheader.h"
#include "detector.h"

#include "clock.h"
#include "hw_configuration.h"

#include "watchdog.h"



DETECTOR_STATUS_T _detector_status;

volatile uint8_t     detector_fault_bm;

/**
  * @brief  SENSFault irq handler.
  * @param  None
  * @retval None
  */
void DETECTOR_SENSFault_IRQHandler(void)
{
    detector_fault_bm |= DETECTOR_SENS_FAULT;
};

void DETECTOR_Powermanager_CheckFault(void)
{
    if (GPIO_ReadInputDataBit(SENS_FAULT_PORT, SENS_FAULT_PIN))
        detector_fault_bm &= ~DETECTOR_SENS_FAULT;
}

bool detector_check_sensors(void)
{
    return TRUE;
}

void DETECTOR_Init(void)
{
    _detector_status.failure_timestamp = CLOCK_GetTimestamp();
    _detector_status.uWarningFlags = 0;
    _detector_status.uErrorFlags = 0;
}

void DETECTOR_Task(void *pvParameters)
{
    uint8_t             flags;
    DETECTOR_Init();
    while(1)
    {
//        WATCHDOG_Heartbeat(WATCHDOG_TASK_ID_DETECTOR);

        flags = 0;

//        if (!detector_check_sensors())
//            flags += FAIL_FLAG_SENSOR;

        _detector_status.uWarningFlags = flags;

        // check if an error was cleared

        vTaskDelay(DETECTOR_WAIT_TIME);
    }
}
