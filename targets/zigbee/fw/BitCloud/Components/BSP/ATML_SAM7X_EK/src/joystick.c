/**************************************************************************//**
\file  joystick.c

\brief Implementation of joystick interface.

\author
    Atmel Corporation: http://www.atmel.com \n
    Support email: avr@atmel.com

  Copyright (c) 2008-2011, Atmel Corporation. All rights reserved.
  Licensed under Atmel's Limited License Agreement (BitCloudTM).

\internal
  History:
    10/09/07 A. Khromykh - Created
*******************************************************************************/
#if APP_DISABLE_BSP != 1
  
/******************************************************************************
                   Includes section
******************************************************************************/
#include <joystick.h>
#include <bspTaskManager.h>
#include <appTimer.h>
#include <stddef.h>

#if defined(FREE_RTOS)
  #include <FreeRTOS.h>
  #include <task.h>
#endif

/******************************************************************************
                   Define(s) section
******************************************************************************/
// majoritary etalon (ms)
#define MAJ_SCANN_TIME  330
#define JS_LEFT     AT91C_PIO_PA23
#define JS_RIGHT    AT91C_PIO_PA24
#define JS_UP       AT91C_PIO_PA21
#define JS_DOWN     AT91C_PIO_PA22
#define JS_PUSH     AT91C_PIO_PA25

/******************************************************************************
                   Types section
******************************************************************************/
typedef enum
{
  BSP_JOYSTICK_IDLE = 0,
  BSP_JOYSTICK_BUSY
} BSP_JoystickModulState_t;

// callback joystick functions
typedef void (*BSP_JoystickEventFunc_t)(BSP_JoystickState_t state);

/******************************************************************************
                   Prototypes section
******************************************************************************/
#if defined(FREE_RTOS)
  static void joystickHandler(void) __attribute__((naked));
#else
  static void joystickHandler(void);
#endif

/******************************************************************************
                   Global variables section
******************************************************************************/
static BSP_JoystickModulState_t state = BSP_JOYSTICK_IDLE;
static volatile BSP_JoystickState_t joystickReal;
static BSP_JoystickEventFunc_t bspJoystickHandle = NULL;
static HAL_AppTimer_t glitchTimer;

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************
disable joystick interrupt
******************************************************************/
void bspDisableJoystickPIOInterrupt(void)
{
  /* Disable button interrupts generation through the PIO controller */
  AT91C_BASE_PIOA->PIO_IDR = JS_LEFT | JS_RIGHT | JS_UP | JS_DOWN | JS_PUSH;
}

/******************************************************************
enable joystick interrupt
******************************************************************/
void bspEnableJoystickPIOInterrupt(void)
{
  /* Enable button interrupts generation through the PIO controller */
  AT91C_BASE_PIOA->PIO_IER = JS_LEFT | JS_RIGHT | JS_UP | JS_DOWN | JS_PUSH;
}

/******************************************************************
port IO (joystick) interrupt handler
******************************************************************/
static void joystickHandler(void)
{
#if defined(FREE_RTOS)
  portSAVE_CONTEXT();
#endif

  volatile uint32_t pio_isr = AT91C_BASE_PIOA->PIO_ISR;
  volatile uint32_t pio_state = AT91C_BASE_PIOA->PIO_PDSR;
  uint8_t irqUp = 0;

  if(JS_LEFT & pio_isr)
  {
    if (!(pio_state & JS_LEFT))
    {
      joystickReal = JOYSTICK_LEFT;
      irqUp = 1;
    }
  }
  else if(JS_RIGHT & pio_isr)
  {
    if (!(pio_state & JS_RIGHT))
    {
      joystickReal = JOYSTICK_RIGHT;
      irqUp = 1;
    }
  }
  else if(JS_UP & pio_isr)
  {
    if (!(pio_state & JS_UP))
    {
      joystickReal = JOYSTICK_UP;
      irqUp = 1;
    }
  }
  else if(JS_DOWN & pio_isr)
  {
    if (!(pio_state & JS_DOWN))
    {
      joystickReal = JOYSTICK_DOWN;
      irqUp = 1;
    }
  }
  else if(JS_PUSH & pio_isr)
  {
    if (!(pio_state & JS_PUSH))
    {
      joystickReal = JOYSTICK_PUSH;
      irqUp = 1;
    }
  }

  if (irqUp)
  {
    bspDisableJoystickPIOInterrupt();
    bspPostTask(BSP_JOYSTICK);
  }

#if defined(FREE_RTOS)
  /* End the interrupt in the AIC. */
  AT91C_BASE_AIC->AIC_EOICR = 0;

  portRESTORE_CONTEXT();
#endif
}

