
#include "stdheader.h"
#include "dim.h"

#include "adc.h"
#include "led_compensation.h"
#include "hw_configuration.h"
#include "version.h"
#include "sensors.h"
#include "heater.h"
#include "watchdog.h"

#include "debug.h"

/******************************************
 * DIM CONTROL
 *
 * Purpose:
 * Generate PWM for ...
 *
 * Input:
 * DimType = log, linear
 * DimTargetValue = PWM Value
 * DimTime = milliseconds
 *
 * Output:
 * Timer PWM Control
 *
 *
 */

/* Private type definitions --------------------------------------------------*/

typedef enum
{
    DIM_HOLD, DIM_UP, DIM_DOWN
} DIM_DIR_T;

typedef struct
{
    DIM_DIR_T dir;
    uint16_t start_value;
    uint16_t target_value;
    uint16_t difference;
    uint16_t steps_overall;
    uint16_t step;
    bool led_top;
} DIM_SITUATION_T;


const char * dim_led_names[DIM_LED_COUNT] =
{
#ifdef HWP_LED_WHITE_20K
    "White",
#endif
#ifdef HWP_LED_RED_UNI
    "Red 1",
    "Red 2",
#endif
#ifdef HWP_LED_RED_2K
    "Red C",
#endif
#ifdef HWP_LED_RED_UNI
    "Red 3",
    "Red 4",
#endif
};

static volatile DIM_PARAMETER_T DimParameter;
static volatile DIM_LED_POWERLEVEL RectifiedLedPowerLevel[DIM_LED_COUNT];
static volatile DIM_LED_POWERLEVEL CurrentLedPWMOutput[DIM_LED_COUNT];

static volatile uint8_t __isr_adc_channel;

/* Private variables ---------------------------------------------------------*/

static DIM_SITUATION_T dim;
static uint32_t led_operating_time[DIM_LED_COUNT];
static uint32_t led_operating_time_temperature[DIM_LED_COUNT];

static xSemaphoreHandle xDimInterruptBSemaphore = NULL;

/* Parameters ----------------------------------------------------------------*/

#define DIM_PWM_FREQ    250
#define DIM_PWM_MAX_VAL 65535
#define DIM_GAMMA       33

/* Private functions ---------------------------------------------------------*/

void DIM_SetStage(uint16_t permill);
void DIM_RectifyPowerLevel(void);
void DIM_LED_Enable(void);


/* Implementations -----------------------------------------------------------*/

/**
 * @brief  Get operating time of selected led.
 * @param  led
 * @retval operating time in ms
 */

const char * DIM_GetLedRowName(DIM_LED_T led)
{
    return dim_led_names[led];
}




uint32_t DIM_GetLedOperatingTime(DIM_LED_T led)
{
    if (led >= DIM_LED_COUNT)
        return 0;

    uint64_t temp = 0;

    temp = ((uint64_t) led_operating_time[led]) * 1000;
    temp /= DIM_PWM_FREQ * DIM_PWM_MAX_VAL;

    led_operating_time[led] = 0;

    return (uint32_t) temp;
}

/**
 * @brief  Get operating time of selected led.
 * @param  led
 * @retval operating time in ms
 */
uint32_t DIM_GetCompensatedLedOperatingTime(DIM_LED_T led)
{
    if (led >= DIM_LED_COUNT)
        return 0;

    uint64_t temp = 0;

    temp = ((uint64_t) led_operating_time_temperature[led]) * 1000;
    temp /= DIM_PWM_FREQ * DIM_PWM_MAX_VAL;

    led_operating_time_temperature[led] = 0;

    return (uint32_t) temp;
}

/**
 * @brief  Set operating time of leds.
 * @param  none
 * @retval none
 */
