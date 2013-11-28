
#include "stdheader.h"
#include "debug_freertos.h"
#include "adc.h"
#include "debug.h"
#include "config.h"
#include "primestruct.h"

#include "hw_configuration.h"
#include "aguard.h"


static volatile uint8_t i_act_led_ch_index;
static volatile uint16_t u_circular_buffer[ADC_CHANNEL_CIRC_COUNT];

static xQueueHandle xQueueAdcLedVoltageMeasurements;

float ADC_CalcTemperature(uint16_t adc_value);


/**
  * @brief  Function to calculate temperature of NCP18 thermistor.
  * @param  adc_value: value from adc
  * @retval floating point temperature value
  */
float ADC_CalcTemperature(uint16_t adc_value)
{
    float calculated_temperature = 20.0f;

    // to avoid division by 0 in calculation below
    if (adc_value >= ADC_MAX_VALUE)
        adc_value = ADC_MAX_VALUE - 1;

#ifdef HWP_ANALOG_TEMP_SENSOR_NCP18
    double calculated_resistance = ((float)adc_value * SENSORS_TEMPERATURE_SENSOR_PULL_UP_R_VALUE) / ((float)(ADC_MAX_VALUE - adc_value));

    calculated_temperature = SENSORS_TEMPERATURE_SENSOR_B_CONSTANT * SENSORS_TEMPERATURE_SENSOR_REFERENCE_TEMPERATURE /
                             (SENSORS_TEMPERATURE_SENSOR_REFERENCE_TEMPERATURE * (log(calculated_resistance) - log(SENSORS_TEMPERATURE_SENSOR_REFERENCE_RESISTANCE)) + SENSORS_TEMPERATURE_SENSOR_B_CONSTANT);

    calculated_temperature -= SENSORS_TEMPERATURE_SENSOR_KELVIN_0_DEGREE_CELSIUS;
#endif

#ifdef HWP_ANALOG_TEMP_SENSOR_MCP9700
    calculated_temperature = (float) (((adc_value * ADC_MAX_COUNT_VOLTAGE / ADC_MAX_VALUE) - SENSORS_MCP9700_VOLTAGE_ZERO_DEGREE) / SENSORS_MCP9700_DELTA_VOLTAGE_PER_DEGREE);
#endif

    return  calculated_temperature;
}

const adc_channel_s adc_channel_list[ADC_LED_CH_INDEX_COUNT] =
{
    //CHANNEL            FACTOR                                                LED_TYPE
#ifdef HWP_LED_WHITE_20K
    {    ADC_Channel_8,     ADC_COUNT_TO_LED_VOLTAGE_FACTOR_HW_REV_2_4_WHITE,   LED_TYPE_CREE_XMLAWT_LT60E3,        LED_COLOR_WHITE,    "WHITE 1"},
    {    ADC_Channel_9,     ADC_COUNT_TO_LED_VOLTAGE_FACTOR_HW_REV_2_4_WHITE,   LED_TYPE_CREE_XMLAWT_LT60E3,        LED_COLOR_WHITE,    "WHITE 2"},
    {    ADC_Channel_2,     ADC_COUNT_TO_LED_VOLTAGE_FACTOR_HW_REV_2_4_WHITE,   LED_TYPE_CREE_XMLAWT_LT60E3,        LED_COLOR_WHITE,    "WHITE 3"},
    {    ADC_Channel_3,     ADC_COUNT_TO_LED_VOLTAGE_FACTOR_HW_REV_2_4_WHITE,   LED_TYPE_CREE_XMLAWT_LT60E3,        LED_COLOR_WHITE,    "WHITE 4"},
#endif
#ifdef HWP_LED_RED_UNI
    {    ADC_Channel_11,    ADC_COUNT_TO_LED_VOLTAGE_FACTOR_HW_REV_2_4_RED,     LED_TYPE_LUMILED_LXM2_PH01_0070,    LED_COLOR_RED,      "RED 1"},
    {    ADC_Channel_12,    ADC_COUNT_TO_LED_VOLTAGE_FACTOR_HW_REV_2_4_RED,     LED_TYPE_LUMILED_LXM2_PH01_0070,    LED_COLOR_RED,      "RED 2"},
    {    ADC_Channel_13,    ADC_COUNT_TO_LED_VOLTAGE_FACTOR_HW_REV_2_4_RED,     LED_TYPE_LUMILED_LXM2_PH01_0070,    LED_COLOR_RED,      "RED 3"},
    {    ADC_Channel_14,    ADC_COUNT_TO_LED_VOLTAGE_FACTOR_HW_REV_2_4_RED,     LED_TYPE_LUMILED_LXM2_PH01_0070,    LED_COLOR_RED,      "RED 4"},
#endif
#ifdef HWP_LED_RED_2K
#ifdef HWT_MEDIUM_RED  // fail detection using voltage at dimming FET (measure during pwm low time)
    {   ADC_Channel_10,     ADC_COUNT_TO_VOLTAGE_FACTOR,                        LED_TYPE_LUMILED_LXM2_PH01_0070,    LED_COLOR_RED,      "RED C1"},
    {   ADC_Channel_15,     ADC_COUNT_TO_VOLTAGE_FACTOR,                        LED_TYPE_LUMILED_LXM2_PH01_0070,    LED_COLOR_RED,      "RED C2"},
#else  // normal voltage measurement
    {   ADC_Channel_10,     ADC_COUNT_TO_LED_VOLTAGE_FACTOR_HW_REV_2_4_RED,     LED_TYPE_LUMILED_LXM2_PH01_0070,    LED_COLOR_RED,      "RED C1"},
    {   ADC_Channel_15,     ADC_COUNT_TO_LED_VOLTAGE_FACTOR_HW_REV_2_4_RED,     LED_TYPE_LUMILED_LXM2_PH01_0070,    LED_COLOR_RED,      "RED C2"},
#endif
#endif
};

