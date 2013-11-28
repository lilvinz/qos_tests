/*
 * svinfo_manager.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 31.10.2011
 *      Author: DHA
 *
 * Purpose:
 * Control SVInfo signals and communication
 *
 */

#include "stdheader.h"

#include "svinfo_manager.h"

//#include "rtc.h"
//#include "clock.h"
#include "log.h"
#include "watchdog.h"

#include "svinfo.h"
#include "sequence.h"




void SVINFO_MANAGER_Init(void)
{

}

/**
  * @brief  SVINFO_MANAGER task.
  * @param  pvParameters
  * @retval None
  */
void SVINFO_MANAGER_Task(void *pvParameters)
{
    bool b_test_mode_new = FALSE;
    bool b_test_mode_old = FALSE;

    SVINFO_MANAGER_Init();

    while(1)
    {
        WATCHDOG_Heartbeat(WATCHDOG_TASK_ID_SVINFO_MANAGER);

        b_test_mode_new = SVINFO_GetTestMode();
        if (b_test_mode_new == TRUE)
        {
            if (b_test_mode_old != TRUE)
            {
                b_test_mode_old = TRUE;
                LOG(LOG_ID_SVINFO_TEST_MODE, 0, 0, 0, NULL);
                SEQUENCE_TestDriveStartBlocking(SVINFO_MANAGER_LED_TESTMODE_INTERVAL_MS);
            }
        }
        else
            b_test_mode_old = FALSE;

        vTaskDelay(100);
    }
}
