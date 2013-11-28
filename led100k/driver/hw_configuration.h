/*
 * hw_configuration.h
 *
 *  Created on: 06.05.2010
 *      Author: Carsten Bartels
 */

#ifndef HW_CONFIGURATION_H_
#define HW_CONFIGURATION_H_

// Status LEDs
#define STATUS_LED_PORT                     GPIOG
#define STATUS_LED_RED_PIN                  GPIO_Pin_5
#define STATUS_LED_GREEN_PIN                GPIO_Pin_6

//UART PORTS
#define UART3_PORT                          GPIOD
#define UART3_TX_PIN                        GPIO_Pin_8
#define UART3_TX_PINSOURCE                  GPIO_PinSource8
#define UART3_RX_PIN                        GPIO_Pin_9
#define UART3_RX_PINSOURCE                  GPIO_PinSource9
#define UART3_CK_PIN                        GPIO_Pin_10
#define UART3_CK_PINSOURCE                  GPIO_PinSource10
#define UART3_RTS_PIN                       GPIO_Pin_12
#define UART3_RTS_PINSOURCE                 GPIO_PinSource12
#define UART3_CTS_PIN                       GPIO_Pin_11
#define UART3_CTS_PINSOURCE                 GPIO_PinSource11

// LED ENABLE
#define LED_ENABLE_PORT         GPIOE
#define LED_ENABLE_SV1_PIN      GPIO_Pin_8
#define LED_ENABLE_SV2_PIN      GPIO_Pin_9
#define LED_ENABLE_SV3_PIN      GPIO_Pin_10
#define LED_ENABLE_SV4_PIN      GPIO_Pin_11
#define LED_ENABLE_SV5_PIN      GPIO_Pin_12
#define LED_ENABLE_SV6_PIN      GPIO_Pin_13
#define LED_ENABLE_SV7_PIN      GPIO_Pin_14
#define LED_ENABLE_SV8_PIN      GPIO_Pin_15
#define LED_ENABLE_SV9_PIN      GPIO_Pin_0
#define LED_ENABLE_SV10_PIN     GPIO_Pin_1
#define LED_ENABLE_SV11_PIN     GPIO_Pin_2
#define LED_ENABLE_SV12_PIN     GPIO_Pin_3
#define LED_ENABLE_SV13_PIN     GPIO_Pin_4
#define LED_ENABLE_SV14_PIN     GPIO_Pin_5

// ANALOG SWITCH
#define ANALOG_SWITCH_PORT          GPIOG
#define ANALOG_SWITCH_SW1_PIN       GPIO_Pin_0
#define ANALOG_SWITCH_SW2_PIN       GPIO_Pin_1
#define ANALOG_SWITCH_SW3_PIN       GPIO_Pin_2
#define ANALOG_SWITCH_SW4_PIN       GPIO_Pin_3

// ID SWITCH
#define ID_SWITCH_PORT          GPIOC
#define ID_SWITCH_SW1_PIN       GPIO_Pin_10
#define ID_SWITCH_SW2_PIN       GPIO_Pin_11
#define ID_SWITCH_SW3_PIN       GPIO_Pin_12

// SENS
#define SENS_FAULT_PORT         GPIOG
#define SENS_FAULT_PIN          GPIO_Pin_10
#define SENS_ENABLE_PORT        GPIOF
#define SENS_ENABLE_PIN         GPIO_Pin_11

void HWCONFIG_Init(void);

bool GPIO_PinInit(GPIO_TypeDef * port, uint16_t pin);
void GPIO_PinUnInit(GPIO_TypeDef * port, uint16_t pin);

#endif /* SETUP_GPIO_H_ */
