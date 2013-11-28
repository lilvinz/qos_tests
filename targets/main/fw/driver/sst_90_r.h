/*
 * sst_90_r.h
 *
 *  Created on: 14.03.2011
 *      Author: DHA
 */

#ifndef SST_90_R_H_
#define SST_90_R_H_

#define SST_90_R_LED_AVG_VOLTAGE  24000  // 24V average LED voltage

#define SST_90_R_NO_COMPENSATION_FACTOR 100  // 100%
#define SST_90_R_DEFAULT_JUNCTION_TEMPERATURE     70  // 120°C
#define SST_90_R_TIMEMETER_COMPENSATION_MIN_TEMPERATURE 80  // 80 °C
#define SST_90_R_TIMEMETER_REF_TEMPERATURE              80  // 80 °C
#define SST_90_R_AGING_SLOPE_80                         149  // 1.49 E-6
#define SST_90_R_AGING_SLOPE_120                        244  // 2.44 E-6
#define SST_90_R_TEMPERATURE_DIFFERENCE                 40  // 120 - 80

//#define SST_90_R_AGING_FACTOR

#define SST_90_R_LED_CURRENT                3200  // 3200 mA todo [high] replace value by real value
#define SST_90_R_LED_POWER_LOSS_FACTOR      728  // efficiency 0.282 todo [high] replace value by real value
#define SST_90_R_NUMBER_LEDS_PER_CIRCUIT    12
#define SST_90_R_RTHJB         215  // 2.15 K/W

bool SST_90_R_GetJunctionTemperature(int32_t *junction_temperature);
uint16_t SST_90_R_GetCompensation(void);
uint16_t SST_90_R_GetTimemeterCompensation(void);

#endif /* SST_90_W__H_ */
