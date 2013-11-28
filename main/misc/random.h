/*
 * random.h
 *
 *  Created on: 28.02.2011
 *      Author: cba
 */

#ifndef RANDOM_H_
#define RANDOM_H_

void random_init(void);
int32_t random_get(int32_t min, int32_t max);
int32_t random_jitter(int32_t base, uint16_t jitter);

#endif /* RANDOM_H_ */
