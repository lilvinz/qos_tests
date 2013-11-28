/*
 * delay.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 02.09.2011
 *      Author: DHA
 *
 * Purpose: Provide a function to delay program execution.
 *
 */
#include "stdheader.h"

#include "delay.h"
#include "stm32f2xx_tim.h"
#include "misc.h"

static volatile bool b_waiting = false;

/**
  * @brief  Initialize timer for delay
  * @param  None
  * @retval None
  */
void DELAY_Init(void)
{
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM7, ENABLE);  // enable reset to force a reset of the selected component
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM7, DISABLE);  // disable reset

    NVIC_InitTypeDef NVIC_InitTim7 = { TIM7_IRQn, 13, 0, ENABLE };
    NVIC_Init(&NVIC_InitTim7);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    // --------------------------------------------------------------------------------
    // TIM7 configuration: generate interrupt for delay
    // The frequency of TIM7 depends on the frequency of APB1, APB1 prescaler,
    // the prescaler of TIM7, and the value of TIM_Period.
    // The frequency of APB1 depends on the SystemCoreClock and the APB1 prescaler.
    // Both are defined in "system_stm32f2xx.c".
    // Calculating the value of the prescaler for TIM7:
    // The frequency of the counter should be about 1 MHz (1 us per count).
    // System frequency is 120 MHz, prescaler of APB1 is 4 so clock to timer is 60 MHz.
    // Number of counts is 2.
    // Prescaler = (60 MHz / 1 MHz) = 60.
    // --------------------------------------------------------------------------------

    // Time Base configuration
    TIM_TimeBaseStructure.TIM_Prescaler = 60;  // note comment above
    TIM_TimeBaseStructure.TIM_Period = 0;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);
    TIM_UpdateDisableConfig(TIM7, DISABLE);
    TIM_UpdateRequestConfig(TIM7, TIM_UpdateSource_Global);
    TIM_SelectOnePulseMode(TIM7, TIM_OPMode_Single);
    TIM_ARRPreloadConfig(TIM7, DISABLE);
}

void TIM7_IRQHandler(void)
{
    TIM_Cmd(TIM7, DISABLE);  // TIM7 counter disable
    TIM_ITConfig(TIM7, TIM_IT_Update, DISABLE);
    TIM_ClearITPendingBit(TIM7, TIM_IT_Update);

    b_waiting = false;
}

void DELAY_uS(uint32_t nCount)
{
    b_waiting = true;

    TIM_SetAutoreload(TIM7, nCount);
    TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM7, ENABLE);  // TIM7 counter enable

    while(b_waiting)
    {
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
    }
}
