/*
 * interrupts.c
 *
 *  Created on: 15.07.2010
 *      Author: Administrator
 */
#include "stdheader.h"
#include "interrupts.h"

#include "gpssync.h"
#include "rtc.h"
#include "alarm_clock.h"
#include "debug.h"
#include "sequence.h"
#include "detector.h"
#include "usb_conf.h"
#include "usb_core.h"
#include "usbd_core.h"
#include "usbd_cdc_core.h"
#include "svinfo.h"

// todo [medium] check if necessary
extern USB_OTG_CORE_HANDLE           USB_OTG_dev;
extern uint32_t USBD_OTG_ISR_Handler (USB_OTG_CORE_HANDLE *pdev);
#ifdef USB_OTG_HS_DEDICATED_EP1_ENABLED
extern uint32_t USBD_OTG_EP1IN_ISR_Handler (USB_OTG_CORE_HANDLE *pdev);
extern uint32_t USBD_OTG_EP1OUT_ISR_Handler (USB_OTG_CORE_HANDLE *pdev);
#endif
// end todo

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
        SVINFO_ExtIRQHandler();
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
    if (EXTI_GetITStatus(EXTI_Line15) != RESET)   // 1PPS
    {
        GPSSYNC_GPS_IRQHandler();
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
    if (EXTI_GetITStatus(EXTI_Line12) != RESET)   // GPSVCCFault
    {
        DETECTOR_GPS_VCC_Fault_IRQHandler();
        EXTI_ClearITPendingBit(EXTI_Line12);
    }
    if (EXTI_GetITStatus(EXTI_Line11) != RESET)   // GPSRFFault
    {
        DETECTOR_GPS_RF_Fault_IRQHandler();
        EXTI_ClearITPendingBit(EXTI_Line11);
    }
    if (EXTI_GetITStatus(EXTI_Line10) != RESET)   // LUX2.8VFault
    {
        DETECTOR_LUX_2_8_V_Fault_IRQHandler();
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
            ALARM_CLOCK_GiveMutexFromISR();
            SEQUENCE_RTC_TickHandlerFromISR();
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


/**
  * @brief  This function handles EXTI15_10_IRQ Handler.
  * @param  None
  * @retval None
  */
#ifdef USE_USB_OTG_FS
void OTG_FS_WKUP_IRQHandler(void)
{
    if(USB_OTG_dev.cfg.low_power)
    {
        *(uint32_t *)(0xE000ED10) &= 0xFFFFFFF9 ;
        SystemInit();
        USB_OTG_UngateClock(&USB_OTG_dev);
    }
    EXTI_ClearITPendingBit(EXTI_Line18);
}
#endif

/**
  * @brief  This function handles EXTI15_10_IRQ Handler.
  * @param  None
  * @retval None
  */
#ifdef USE_USB_OTG_HS
void OTG_HS_WKUP_IRQHandler(void)
{
    if(USB_OTG_dev.cfg.low_power)
    {
        *(uint32_t *)(0xE000ED10) &= 0xFFFFFFF9 ;
        SystemInit();
        USB_OTG_UngateClock(&USB_OTG_dev);
    }
    EXTI_ClearITPendingBit(EXTI_Line20);
}
#endif

/**
  * @brief  This function handles OTG_HS Handler.
  * @param  None
  * @retval None
  */
#ifdef USE_USB_OTG_HS
void OTG_HS_IRQHandler(void)
#else
void OTG_FS_IRQHandler(void)
#endif
{
    USBD_OTG_ISR_Handler (&USB_OTG_dev);
}

#ifdef USB_OTG_HS_DEDICATED_EP1_ENABLED
/**
  * @brief  This function handles EP1_IN Handler.
  * @param  None
  * @retval None
  */
void OTG_HS_EP1_IN_IRQHandler(void)
{
    USBD_OTG_EP1IN_ISR_Handler (&USB_OTG_dev);
}

/**
  * @brief  This function handles EP1_OUT Handler.
  * @param  None
  * @retval None
  */
void OTG_HS_EP1_OUT_IRQHandler(void)
{
    USBD_OTG_EP1OUT_ISR_Handler (&USB_OTG_dev);
}
#endif
