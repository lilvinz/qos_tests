/*
 * led_feedback_control.h
 *
 *  Created on: 25.03.2011
 *      Author: cba
 */

#ifndef LED_FEEDBACK_CONTROL_H_
#define LED_FEEDBACK_CONTROL_H_

float getLedFeedbackControl(float soll, float ist);

extern bool b_led_feedback_reset_last_error;
extern float led_feedback_control_cur_error;
extern float led_feedback_control_last_error;
extern float led_feedback_control_k_p;
extern float led_feedback_control_k_i;
extern float led_feedback_control_output;
extern float led_feedback_control_p_value;
extern float led_feedback_control_i_value;

#endif /* LED_FEEDBACK_CONTROL_H_ */
