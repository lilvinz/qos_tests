/*
 * led_compensation.c
 *
 *  Created on: 17.06.2010
 *      Author: Carsten Bartels, DHA
 */

#include "stdheader.h"
#include "led_compensation.h"

#include "sensors.h"
#include "adc.h"
#include "dim.h"
#include "aguard.h"
#include "sst_90_w.h"
#include "sst_90_r.h"
#include "time_meter.h"


/**
  * @brief  Returns a factor rectify the influence of junction temperature on aging.
  * @param  Type of led
  * @retval Compensation factor in percent
  */
uint16_t LED_COMPENSATION_GetTimemeterCompensation(DIM_LED_T led)
{
    switch(led)
    {
#ifdef HWP_LED_RED_UNI
    case DIM_LED_RED_1:
    case DIM_LED_RED_2:
    case DIM_LED_RED_3:
    case DIM_LED_RED_4:
        return SST_90_R_GetTimemeterCompensation();
#endif
#ifdef HWP_LED_RED_2K
    case DIM_LED_RED_C:
        return SST_90_R_GetTimemeterCompensation();
#endif
#ifdef HWP_LED_WHITE_20K
    case DIM_LED_WHITE:
        return SST_90_W_GetTimemeterCompensation();
#endif
    default:
        break;
    }
    return LED_COMPENSATION_NO_COMPENSATION_FACTOR;  // set 100 % as default compensation (no compensation)
}

/**
  * @brief  Returns relative light output in percent according to temperature.
  * @param  Type of led
  * @retval Compensation factor in percent
  */
uint16_t LED_COMPENSATION_GetCompensation(DIM_LED_T led)
{
    switch(led)
    {
#ifdef HWP_LED_RED_UNI
    case DIM_LED_RED_1:
    case DIM_LED_RED_2:
    case DIM_LED_RED_3:
    case DIM_LED_RED_4:
        return SST_90_R_GetCompensation();
#endif
#ifdef HWP_LED_RED_2K
    case DIM_LED_RED_C:
        return SST_90_R_GetCompensation();
#endif
#ifdef HWP_LED_WHITE_20K
    case DIM_LED_WHITE:
        return SST_90_W_GetCompensation();
#endif
    default:
        break;
    }
    return LED_COMPENSATION_NO_COMPENSATION_FACTOR;  // set 100 % as default compensation (no compensation)
}

