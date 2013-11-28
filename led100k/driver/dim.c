
#include "stdheader.h"
#include "dim.h"

#include "adc.h"
#include "hw_configuration.h"
#include "version.h"
#include "watchdog.h"

#include "debug.h"

/******************************************
 * DIM CONTROL
 *
 * Purpose:
 * Generate PWM for ...
 *
 * Input:
 * DimTargetValue = PWM Value
 *
 * Output:
 * Timer PWM Control
 *
 *
 */

/* Private type definitions --------------------------------------------------*/


static volatile uint8_t __isr_adc_channel = ADC_LED_CHANNEL_FIRST;

/* Private variables ---------------------------------------------------------*/

static DIM_VALUE_T pwm_value;

/* Parameters ----------------------------------------------------------------*/

#define DIM_PWM_FREQ    250
#define DIM_PWM_MAX_VAL 65535

/* Private functions ---------------------------------------------------------*/

void DIM_LED_Enable(void);


/* Implementations -----------------------------------------------------------*/


void DIM_Set(DIM_VALUE_T value)
{
    ADC_DisableRegularMeasurements();
    pwm_value = value;
}

void DIM_Reset(void)
{
    pwm_value = 0;
    ADC_EnableRegularMeasurements();
}



void DIM_LED_Enable(void)
{
#ifndef DIM_NEVER_EVER_ENABLE_LEDS
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV1_PIN, Bit_SET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV2_PIN, Bit_SET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV3_PIN, Bit_SET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV4_PIN, Bit_SET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV5_PIN, Bit_SET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV6_PIN, Bit_SET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV7_PIN, Bit_SET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV8_PIN, Bit_SET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV9_PIN, Bit_SET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV10_PIN, Bit_SET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV11_PIN, Bit_SET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV12_PIN, Bit_SET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV13_PIN, Bit_SET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV14_PIN, Bit_SET);
#endif
}

void DIM_LED_Disable(void)
{
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV1_PIN, Bit_RESET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV2_PIN, Bit_RESET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV3_PIN, Bit_RESET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV4_PIN, Bit_RESET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV5_PIN, Bit_RESET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV6_PIN, Bit_RESET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV7_PIN, Bit_RESET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV8_PIN, Bit_RESET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV9_PIN, Bit_RESET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV10_PIN, Bit_RESET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV11_PIN, Bit_RESET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV12_PIN, Bit_RESET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV13_PIN, Bit_RESET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_SV14_PIN, Bit_RESET);
}

void TIM2_IRQHandler(void)
{
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}

void TIM4_IRQHandler(void)
{
#define DIM_ADC_MEASURE_PWM_VALUE_MIN (1*(DIM_PWM_MAX_VAL/100))       // 1% of PWM max level
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
    {
//        TPIN_TOGGLE(1);

        TIM_SetCompare1(TIM2, pwm_value);

        if (pwm_value > DIM_ADC_MEASURE_PWM_VALUE_MIN)
        {
            TIM_ClearITPendingBit(TIM4, TIM_IT_CC4);
            ADC_Arm(__isr_adc_channel);
            TIM_SetCompare4(TIM4, pwm_value/2);
        }

        __isr_adc_channel++;

        if (__isr_adc_channel > ADC_LED_CHANNEL_LAST)
            __isr_adc_channel = ADC_LED_CHANNEL_FIRST;

        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    }

    if (TIM_GetITStatus(TIM4, TIM_IT_CC4) != RESET)
    {
        TIM_ClearITPendingBit(TIM4, TIM_IT_CC4);
    }
}

void DIM_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);

    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);

    // disable all LED stages
    DIM_LED_Disable();

    pwm_value = 0;

    // --------------------------------------------------------------------------------
    // TIM2 and TIM3 configuration: generate 2 * 4 PWM signals with same
    // frequency but different duty cycles:
    // The frequency of TIM2 and TIM3 depends on the frequency of APB1, APB1 prescaler,
    // the prescaler of TIM2 respectively TIM3, and the value of TIM_Period.
    // The frequency of APB1 depends on the SystemCoreClock and the APB1 prescaler.
    // Both are defined in "system_stm32f2xx.c".
    // Calculating the value of the prescaler for TIM2 and TIM3:
    // The frequency of the pwm should be about 250 hz.
    // The complete range of 16 bit should be used for counting.
    // System frequency is 120 MHz, prescaler of APB1 is 4 so clock to timer is 60 MHz.
    // Prescaler = (60 MHz / 250 * 65536) = 3.66, so the prescaler should be 4.
    // To achieve this the prescaler has to be set to 3 (see documentation for details).
    // --------------------------------------------------------------------------------

    // Time Base configuration
    TIM_TimeBaseStructure.TIM_Prescaler = DIM_PRESCALER_VALUE;  // note comment above
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;

    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    // PWM generation configuration
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0x0;

    // Timer 2 for WHITE
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);

    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC2Init(TIM2, &TIM_OCInitStructure);

    // Timer 4 for ADC Conversion Trigger
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OC4Init(TIM4, &TIM_OCInitStructure);


    /* Select the Master Slave Mode */
    TIM_SelectMasterSlaveMode(TIM2, TIM_MasterSlaveMode_Enable);

    /* Master Mode selection */
    TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);

    /* Slave Mode selection: TIM4 */
    TIM_SelectSlaveMode(TIM4, TIM_SlaveMode_Reset);
    TIM_SelectInputTrigger(TIM4, TIM_TS_ITR1);

    TIM_Cmd(TIM2, ENABLE);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM4, ENABLE);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

    TIM_CCxCmd(TIM2, TIM_Channel_1, TIM_CCx_Enable);

    DIM_LED_Enable();
}
