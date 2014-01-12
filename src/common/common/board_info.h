/*
 * board_info.h
 *
 *  Created on: 09.01.2014
 *      Author: vke
 */

#ifndef BOARD_INFO_H_
#define BOARD_INFO_H_

/* chibios */
#include "qhal.h"

/* newlibc */
#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint32_t magic;
    uint16_t id;
    uint16_t rev;
} BOARD_INFO_T;

bool board_info_read(BaseNVMDevice* nvmp, BOARD_INFO_T* bip);
bool board_info_write(BaseNVMDevice* nvmp, BOARD_INFO_T* bip);

#endif /* BOARD_INFO_H_ */
