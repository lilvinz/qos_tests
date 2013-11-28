/*
 * hw_clock.h
 *
 *  Created on: 05.07.2010
 *      Author: CBA
 */

#ifndef HW_CLOCK_H_
#define HW_CLOCK_H_

#define HW_CLOCK_INIT_MAGIC_CONSTANT 0xdf90712b

void HW_CLOCK_Init(void);
uint32_t HW_CLOCK_Get(void);
void HW_CLOCK_Set(uint32_t timestamp);

#endif /* HW_CLOCK_H_ */
