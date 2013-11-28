#ifndef DIM_H
#define DIM_H

#include "settings.h"
#include "config.h"

#define DIM_NEVER_EVER_ENABLE_LEDS 0

#define DIM_SETTING_USE_LED_COMPENSATION _settings_list_ram[SETTINGS_DIM_LED_USE_COMPENSATION]

#define DIM_INTERRUPT_PRIORITY 24       // CMSIS Priority (wird vielleicht evtl u.U. um 3 nach links verschoben)

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

typedef uint16_t DIM_VALUE_T;

typedef enum
{
    DIM_TYPE_LINEAR,
    DIM_TYPE_LOG,
    DIM_TYPE_NONE
} DIM_TYPE_T;

typedef enum
{
#ifdef HWP_LED_WHITE_20K
    DIM_LED_WHITE,
#endif
#ifdef HWP_LED_RED_UNI
    DIM_LED_RED_1,  // top row
    DIM_LED_RED_2,  // second row
#endif
#ifdef HWP_LED_RED_2K
    DIM_LED_RED_C,  // middle
#endif
#ifdef HWP_LED_RED_UNI
    DIM_LED_RED_3,  // third row
    DIM_LED_RED_4,  // bottom row
#endif

    DIM_LED_COUNT
} DIM_LED_T;

typedef struct dim_t
{
    DIM_TYPE_T  type;
    uint8_t     gamma;
    uint8_t     reduction;
    bool        use_compensation;
    bool        use_reduction;
    bool        keep_gloom_sensor_on;
} DIM_SETTINGS_T;

typedef uint16_t DIM_LED_POWERLEVEL;

typedef struct
{
    DIM_SETTINGS_T      settings;
    DIM_LED_POWERLEVEL  led_powerlevel[DIM_LED_COUNT];
} DIM_PARAMETER_T;

typedef struct
{
    DIM_TYPE_T  type;
    uint8_t     gamma;
} DIM_BMM_DIM_SETTINGS_T; // 8 (not packed)

typedef struct
{
    DIM_BMM_DIM_SETTINGS_T  settings; // 8 (not packed)
    DIM_LED_POWERLEVEL      led_powerlevel[DIM_LED_COUNT]; // 24 (not packed)
} DIM_BMM_DIM_PARAMETER_T; // 32 (not packed)

void DIM_Task(void *pvParameters);
void DIM_Set(DIM_VALUE_T value, uint32_t duration);
void DIM_SetParameter(const DIM_BMM_DIM_PARAMETER_T * pNewParameter, const uint16_t *reduction, bool bGloomSensorAlwaysOn);
void DIM_SetNow(DIM_LED_POWERLEVEL * pNewPowerLevel, bool use_compensation, bool use_reduction);
void DIM_SetNowRefresh(void);
void DIM_Reset(void);
const char * DIM_GetLedRowName(DIM_LED_T led);
uint16_t DIM_GetLedPowerLevel(DIM_LED_T led);
uint16_t DIM_GetRectifiedLedPowerLevel(DIM_LED_T led);
uint32_t DIM_GetLedOperatingTime(DIM_LED_T led);
uint32_t DIM_GetCompensatedLedOperatingTime(DIM_LED_T led);

#endif
