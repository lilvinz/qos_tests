/*
 * interrupts.c
 *
 *  Created on: 15.07.2010
 *      Author: Administrator
 */
#include "stdheader.h"
#include "stm32f2xx_exti.h"
#include "interrupts.h"

#include "rtc.h"
#include "debug.h"
#include "sequence.h"
#include "detector.h"

void EXTI0_IRQHandler(void)
{

}

void EXTI1_IRQHandler(void)
{

}

void EXTI2_IRQHandler(void)
{

}

void EXTI3_IRQHandler(void)
{

}

void EXTI9_5_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line9) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line9);
    }
    if (EXTI_GetITStatus(EXTI_Line8) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line8);
    }
    if (EXTI_GetITStatus(EXTI_Line7) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line7);
    }
    if (EXTI_GetITStatus(EXTI_Line6) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line6);
    }
    if (EXTI_GetITStatus(EXTI_Line5) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line5);
    }
}

void EXTI15_10_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line15) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line15);
    }
    if (EXTI_GetITStatus(EXTI_Line14) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line14);
    }
    if (EXTI_GetITStatus(EXTI_Line13) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line13);
    }
    if (EXTI_GetITStatus(EXTI_Line12) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line12);
    }
    if (EXTI_GetITStatus(EXTI_Line11) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line11);
    }
    if (EXTI_GetITStatus(EXTI_Line10) != RESET)   // SENSFault
    {
        DETECTOR_SENSFault_IRQHandler();
        EXTI_ClearITPendingBit(EXTI_Line10);
    }
}

// If our GPS-synchronization will break:
// Due to crystal deviation RTC is 16ms off within 8min. 16ms is critical cause you
// gonna see the offset in the blink sequence compared to neighbor beacons with
// fully functional GPS.
void RTC_WKUP_IRQHandler(void)
{
    if (RTC_GetITStatus(RTC_IT_WUT) != RESET ||
        EXTI_GetITStatus(EXTI_Line22) != RESET)
    {
        //ToDo: Erkennung ob Interrupts hinreichend weit auseinander liegen.
        // überlaufen von einem 32bit millisekundenzähler beachten.

        //static volatile uint32_t last_irq_ts;

        //if (last_irq_ts > (uint32_t)(CLOCK_GetMillitimestamp())
        {
            RTC_TickUnixTime();
//            SEQUENCE_RTC_TickHandlerFromISR();
        }

        //last_irq_ts = (uint32_t)CLOCK_GetMillitimestamp();
        //last_irq_ts += 500;

        RTC_ClearITPendingBit(RTC_IT_WUT);
        EXTI_ClearITPendingBit(EXTI_Line22);
    }
}


void WWDG_IRQHandler(void)
{
    asm("nop");
}