void (*powerloss_callback)(void);

/**
  * @brief  This function returns adc voltage factor of selected led channel.
  * @param  channel: selected led channel
  * @retval None
  */
float ADC_GetLedAdcFactor(ADC_LED_CH_INDEX_T channel)
{
    float fl_factor = 0;

#ifdef HWP_LOCAL_LEDS
    if (channel >= ADC_LED_CH_INDEX_COUNT)
        return 0;

#if defined(HWT_MULTI) || defined (HWT_MEDIUM_WHITE)
    // hardware id sirius
    HW_REV_T hw_rev = VERSION_GetHWRev();
    switch (hw_rev)
    {
    case HW_REV_2_3:
        fl_factor = ADC_COUNT_TO_LED_VOLTAGE_FACTOR_BEFORE_HW_REV_2_4;
        break;
    case HW_REV_2_4:
        fl_factor = adc_channel_list[channel].adc_count_to_led_voltage_factor_hw_rev_2_4;
        break;
    default:
        fl_factor = 0;
        break;
    }
#elif defined HWT_MEDIUM_RED
    // hardware id 2000 cd
    fl_factor = adc_channel_list[channel].adc_count_to_led_voltage_factor_hw_rev_2_4;
#else
#error "Hardware type not supported!"
#endif

#endif

    return fl_factor;
}

/**
  * @brief  This function handles ADC1 and ADC2 global interrupts requests.
  * @param  None
  * @retval None
  */
void ADC_IRQHandler(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if (ADC_GetITStatus(ADC1, ADC_IT_EOC) != RESET)
    {
#ifdef HWP_LOCAL_LEDS
        if (i_act_led_ch_index < ADC_LED_NUM_CHANNELS)
        {
            // get value from ADC1
            ADC_LED_VOLTAGE_QUEUE_ITEM_T adc_queue_item;

            adc_queue_item.adc_count = ADC_GetConversionValue(ADC1);
            adc_queue_item.led_channel = i_act_led_ch_index;

            xQueueSendFromISR(xQueueAdcLedVoltageMeasurements, &adc_queue_item, &xHigherPriorityTaskWoken);
        }
#endif
        ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);

        // Disable ADC1 EOC interupt
        ADC_ITConfig(ADC1, ADC_IT_EOC, DISABLE);
    }
    else if (ADC_GetITStatus(ADC1, ADC_IT_AWD) != RESET)
    {
        // Clear ADC1 AWD pending interrupt bit
        ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);
    }
    if (ADC_GetITStatus(ADC3, ADC_IT_AWD) != RESET)
    {
        if (powerloss_callback != NULL)
            powerloss_callback();
        // Clear ADC3 AWD pending interrupt bit
        ADC_ClearITPendingBit(ADC3, ADC_IT_AWD);
    }

    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}


