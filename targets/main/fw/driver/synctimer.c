/*
 * synctimer.c
 *
 *  Created on: 12.05.2011
 *      Author: cba
 */

/*
 * PURPOSE:
 *
 * - Setup Hardware Timer and synchronize it by external 1PPS timebase.
 * - precise 1ms resolution time interrupts generated 4 independend compare channels.
 */

#include "stdheader.h"
#include "synctimer.h"

#include "hw_configuration.h"


void SYNCTIMER_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure(&TIM_TimeBaseStructure);

    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);

    /***** Time Base configuration *****/
    TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t) (SystemCoreClock / 10000);     //todo [stm32f2xx] check new prescaler and impacts
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = 1000;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV2;

    TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);

    /***** PWM generation configuration *****/
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
    TIM_OCInitStructure.TIM_Pulse = 0x0;

    /* Timer 8 */
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OC1Init(TIM8, &TIM_OCInitStructure);
}


void TIM8_UP_TIM13_IRQHandler(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if (TIM_GetITStatus(TIM8, TIM_IT_Update) != RESET)
    {
        //xSemaphoreGiveFromISR(xDimInterruptBSemaphore, &xHigherPriorityTaskWoken);
        *
        TIM_ClearITPendingBit(TIM8, TIM_IT_Update);
    }


    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

void SYNCTIMER_Kick(void)
{

}
