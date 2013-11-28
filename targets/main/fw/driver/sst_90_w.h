/*
 * sst_90_w.h
 *
 *  Created on: 14.03.2011
 *      Author: DHA
 */

#ifndef SST_90_W_H_
#define SST_90_W_H_

#define SST_90_W_LED_AVG_VOLTAGE  44400  // 44,4V average LED voltage

#define SST_90_W_NO_COMPENSATION_FACTOR 100  // 100%
#define SST_90_W_DEFAULT_JUNCTION_TEMPERATURE     120  // 120°C
#define SST_90_W_TIMEMETER_COMPENSATION_MIN_TEMPERATURE 80  // 80 °C
#define SST_90_W_TIMEMETER_REF_TEMPERATURE              80  // 80 °C
#define SST_90_W_AGING_SLOPE_80                         149  // 1.49 E-6
#define SST_90_W_AGING_SLOPE_120                        244  // 2.44 E-6
#define SST_90_W_TEMPERATURE_DIFFERENCE                 40  // 120 - 80

//#define SST_90_W_AGING_FACTOR

#define SST_90_W_LED_CURRENT                6200  // 6200 mA
#define SST_90_W_LED_POWER_LOSS_FACTOR      728  // efficiency 0.282
#define SST_90_W_NUMBER_LEDS_PER_CIRCUIT    12
#define SST_90_W_RTHJB         2  // K/W

bool SST_90_W_GetJunctionTemperature(int32_t *junction_temperature);
uint16_t SST_90_W_GetCompensation(void);
uint16_t SST_90_W_GetTimemeterCompensation(void);

#define SST_90_W_NUM_WHITE_LEDS 4
#endif /* SST_90_W__H_ */
