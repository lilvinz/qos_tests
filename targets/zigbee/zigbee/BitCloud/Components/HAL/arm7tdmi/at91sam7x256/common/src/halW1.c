/**************************************************************************//**
\file  halW1.c

\brief Implementation of 1-wire hardware-dependent module.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    05/09/07 A. Khromykh - Created
*******************************************************************************/
/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

/******************************************************************************
                   Includes section
******************************************************************************/
#include <halW1.h>
#include <halClkCtrl.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define W1_ZERO_BIT   0
#define W1_ONE_BIT    1

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
 Delay in microseconds.
 Parameters:
   us - delay time in microseconds
******************************************************************************/
void halDelayUs(uint16_t us)
{
  uint32_t startCounter;
  uint32_t delta;

  us *= 6; // one timer tick is 6 us
  // begin counter meaning
  startCounter = AT91C_BASE_TC0->TC_CV;
  // different between begin meaning and register C
  delta = AT91C_BASE_TC0->TC_RC - startCounter;

  if (delta > us)
  {
    while((AT91C_BASE_TC0->TC_CV - startCounter) < us);
  }
  else
  {
    us -= delta;
    while((AT91C_BASE_TC0->TC_CV > startCounter) || (AT91C_BASE_TC0->TC_CV < us));
  }
}

/******************************************************************************
 Resets all devices connected to the bus. Function asserts on
 the bus reset pulse and detects presence pulse.
 Parameters:
 Returns:
   W1_SUCCESS_STATUS   - If device(s) was(were) detected.
   W1_NO_DEVICE_STATUS - If device(s) was(were) not detected.
******************************************************************************/
W1Status_t halResetW1(void)
{
  W1Status_t  result;

  /* Set the PIO controller in PIO mode instead of peripheral mode */
  BASE_ONE_WIRE_PORT->PIO_PER = ONE_WIRE_PIN;
  /* enable clock on port B */
  AT91C_BASE_PMC->PMC_PCER = (1 << ONE_WIRE_PORT_ID);

  ATOMIC_SECTION_ENTER
  /* port sets as output. Reset low time 500 us */
  GPIO_ONE_WIRE_make_out();
  // clear pin
  GPIO_ONE_WIRE_clr();
  // delay 500 us
  halDelayUs(500);
  // Tri-state (external pullup)
  GPIO_ONE_WIRE_make_in();
  // Presence-Detect Sample Time 70 us
  // delay 70 us
  halDelayUs(70);
  // Precense-Detect
  if (GPIO_ONE_WIRE_read())  // logical level one
    result = W1_NO_DEVICE_STATUS;
  else  // logical level zero
    result = W1_SUCCESS_STATUS;
  // Tail of Reset High Time
  // delay 240 us
  halDelayUs(240);
  ATOMIC_SECTION_LEAVE

  return result;
}

/******************************************************************************
 Reads bit from the bus
 Returns:
   bit read from the bus
******************************************************************************/
uint8_t halReadW1Bit(void)
{
  uint8_t result;

  ATOMIC_SECTION_ENTER
  // port sets as output.
  GPIO_ONE_WIRE_make_out();
  // clear pin
  GPIO_ONE_WIRE_clr();
  // Read Low Time (in avr = 2 us), manual not overdrive mode 5us
  // delay 6 us
  halDelayUs(6);
  // Tri-state (external pullup)
  GPIO_ONE_WIRE_make_in();
  // Tail of Read Sample Time (10 us)
  // delay 10 us
  halDelayUs(10);
  // Read Sample
  if (GPIO_ONE_WIRE_read())  // logical level one
    result = W1_ONE_BIT;
  else  // logical level zero
    result = W1_ZERO_BIT;
  // Tail of Timeslot Duration
  // delay 100 us
  halDelayUs(100);
  ATOMIC_SECTION_LEAVE

  return result;
}

/******************************************************************************
 Reads byte from the bus
 Returns:
   byte read from the bus
******************************************************************************/
uint8_t halReadW1(void)
{
  uint8_t i;
  uint8_t result = 0;

  for (i=0; i<8; i++)
    if (W1_ONE_BIT == halReadW1Bit())
      result |= (1 << i);

  return result;
}

/******************************************************************************
 Writes bit to the bus
 Parameters:
   value - bit that should be written to the bus.
******************************************************************************/
void halWriteW1bit(uint8_t value)
{
  ATOMIC_SECTION_ENTER
  // port sets as output.
  GPIO_ONE_WIRE_make_out();
  // clear pin
  GPIO_ONE_WIRE_clr();
  // Write-1 Low Time
  // delay 6 us
  halDelayUs(6);
  if (W1_ZERO_BIT == value)
  {
    // delay 100 us
    halDelayUs(100);
    // Tri-state (external pullup)
    GPIO_ONE_WIRE_make_in();
    // delay 6 us recovery time
    halDelayUs(6);
  }
  else
  {
    // Tri-state (external pullup)
    GPIO_ONE_WIRE_make_in();
    // delay 100 us
    halDelayUs(100);
  }
  ATOMIC_SECTION_LEAVE
}

/******************************************************************************
 Writes byte to the bus
 Parameters:
   value - byte that should be written to the bus.
******************************************************************************/
void halWriteW1(uint8_t value)
{
  uint8_t i;

  for (i=0; i<8; i++)
  {
    if (value & (1 << i))
      halWriteW1bit(W1_ONE_BIT);
    else
      halWriteW1bit(W1_ZERO_BIT);
  }
}

// eof halW1.s
