/*
 * vt_style.h
 *
 *  Created on: 03.05.2011
 *      Author: cba
 */

#ifndef VT_STYLE_H_
#define VT_STYLE_H_

#include "vt102.h"


#define PRINT_OK            printf(ESC_COLOR_GREEN ESC_INVERSE " OK " ESC_ATTRIBUTE_OFF)
#define PRINT_FAIL          printf(ESC_COLOR_RED ESC_INVERSE "FAIL" ESC_ATTRIBUTE_OFF)


#endif /* VT_STYLE_H_ */