/******************************************************************************
 Initializes joystick module.
******************************************************************************/
void bspInitJoystick(void)
{
  uint32_t dummy;

  /* sets parameters PMC for PIOA */
  /* enable clock on port A */
  AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_PIOA);

  /* sets parameters AIC for PIOA */
  /* Disable the interrupt on the interrupt controller */
  AT91C_BASE_AIC->AIC_IDCR = (1 << AT91C_ID_PIOA);
  /* Save the interrupt handler routine pointer and the interrupt priority */
  AT91C_BASE_AIC->AIC_SVR[AT91C_ID_PIOA] = (uint32_t)joystickHandler;
  /* Store the Source Mode Register */
  AT91C_BASE_AIC->AIC_SMR[AT91C_ID_PIOA] = AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL | AT91C_AIC_PRIOR_LOWEST;
  /* Clear the interrupt on the interrupt controller */
  AT91C_BASE_AIC->AIC_ICCR = (1 << AT91C_ID_PIOA);
  /* Enable PIO interrupt in the interrupt controller */
  AT91C_BASE_AIC->AIC_IECR = (1 << AT91C_ID_PIOA);

  /* disable all interrupts in port */
  AT91C_BASE_PIOA->PIO_IDR = ALL_PERIPHERIAL_INTERRUPT_DISABLE;
  /* Set the PIO controller in PIO mode instead of peripheral mode */
  AT91C_BASE_PIOA->PIO_PER = JS_LEFT | JS_RIGHT | JS_UP | JS_DOWN | JS_PUSH;
  /* Set pins as input. */
  AT91C_BASE_PIOA->PIO_ODR = JS_LEFT | JS_RIGHT | JS_UP | JS_DOWN | JS_PUSH;
  /* Enable pullup on pins. */
  AT91C_BASE_PIOA->PIO_PPUER = JS_LEFT | JS_RIGHT | JS_UP | JS_DOWN | JS_PUSH;
  /* Enable glitch filtering on pins. */
  AT91C_BASE_PIOA->PIO_IFER = JS_LEFT | JS_RIGHT | JS_UP | JS_DOWN | JS_PUSH;
  /* clear all PIO interrupts */
  dummy = AT91C_BASE_PIOA->PIO_ISR;
  (void)dummy;
  /* Enable button interrupts generation through the PIO controller */
  AT91C_BASE_PIOA->PIO_IER = JS_LEFT | JS_RIGHT | JS_UP | JS_DOWN | JS_PUSH;
}

/******************************************************************************
Registers handlers for joystick events.
Paremeters:
  generalHandler - the joystick handler
  state- joystick state
Returns:
  BC_FAIL - joystick module is busy, 
  BC_SUCCESS in other cases.
******************************************************************************/
result_t BSP_OpenJoystick(void (*generalHandler)(BSP_JoystickState_t state))
{
  if(state != BSP_JOYSTICK_IDLE)
    return BC_FAIL;
  state = BSP_JOYSTICK_BUSY;
  bspJoystickHandle = generalHandler;
  bspInitJoystick();
  return BC_SUCCESS;
};

/******************************************************************************
Remove joystick handlers.
Parameters:
  none.
Returns:
  BC_FAIL - joystick module was not opened,
  BC_SUCCESS in other cases.
******************************************************************************/
result_t BSP_CloseJoystick(void)
{
  if(state != BSP_JOYSTICK_BUSY)
    return BC_FAIL;
  bspDisableJoystickPIOInterrupt();
  bspJoystickHandle = NULL;
  state = BSP_JOYSTICK_IDLE;
  return BC_SUCCESS;
};

/******************************************************************************
Reads state of joystick.
Parameters:
  none.
Returns:
    Current joystick state.
******************************************************************************/
BSP_JoystickState_t BSP_ReadJoystickState(void)
{
  /* read joystick state */
  return joystickReal;
}

/******************************************************************************
BSP's event about joystick has changed state.
******************************************************************************/
void bspJoystickHandler(void)
{
  glitchTimer.interval = MAJ_SCANN_TIME;
  glitchTimer.mode = TIMER_ONE_SHOT_MODE;
  glitchTimer.callback = bspEnableJoystickPIOInterrupt;
  HAL_StartAppTimer(&glitchTimer);

  if (NULL != bspJoystickHandle)
    bspJoystickHandle(joystickReal);
}

#endif // APP_DISABLE_BSP != 1

// eof joystick.c