void DIM_SetLedOperatingTime(void)
{
    //uint32_t temp;

    for (uint8_t ch = 0; ch < DIM_LED_COUNT; ch++)
    {
        led_operating_time[ch] += CurrentLedPWMOutput[ch];
    }

    //TODO: Repair compensation calculation here

#if 0

    // adding temperature compensated time

    temp = LED_COMPENSATION_GetTimemeterCompensation(LED_COMPENSATION_LED_PHLATLIGHT_SST_90_WHITE);  // get factor in %
    temp *= CurrentLedPWMOutput[DIM_LED_WHITE];
    temp /= 100;
    led_operating_time_temperature[DIM_LED_WHITE] += temp;
//    led_operating_time_temperature[DIM_LED_RED_1] += CurrentLedPWMOutput.red_1;
//    led_operating_time_temperature[DIM_LED_RED_2] += CurrentLedPWMOutput.red_2;
//    led_operating_time_temperature[DIM_LED_RED_3] += CurrentLedPWMOutput.red_3;
//    led_operating_time_temperature[DIM_LED_RED_4] += CurrentLedPWMOutput.red_4;
    temp = LED_COMPENSATION_GetTimemeterCompensation(LED_COMPENSATION_LED_LUXEON_LXML_PH01_REDORANGE);  // get factor in %
    temp *= CurrentLedPWMOutput.red_c;
    temp /= 100;
    led_operating_time_temperature[DIM_LED_RED_C] += temp;
#endif
}

/**
 * @brief  Get actual LED power level (value from profile).
 * @param  led
 * @retval power level
 */
uint16_t DIM_GetLedPowerLevel(DIM_LED_T led)
{
    switch (led)
    {
#ifdef HWP_LED_WHITE_20K
    case DIM_LED_WHITE:
        return DimParameter.led_powerlevel[DIM_LED_WHITE];

#endif
#ifdef HWP_LED_RED_UNI
    case DIM_LED_RED_1:
        return DimParameter.led_powerlevel[DIM_LED_RED_1];
    case DIM_LED_RED_2:
        return DimParameter.led_powerlevel[DIM_LED_RED_2];
    case DIM_LED_RED_3:
        return DimParameter.led_powerlevel[DIM_LED_RED_3];
    case DIM_LED_RED_4:
        return DimParameter.led_powerlevel[DIM_LED_RED_4];
#endif
#ifdef HWP_LED_RED_2K
    case DIM_LED_RED_C:
        return DimParameter.led_powerlevel[DIM_LED_RED_C];
#endif
    default:
        return 0;
    }
}

/**
 * @brief  Get rectified LED power level (considers temperature and age).
 * @param  led
 * @retval power level
 */
uint16_t DIM_GetRectifiedLedPowerLevel(DIM_LED_T led)
{
    return RectifiedLedPowerLevel[led];
}

void DIM_Set(DIM_VALUE_T target_value, uint32_t duration)
{


    if (target_value > 1000)
        target_value = 1000;

    if (target_value == dim.target_value)
        return;

    if (target_value > dim.target_value)
    {
        dim.difference = target_value - dim.target_value;
        dim.dir = DIM_UP;
    }
    else if (target_value < dim.target_value)
    {
        dim.difference = dim.target_value - target_value;
        dim.dir = DIM_DOWN;
    }

    //use last target value as new start value
    dim.start_value = dim.target_value;
    dim.target_value = target_value;
    dim.steps_overall = (uint16_t) (((uint32_t) duration * DIM_PWM_FREQ) / 1000);
    dim.step = 0;
    dim.led_top = FALSE;

    DIM_RectifyPowerLevel();
    DIM_LED_Enable();
}

void DIM_SetNow(DIM_LED_POWERLEVEL * pPowerLevel, bool use_compensation, bool use_reduction)
{
    dim.dir = DIM_HOLD;
    dim.led_top = TRUE;
    dim.target_value = 1000;

    for (uint8_t ch = 0; ch < DIM_LED_COUNT; ch++)
        DimParameter.led_powerlevel[ch] = pPowerLevel[ch];

    DimParameter.settings.use_compensation = use_compensation;
    DimParameter.settings.use_reduction = use_reduction;

    DIM_RectifyPowerLevel();

    DIM_LED_Enable();

    DIM_SetStage(1000);
}

void DIM_SetNowRefresh(void)
{
    DIM_RectifyPowerLevel();
    DIM_SetStage(1000);
}

