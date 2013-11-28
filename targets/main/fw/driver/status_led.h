#ifndef LED_H
#define LED_H

#include "hw_configuration.h"

#define STATUS_LED_RED_ON()         GPIO_WriteBit(STATUS_LED_PORT, STATUS_LED_RED_PIN, Bit_SET);
#define STATUS_LED_RED_OFF()        GPIO_WriteBit(STATUS_LED_PORT, STATUS_LED_RED_PIN, Bit_RESET);
#define STATUS_LED_RED_STATE()      GPIO_ReadOutputDataBit(STATUS_LED_PORT, STATUS_LED_RED_PIN)
#define STATUS_LED_GREEN_ON()       GPIO_WriteBit(STATUS_LED_PORT, STATUS_LED_GREEN_PIN, Bit_SET);
#define STATUS_LED_GREEN_OFF()      GPIO_WriteBit(STATUS_LED_PORT, STATUS_LED_GREEN_PIN, Bit_RESET);
#define STATUS_LED_GREEN_STATE()    GPIO_ReadOutputDataBit(STATUS_LED_PORT, STATUS_LED_GREEN_PIN)
#define STATUS_LED_SIGNAL_ON()      GPIO_WriteBit(STATUS_LED_PORT, STATUS_LED_SIGNAL_PIN, Bit_SET);
#define STATUS_LED_SIGNAL_OFF()     GPIO_WriteBit(STATUS_LED_PORT, STATUS_LED_SIGNAL_PIN, Bit_RESET);
#define STATUS_LED_SIGNAL_STATE()   GPIO_ReadOutputDataBit(STATUS_LED_PORT, STATUS_LED_SIGNAL_PIN)

#define LED_NO_TIMEOUT (-1)

typedef int16_t LED_TIMEOUT_T;

typedef enum
{
    LED_RED,
    LED_GREEN,
    LED_SIGNAL,
    LED_COUNT
} LED_T;

typedef enum
{
    LED_STATE_HOLD,
    LED_STATE_OFF,
    LED_STATE_ON,
    LED_STATE_TOGGLE,
    LED_STATE_FLASH,
    LED_STATE_BLINK_SLOW,
    LED_STATE_BLINK_FAST,
    LED_STATE_MULTI_2,
    LED_STATE_MULTI_3
} LED_STATE_T;

typedef struct led_state_s
{
    LED_STATE_T     state;
    uint8_t         timer;
    LED_TIMEOUT_T   timeout;
} LED_CONTROL_T;

extern LED_CONTROL_T    _led_state[LED_COUNT];

void STATUS_LED_Set(LED_T led, LED_STATE_T state, LED_TIMEOUT_T timeout);
void STATUS_LED_Reset(LED_T ind);

void STATUS_LED_On(LED_T led);
void STATUS_LED_Off(LED_T led);
void STATUS_LED_Flash(LED_T led);

void STATUS_LED_Tick(void);
void STATUS_LED_Init(void);

#endif
