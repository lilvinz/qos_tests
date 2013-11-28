/*
 * svinfo.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 31.10.2011
 *      Author: DHA
 *
 * Purpose:
 * Receive messages from svinfo
 *
 */


#include "stdheader.h"
#include "svinfo.h"

#include "clock.h"
#include "timer.h"
#include "hw_configuration.h"
#include "debug.h"


static volatile bool b_test_mode = false;  // flag to show that test mode is active
static volatile uint8_t i_num_logic_high_detected = 0;  // counter for number of logic high levels detected on svinfo
static volatile xQueueHandle xQueueSVINFORX;

bool SVINFO_GetTestMode(void)
{
    return b_test_mode;
}

bool SVINFO_CharAvail(void)
{
    if (0 < uxQueueMessagesWaiting(xQueueSVINFORX))
        return TRUE;
    return FALSE;
}

int SVINFO_NumCharAvail(void)
{
    return uxQueueMessagesWaiting(xQueueSVINFORX);
}

int SVINFO_GetChar(void)
{
    char c;
    if (pdTRUE == xQueueReceive(xQueueSVINFORX, (void *) &c, portMAX_DELAY))
        return (int) c;
    return -1;
}

/**
  * @brief  Enable or disable external interrupt for svinfo input.
  * @param  new_state:  - enable: external interrupt will be enabled.
  * @param              - disable: external interrupt will be disabled.
  * @retval None
  */
void SVINFO_ExtiConfig(FunctionalState new_state)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    //Clear any possibly pending interrupts
    EXTI_ClearITPendingBit(EXTI_Line8);

    EXTI_InitStructure.EXTI_Line = EXTI_Line8;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = new_state;
    EXTI_Init(&EXTI_InitStructure);
}

/**
  * @brief  Create queue for svinfo.
  * @param  None
  * @retval None
  */
void SVINFO_Create(void)
{
    xQueueSVINFORX = xQueueCreate(64, sizeof(uint8_t));
    assert_param(xQueueSVINFORX != NULL);
}

/**
  * @brief  Uninitialize svinfo.
  * @param  None
  * @retval None
  */
void SVINFO_Deinit(void)
{
    SVINFO_ExtiConfig(DISABLE);
    TIM_ITConfig(TIM1, TIM_IT_CC1, DISABLE);
    TIM_Cmd(TIM1, DISABLE);  // TIM1 counter disable
    b_test_mode = FALSE;
}

/**
  * @brief  Initialize svinfo.
  * @param  None
  * @retval None
  */
void SVINFO_Init(void)
{
    // --------------------------------------------------------------------------------
    // TIM1 configuration: generate signals to capture svinfo (software uart).
    // The frequency of TIM1 depends on the frequency of APB2, APB2 prescaler,
    // the prescaler of TIM1 and the value of TIM_Period.
    // The frequency of APB2 depends on the SystemCoreClock and the APB2 prescaler.
    // Both are defined in "system_stm32f2xx.c".
    // Calculating the value of the prescaler for TIM1:
    // The period of the timer should be one bit time (1/300s).
    // System frequency is 120 MHz, prescaler of APB2 is 2 so clock to timer is 120 MHz.
    // (120 MHz / 300 Hz) = 200000.
    // The counter should count to 20000.
    // Prescaler = 20.
    // To achieve this the prescaler has to be set to 19 (see documentation for details).
    // --------------------------------------------------------------------------------
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);

    // Time Base configuration
    TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t) (SystemCoreClock / (SVINFO_SIGNAL_UART_BAUDRATE * SVINFO_SIGNAL_TIMER_MAX_COUNT)) - 1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = SVINFO_SIGNAL_TIMER_MAX_COUNT;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;

    TIM_DeInit(TIM1);
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    TIM_SetCompare1(TIM1, SVINFO_SIGNAL_TIMER_MAX_COUNT / 2);  // capture after half bit time

    SVINFO_ExtiConfig(ENABLE);
}

void TIM1_CC_IRQHandler(void)
{
    static uint8_t i_rec_bit_ctr = 0;
    static uint8_t i_rec_byte = 0;
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if (TIM_GetITStatus(TIM1, TIM_IT_CC1) != RESET)
    {
        uint8_t i_bit_value = 0;
        i_bit_value = SVINFO_STATE();

        if (b_test_mode)
        {
            // test mode active, try to detect end of test mode
            if (i_bit_value == 1)
            {
                // detected end of test mode
                b_test_mode = FALSE;
                TIM_ITConfig(TIM1, TIM_IT_CC1, DISABLE);
                TIM_Cmd(TIM1, DISABLE);  // TIM1 counter disable
                SVINFO_ExtiConfig(ENABLE);  // enable external interrupt to recognize next edge
            }
        }
        else
        {
            if (i_rec_bit_ctr == 0)
            {
                // check if start bit is still available
                if (i_bit_value != 0)
                {
                    // start bit is not available --> error
                    TIM_ITConfig(TIM1, TIM_IT_CC1, DISABLE);
                    TIM_Cmd(TIM1, DISABLE);  // TIM1 counter disable
                    SVINFO_ExtiConfig(ENABLE);  // enable external interrupt to recognize next edge
                }
                else
                    i_rec_bit_ctr++;
            }
            else if (i_rec_bit_ctr >= 9)
            {
                // check if stop bit is available
                if (i_bit_value == 1)
                {
                    // stop bit is available --> byte received
                    xQueueSendFromISR(xQueueSVINFORX, &i_rec_byte, &xHigherPriorityTaskWoken);

                    TIM_ITConfig(TIM1, TIM_IT_CC1, DISABLE);
                    TIM_Cmd(TIM1, DISABLE);  // TIM1 counter disable
                    SVINFO_ExtiConfig(ENABLE);  // enable external interrupt to recognize next edge
                }
                else
                {
                    // stop bit is not available --> error or test mode
                    if (i_num_logic_high_detected > 7)
                        b_test_mode = true;
                    else
                    {
                        TIM_ITConfig(TIM1, TIM_IT_CC1, DISABLE);
                        TIM_Cmd(TIM1, DISABLE);  // TIM1 counter disable
                        SVINFO_ExtiConfig(ENABLE);  // enable external interrupt to recognize next edge
                    }
                }
                i_rec_bit_ctr = 0;
                i_rec_byte = 0;
                i_num_logic_high_detected = 0;
            }
            else
            {
                // received a bit
                i_rec_byte |= i_bit_value << (i_rec_bit_ctr - 1);
                i_rec_bit_ctr++;

                if (i_bit_value == 0)
                    i_num_logic_high_detected++;
            }
        }
        TIM_ClearITPendingBit(TIM1, TIM_IT_CC1);
    }
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

/**
  * @brief  Enable or disable external interrupt for svinfo input.
  * @param  new_state:  - enable: external interrupt will be enabled.
  * @param              - disable: external interrupt will be disabled.
  * @retval None
  */
void SVINFO_ExtIRQHandler(void)
{
    SVINFO_ExtiConfig(DISABLE);

    TIM_SetCounter(TIM1, 0);  // reset timer
    TIM_ClearITPendingBit(TIM1, TIM_IT_CC1);
    TIM_ITConfig(TIM1, TIM_IT_CC1, ENABLE);
    TIM_Cmd(TIM1, ENABLE);  // TIM1 counter enable
}
