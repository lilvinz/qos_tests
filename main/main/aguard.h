#ifndef aguard_H
#define aguard_H

#include "version.h"
#include "primestruct.h"


// LED measurement start delay to get around led driver start fail
// times are defined in respect of 100ms tick interval


#define AGUARD_TEMPERATURE_MIN_VALUE    -50.0f  // -50°C minimum LED temperature
#define AGUARD_TEMPERATURE_MAX_VALUE    85.0f // 85°C maximum temperature of some components used on mainboard
#define AGUARD_TEMPERATURE_HYSTERESIS   10.0f  // 10°C hysteresis

#define AGUARD_POWER_MINMAX_HOLD_TIME_S     (10)

// calculation of maximum led voltage for white leds (Cree XLamp XM-L 280lm)
// OVP voltage is 64,9 V
// maximum led voltage (from datasheet) is 12 leds * 3,5 V each led = 42 V
// lowest measured value without leds: about 53,5 V
// highest measured value with cold leds: about 48 V
// chosen maximum allowed voltage: 52 V
#define AGUARD_MAX_LED_VOLTAGE_WHITE_V  52.0f  // todo[high] determine limits (this has to be done very very carefully to avoid false alarm, new test run in climate chamber required)

// calculation of maximum led voltage for red leds (Lumiled LXM2-PH01-0070)
// OVP voltage is 86,1 V
// maximum led voltage (from datasheet) is 24 leds * 2,8 V each led = 67,2 V
// lowest measured value without leds: about 69,5 V
// highest measured value with cold leds: about 75,5 V (at -66,5°C) about 66,6 V at -47°C
// chosen maximum allowed voltage: 76 V
#define AGUARD_MAX_LED_VOLTAGE_RED_V    76.0f  // todo[high] determine limits (this has to be done very very carefully to avoid false alarm, new test run in climate chamber required)

#define AGUARD_LED_VOLTAGE_EMA_MULTIPLIER 1  // "multiplier / divisor" ==> "5 / 100" means 0.05
#define AGUARD_LED_VOLTAGE_EMA_DIVISOR 100   //

#define AGUARD_LED_AVG_VALUE_COUNTER_MAX (UINT16_T_MAX)
#define AGUARD_LED_AVG_VALUE_COUNTER_STARTUP_LIMIT 10

// attention, the sequence should not be longer than this time
// todo[medium] replace by an algorithm using the length of the secquence, attention: the length of the sequence can be zero!
#define AGUARD_LED_VOLTAGE_FAIL_FLAG_CHECK_INTERVAL_S 10  // 10 s
#define AGUARD_LED_AVG_VOLTAGE_TIMEOUT_S 60  // 60 s

#define AGUARD_RED_LED_MIN_VOLTAGE_AT_DIMMING_FET 1.0f  // minimum voltage to detect a normal operating led circuit, in case of a broken led stage, this voltage is near 0
#define AGUARD_LED_FAIL_COUNTER_MAX 3  // maximum allowed number of consecutive intervals with errors
#define AGUARD_MINIMUM_NUMBER_MEASUREMENTS_FOR_FAIL_DETECTION 20  // minimum number of measurement values which have to be processed before fail detection is activated
#define AGUARD_LED_FAIL_DETECTION_FAIL_SAMPLE_COUNTER_MAX (AGUARD_MINIMUM_NUMBER_MEASUREMENTS_FOR_FAIL_DETECTION / 2)  // maximum allowed number of errors

#define AGUARD_LED_OVERVOLTAGE_FAIL_TIMEOUT_S 600  // timeout 10 minutes

typedef struct aguard_minmax_t
{
    float   min;
    float   max;
    float   average;
} AGUARD_MINMAX_T;

typedef struct aguard_extremes_t
{
    AGUARD_MINMAX_T pwr24v;
} AGUARD_EXTREMES_T;

typedef struct aguard_led_calibration_t
{
    AGUARD_MINMAX_T value;
} AGUARD_LED_CALIBRATION_T;
#define AGUARD_LED_CALIBRATION_T_SIZE 12  // Attention: This must be in sync with the size of AGUARD_LED_CALIBRATION_T for size checks during compile time!

typedef enum
{
    AGUARD_TEMPERATURE_STATE_OK = 0,
    AGUARD_TEMPERATURE_STATE_ABOVE_MAXIMUM,
    AGUARD_TEMPERATURE_STATE_BELOW_MINIMUM
} AGUARD_TEMPERATURE_STATE_T;

typedef struct
{
    ADC_LED_CH_INDEX_T led_channel;
    float led_voltage;
} AGUARD_LED_VOLTAGE_QUEUE_ITEM_T;

#define AGUARD_LED_VOLTAGE_QUEUE_SIZE 100
#define AGUARD_ADD_LED_VOLTAGE_QUEUE_TIMEOUT_MS (portTickType) 10

void AGUARD_GetLEDCalibration(uint8_t LedId, AGUARD_LED_CALIBRATION_T *p);
void AGUARD_SetLEDCalibration(uint8_t LedId, AGUARD_LED_CALIBRATION_T *p);
void AGUARD_SaveLEDCalibration(void);


uint16_t AGUARD_GetLedStageOvFailBitmask(PS_LED_OVERVOLTAGE_BM_E bm_index);
uint16_t AGUARD_GetTemperatureFailBitmask(void);  // todo [medium] use or remove
void AGUARD_GetExtremes(AGUARD_EXTREMES_T *p);
void AGUARD_AddLedVoltage(ADC_LED_CH_INDEX_T led_channel, float led_voltage);

void AGUARD_CreateQueue(void);
void AGUARD_Init(void);
void AGUARD_Tick(void);
void AGUARD_Tick_10_MS(void);

#endif
