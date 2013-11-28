/*
 * heater.h
 *
 *  Created on: 08.04.2013
 *      Author: cba
 */

#ifndef HEATER_H_
#define HEATER_H_


/****** Global Type Definitions ********/
typedef enum
{
    HEATER_OFF,
    HEATER_ON
} HEATER_STATE_T;


void HEATER_Off(void);
void HEATER_CouldBeSwitchedOn(void);

HEATER_STATE_T HEATER_GetState(void);
void HEATER_Tick(void);
void HEATER_Init(void);

#endif /* HEATER_H_ */
