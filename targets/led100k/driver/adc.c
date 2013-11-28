
#include "stdheader.h"
#include "adc.h"
#include "debug.h"

#include "hw_configuration.h"


typedef struct
{
    uint8_t adc_channel;
    uint8_t adc_switch;
    float   (*adc2value_function)(float);
} ADC_MATRIX_T;

float adc2value_led_voltage(float raw);
float adc2value_pwr24v(float raw);
float adc2value_pwrvcc(float raw);
float adc2value_temperature(float raw);
float adc2value_void(float raw);

const ADC_MATRIX_T adc_matrix[ADC_CH_COUNT] =
{
        { ADC_U1,     ADC_SW1,    adc2value_led_voltage }, // ADC_CH_SV_1
        { ADC_U1,     ADC_SW2,    adc2value_led_voltage }, // ADC_CH_SV_2
        { ADC_U2,     ADC_SW1,    adc2value_led_voltage }, // ADC_CH_SV_3
        { ADC_U2,     ADC_SW2,    adc2value_led_voltage }, // ADC_CH_SV_4
        { ADC_U3,     ADC_SW1,    adc2value_led_voltage }, // ADC_CH_SV_5
        { ADC_U3,     ADC_SW2,    adc2value_led_voltage }, // ADC_CH_SV_6
        { ADC_U4,     ADC_SW1,    adc2value_led_voltage }, // ADC_CH_SV_7
        { ADC_U4,     ADC_SW2,    adc2value_led_voltage }, // ADC_CH_SV_8
        { ADC_U1,     ADC_SW3,    adc2value_led_voltage }, // ADC_CH_SV_9
        { ADC_U1,     ADC_SW4,    adc2value_led_voltage }, // ADC_CH_SV_10
        { ADC_U2,     ADC_SW3,    adc2value_led_voltage }, // ADC_CH_SV_11
        { ADC_U2,     ADC_SW4,    adc2value_led_voltage }, // ADC_CH_SV_12
        { ADC_U3,     ADC_SW3,    adc2value_led_voltage }, // ADC_CH_SV_13
        { ADC_U3,     ADC_SW4,    adc2value_led_voltage }, // ADC_CH_SV_14

        { ADC_U4,     ADC_SW3,    adc2value_pwr24v }, // ADC_CH_24V
        { ADC_U4,     ADC_SW4,    adc2value_pwrvcc }, // ADC_CH_VCC

        { ADC_T1,     ADC_SW1,    adc2value_temperature }, // ADC_CH_TEMP_1
        { ADC_T1,     ADC_SW2,    adc2value_temperature }, // ADC_CH_TEMP_2
        { ADC_T2,     ADC_SW1,    adc2value_temperature }, // ADC_CH_TEMP_3
        { ADC_T2,     ADC_SW2,    adc2value_temperature }, // ADC_CH_TEMP_4
        { ADC_T3,     ADC_SW1,    adc2value_temperature }, // ADC_CH_TEMP_5
        { ADC_T3,     ADC_SW2,    adc2value_temperature }, // ADC_CH_TEMP_6
        { ADC_T4,     ADC_SW1,    adc2value_temperature }, // ADC_CH_TEMP_7
        { ADC_T4,     ADC_SW2,    adc2value_temperature }, // ADC_CH_TEMP_8
        { ADC_T1,     ADC_SW3,    adc2value_temperature }, // ADC_CH_TEMP_9
        { ADC_T1,     ADC_SW4,    adc2value_temperature }, // ADC_CH_TEMP_10
        { ADC_T2,     ADC_SW3,    adc2value_temperature }, // ADC_CH_TEMP_11
        { ADC_T2,     ADC_SW4,    adc2value_temperature }, // ADC_CH_TEMP_12
        { ADC_T3,     ADC_SW3,    adc2value_temperature }, // ADC_CH_TEMP_13
        { ADC_T3,     ADC_SW4,    adc2value_temperature }, // ADC_CH_TEMP_14

        { ADC_T4,     ADC_SW4,    adc2value_void}   // ADC_CH_TEMP_BOARD
};

