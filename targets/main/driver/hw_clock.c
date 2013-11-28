/*
 * hw_clock.c
 *
 *  Created on: 05.07.2010
 *      Author: CBA
 */

#include "stdheader.h"
#include "debug.h"
#include "hw_clock.h"
#include "rtc.h"


#include "backup_register_map.h"

void HW_CLOCK_Set(uint32_t timestamp)
{
    RTCTM           RTC_Time;
    RTC_TimeTypeDef RTC_TimeStructure;
    RTC_DateTypeDef RTC_DateStructure;

    // Convert UNIX timestamp to RTC_Time;
    RTC_RawLocalTime(&RTC_Time, timestamp);

    // Set the Time
    RTC_TimeStructure.RTC_Hours = RTC_Time.tm_hour;
    RTC_TimeStructure.RTC_Minutes = RTC_Time.tm_min;
    RTC_TimeStructure.RTC_Seconds = RTC_Time.tm_sec;

    // Set the Date
    RTC_DateStructure.RTC_Month = RTC_Time.tm_mon + 1;   // add 1 cause stm32 rtc jan = 1 and RTCTM jan = 0!!
    RTC_DateStructure.RTC_Date = RTC_Time.tm_mday;
    RTC_DateStructure.RTC_Year = RTC_Time.tm_year % 100;
    RTC_DateStructure.RTC_WeekDay = RTC_Time.tm_wday;
    if (RTC_DateStructure.RTC_WeekDay == 0)
        RTC_DateStructure.RTC_WeekDay = 7;

    DCATCH(RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure), SUCCESS);
    DCATCH(RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure), SUCCESS);
}

uint32_t HW_CLOCK_Get(void)
{
    RTCTM           RTC_Time;
    RTC_TimeTypeDef RTC_TimeStructure;
    RTC_DateTypeDef RTC_DateStructure;

    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
    RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);

    // Set the Time
    RTC_Time.tm_hour = RTC_TimeStructure.RTC_Hours;
    RTC_Time.tm_min = RTC_TimeStructure.RTC_Minutes;
    RTC_Time.tm_sec = RTC_TimeStructure.RTC_Seconds;

    // Set the Date
    RTC_Time.tm_mon = RTC_DateStructure.RTC_Month - 1;  // sub 1 cause stm32 rtc jan = 1 and RTCTM jan = 0!!
    RTC_Time.tm_mday = RTC_DateStructure.RTC_Date;
    RTC_Time.tm_year = RTC_DateStructure.RTC_Year + 100;
    RTC_Time.tm_wday = RTC_DateStructure.RTC_WeekDay % 7;

    /* Unfreeze the RTC DR Register */
    (void)RTC->DR;

    return RTC_MkTime(&RTC_Time);
}

void HW_CLOCK_Init(void)
{
    /* Allow access to BKP Domain */
    PWR_BackupAccessCmd(ENABLE);

    if (RTC_ReadBackupRegister(BKP_REG_HW_CLOCK) != HW_CLOCK_INIT_MAGIC_CONSTANT)
    {
        /* Backup data register value is not correct or not yet programmed (when
           the first time the program is executed)

           WARNING: If something is changed within this branch, you have to
           change the HW_CLOCK_INIT_MAGIC_CONSTANT to force a reinitialisation of
           the backup domain when doing firmware updates without power cycle.
         */

        // Reset Backup Domain
        // This is necessary to be able to switch clock sources without power on reset (including VBat)
        RCC_BackupResetCmd(ENABLE);
        RCC_BackupResetCmd(DISABLE);

        // Enable LSE
        RCC_LSEConfig(RCC_LSE_ON);

        // Wait till LSE is ready
        // Warning, after initial power up this can take several seconds
        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) {}

        // Select LSE as RTC Clock Source
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

        // Enable RTC Clock
        RCC_RTCCLKCmd(ENABLE);

        // Wait for RTC registers synchronization
        DCATCH(RTC_WaitForSynchro(), SUCCESS);

        RTC_InitTypeDef RTC_InitStructure;
        RTC_InitStructure.RTC_SynchPrediv = 0xFF; // (32KHz / 128) - 1 = 0xFF
        RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
        RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
        DCATCH(RTC_Init(&RTC_InitStructure), SUCCESS);

        HW_CLOCK_Set(0);

        //Set backup register
        RTC_WriteBackupRegister(BKP_REG_HW_CLOCK, HW_CLOCK_INIT_MAGIC_CONSTANT);
    }
    else
    {
        // Wait for RTC registers synchronization
        DCATCH(RTC_WaitForSynchro(), SUCCESS);
    }

    // Configure the RTC WakeUp Clock source: CK_SPRE (1Hz)
    RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
    RTC_SetWakeUpCounter(0);

    // Clear the RTC Wakeup Flag & Interrupt Bit
    RTC_ClearFlag(RTC_FLAG_WUTF);
    RTC_ClearITPendingBit(RTC_IT_WUT);
    // Clear the EXTI Line 22 Pending bit (Connected internally to RTC Wakeup)
    EXTI_ClearITPendingBit(EXTI_Line22);
    // Enable the RTC Wakeup Interrupt
    RTC_ITConfig(RTC_IT_WUT, ENABLE);

    // Enable Wakeup Counter
    DCATCH(RTC_WakeUpCmd(ENABLE), SUCCESS);
}