void DIM_SetParameter(const DIM_BMM_DIM_PARAMETER_T * pNewParameter, const uint16_t *reduction, bool bGloomSensorAlwaysOn)
{
    for (uint8_t ch = 0; ch < DIM_LED_COUNT; ch++)
        DimParameter.led_powerlevel[ch] = pNewParameter->led_powerlevel[ch];

    DimParameter.settings.gamma = pNewParameter->settings.gamma;
    DimParameter.settings.reduction = *reduction;
    DimParameter.settings.type = pNewParameter->settings.type;
    DimParameter.settings.use_compensation = DIM_SETTING_USE_LED_COMPENSATION;
    DimParameter.settings.use_reduction = TRUE;  // set to true to do not overwrite setting from profile
    DimParameter.settings.keep_gloom_sensor_on = bGloomSensorAlwaysOn;

    if (DimParameter.settings.keep_gloom_sensor_on)
        SENSORS_EnableGloomMeasurement();
}

void DIM_LED_Enable(void)
{
    if (!DimParameter.settings.keep_gloom_sensor_on)
        SENSORS_DisableGloomMeasurement();

#if (DIM_NEVER_EVER_ENABLE_LEDS != 1)
#ifdef HWP_LED_WHITE_20K
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_WHITE_1_PIN, Bit_SET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_WHITE_2_PIN, Bit_SET);
#endif
#ifdef HWP_LED_RED_UNI
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_RED_1_PIN, Bit_SET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_RED_2_PIN, Bit_SET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_RED_3_PIN, Bit_SET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_RED_4_PIN, Bit_SET);
#endif
#ifdef HWP_LED_RED_2K
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_RED_C1_PIN, Bit_SET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_RED_C2_PIN, Bit_SET);
#endif
#endif
}

void DIM_LED_Disable(void)
{
#ifdef HWP_LED_WHITE_20K
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_WHITE_1_PIN, Bit_RESET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_WHITE_2_PIN, Bit_RESET);
#endif
#ifdef HWP_LED_RED_UNI
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_RED_1_PIN, Bit_RESET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_RED_2_PIN, Bit_RESET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_RED_3_PIN, Bit_RESET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_RED_4_PIN, Bit_RESET);
#endif
#ifdef HWP_LED_RED_2K
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_RED_C1_PIN, Bit_RESET);
    GPIO_WriteBit(LED_ENABLE_PORT, LED_ENABLE_RED_C2_PIN, Bit_RESET);
#endif
}

void DIM_Reset(void)
{
    DIM_SetStage(0);
    dim.dir = DIM_HOLD;
    SENSORS_EnableGloomMeasurement();
}

void DIM_SetStage(uint16_t permill)
{
    if (permill > 1000)
        permill = 1000;

    // control heater
    if (permill > 0)
        HEATER_Off();
    else
        HEATER_CouldBeSwitchedOn();

    for (uint8_t ch = 0; ch < DIM_LED_COUNT; ch++)
        CurrentLedPWMOutput[ch] = RectifiedLedPowerLevel[ch] * permill / 1000;
}

void DIM_RectifyPowerLevel(void)
{
    uint32_t new_powerlevel;

    // calculate new target power level regarding temperature and aging

    for (uint8_t ch = 0; ch < DIM_LED_COUNT; ch++)
    {
        new_powerlevel = (uint32_t) DimParameter.led_powerlevel[ch];

        if (DimParameter.settings.use_compensation)
            new_powerlevel *= LED_COMPENSATION_GetCompensation(ch);
        else
            new_powerlevel *= 100;

        if (DimParameter.settings.use_reduction)
            new_powerlevel *= DimParameter.settings.reduction;
        else
            new_powerlevel *= 100;

        new_powerlevel /= 10000;

        if (new_powerlevel > UINT16_T_MAX)
            new_powerlevel = UINT16_T_MAX;
        RectifiedLedPowerLevel[ch] = new_powerlevel;
    }
}

