#ifndef ADC_H
#define ADC_H

#include "config.h"
#include "led_types.h"


enum
{
    ADC_CIRCULAR_VALUE_24V_E = 0,
#if defined(HWT_MULTI)
    ADC_CIRCULAR_VALUE_TEMP1_E,
    ADC_CIRCULAR_VALUE_TEMP2_E,
    ADC_CIRCULAR_VALUE_TEMP3_E,
    ADC_CIRCULAR_VALUE_TEMP4_E,
#elif defined (HWT_MEDIUM_RED)
    ADC_CIRCULAR_VALUE_TEMP2_E,
#elif defined (HWT_MEDIUM_WHITE)
    ADC_CIRCULAR_VALUE_TEMP1_E,
    ADC_CIRCULAR_VALUE_TEMP3_E,
#endif
    ADC_CHANNEL_CIRC_COUNT
};

#define ADC_CHANNEL_CIRC1   ADC_Channel_4

#if defined(HWT_MULTI)
#define ADC_CHANNEL_CIRC2   ADC_Channel_5
#define ADC_CHANNEL_CIRC3   ADC_Channel_6
#define ADC_CHANNEL_CIRC4   ADC_Channel_7
#define ADC_CHANNEL_CIRC5   ADC_Channel_8
#elif defined (HWT_MEDIUM_RED)
#define ADC_CHANNEL_CIRC3   ADC_Channel_6
#elif defined (HWT_MEDIUM_WHITE)
#define ADC_CHANNEL_CIRC2   ADC_Channel_5
#define ADC_CHANNEL_CIRC4   ADC_Channel_7
#endif



#define ADC_MAX_VALUE   4095
#define ADC_MAX_COUNT_VOLTAGE   3.12f

typedef struct
{
    uint8_t     adc_channel;
    float       adc_count_to_led_voltage_factor_hw_rev_2_4;
    LED_TYPE_T  led_type;
    LED_COLOR_T led_color;
    char *      name;
} adc_channel_s;

typedef enum
{
#ifdef HWP_LED_WHITE_20K
    ADC_LED_CH_INDEX_WHITE_A,
    ADC_LED_CH_INDEX_WHITE_B,
    ADC_LED_CH_INDEX_WHITE_C,
    ADC_LED_CH_INDEX_WHITE_D,
#endif
#ifdef HWP_LED_RED_UNI
    ADC_LED_CH_INDEX_RED_1,
    ADC_LED_CH_INDEX_RED_2,
    ADC_LED_CH_INDEX_RED_3,
    ADC_LED_CH_INDEX_RED_4,
#endif
#ifdef HWP_LED_RED_2K
    ADC_LED_CH_INDEX_RED_C_A,
    ADC_LED_CH_INDEX_RED_C_B,
#endif
    ADC_LED_CH_INDEX_COUNT  // attention: This is used to specify the size of some arrays. Make sure to adjust initialization lists appropriately if the number of items is changed.
} ADC_LED_CH_INDEX_T;

// WARNING: pay attention if ADC_LED_CH_INDEX_COUNT or ADC_LED_NUM_CHANNELS should be used!
#define ADC_LED_CH_INDEX_FIRST  0
#ifdef HWP_LOCAL_LEDS
#define ADC_LED_CH_INDEX_LAST   ADC_LED_CH_INDEX_COUNT-1
#define ADC_LED_NUM_CHANNELS    (ADC_LED_CH_INDEX_LAST - ADC_LED_CH_INDEX_FIRST + 1)
#else
#define ADC_LED_NUM_CHANNELS    0
#endif

#define ADCWD_CHANNEL_U_24_V_HIGHTHRESHOLD 0x0FFE
#define ADCWD_CHANNEL_U_24_V_LOWTHRESHOLD  0x0001

#if defined(HWP_LED_RED_UNI) || defined(HWP_LED_RED_2K)
// wait at least 200 µs, pwm frequency is 60 MHz / 4 / 65536 = 228,88 Hz
// pwm clock is 60 MHz / 4 = 15 MHz ==>  66,7 ns per count
// new 2.4 µs / 66,7 ns = 36
#define ADC_RED_LED_FAIL_DETECTION_MIN_PWM_WAIT_COUNT 36  // counts to wait to give voltage time to rise
#define ADC_RED_LED_FAIL_DETECTION_MIN_PWM 1000  // minimum required pwm for fail detection (below this value the voltage measurement might fail)  // todo[high] find out why
#define ADC_RED_LED_FAIL_DETECTION_MAX_PWM 60000  // maximum allowed pwm for fail detection (above this value the time left for measuring might be insufficient)
#endif

