/*
 * sound.c
 *
 *  Created on: 16.07.2010
 *      Author: Carsten Bartels
 */

#include "stdheader.h"
#include "piezo.h"

#define PIEZO_PWM_BASE_FREQ     256

static volatile uint16_t pwm_length;
static volatile uint16_t pwm_value;
static volatile uint16_t pwm_attack_count;
static volatile uint16_t pwm_decay_count;
static volatile uint16_t pwm_sustain_count;
static volatile uint16_t pwm_release_count;
static volatile uint16_t pwm_count_helper;

typedef enum
{
    PWM_ATTACK,
    PWM_DECAY,
    PWM_SUSTAIN,
    PWM_RELEASE,
    PWM_DONE
} PWM_STATE_T;

static volatile PWM_STATE_T pwm_state;

// Note to frequency conversion table. Note, frequency is multiplied by 10 for precision.
const uint32_t frequency_table[NOTE_COUNT] =
{
    2616, 2772, 2937, 3111, 3296, 3492, 3700, 3920, 4153, 4400, 4662, 4939,
    5233, 5544, 5873, 6223, 6593, 6985, 7400, 7840, 8306, 8800, 9323, 9878,
    10465,11087,11747,12445,13185,13969,14800,15680,16612,17600,18647,19755,
    20930,22175,23493,24890,26370,27938,29600,31360,33224,35200,37293,39511,
    41860,44349,46986,49780,52740,55877,59199,62719,66449,70400,74586,79021
};

void TIM1_BRK_TIM9_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM9, TIM_IT_Update) != RESET)
    {
        switch (pwm_state)
        {
        case PWM_ATTACK:
            if (pwm_attack_count-- > 0)
                TIM_SetCompare2(TIM9, pwm_value / (pwm_attack_count + 1));
            else
            {
                pwm_count_helper = 0;
                pwm_state++;
            }
            break;
        case PWM_DECAY:
            if (pwm_count_helper++ < pwm_decay_count)
                TIM_SetCompare2(TIM9, pwm_value / pwm_count_helper);
            else
            {
                pwm_value = pwm_value / (pwm_count_helper - 1);
                pwm_state++;
            }
            break;
        case PWM_SUSTAIN:
            if (pwm_sustain_count-- == 0)
            {
                pwm_count_helper = 0;
                pwm_state++;
            }
            break;
        case PWM_RELEASE:
            if (pwm_count_helper++ < pwm_release_count)
                TIM_SetCompare2(TIM9, pwm_value / pwm_count_helper);
            else
            {
                pwm_state = PWM_DONE;
                TIM_CCxCmd(TIM9, TIM_Channel_2, TIM_CCx_Disable);
            }
            break;
        case PWM_DONE:
            TIM_Cmd(TIM9, DISABLE);
            break;
        default:
            break;
        }
        TIM_ClearITPendingBit(TIM9, TIM_IT_Update);
    }
}


void PIEZO_Play(const PIEZO_TONE_T *tone)
{
    pwm_length = (65536UL * PIEZO_PWM_BASE_FREQ * 10) / frequency_table[tone->note];
    pwm_value = ((uint32_t)pwm_length * tone->volume)>>9;   //volume 0...255 -> dutycycle 0...50%

    pwm_attack_count    = frequency_table[tone->note] * tone->attack / 10000;
    pwm_decay_count     = frequency_table[tone->note] * tone->decay / 10000;
    pwm_sustain_count   = frequency_table[tone->note] * tone->sustain / 10000;
    pwm_release_count   = frequency_table[tone->note] * tone->release / 10000;

    pwm_state = PWM_ATTACK;

    TIM_Cmd(TIM9, ENABLE);
    TIM_SetAutoreload(TIM9, pwm_length);
    TIM_SetCompare2(TIM9, 0);
    TIM_CCxCmd(TIM9, TIM_Channel_2, TIM_CCx_Enable);
}

void PIEZO_Stop(void)
{
    TIM_CCxCmd(TIM9, TIM_Channel_2, TIM_CCx_Disable);
    TIM_Cmd(TIM9, DISABLE);
}


void PIEZO_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);

    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);

    /***** Time Base configuration *****/
    TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t) (SystemCoreClock / (65536 * PIEZO_PWM_BASE_FREQ));     //todo [stm32f2xx] check new prescaler and impacts
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;

    TIM_TimeBaseInit(TIM9, &TIM_TimeBaseStructure);

    /***** PWM generation configuration *****/
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0x0;

    // Timer 9 for Buzzer
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC2Init(TIM9, &TIM_OCInitStructure);

    //TIM_OC4PreloadConfig(TIM9, TIM_OCPreload_Enable);
    //TIM_ARRPreloadConfig(TIM9, ENABLE);

    TIM_ITConfig(TIM9, TIM_IT_Update, ENABLE);

    TIM_CCxCmd(TIM9, TIM_Channel_2, TIM_CCx_Disable);
    TIM_Cmd(TIM9, DISABLE);
}