void TIM2_IRQHandler(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
        xSemaphoreGiveFromISR(xDimInterruptBSemaphore, &xHigherPriorityTaskWoken);

    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

void TIM3_IRQHandler(void)
{
    /*
     portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

     if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
     {
     xSemaphoreGiveFromISR(xDimInterruptBSemaphore, &xHigherPriorityTaskWoken);
     }

     TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

     portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
     */
}

void TIM4_IRQHandler(void)
{
#define DIM_ADC_MEASURE_PWM_VALUE_MIN (1*(DIM_PWM_MAX_VAL/100))       // 1% of PWM max level
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
    {
#ifdef HWP_LED_RED_UNI
        TIM_SetCompare1(TIM3, CurrentLedPWMOutput[DIM_LED_RED_1]);
        TIM_SetCompare2(TIM3, CurrentLedPWMOutput[DIM_LED_RED_2]);
        TIM_SetCompare3(TIM3, 0xFFFF - CurrentLedPWMOutput[DIM_LED_RED_3]);
        TIM_SetCompare4(TIM3, 0xFFFF - CurrentLedPWMOutput[DIM_LED_RED_4]);
#endif
#ifdef HWP_LED_RED_2K
        TIM_SetCompare3(TIM2, CurrentLedPWMOutput[DIM_LED_RED_C]);
        TIM_SetCompare4(TIM2, 0xFFFF - CurrentLedPWMOutput[DIM_LED_RED_C]);
#endif
#ifdef HWP_LED_WHITE_20K
        TIM_SetCompare1(TIM2, CurrentLedPWMOutput[DIM_LED_WHITE]);
        TIM_SetCompare2(TIM2, 0xFFFF - CurrentLedPWMOutput[DIM_LED_WHITE]);
#endif



        uint16_t pwm_shoot_value;

        pwm_shoot_value = 0;
        switch (__isr_adc_channel)
        {
#ifdef HWP_LED_WHITE_20K
        case ADC_LED_CH_INDEX_WHITE_A:
        case ADC_LED_CH_INDEX_WHITE_C:
            if (CurrentLedPWMOutput[DIM_LED_WHITE] > DIM_ADC_MEASURE_PWM_VALUE_MIN)
                pwm_shoot_value = CurrentLedPWMOutput[DIM_LED_WHITE] / 2;
            break;
        case ADC_LED_CH_INDEX_WHITE_B:
        case ADC_LED_CH_INDEX_WHITE_D:
            if (CurrentLedPWMOutput[DIM_LED_WHITE] > DIM_ADC_MEASURE_PWM_VALUE_MIN)
                pwm_shoot_value = DIM_PWM_MAX_VAL - (CurrentLedPWMOutput[DIM_LED_WHITE] / 2);
            break;
#endif
#ifdef HWP_LED_RED_UNI
        case ADC_LED_CH_INDEX_RED_1:
            if (CurrentLedPWMOutput[DIM_LED_RED_1] > DIM_ADC_MEASURE_PWM_VALUE_MIN)
                pwm_shoot_value = CurrentLedPWMOutput[DIM_LED_RED_1] / 2;
            break;
        case ADC_LED_CH_INDEX_RED_2:
            if (CurrentLedPWMOutput[DIM_LED_RED_2] > DIM_ADC_MEASURE_PWM_VALUE_MIN)
                pwm_shoot_value = CurrentLedPWMOutput[DIM_LED_RED_2] / 2;
            break;
        case ADC_LED_CH_INDEX_RED_3:
            if (CurrentLedPWMOutput[DIM_LED_RED_3] > DIM_ADC_MEASURE_PWM_VALUE_MIN)
                pwm_shoot_value = DIM_PWM_MAX_VAL - (CurrentLedPWMOutput[DIM_LED_RED_3] / 2);
            break;
        case ADC_LED_CH_INDEX_RED_4:
            if (CurrentLedPWMOutput[DIM_LED_RED_4] > DIM_ADC_MEASURE_PWM_VALUE_MIN)
                pwm_shoot_value = DIM_PWM_MAX_VAL - (CurrentLedPWMOutput[DIM_LED_RED_4] / 2);
            break;
#endif
#ifdef HWP_LED_RED_2K
#ifdef HWT_MEDIUM_RED  // fail detection using voltage at dimming FET (measure during pwm low time)
        case ADC_LED_CH_INDEX_RED_C_A:
            if ((CurrentLedPWMOutput[DIM_LED_RED_C] >= ADC_RED_LED_FAIL_DETECTION_MIN_PWM) &&
                    (CurrentLedPWMOutput[DIM_LED_RED_C] < ADC_RED_LED_FAIL_DETECTION_MAX_PWM))
                pwm_shoot_value = ADC_RED_LED_FAIL_DETECTION_MIN_PWM_WAIT_COUNT;
            break;
        case ADC_LED_CH_INDEX_RED_C_B:
            if ((CurrentLedPWMOutput[DIM_LED_RED_C] >= ADC_RED_LED_FAIL_DETECTION_MIN_PWM) &&
                    (CurrentLedPWMOutput[DIM_LED_RED_C] < ADC_RED_LED_FAIL_DETECTION_MAX_PWM))
                pwm_shoot_value = DIM_PWM_MAX_VAL - CurrentLedPWMOutput[DIM_LED_RED_C] + ADC_RED_LED_FAIL_DETECTION_MIN_PWM_WAIT_COUNT;
            break;
#else  // normal voltage measurement
        case ADC_LED_CH_INDEX_RED_C_A:
            if (CurrentLedPWMOutput[DIM_LED_RED_C] > DIM_ADC_MEASURE_PWM_VALUE_MIN)
                pwm_shoot_value = (CurrentLedPWMOutput[DIM_LED_RED_C] / 2);
            break;
        case ADC_LED_CH_INDEX_RED_C_B:
            if (CurrentLedPWMOutput[DIM_LED_RED_C] > DIM_ADC_MEASURE_PWM_VALUE_MIN)
                pwm_shoot_value = DIM_PWM_MAX_VAL - (CurrentLedPWMOutput[DIM_LED_RED_C] / 2);
            break;
#endif
#endif
        default:
            break;
        }

        if (pwm_shoot_value > 0)
        {
            TIM_ClearITPendingBit(TIM4, TIM_IT_CC4);
            ADC_Arm(__isr_adc_channel);
            TIM_SetCompare4(TIM4, pwm_shoot_value);
        }

        __isr_adc_channel++;

        if (__isr_adc_channel > ADC_LED_CH_INDEX_LAST)
            __isr_adc_channel = ADC_LED_CH_INDEX_FIRST;

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

    // initialize led operating time counter
    for (DIM_LED_T idx = 0; idx < DIM_LED_COUNT; idx++)
    {
        led_operating_time[idx] = 0;
        led_operating_time_temperature[idx] = 0;
    }

    // disable all LED stages
    DIM_LED_Disable();

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
    TIM_TimeBaseStructure.TIM_Prescaler = 3;  // note comment above
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;

    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
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

    // Timer 2 for 2000cd RED
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OC3Init(TIM2, &TIM_OCInitStructure);

    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC4Init(TIM2, &TIM_OCInitStructure);

    // Timer 3 for FW RED
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
    TIM_OC2Init(TIM3, &TIM_OCInitStructure);

    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
    TIM_OC4Init(TIM3, &TIM_OCInitStructure);

    // Timer 4 for ADC Conversion Trigger
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OC4Init(TIM4, &TIM_OCInitStructure);


    /* Select the Master Slave Mode */
    TIM_SelectMasterSlaveMode(TIM2, TIM_MasterSlaveMode_Enable);

    /* Master Mode selection */
    TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);
    /* Slave Mode selection: TIM3 */
    TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Reset);
    TIM_SelectInputTrigger(TIM3, TIM_TS_ITR1);

    /* Slave Mode selection: TIM4 */
    TIM_SelectSlaveMode(TIM4, TIM_SlaveMode_Reset);
    TIM_SelectInputTrigger(TIM4, TIM_TS_ITR1);

    TIM_Cmd(TIM2, ENABLE);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
    TIM_Cmd(TIM4, ENABLE);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

    TIM_CCxCmd(TIM2, TIM_Channel_1, TIM_CCx_Enable);
    TIM_CCxCmd(TIM2, TIM_Channel_2, TIM_CCx_Enable);
    TIM_CCxCmd(TIM2, TIM_Channel_3, TIM_CCx_Enable);
    TIM_CCxCmd(TIM2, TIM_Channel_4, TIM_CCx_Enable);
    TIM_CCxCmd(TIM3, TIM_Channel_1, TIM_CCx_Enable);
    TIM_CCxCmd(TIM3, TIM_Channel_2, TIM_CCx_Enable);
    TIM_CCxCmd(TIM3, TIM_Channel_3, TIM_CCx_Enable);
    TIM_CCxCmd(TIM3, TIM_Channel_4, TIM_CCx_Enable);

    // Initialize dim structure
    dim.dir = DIM_HOLD;
    dim.difference = 0;
    dim.led_top = FALSE;
    dim.start_value = 0;
    dim.step = 0;
    dim.steps_overall = 0;
    dim.target_value = 0;
}

