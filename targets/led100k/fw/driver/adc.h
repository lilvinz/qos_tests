#ifndef ADC_H
#define ADC_H

#include "sensor.h"

#define ADC_MAX_VALUE   4095

typedef enum
{
    ADC_CH_SV_1 = 0,
    ADC_CH_SV_2,
    ADC_CH_SV_3,
    ADC_CH_SV_4,
    ADC_CH_SV_5,
    ADC_CH_SV_6,
    ADC_CH_SV_7,
    ADC_CH_SV_8,
    ADC_CH_SV_9,
    ADC_CH_SV_10,
    ADC_CH_SV_11,
    ADC_CH_SV_12,
    ADC_CH_SV_13,
    ADC_CH_SV_14,

    ADC_CH_24V,
    ADC_CH_VCC,

    ADC_CH_TEMP_1,
    ADC_CH_TEMP_2,
    ADC_CH_TEMP_3,
    ADC_CH_TEMP_4,
    ADC_CH_TEMP_5,
    ADC_CH_TEMP_6,
    ADC_CH_TEMP_7,
    ADC_CH_TEMP_8,
    ADC_CH_TEMP_9,
    ADC_CH_TEMP_10,
    ADC_CH_TEMP_11,
    ADC_CH_TEMP_12,
    ADC_CH_TEMP_13,
    ADC_CH_TEMP_14,

    ADC_CH_TEMP_BOARD,

    ADC_CH_COUNT
}ADC_CH_INDEX;


#define ADC_U1      ADC_Channel_8
#define ADC_U2      ADC_Channel_9
#define ADC_U3      ADC_Channel_10
#define ADC_U4      ADC_Channel_11

#define ADC_T1      ADC_Channel_12
#define ADC_T2      ADC_Channel_13
#define ADC_T3      ADC_Channel_14
#define ADC_T4      ADC_Channel_15

#define ADC_SW1     ANALOG_SWITCH_SW1_PIN
#define ADC_SW2     ANALOG_SWITCH_SW2_PIN
#define ADC_SW3     ANALOG_SWITCH_SW3_PIN
#define ADC_SW4     ANALOG_SWITCH_SW4_PIN

#define ADC_LED_CHANNEL_FIRST           ADC_CH_SV_1
#define ADC_LED_CHANNEL_LAST            ADC_CH_SV_14
#define ADC_LED_CHANNEL_COUNT           (ADC_LED_CHANNEL_FIRST - ADC_LED_CHANNEL_LAST + 1)
#define ADC_CONTINUOUS_CHANNEL_FIRST    ADC_CH_24V
#define ADC_CONTINUOUS_CHANNEL_LAST     ADC_CH_TEMP_BOARD
#define ADC_CONTINUOUS_CHANNEL_COUNT    (ADC_CONTINUOUS_CHANNEL_FIRST - ADC_CONTINUOUS_CHANNEL_LAST + 1)

typedef struct
{
    float value;
    union
    {
        SENSOR_STATUS_T status;
        uint32_t        placeholder;
    };
} ADC_SENSOR_T;

#define ADCWD_CHANNEL_U_24_V_HIGHTHRESHOLD 0x0FFE
#define ADCWD_CHANNEL_U_24_V_LOWTHRESHOLD  0x0001




void ADC_Driver_Init(void);
void ADC_Arm(uint8_t adc_ch_index);
void ADC_Disarm(void);
void ADC_EnableRegularMeasurements(void);
void ADC_DisableRegularMeasurements(void);
void ADC_Tick(void);


#endif
