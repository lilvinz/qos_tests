/***************************************************************************//**
\file  leds.c

\brief The module to access to the leds.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    29/05/07 E. Ivanov - Created
*******************************************************************************/
#if APP_DISABLE_BSP != 1
 
/******************************************************************************
                   Includes section
******************************************************************************/
#include <bspLeds.h>

/******************************************************************************
                   Implementations section
******************************************************************************/
/****************************************************************
Init LED control module.
****************************************************************/
static void initLeds(void)
{
  halInitFirstLed();
  halInitSecondLed();
  halInitThirdLed();
  halInitFourthLed();
}

/****************************************************************
Opens leds module to use.
****************************************************************/
result_t BSP_OpenLeds(void)
{
  initLeds();
  halOffFirstLed();
  halOffSecondLed();
  halOffThirdLed();
  halOffFourthLed();
  return BC_SUCCESS;
}

/****************************************************************
Closes leds module.
****************************************************************/
result_t BSP_CloseLeds(void)
{
  halUnInitFirstLed();
  halUnInitSecondLed();
  halUnInitThirdLed();
  halUnInitFourthLed();
  return BC_SUCCESS;
}

/****************************************************************
Turn on LED.
Parameters:
    id - number of led
****************************************************************/
void BSP_OnLed(uint8_t id)
{
  switch(id)
  {
    case LED_FIRST:
          halOnFirstLed();
         break;
    case LED_SECOND:
          halOnSecondLed();
         break;
    case LED_THIRD:
          halOnThirdLed();
         break;
    case LED_FOURTH:
          halOnFourthLed();
         break;
  }
}

/****************************************************************
Turn off LED.
Parameters:
      id - number of led
****************************************************************/
void BSP_OffLed(uint8_t id)
{
  switch(id)
  {
    case LED_FIRST:
          halOffFirstLed();
         break;
    case LED_SECOND:
          halOffSecondLed();
         break;
    case LED_THIRD:
          halOffThirdLed();
         break;
    case LED_FOURTH:
          halOffFourthLed();
         break;
  }
}

/****************************************************************
Change LED state to opposite.
    Parameters:
      id - number of led
****************************************************************/
void BSP_ToggleLed(uint8_t id)
{
  switch(id)
  {
    case LED_FIRST:
          halToggleFirstLed();
         break;
    case LED_SECOND:
          halToggleSecondLed();
         break;
    case LED_THIRD:
          halToggleThirdLed();
         break;
    case LED_FOURTH:
          halToggleFourthLed();
         break;
  }
}

#endif // APP_DISABLE_BSP != 1

// eof leds.c
