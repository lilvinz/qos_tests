/*
 * led_feedback_control.c
 *
 *  Created on: 25.03.2011
 *      Author: cba
 */

#include "stdheader.h"
#include "led_feedback_control.h"


static bool b_led_feedback_reset_last_error = FALSE;
static float led_feedback_control_cur_error;
static float led_feedback_control_last_error;
static float led_feedback_control_k_p = 0.0;          //1.0
static float led_feedback_control_k_i = 0.0;           //0.5
static float led_feedback_control_p_value;
static float led_feedback_control_i_value;

static float led_feedback_control_output;


float getLedFeedbackControl(float soll, float ist)
{
    if (b_led_feedback_reset_last_error)
    {
        b_led_feedback_reset_last_error = FALSE;
        led_feedback_control_last_error = 0;
    }

    led_feedback_control_cur_error = soll - ist;

    //P control
    led_feedback_control_p_value = led_feedback_control_k_p * (led_feedback_control_cur_error - led_feedback_control_last_error);
    //I control
    led_feedback_control_i_value = led_feedback_control_k_i * (led_feedback_control_cur_error + led_feedback_control_last_error) / 2;

    led_feedback_control_last_error = led_feedback_control_cur_error;

    led_feedback_control_output = led_feedback_control_p_value + led_feedback_control_i_value;
    return led_feedback_control_output;
}