volatile ADC_SENSOR_T adc_sensors[ADC_CH_COUNT];
volatile int8_t i_act_ch_index;
static bool bRegularMeasurements;




float adc2value_led_voltage(float raw)
{
    //Factor to get LED supply voltage in mV from ADC counts
    // U_led = ADC * ADC2VALUE_FACTOR_LED_VOLTAGE
    // ADC2VALUE_FACTOR_LED_VOLTAGE = (Vref/4095) * v
    // U_led = (ADC * (Vref/4095) * v)
    // with Vref=3120mV, factor v=((22kOhm+470kOhm) / 22kOhm)

    return raw * 0.017038961f;
}

float adc2value_pwr24v(float raw)
{
    //Factor to get 24V Pwr level U_pwr in mV from ADC counts
    // U_pwr = ADC * ADC_COUNT_TO_PWR24V_FACTOR
    // ADC2VALUE_FACTOR_PWR24V = (Vref/4095) * v
    // U_pwr = (ADC * (Vref/4095) * v)
    // with Vref=3120mV and factor v=((220kOhm+22kOhm) / 22kOhm)

    return raw * 0.00838095f;
}

float adc2value_pwrvcc(float raw)
{
    //Factor to get VCC Pwr level U_pwr in mV from ADC counts
    // U_pwr = ADC * ADC2VALUE_FACTOR_PWRVCC
    // ADC2VALUE_FACTOR_PWRVCC = (Vref/4095) * v
    // U_pwr = (ADC * (Vref/4095) * v)
    // with Vref=3120mV and factor v=((22kOhm+22kOhm) / 22kOhm)
    return raw * 0.00152381f;
}

float adc2value_temperature(float raw)
{
#define TEMPERATURE_ADC_MAX_COUNT           ADC_MAX_VALUE
#define TEMPERATURE_ADC_MAX_COUNT_VOLTAGE   3.12f
#define MCP9701_VOLTAGE_ZERO_DEGREE         0.4F
#define MCP9701_DELTA_VOLTAGE_PER_DEGREE    0.0195f

    return (float) (((raw * TEMPERATURE_ADC_MAX_COUNT_VOLTAGE / TEMPERATURE_ADC_MAX_COUNT) - MCP9701_VOLTAGE_ZERO_DEGREE) / MCP9701_DELTA_VOLTAGE_PER_DEGREE);
}

float adc2value_void(float raw)
{
    return raw;
}



/**
  * @brief  This function handles ADC1 and ADC2 global interrupts requests.
  * @param  None
  * @retval None
  */
void ADC_IRQHandler(void)
{
    if (ADC_GetITStatus(ADC1, ADC_IT_EOC) != RESET)
    {
        if (i_act_ch_index >= 0)
        {
            adc_sensors[i_act_ch_index].value = adc_matrix[i_act_ch_index].adc2value_function(ADC_GetConversionValue(ADC1));
            adc_sensors[i_act_ch_index].status = SENSOR_VALUE_AVAILABLE;
            i_act_ch_index = -1;
        }
        ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);

        // Disable ADC1 EOC interupt
        ADC_ITConfig(ADC1, ADC_IT_EOC, DISABLE);
    }

    if (ADC_GetITStatus(ADC2, ADC_IT_EOC) != RESET)
    {
        if (i_act_ch_index >= 0)
        {
            adc_sensors[i_act_ch_index].value = adc_matrix[i_act_ch_index].adc2value_function(ADC_GetConversionValue(ADC2));
            adc_sensors[i_act_ch_index].status = SENSOR_VALUE_AVAILABLE;
            i_act_ch_index = -1;
        }
        ADC_ClearITPendingBit(ADC2, ADC_IT_EOC);

        // Disable ADC2 EOC interupt
        ADC_ITConfig(ADC2, ADC_IT_EOC, DISABLE);
    }
}

/**
  * @brief  Initializes adc.
  * @param  None
  * @retval None
  */