void DIM_Task(void *pvParameters)
{
    double dim_permill = 0;
    double exponent = 0;
    bool dim_top_last = 0;

    vSemaphoreCreateBinary(xDimInterruptBSemaphore);

    DIM_Init();

    while (1)
    {
        WATCHDOG_Heartbeat(WATCHDOG_TASK_ID_DIM);

        if (pdTRUE == xSemaphoreTake( xDimInterruptBSemaphore, WATCHDOG_TASK_WAIT_TIME_MS))
        {
            if (dim.dir == DIM_UP)
            {
                if (dim.steps_overall == 0)
                    DIM_SetStage(dim.target_value);
                else if (DimParameter.settings.type == DIM_TYPE_LOG)
                {
                    //Stevens' power law fading
                    exponent = 100.0d / (double) DimParameter.settings.gamma;
                    dim_permill = (uint16_t) round((double) dim.difference * pow((double) ((double) dim.step / dim.steps_overall), exponent));
                    dim_permill += dim.start_value;
                    DIM_SetStage((uint16_t) dim_permill);
                }
                else
                    DIM_SetStage(dim.start_value + (uint16_t) ((uint32_t) dim.difference * dim.step / dim.steps_overall));
            }
            else if (dim.dir == DIM_DOWN)
            {
                if (dim.steps_overall == 0)
                    DIM_SetStage(dim.target_value);
                else if (DimParameter.settings.type == DIM_TYPE_LOG)
                {
                    //Stevens' power law fading
                    exponent = 100.0d / (double) DimParameter.settings.gamma;
                    dim_permill = (uint16_t) round((double) dim.difference * pow(
                                                       (double) ((double) (dim.steps_overall - dim.step) / dim.steps_overall), exponent));
                    dim_permill = dim.target_value + dim_permill;
                    DIM_SetStage((uint16_t) dim_permill);
                }
                else
                {
                    DIM_SetStage(dim.target_value + (uint16_t) (((uint32_t) dim.difference * (dim.steps_overall - dim.step))
                                 / dim.steps_overall));
                }
            }

            DIM_SetLedOperatingTime();

            if (dim.step < dim.steps_overall)
                dim.step++;  // dim process not finished yet. continue with next step.
            else if (dim.dir != DIM_HOLD)
            {
                // all dim steps processed.
                if (dim.dir == DIM_UP)
                    dim.led_top = TRUE;  //highest level reached.
                else if ((dim.dir == DIM_DOWN) && dim.target_value == 0)
                    DIM_Reset();  // switch everything off, if target value is 0 (off)

                dim.dir = DIM_HOLD;
            }
            else
            {
                // DIM HOLD STATE
                if (dim.led_top == TRUE)
                {
                    // recalculate power levels regulary
                    DIM_RectifyPowerLevel();
                    DIM_SetStage(dim.target_value);
                }
            }

            if (dim.led_top != dim_top_last)
            {
                dim_top_last = dim.led_top;
                if (dim.led_top == TRUE)
                    TIM_ITConfig(TIM4, TIM_IT_CC4, ENABLE);
                else
                    TIM_ITConfig(TIM4, TIM_IT_CC4, DISABLE);
            }
        }
    }
}
