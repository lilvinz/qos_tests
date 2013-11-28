/*
 * sensor.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 30.04.2010
 *      Author:
 */

#ifndef SENSOR_H_
#define SENSOR_H_

typedef enum
{
    SENSOR_FAILED           = -4,
    SENSOR_NOT_AVAILABLE    = -3,
    SENSOR_WAIT_FOR_INIT    = -2,
    SENSOR_RUNAWAY_VALUE    = -1,
    SENSOR_OK               = 0,
    SENSOR_UPPER_LIMIT      = 1,
    SENSOR_LOWER_LIMIT      = 2,
    SENSOR_VALUE_AVAILABLE  = 3,
} SENSOR_STATUS_T;


#endif /* SENSOR_H_ */