typedef struct
{
    ADC_LED_CH_INDEX_T led_channel;
    uint16_t adc_count;
} ADC_LED_VOLTAGE_QUEUE_ITEM_T;

#define ADC_LED_VOLTAGE_MEASUREMENTS_QUEUE_SIZE 100

//Factor to get LED supply voltage in V from ADC counts
// U_led = ADC * ADC_COUNT_TO_LED_VOLTAGE_FACTOR
// ADC_COUNT_TO_LED_VOLTAGE_FACTOR = (Vref/4095) * v
// U_led = (ADC * (Vref/4095) * v)
// with Vref=3.120V, factor v=((22kOhm+470kOhm) / 22kOhm)
// ADC_COUNT_TO_LED_VOLTAGE_FACTOR = 17.04
#define ADC_COUNT_TO_LED_VOLTAGE_FACTOR_BEFORE_HW_REV_2_4   0.017038961f

//Factor to get LED supply voltage in V from ADC counts
// U_led = ADC * ADC_COUNT_TO_LED_VOLTAGE_FACTOR
// ADC_COUNT_TO_LED_VOLTAGE_FACTOR = (Vref/4095) * v
// U_led = (ADC * (Vref/4095) * v)
// with Vref=3.120V, factor v=((22kOhm+560kOhm) / 22kOhm)
// ADC_COUNT_TO_LED_VOLTAGE_FACTOR = 0.02016
#define ADC_COUNT_TO_LED_VOLTAGE_FACTOR_HW_REV_2_4_RED      0.020155844f

//Factor to get LED supply voltage in V from ADC counts
// U_led = ADC * ADC_COUNT_TO_LED_VOLTAGE_FACTOR
// ADC_COUNT_TO_LED_VOLTAGE_FACTOR = (Vref/4095) * v
// U_led = (ADC * (Vref/4095) * v)
// with Vref=3.120V, factor v=((22kOhm+470kOhm) / 22kOhm)
// ADC_COUNT_TO_LED_VOLTAGE_FACTOR = 0.01704
#define ADC_COUNT_TO_LED_VOLTAGE_FACTOR_HW_REV_2_4_WHITE    0.017038961f

//Factor to get 24V Pwr level U_pwr in V from ADC counts
// U_pwr = ADC * ADC_COUNT_TO_PWR24V_FACTOR
// ADC_COUNT_TO_PWR24V_FACTOR = (Vref/4095) * v
// U_pwr = (ADC * (Vref/4095) * v)
// with Vref=3.120V and factor v=((220kOhm+22kOhm) / 22kOhm)
// ADC_COUNT_TO_PWR24V_FACTOR = 0.00838
#define ADC_COUNT_TO_PWR24V_FACTOR 0.00838f

#ifdef HWT_MEDIUM_RED  // fail detection using voltage at dimming FET (measure during pwm low time)
// Factor to get voltage at input in V from ADC counts
// U = ADC * ADC_COUNT_TO_VOLTAGE_FACTOR
// ADC_COUNT_TO_VOLTAGE_FACTOR = (Vref/4095)
// U = (ADC * (Vref/4095))
// with Vref=3.120V
// ADC_COUNT_TO_VOLTAGE_FACTOR = 0.0007619
#define ADC_COUNT_TO_VOLTAGE_FACTOR 0.0007619f
#endif

#define SENSORS_TEMPERATURE_SENSOR_PULL_UP_R_VALUE          560.0f  // 560 ohms
#define SENSORS_TEMPERATURE_SENSOR_B_CONSTANT               3650.0f
#define SENSORS_TEMPERATURE_SENSOR_REFERENCE_TEMPERATURE    298.16f
#define SENSORS_TEMPERATURE_SENSOR_KELVIN_0_DEGREE_CELSIUS  273.16f
#define SENSORS_TEMPERATURE_SENSOR_REFERENCE_RESISTANCE     1000.0f

#define SENSORS_MCP9700_VOLTAGE_ZERO_DEGREE         0.5F
#define SENSORS_MCP9700_DELTA_VOLTAGE_PER_DEGREE    0.01f

extern const adc_channel_s adc_channel_list[ADC_LED_CH_INDEX_COUNT];

void ADC_Driver_CreateQueue(void);
void ADC_Driver_Init(void);
void ADC_Driver_Tick(void);
void ADC_Arm(uint8_t adc_ch_index);
void ADC_Disarm(void);


#endif
