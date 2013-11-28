/***************************************************************************//**
\file  leds.h

\brief Declaration of leds interface.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    29/05/07 E. Ivanov - Created
*******************************************************************************/

#ifndef _BSPLEDS_H
#define _BSPLEDS_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <gpio.h>
#include <leds.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define halInitFirstLed()     GPIO_FIRST_EK_make_out()
#define halUnInitFirstLed()   GPIO_FIRST_EK_make_in()
#define halOnFirstLed()       GPIO_FIRST_EK_clr()
#define halOffFirstLed()      GPIO_FIRST_EK_set()
#define halReadFirstLed()     GPIO_FIRST_EK_read()
#define halToggleFirstLed()   GPIO_FIRST_EK_toggle()

#define halInitSecondLed()    GPIO_SECOND_EK_make_out()
#define halUnInitSecondLed()  GPIO_SECOND_EK_make_in()
#define halOnSecondLed()      GPIO_SECOND_EK_clr()
#define halOffSecondLed()     GPIO_SECOND_EK_set()
#define halReadSecondLed()    GPIO_SECOND_EK_read()
#define halToggleSecondLed()  GPIO_SECOND_EK_toggle()

#define halInitThirdLed()     GPIO_THIRD_EK_make_out()
#define halUnInitThirdLed()   GPIO_THIRD_EK_make_in()
#define halOnThirdLed()       GPIO_THIRD_EK_clr()
#define halOffThirdLed()      GPIO_THIRD_EK_set()
#define halReadThirdLed()     GPIO_THIRD_EK_read()
#define halToggleThirdLed()   GPIO_THIRD_EK_toggle()

#define halInitFourthLed()    GPIO_FOURTH_EK_make_out()
#define halUnInitFourthLed()  GPIO_FOURTH_EK_make_in()
#define halOnFourthLed()      GPIO_FOURTH_EK_clr()
#define halOffFourthLed()     GPIO_FOURTH_EK_set()
#define halReadFourthLed()    GPIO_FOURTH_EK_read()
#define halToggleFourthLed()  GPIO_FOURTH_EK_toggle()

#endif /*_BSPLEDS_H*/
// eof leds.h