void ADC_Driver_Init(void)
{
    for(uint8_t i = 0; i < ADC_CH_COUNT; i++)
    {
        adc_sensors[i].value = -1.0;
        adc_sensors[i].status = SENSOR_WAIT_FOR_INIT;
    }

    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_CommonStructInit(&ADC_CommonInitStructure);

    // ADC Common Init
    ADC_DeInit();
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);

    ADC_InitTypeDef ADC_InitStructure;

    // ADC1 configuration
    // ADC1 is used for LED power supply voltage measurements
    ADC_StructInit(&ADC_InitStructure);
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T4_CC4;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 1;

    ADC_Init(ADC1, &ADC_InitStructure);

    // ADC2 configuration
    // ADC2 is used for temperature and other measurements
    ADC_StructInit(&ADC_InitStructure);
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 1;

    ADC_Init(ADC2, &ADC_InitStructure);

    // Disable ADC EOC interupt
    ADC_ITConfig(ADC1, ADC_IT_EOC, DISABLE);
    ADC_ITConfig(ADC2, ADC_IT_EOC, DISABLE);

    // Enable ADC
    ADC_Cmd(ADC1, ENABLE);
    ADC_Cmd(ADC2, ENABLE);

    i_act_ch_index = -1;
    bRegularMeasurements = TRUE;
}


void ADC_SetChannel(uint8_t adc_ch_index)
{
    // Setup analog switch matrix
    GPIO_WriteBit(ANALOG_SWITCH_PORT, ADC_SW1, Bit_RESET);
    GPIO_WriteBit(ANALOG_SWITCH_PORT, ADC_SW2, Bit_RESET);
    GPIO_WriteBit(ANALOG_SWITCH_PORT, ADC_SW3, Bit_RESET);
    GPIO_WriteBit(ANALOG_SWITCH_PORT, ADC_SW4, Bit_RESET);
    GPIO_WriteBit(ANALOG_SWITCH_PORT, adc_matrix[adc_ch_index].adc_switch, Bit_SET);
}

/**
  * @brief  Arm ADC channel 1. An external interrupt will pull the trigger..
  * @param  uint8_t adc_channel
  * @retval None
  */

void ADC_Arm(uint8_t adc_ch_index)
{
    //TPIN_ON(0);

    // check if other conversion is running
    if (i_act_ch_index != -1)
        return;

    // check if requested channel is within range
    if (adc_ch_index > ADC_LED_CHANNEL_LAST)
        return;

    // Clear possible pending interrupt. This is really necessary.
    ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
    ADC_ClearITPendingBit(ADC2, ADC_IT_EOC);

    ADC_ITConfig(ADC1, ADC_IT_EOC, DISABLE);
    ADC_ITConfig(ADC2, ADC_IT_EOC, DISABLE);

    i_act_ch_index = adc_ch_index;

    // ADC1 regular channels configuration
    ADC_RegularChannelConfig(ADC1, adc_matrix[adc_ch_index].adc_channel, 1, ADC_SampleTime_480Cycles);

    // Prepare analog channel channel
    ADC_SetChannel(adc_ch_index);

    // Enable ADC1 EOC interupt
    ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
}

void ADC_EnableRegularMeasurements(void)
{
    i_act_ch_index = -1;
    bRegularMeasurements = TRUE;
}

void ADC_DisableRegularMeasurements(void)
{
    i_act_ch_index = -1;
    bRegularMeasurements = FALSE;
}

void ADC_Tick(void)
{
    static uint8_t u_circular_channel = ADC_CONTINUOUS_CHANNEL_FIRST;

    if ((bRegularMeasurements == TRUE) && (i_act_ch_index == -1))
    {
        i_act_ch_index = u_circular_channel;

        ADC_SetChannel(u_circular_channel);

        // ADC2 regular channels configuration
        ADC_RegularChannelConfig(ADC2, adc_matrix[u_circular_channel].adc_channel, 1, ADC_SampleTime_480Cycles);

        // Enable ADC2 EOC interrupt
        ADC_ITConfig(ADC2, ADC_IT_EOC, ENABLE);

        // Start conversion
        ADC_SoftwareStartConv(ADC2);

        if (++u_circular_channel > ADC_CONTINUOUS_CHANNEL_LAST)
            u_circular_channel = ADC_CONTINUOUS_CHANNEL_FIRST;
    }
}


