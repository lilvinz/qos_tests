#include "stdheader.h"
#include "status_led.h"

#include "timer.h"

static TIMER_T statusledtimer;

LED_CONTROL_T _led_state[LED_COUNT];


void STATUS_LED_Set(LED_T led, LED_STATE_T state, LED_TIMEOUT_T timeout)
{
    if (timeout <= 0)
        _led_state[led].timeout = LED_NO_TIMEOUT;
    else
        _led_state[led].timeout = timeout;

    _led_state[led].state = state;
    _led_state[led].timer = 0;
}

void STATUS_LED_Flash(LED_T led)
{
    _led_state[led].state = LED_STATE_FLASH;
    _led_state[led].timer = 0;
}

void STATUS_LED_Reset(LED_T led)
{
    _led_state[led].state = LED_STATE_OFF;
    _led_state[led].timeout = LED_NO_TIMEOUT;
}

void STATUS_LED_On(LED_T led)
{
    switch (led)
    {
    case LED_RED:
        STATUS_LED_RED_ON();
        break;
    case LED_GREEN:
        STATUS_LED_GREEN_ON();
        break;
    case LED_SIGNAL:
        STATUS_LED_SIGNAL_ON();
        break;
    default:
        break;
    }
}

void STATUS_LED_Off(LED_T led)
{
    switch (led)
    {
    case LED_RED:
        STATUS_LED_RED_OFF();
        break;
    case LED_GREEN:
        STATUS_LED_GREEN_OFF();
        break;
    case LED_SIGNAL:
        STATUS_LED_SIGNAL_OFF();
        break;
    default:
        break;
    }
}

void STATUS_LED_toggle(LED_T led)
{
    switch (led)
    {
    case LED_RED:
        if (STATUS_LED_RED_STATE() == 1) STATUS_LED_RED_OFF()
            else STATUS_LED_RED_ON()
                break;
    case LED_GREEN:
        if (STATUS_LED_GREEN_STATE() == 1) STATUS_LED_GREEN_OFF()
            else STATUS_LED_GREEN_ON()
                break;
    case LED_SIGNAL:
        if (STATUS_LED_SIGNAL_STATE() == 1) STATUS_LED_SIGNAL_OFF()
            else STATUS_LED_SIGNAL_ON()
                break;
    default:
        break;
    }
}

void STATUS_LED_Callback(uint32_t parameter)
{

    for (uint8_t idx = 0; idx < LED_COUNT; idx++)
    {
        if (_led_state[idx].timer == 0)
        {
            if (_led_state[idx].timeout > 0)
                _led_state[idx].timeout--;
            else if (_led_state[idx].timeout == 0)
                STATUS_LED_Reset(idx);
        }

        switch(_led_state[idx].state)
        {
        case LED_STATE_HOLD:
            break;
        case LED_STATE_OFF:
            STATUS_LED_Off(idx);
            break;
        case LED_STATE_ON:
            STATUS_LED_On(idx);
            break;
        case LED_STATE_TOGGLE:
            STATUS_LED_toggle(idx);
            _led_state[idx].state = LED_STATE_HOLD;
            break;
        case LED_STATE_FLASH:
            if (_led_state[idx].timer == 0)
                STATUS_LED_On(idx);
            else
            {
                STATUS_LED_Off(idx);
                _led_state[idx].state = LED_STATE_HOLD;
            }
            break;
        case LED_STATE_BLINK_SLOW:
            if (_led_state[idx].timer == 0)
                STATUS_LED_toggle(idx);
            break;
        case LED_STATE_BLINK_FAST:
            if (_led_state[idx].timer % 2)
                STATUS_LED_toggle(idx);
            break;
        case LED_STATE_MULTI_2:
            if (_led_state[idx].timer == 0)
                STATUS_LED_On(idx);
            else if (_led_state[idx].timer < 4)
                STATUS_LED_toggle(idx);
            break;
        case LED_STATE_MULTI_3:
            if (_led_state[idx].timer == 0)
                STATUS_LED_On(idx);
            else if (_led_state[idx].timer < 6)
                STATUS_LED_toggle(idx);
            break;
        default:
            break;
        }

        // 10Hz LED tick
        if (++_led_state[idx].timer >= 10)
            _led_state[idx].timer = 0;

    }
}

void STATUS_LED_Init(void)
{
    STATUS_LED_RED_OFF();
    STATUS_LED_GREEN_OFF();
    STATUS_LED_SIGNAL_OFF();

    statusledtimer.callback = STATUS_LED_Callback;
    statusledtimer.interval = 100;
    statusledtimer.mode = TIMER_REPEAT_MODE;
    TIMER_Start(&statusledtimer);
}




