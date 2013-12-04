/*
 * delay.h
 *
 *  Created on: 04.12.2013
 *      Author: vke
 */

#ifndef DELAY_H_
#define DELAY_H_

#include <stdint.h>

void Delay_Init(void);
uint32_t Delay_GetRaw(void);
void Delay_WaituS(uint32_t uS);
void Delay_WaitmS(uint32_t mS);
uint32_t Delay_DiffuS(uint32_t raw);

#endif /* DELAY_H_ */
