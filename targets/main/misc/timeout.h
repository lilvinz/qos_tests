/*
 * timeout.h
 *
 *  Created on: 21.12.2011
 *      Author: cba
 */

#ifndef TIMEOUT_H_
#define TIMEOUT_H_

#include "clock.h"

typedef CLOCK_MS_T TIMEOUT_MS_T;
typedef CLOCK_S_T TIMEOUT_T;

#define TIMEOUT_Reset_ms(var)  var = CLOCK_GetMillitimestamp()
#define TIMEOUT_Check_ms(var, to) ((var + to) < CLOCK_GetMillitimestamp())?1:0

#define TIMEOUT_Reset(var)  var = CLOCK_GetTimestamp()
#define TIMEOUT_Check(var, to) ((var + to) < CLOCK_GetTimestamp())?1:0


#endif /* TIMEOUT_H_ */
