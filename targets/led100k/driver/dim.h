#ifndef DIM_H
#define DIM_H




//#define DIM_NEVER_EVER_ENABLE_LEDS

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
#define DIM_PRESCALER_VALUE 3  // note comment above


typedef uint16_t DIM_VALUE_T;

void DIM_Set(DIM_VALUE_T value);
void DIM_Reset(void);
void DIM_Init(void);
#endif