/**
  * @brief  Set function pointer that will be called if power loss has been detected by ADC Watchdog
  * @param  ptr to handler function
  * @retval None
  */
void ADC_SetPowerlossCallback(void * pFunc)
{
    powerloss_callback = pFunc;
}


/**
  * @brief  Arm ADC channel 1. An external interrupt will pull the trigger..
  * @param  uint8_t adc_channel
  * @retval None
  */
void ADC_Arm(uint8_t adc_ch_index)
{
#ifdef HWP_LOCAL_LEDS
    if (adc_ch_index > ADC_LED_CH_INDEX_COUNT - 1)
        return;

    // Clear possible pending interrupt. This is really necessary.
    ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);

    // ADC1 regular channels configuration
    ADC_RegularChannelConfig(ADC1, adc_channel_list[adc_ch_index].adc_channel, 1, ADC_SampleTime_480Cycles);
    i_act_led_ch_index = adc_ch_index;

    // Enable ADC1 EOC interupt
    ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
#endif
}

/**
  * @brief  Calculate led voltages and check if leds are broken.
  * @param  None
  * @retval None
  */
void ADC_Driver_HandleLedVoltages(void)
{
#if defined(HWP_LOCAL_LEDS)
    while (uxQueueMessagesWaiting(xQueueAdcLedVoltageMeasurements) > 0)
    {
        ADC_LED_VOLTAGE_QUEUE_ITEM_T adc_queue_item;

        if (xQueueReceive(xQueueAdcLedVoltageMeasurements, &adc_queue_item, 0) == pdTRUE)
        {
            _primestruct.sensors.analog.led[adc_queue_item.led_channel].value = adc_queue_item.adc_count * ADC_GetLedAdcFactor(adc_queue_item.led_channel);
            _primestruct.sensors.analog.led[adc_queue_item.led_channel].status = SENSOR_OK;

            AGUARD_AddLedVoltage(adc_queue_item.led_channel, _primestruct.sensors.analog.led[adc_queue_item.led_channel].value);
        }
    }
#endif
}

/**
  * @brief  Driver Tick.
  * @param  None
  * @retval None
  */
void ADC_Driver_Tick(void)
{
    ADC_Driver_HandleLedVoltages();

    _primestruct.sensors.analog.supply[PS_ANALOG_VOLTAGE_MAIN_24V].value = ADC_COUNT_TO_PWR24V_FACTOR * u_circular_buffer[ADC_CIRCULAR_VALUE_24V_E];
#if defined(HWT_MULTI)
    _primestruct.sensors.led_temperature[PS_LED_TEMPERATURE_1].value = ADC_CalcTemperature(u_circular_buffer[ADC_CIRCULAR_VALUE_TEMP1_E]);
    _primestruct.sensors.led_temperature[PS_LED_TEMPERATURE_2].value = ADC_CalcTemperature(u_circular_buffer[ADC_CIRCULAR_VALUE_TEMP2_E]);
    _primestruct.sensors.led_temperature[PS_LED_TEMPERATURE_3].value = ADC_CalcTemperature(u_circular_buffer[ADC_CIRCULAR_VALUE_TEMP3_E]);
    _primestruct.sensors.led_temperature[PS_LED_TEMPERATURE_4].value = ADC_CalcTemperature(u_circular_buffer[ADC_CIRCULAR_VALUE_TEMP4_E]);
#elif defined(HWT_MEDIUM_RED)
    _primestruct.sensors.led_temperature[PS_LED_TEMPERATURE_1].value = ADC_CalcTemperature(u_circular_buffer[ADC_CIRCULAR_VALUE_TEMP2_E]);
#elif defined(HWT_MEDIUM_WHITE)
    _primestruct.sensors.led_temperature[PS_LED_TEMPERATURE_1].value = ADC_CalcTemperature(u_circular_buffer[ADC_CIRCULAR_VALUE_TEMP1_E]);
    _primestruct.sensors.led_temperature[PS_LED_TEMPERATURE_2].value = ADC_CalcTemperature(u_circular_buffer[ADC_CIRCULAR_VALUE_TEMP3_E]);
#endif
}

