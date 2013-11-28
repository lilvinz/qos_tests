/*
 * hw_configuration.h
 *
 *  Created on: 06.05.2010
 *      Author: Carsten Bartels
 */

#ifndef HW_CONFIGURATION_H_
#define HW_CONFIGURATION_H_

// LUXVCC Attention: LUXVCC is used for I2C!
#define LUXVCC_PORT                         GPIOF
#define LUXVCC_PIN                          GPIO_Pin_11

// Relais 1 control pin
#define INDICATION_RELAIS_A_PORT            GPIOG
#define INDICATION_RELAIS_A_BIT             GPIO_Pin_1

// Relais 2 control pin
#define INDICATION_RELAIS_B_PORT            GPIOG
#define INDICATION_RELAIS_B_BIT             GPIO_Pin_2

// Buzzer
#define BUZZER_PORT                         GPIOE
#define BUZZER_PIN                          GPIO_Pin_6
#define BUZZER_PINSOURCE                    GPIO_PinSource6

// Status LEDs
#define STATUS_LED_PORT                     GPIOG
#define STATUS_LED_RED_PIN                  GPIO_Pin_5
#define STATUS_LED_GREEN_PIN                GPIO_Pin_6
#define STATUS_LED_SIGNAL_PIN               GPIO_Pin_3

// I2C BUS
#define I2C_BUS_PORT                        GPIOB
#define I2C_BUS_SDA                         GPIO_Pin_7
#define I2C_BUS_SDA_PINSOURCE               GPIO_PinSource7
#define I2C_BUS_SCL                         GPIO_Pin_6
#define I2C_BUS_SCL_PINSOURCE               GPIO_PinSource6

// SPI SERIAL FLASH
#define FLASH_EEP_CS_PORT                   GPIOF
#define FLASH_EEP_CS_PIN                    GPIO_Pin_15

// SPI SERIAL FRAM
#define FRAM_CS_PORT                        GPIOF
#define FRAM_CS_PIN                         GPIO_Pin_14

// RS485 DIRECTION
#define RS485_1_DIR_PORT                    GPIOF
#define RS485_1_DIR_PIN                     GPIO_Pin_4
#define RS485_2_DIR_PORT                    GPIOF
#define RS485_2_DIR_PIN                     GPIO_Pin_5

//UART PORTS
#define UART1_PORT                          GPIOA
#define UART1_TX_PIN                        GPIO_Pin_9
#define UART1_TX_PINSOURCE                  GPIO_PinSource9
#define UART1_RX_PIN                        GPIO_Pin_10
#define UART1_RX_PINSOURCE                  GPIO_PinSource10

#define UART2_PORT                          GPIOD
#define UART2_TX_PIN                        GPIO_Pin_5
#define UART2_TX_PINSOURCE                  GPIO_PinSource5
#define UART2_RX_PIN                        GPIO_Pin_6
#define UART2_RX_PINSOURCE                  GPIO_PinSource6
#define UART2_RTS_PIN                       GPIO_Pin_4
#define UART2_RTS_PINSOURCE                 GPIO_PinSource4
#define UART2_CTS_PIN                       GPIO_Pin_3
#define UART2_CTS_PINSOURCE                 GPIO_PinSource3

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

#define UART4_PORT                          GPIOC
#define UART4_TX_PIN                        GPIO_Pin_10
#define UART4_TX_PINSOURCE                  GPIO_PinSource10
#define UART4_RX_PIN                        GPIO_Pin_11
#define UART4_RX_PINSOURCE                  GPIO_PinSource11

#define UART5_TX_PORT                       GPIOC
#define UART5_RX_PORT                       GPIOD
#define UART5_TX_PIN                        GPIO_Pin_12
#define UART5_TX_PINSOURCE                  GPIO_PinSource12
#define UART5_RX_PIN                        GPIO_Pin_2
#define UART5_RX_PINSOURCE                  GPIO_PinSource2

// LED ENABLE
#define LED_ENABLE_PORT                     GPIOE
#define LED_ENABLE_WHITE_1_PIN              GPIO_Pin_8
#define LED_ENABLE_WHITE_2_PIN              GPIO_Pin_9
#define LED_ENABLE_RED_C1_PIN               GPIO_Pin_10
#define LED_ENABLE_RED_1_PIN                GPIO_Pin_11
#define LED_ENABLE_RED_2_PIN                GPIO_Pin_12
#define LED_ENABLE_RED_3_PIN                GPIO_Pin_13
#define LED_ENABLE_RED_4_PIN                GPIO_Pin_14
#define LED_ENABLE_RED_C2_PIN               GPIO_Pin_15

// HEATER
#define HEATER_PORT                         GPIOD
#define HEATER_PIN                          GPIO_Pin_13

// GPS
#define GPS_PORT                            GPIOF
#define GPS_RESET_PIN                       GPIO_Pin_0
#define GPS_ENABLE_VCC_POWER_PIN            GPIO_Pin_1
#define GPS_ENABLE_RF_POWER_PIN             GPIO_Pin_2

// MESH
#define MESH_RESET_PIN                    GPIO_Pin_3
#define MESH_RESET_PORT                   GPIOF
#define MESH_VCC_EN_PIN                   GPIO_Pin_12
#define MESH_VCC_EN_PORT                  GPIOF


//SVINFO
#define SVINFO_PORT                         GPIOA
#define SVINFO_PIN                          GPIO_Pin_8
#define SVINFO_STATE()                      GPIO_ReadInputDataBit(SVINFO_PORT, SVINFO_PIN)

#define DETECTOR_FAULT_SENSE_PORT           GPIOG
#define DETECTOR_GPS_VCC_FAULT_SENSE_PIN    GPIO_Pin_12
#define DETECTOR_GPS_RF_FAULT_SENSE_PIN     GPIO_Pin_11
#define DETECTOR_LUX_2_8_V_FAULT_SENSE_PIN  GPIO_Pin_10

// DEBUG / TestPins
#define DEBUG_PIN_PORT                      GPIOE
#define DEBUG_PIN_0                         GPIO_Pin_0
#define DEBUG_PIN_1                         GPIO_Pin_1
#define DEBUG_PIN_2                         GPIO_Pin_2
#define DEBUG_PIN_3                         GPIO_Pin_3
#define DEBUG_PIN_4                         GPIO_Pin_4
#define DEBUG_PIN_5                         GPIO_Pin_5

// DEBUG / TestPins
#define HWREV_PIN_PORT                      GPIOE
#define HWREV_PIN_0                         GPIO_Pin_7



void HWCONFIG_Init(void);

bool GPIO_PinInit(GPIO_TypeDef * port, uint16_t pin);
void GPIO_PinUnInit(GPIO_TypeDef * port, uint16_t pin);

#endif /* SETUP_GPIO_H_ */