/**
  * @brief  Create queue for adc (used for captured led voltages).
  * @param  None
  * @retval None
  */
void ADC_Driver_CreateQueue(void)
{
    xQueueAdcLedVoltageMeasurements = xQueueCreate(ADC_LED_VOLTAGE_MEASUREMENTS_QUEUE_SIZE, sizeof(ADC_LED_VOLTAGE_QUEUE_ITEM_T));
    DQueueAddToRegistry(xQueueAdcLedVoltageMeasurements, "xQueueAdcLedVoltageMeasurements");
    assert_param(xQueueAdcLedVoltageMeasurements != NULL);
}

/**
  * @brief  Initializes adc.
  * @param  None
  * @retval None
  */
void ADC_Driver_Init(void)
{
    powerloss_callback = NULL;

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
    ADC_StructInit(&ADC_InitStructure);

    // ADC1 configuration
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T4_CC4;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 1;

    ADC_Init(ADC1, &ADC_InitStructure);

    // Disable ADC1 EOC interupt
    ADC_ITConfig(ADC1, ADC_IT_EOC, DISABLE);

    // Enable ADC1
    ADC_Cmd(ADC1, ENABLE);

    // ADC 3 Configuration
    // ADC3 DMA configuration
    DMA_InitTypeDef DMA_InitStructure;
    DMA_StructInit(&DMA_InitStructure);

    // DMA2 Stream0 channel2 configuration
//    DMA_DeInit(DMA_Channel_2);
    DMA_InitStructure.DMA_Channel = DMA_Channel_2;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &ADC3->DR;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) u_circular_buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitStructure.DMA_BufferSize = ADC_CHANNEL_CIRC_COUNT;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA2_Stream0, &DMA_InitStructure);
    DMA_Cmd(DMA2_Stream0, ENABLE);

    // ADC3 configuration
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = ADC_CHANNEL_CIRC_COUNT;
    ADC_Init(ADC3, &ADC_InitStructure);

    // ADC3 regular channel14 configuration
    uint8_t i_rank = 1;
    ADC_RegularChannelConfig(ADC3, ADC_CHANNEL_CIRC1, i_rank++, ADC_SampleTime_480Cycles);
#ifdef ADC_CHANNEL_CIRC2
    ADC_RegularChannelConfig(ADC3, ADC_CHANNEL_CIRC2, i_rank++, ADC_SampleTime_480Cycles);
#endif
#ifdef ADC_CHANNEL_CIRC3
    ADC_RegularChannelConfig(ADC3, ADC_CHANNEL_CIRC3, i_rank++, ADC_SampleTime_480Cycles);
#endif
#ifdef ADC_CHANNEL_CIRC4
    ADC_RegularChannelConfig(ADC3, ADC_CHANNEL_CIRC4, i_rank++, ADC_SampleTime_480Cycles);
#endif
#ifdef ADC_CHANNEL_CIRC5
    ADC_RegularChannelConfig(ADC3, ADC_CHANNEL_CIRC5, i_rank++, ADC_SampleTime_480Cycles);
#endif

    // Configure high and low analog watchdog thresholds
    ADC_AnalogWatchdogThresholdsConfig(ADC3, ADCWD_CHANNEL_U_24_V_HIGHTHRESHOLD, ADCWD_CHANNEL_U_24_V_LOWTHRESHOLD);
    // Configure channel4 as the single analog watchdog guarded channel
    ADC_AnalogWatchdogSingleChannelConfig(ADC3, ADC_Channel_4);
    // Enable analog watchdog on one regular channel
    ADC_AnalogWatchdogCmd(ADC3, ADC_AnalogWatchdog_SingleRegEnable);

    // Enable AWD interupt
    ADC_ITConfig(ADC3, ADC_IT_AWD, ENABLE);

    // Enable ADC3 DMA
    ADC_DMACmd(ADC3, ENABLE);

    // Enable ADC3
    ADC_Cmd(ADC3, ENABLE);
    // Enable DMA request after last transfer (Single-ADC mode)
    ADC_DMARequestAfterLastTransferCmd(ADC3, ENABLE);
    // Start ADC3 Software Conversion
    ADC_SoftwareStartConv(ADC3);
}
