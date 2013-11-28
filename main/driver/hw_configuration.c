/*
 * setup_gpio.c
 *
 *  Created on: 06.05.2010
 *      Author: Carsten Bartels
 */

/*
 *  ###    ####  #   #  #####  #   #  #   #   ####
 * #   #  #      #   #    #    #   #  ##  #  #
 * #####  #      #####    #    #   #  # # #  # ###
 * #   #  #      #   #    #    #   #  #  ##  #   #
 * #   #   ####  #   #    #     ###   #   #   ####
 *
 * Diese Datei bitte NICHT durch den Auto-Formatter jagen. Danke.
 */



/* Includes ------------------------------------------------------------------*/
#include "stdheader.h"
#include "hw_configuration.h"
#include "version.h"

/* Private function prototypes -----------------------------------------------*/

void RCC_Configuration(void);
void GPIO_Configuration(void);
void NVIC_Configuration(void);
void EXTI_Configuration(void);
void IO_Remap(void);

/* Private type definitions ------------------------------------------------------------------*/

typedef enum
{
    GPIO_STATE_UNUSED = 0,
    GPIO_LOW,
    GPIO_HIGH,
} GPIO_DEFAULT_STATE;

typedef struct
{
    GPIO_TypeDef*       GPIO_Port;
    GPIO_InitTypeDef    GPIO_InitStructure;
    GPIO_DEFAULT_STATE  GPIO_DefaultState;
} SETUP_GPIO_T;

typedef struct
{
    GPIO_TypeDef*       GPIOx;
    uint16_t            GPIO_PinSource;
    uint8_t             GPIO_AF;
} SETUP_IO_REMAP_T;

typedef struct
{
    bool                bSoftInterrupt;
    uint8_t             GPIO_PortSource;
    uint8_t             GPIO_PinSource;
    uint32_t            EXTI_Line;
    EXTIMode_TypeDef    EXTI_Mode;
    EXTITrigger_TypeDef EXTI_Trigger;
    FunctionalState     EXTI_LineCmd;
} EXTI_LINE_CONFIG_T;

typedef enum rcc_peripheral_bus_e
{
    RCC_APB_1 = 1,
    RCC_APB_2,
    RCC_AHB_1,
    RCC_AHB_2,
    RCC_AHB_3,
} RCC_PERIPHERAL_BUS_T;

typedef struct
{
    RCC_PERIPHERAL_BUS_T    APB;
    uint32_t                Periph;
    FunctionalState         NewState;
    bool                    bReset;
} SETUP_RCC_T;

const SETUP_GPIO_T GPIO_InitTable[] =
{
    //Port                          Pin                                     Mode             Speed               Legacy                       OType            PuPd                      DefaultState
    { UART1_PORT,                   { UART1_TX_PIN,                         GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // UART1 TX (GPS)
    { UART1_PORT,                   { UART1_RX_PIN,                         GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_IN_FLOATING,*/   GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // UART1 RX (GPS)
    { GPS_PORT,                     { GPS_RESET_PIN,                        GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_HIGH   }, // GPS_Reset
    { GPS_PORT,                     { GPS_ENABLE_VCC_POWER_PIN,             GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // GPSVCC
    { GPS_PORT,                     { GPS_ENABLE_RF_POWER_PIN,              GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // GPS RF POWER
    { DETECTOR_FAULT_SENSE_PORT,    { DETECTOR_GPS_VCC_FAULT_SENSE_PIN,     GPIO_Mode_IN,    GPIO_Speed_50MHz,   /*GPIO_Mode_IPU,*/           GPIO_OType_PP,   GPIO_PuPd_UP,       },    0           }, // GPS VCC FAULT
    { DETECTOR_FAULT_SENSE_PORT,    { DETECTOR_GPS_RF_FAULT_SENSE_PIN,      GPIO_Mode_IN,    GPIO_Speed_50MHz,   /*GPIO_Mode_IPU,*/           GPIO_OType_PP,   GPIO_PuPd_UP,       },    0           }, // GPS RF FAULT
    { GPIOG,                        { GPIO_Pin_15,                          GPIO_Mode_IN,    GPIO_Speed_50MHz,   /*GPIO_Mode_IN_FLOATING,*/   GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // GPS 1 PPS
    { UART2_PORT,                   { UART2_TX_PIN,                         GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // UART2 TX (Mesh)
    { UART2_PORT,                   { UART2_RX_PIN,                         GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_IPU,*/           GPIO_OType_PP,   GPIO_PuPd_UP,       },    0           }, // UART2 RX (Mesh)
    { UART2_PORT,                   { UART2_RTS_PIN,                        GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // UART2 RTS (Mesh) but we don't use real HW handshake, control output
    { UART2_PORT,                   { UART2_CTS_PIN,                        GPIO_Mode_IN,    GPIO_Speed_50MHz,   /*GPIO_Mode_IPD,*/           GPIO_OType_PP,   GPIO_PuPd_DOWN,     },    0           }, // UART2 CTS (Mesh) but we don't use real HW handshake, control input
    { MESH_RESET_PORT,            { MESH_RESET_PIN,                     GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_OD,*/        GPIO_OType_OD,   GPIO_PuPd_NOPULL,   },    GPIO_HIGH   }, // AT/RESET (Mesh)
    { MESH_VCC_EN_PORT,           { MESH_VCC_EN_PIN,                    GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // VCC ENABLE (Mesh)
    { UART3_PORT,                   { UART3_TX_PIN,                         GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // UART3 TX
    { UART3_PORT,                   { UART3_RX_PIN,                         GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_IN_FLOATING,*/   GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // UART3 RX
    { UART3_PORT,                   { UART3_CK_PIN,                         GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // UART3 CL
    { UART3_PORT,                   { UART3_RTS_PIN,                        GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // UART3 RTS
    { UART3_PORT,                   { UART3_CTS_PIN,                        GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_IN_FLOATING,*/   GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // UART3 CTS
    { UART4_PORT,                   { UART4_TX_PIN,                         GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // UART4 TX (RS485-1) (Slave)
    { UART4_PORT,                   { UART4_RX_PIN,                         GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_IN_FLOATING,*/   GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // UART4 RX (RS485-1) (Slave)
    { GPIOF,                        { GPIO_Pin_4,                           GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // 485RE (RS485-1) (Slave)
    { UART5_TX_PORT,                { UART5_TX_PIN,                         GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // UART5 TX (RS485-2) (PC)
    { UART5_RX_PORT,                { UART5_RX_PIN,                         GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_IN_FLOATING,*/   GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // UART5 RX (RS485-2) (PC)
    { GPIOF,                        { GPIO_Pin_5,                           GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // 485RE2 (RS485-2) (PC)
    { I2C_BUS_PORT,                 { I2C_BUS_SCL,                          GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_OD,*/         GPIO_OType_OD,   GPIO_PuPd_NOPULL,   },    0           }, // I2C1 SCL
    { I2C_BUS_PORT,                 { I2C_BUS_SDA,                          GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_OD,*/         GPIO_OType_OD,   GPIO_PuPd_NOPULL,   },    0           }, // I2C1 SDA
    { LUXVCC_PORT,                  { LUXVCC_PIN,                           GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // LUXVCC ISL POWER
    { DETECTOR_FAULT_SENSE_PORT,    { DETECTOR_LUX_2_8_V_FAULT_SENSE_PIN,   GPIO_Mode_IN,    GPIO_Speed_50MHz,   /*GPIO_Mode_IPU,*/           GPIO_OType_PP,   GPIO_PuPd_UP,       },    0           }, // LUX2.8VFault ISL POWER FAIL
    { GPIOA,                        { GPIO_Pin_5,                           GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // SPI 1 SCK
    { GPIOA,                        { GPIO_Pin_6,                           GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_IN_FLOATING,*/   GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // SPI 1 MISO
    { GPIOA,                        { GPIO_Pin_7,                           GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // SPI 1 MOSI
    { GPIOB,                        { GPIO_Pin_13,                          GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // SPI 2 SCK
    { GPIOB,                        { GPIO_Pin_14,                          GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_IN_FLOATING,*/   GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // SPI 2 MISO
    { GPIOB,                        { GPIO_Pin_15,                          GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // SPI 2 MOSI
    { FRAM_CS_PORT,                 { FRAM_CS_PIN,                          GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_HIGH   }, // FRAM-CS
    { FLASH_EEP_CS_PORT,            { FLASH_EEP_CS_PIN,                     GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_HIGH   }, // FlashEEP-CS
    { GPIOB,                        { GPIO_Pin_0,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // ADC1/ADC2 CH 8
    { GPIOB,                        { GPIO_Pin_1,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // ADC1/ADC2 CH 9
    { GPIOA,                        { GPIO_Pin_2,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // ADC1/ADC2 CH 2
    { GPIOA,                        { GPIO_Pin_3,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // ADC1/ADC2 CH 3
    { GPIOC,                        { GPIO_Pin_0,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // ADC1/ADC2 CH 10
    { GPIOC,                        { GPIO_Pin_1,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // ADC1/ADC2 CH 11
    { GPIOC,                        { GPIO_Pin_2,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // ADC1/ADC2 CH 12
    { GPIOC,                        { GPIO_Pin_3,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // ADC1/ADC2 CH 13
    { GPIOC,                        { GPIO_Pin_4,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // ADC1/ADC2 CH 14
    { GPIOC,                        { GPIO_Pin_5,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // ADC1/ADC2 CH 15
    { GPIOF,                        { GPIO_Pin_6,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // ADC3 CH 4
    { GPIOF,                        { GPIO_Pin_7,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // ADC3 CH 5
    { GPIOF,                        { GPIO_Pin_8,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // ADC3 CH 6
    { GPIOF,                        { GPIO_Pin_9,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // ADC3 CH 7
    { GPIOF,                        { GPIO_Pin_10,                          GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // ADC3 CH 8
    { INDICATION_RELAIS_A_PORT,     { INDICATION_RELAIS_A_BIT,              GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // Indication relais A
    { INDICATION_RELAIS_B_PORT,     { INDICATION_RELAIS_B_BIT,              GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // Indication relais B
    { SVINFO_PORT,                  { SVINFO_PIN,                           GPIO_Mode_IN,    GPIO_Speed_50MHz,   /*GPIO_Mode_IN_FLOATING,*/   GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // SVInfo
    { BUZZER_PORT,                  { BUZZER_PIN,                           GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // TIM9 CH2
    { GPIOA,                        { GPIO_Pin_0,                           GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_HIGH   }, // TIM2 CH1 PartialRemap2
    { GPIOD,                        { GPIO_Pin_0,                           GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // CAN RX
    { GPIOD,                        { GPIO_Pin_1,                           GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // CAN TX
    { GPIOA,                        { GPIO_Pin_1,                           GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_HIGH   }, // TIM2 CH2 PartialRemap2
    { GPIOB,                        { GPIO_Pin_10,                          GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_HIGH   }, // TIM2 CH3 PartialRemap2
    { GPIOB,                        { GPIO_Pin_11,                          GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_HIGH   }, // TIM2 CH4 PartialRemap2
    { GPIOC,                        { GPIO_Pin_6,                           GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_HIGH   }, // TIM3 CH1 RemapFull
    { GPIOC,                        { GPIO_Pin_7,                           GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_HIGH   }, // TIM3 CH2 RemapFull
    { GPIOC,                        { GPIO_Pin_8,                           GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_HIGH   }, // TIM3 CH3 RemapFull
    { GPIOC,                        { GPIO_Pin_9,                           GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_HIGH   }, // TIM3 CH4 RemapFull
    { HEATER_PORT,                  { HEATER_PIN,                           GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // HEATER
    { LED_ENABLE_PORT,              { LED_ENABLE_WHITE_1_PIN,               GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // LED ENABLE 1
    { LED_ENABLE_PORT,              { LED_ENABLE_WHITE_2_PIN,               GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // LED ENABLE 2
    { LED_ENABLE_PORT,              { LED_ENABLE_RED_C1_PIN,                GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // LED ENABLE 3
    { LED_ENABLE_PORT,              { LED_ENABLE_RED_1_PIN,                 GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // LED ENABLE 4
    { LED_ENABLE_PORT,              { LED_ENABLE_RED_2_PIN,                 GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // LED ENABLE 5
    { LED_ENABLE_PORT,              { LED_ENABLE_RED_3_PIN,                 GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // LED ENABLE 6
    { LED_ENABLE_PORT,              { LED_ENABLE_RED_4_PIN,                 GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // LED ENABLE 7
    { LED_ENABLE_PORT,              { LED_ENABLE_RED_C2_PIN,                GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // LED ENABLE 8
    { STATUS_LED_PORT,              { STATUS_LED_SIGNAL_PIN,                GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // SIGLED
    { STATUS_LED_PORT,              { STATUS_LED_RED_PIN,                   GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // STATUS LED RED
    { STATUS_LED_PORT,              { STATUS_LED_GREEN_PIN,                 GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // STATUS LED GREEN
    { GPIOA,                        { GPIO_Pin_4,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // DAC TRACE Output TP21
    { GPIOG,                        { GPIO_Pin_7,                           GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /**/                         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_HIGH   }, // USB disconnect
    { GPIOA,                        { GPIO_Pin_11,                          GPIO_Mode_AF,    GPIO_Speed_100MHz,  /**/                         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // OTG_FS_DM (USB_DM)
    { GPIOA,                        { GPIO_Pin_12,                          GPIO_Mode_AF,    GPIO_Speed_100MHz,  /**/                         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // OTG_FS_DP (USB_DP)
    { DEBUG_PIN_PORT,               { DEBUG_PIN_0,                          GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // DEBUG TESTPIN 6
    { DEBUG_PIN_PORT,               { DEBUG_PIN_1,                          GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // DEBUG TESTPIN 7
    { DEBUG_PIN_PORT,               { DEBUG_PIN_2,                          GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // DEBUG TESTPIN 8
    { DEBUG_PIN_PORT,               { DEBUG_PIN_3,                          GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // DEBUG TESTPIN 9
    { DEBUG_PIN_PORT,               { DEBUG_PIN_4,                          GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // DEBUG TESTPIN 10
    { DEBUG_PIN_PORT,               { DEBUG_PIN_5,                          GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // DEBUG TESTPIN 11
    { HWREV_PIN_PORT,               { HWREV_PIN_0,                          GPIO_Mode_IN,    GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_UP,       },    0           }, // HWREV INPUT 0
#ifdef HWP_CAN_MASTER
    { GPIOD,                        { GPIO_Pin_0,                           GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // CAN RX
    { GPIOD,                        { GPIO_Pin_1,                           GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // CAN TX
#endif

    { 0, { 0, 0, 0, 0, 0 }, 0 } // End Marker
};

const SETUP_IO_REMAP_T IO_RemapTable[] =
{
    //Port           PinSource               AF
    { UART1_PORT,    UART1_TX_PINSOURCE,     GPIO_AF_USART1 },  // Map USART 1 TX to GPIOA Pin 9
    { UART1_PORT,    UART1_RX_PINSOURCE,     GPIO_AF_USART1 },  // Map USART 1 RX to GPIOA Pin 10

    { UART2_PORT,    UART2_TX_PINSOURCE,     GPIO_AF_USART2 },  // Map USART 2 TX to GPIOD Pin 5
    { UART2_PORT,    UART2_RX_PINSOURCE,     GPIO_AF_USART2 },  // Map USART 2 RX to GPIOD Pin 6
//        { UART2_PORT,    UART2_RTS_PINSOURCE,    GPIO_AF_USART2 },  // Map USART 2 RTS to GPIOD Pin 4    but we don't use real HW handshake yet
//        { UART2_PORT,    UART2_CTS_PINSOURCE,    GPIO_AF_USART2 },  // Map USART 2 CTS to GPIOD Pin 3    but we don't use real HW handshake yet

    { UART3_PORT,    UART3_TX_PINSOURCE,     GPIO_AF_USART3 },  // Map USART 3 TX to GPIOD Pin 8
    { UART3_PORT,    UART3_RX_PINSOURCE,     GPIO_AF_USART3 },  // Map USART 3 RX to GPIOD Pin 9
    { UART3_PORT,    UART3_CK_PINSOURCE,     GPIO_AF_USART3 },  // Map USART 3 RTS to GPIOD Pin 10
    { UART3_PORT,    UART3_RTS_PINSOURCE,    GPIO_AF_USART3 },  // Map USART 3 RTS to GPIOD Pin 12
    { UART3_PORT,    UART3_CTS_PINSOURCE,    GPIO_AF_USART3 },  // Map USART 3 CTS to GPIOD Pin 11

    { UART4_PORT,    UART4_TX_PINSOURCE,     GPIO_AF_UART4  },  // Map USART 4 TX to GPIOC Pin 10
    { UART4_PORT,    UART4_RX_PINSOURCE,     GPIO_AF_UART4  },  // Map USART 4 RX to GPIOC Pin 11

    { UART5_TX_PORT, UART5_TX_PINSOURCE,     GPIO_AF_UART5  },  // Map USART 5 TX to GPIOC Pin 12
    { UART5_RX_PORT, UART5_RX_PINSOURCE,     GPIO_AF_UART5  },  // Map USART 5 RX to GPIOD Pin 2

    { I2C_BUS_PORT,  I2C_BUS_SCL_PINSOURCE,  GPIO_AF_I2C1   },  // Map I2C 1 SCL to GPIOB Pin 6
    { I2C_BUS_PORT,  I2C_BUS_SDA_PINSOURCE,  GPIO_AF_I2C1   },  // Map I2C 1 SDA to GPIOB Pin 7

    { GPIOA,         GPIO_PinSource5,        GPIO_AF_SPI1   },  // Map SPI 1 SCK to GPIOA Pin 5
    { GPIOA,         GPIO_PinSource6,        GPIO_AF_SPI1   },  // Map SPI 1 MISO to GPIOA Pin 6
    { GPIOA,         GPIO_PinSource7,        GPIO_AF_SPI1   },  // Map SPI 1 MOSI to GPIOA Pin 7
    { GPIOB,         GPIO_PinSource13,       GPIO_AF_SPI2   },  // Map SPI 2 SCK to GPIOB Pin 13
    { GPIOB,         GPIO_PinSource14,       GPIO_AF_SPI2   },  // Map SPI 2 MISO to GPIOB Pin 14
    { GPIOB,         GPIO_PinSource15,       GPIO_AF_SPI2   },  // Map SPI 2 MOSI to GPIOB Pin 15

    { GPIOA,         GPIO_PinSource8,        GPIO_AF_TIM1   },  // Map TIM 1 Channel 1 to GPIOA Pin 8

    { GPIOA,         GPIO_PinSource0,        GPIO_AF_TIM2   },  // Map TIM 2 Channel 1 to GPIOA Pin 0
    { GPIOA,         GPIO_PinSource1,        GPIO_AF_TIM2   },  // Map TIM 2 Channel 2 to GPIOA Pin 1
    { GPIOB,         GPIO_PinSource10,       GPIO_AF_TIM2   },  // Map TIM 2 Channel 3 to GPIOB Pin 10
    { GPIOB,         GPIO_PinSource11,       GPIO_AF_TIM2   },  // Map TIM 2 Channel 4 to GPIOB Pin 11
    { GPIOC,         GPIO_PinSource6,        GPIO_AF_TIM3   },  // Map TIM 3 Channel 1 to GPIOC Pin 6
    { GPIOC,         GPIO_PinSource7,        GPIO_AF_TIM3   },  // Map TIM 3 Channel 2 to GPIOC Pin 7
    { GPIOC,         GPIO_PinSource8,        GPIO_AF_TIM3   },  // Map TIM 3 Channel 3 to GPIOC Pin 8
    { GPIOC,         GPIO_PinSource9,        GPIO_AF_TIM3   },  // Map TIM 3 Channel 4 to GPIOC Pin 9

    { BUZZER_PORT,   BUZZER_PINSOURCE,       GPIO_AF_TIM9   },  // Map TIM 9 Channel 2 to GPIOE Pin 6
    { GPIOA,         GPIO_PinSource11,       GPIO_AF_OTG1_FS},  // Map OTG_FS_DM (USB_DM) to GPIOA Pin 11
    { GPIOA,         GPIO_PinSource12,       GPIO_AF_OTG1_FS},  // Map OTG_FS_DP (USB_DP) to GPIOA Pin 12

#ifdef HWP_CAN_MASTER
    { GPIOD,         GPIO_PinSource0,        GPIO_AF_CAN1   },  // Map CAN 1 to GPIOD Pin 0
    { GPIOD,         GPIO_PinSource1,        GPIO_AF_CAN1   },  // Map CAN 1 to GPIOD Pin 1
#endif

    { 0, 0, 0 }
};

const SETUP_RCC_T RCC_InitTable[] =
{
    //APB           Peripheral                  State    Reset
    { RCC_APB_1,    RCC_APB1Periph_TIM2,        ENABLE,  true,  },
    { RCC_APB_1,    RCC_APB1Periph_TIM3,        ENABLE,  true,  },
    { RCC_APB_1,    RCC_APB1Periph_TIM4,        ENABLE,  true,  },
    { RCC_APB_1,    RCC_APB1Periph_TIM5,        ENABLE,  true,  },
    { RCC_APB_1,    RCC_APB1Periph_TIM6,        ENABLE,  true,  },
    { RCC_APB_1,    RCC_APB1Periph_TIM7,        DISABLE, true,  },
    { RCC_APB_1,    RCC_APB1Periph_TIM12,       DISABLE, true,  },
    { RCC_APB_1,    RCC_APB1Periph_TIM13,       DISABLE, true,  },
    { RCC_APB_1,    RCC_APB1Periph_TIM14,       DISABLE, true,  },
    { RCC_APB_1,    RCC_APB1Periph_WWDG,        DISABLE, true,  },
    { RCC_APB_1,    RCC_APB1Periph_SPI2,        ENABLE,  true,  },
    { RCC_APB_1,    RCC_APB1Periph_SPI3,        DISABLE, true,  },
    { RCC_APB_1,    RCC_APB1Periph_USART2,      ENABLE,  true,  },
    { RCC_APB_1,    RCC_APB1Periph_USART3,      ENABLE,  true,  },
    { RCC_APB_1,    RCC_APB1Periph_UART4,       ENABLE,  true,  },
    { RCC_APB_1,    RCC_APB1Periph_UART5,       ENABLE,  true,  },
    { RCC_APB_1,    RCC_APB1Periph_I2C1,        ENABLE,  true,  },
    { RCC_APB_1,    RCC_APB1Periph_I2C2,        DISABLE, true,  },
    { RCC_APB_1,    RCC_APB1Periph_I2C3,        DISABLE, true,  },
#ifdef HWP_CAN_MASTER
    { RCC_APB_1,    RCC_APB1Periph_CAN1,        ENABLE,  true,  },
#else
    { RCC_APB_1,    RCC_APB1Periph_CAN1,        DISABLE, true,  },
#endif
    { RCC_APB_1,    RCC_APB1Periph_CAN2,        DISABLE, true,  },
    { RCC_APB_1,    RCC_APB1Periph_PWR,         ENABLE,  false, },              //may not be reset to preserve RTC function
    { RCC_APB_1,    RCC_APB1Periph_DAC,         ENABLE,  true,  },

    { RCC_APB_2,    RCC_APB2Periph_TIM1,        ENABLE,  true,  },
    { RCC_APB_2,    RCC_APB2Periph_TIM8,        DISABLE, true,  },
    { RCC_APB_2,    RCC_APB2Periph_USART1,      ENABLE,  true,  },
    { RCC_APB_2,    RCC_APB2Periph_USART6,      DISABLE, true,  },
    { RCC_APB_2,    RCC_APB2Periph_ADC1,        ENABLE,  true,  },
    { RCC_APB_2,    RCC_APB2Periph_ADC2,        DISABLE, false, },              //not a resettable peripheral (is reset via ADC1)
    { RCC_APB_2,    RCC_APB2Periph_ADC3,        ENABLE,  false, },              //not a resettable peripheral (is reset via ADC1)
    { RCC_APB_2,    RCC_APB2Periph_SDIO,        DISABLE, true,  },
    { RCC_APB_2,    RCC_APB2Periph_SPI1,        ENABLE,  true,  },
    { RCC_APB_2,    RCC_APB2Periph_SYSCFG,      ENABLE,  true,  },              //used by EXTI and USB
    { RCC_APB_2,    RCC_APB2Periph_TIM9,        ENABLE,  true,  },
    { RCC_APB_2,    RCC_APB2Periph_TIM10,       DISABLE, true,  },
    { RCC_APB_2,    RCC_APB2Periph_TIM11,       DISABLE, true,  },

    { RCC_AHB_1,    RCC_AHB1Periph_GPIOA,       ENABLE,  true,  },
    { RCC_AHB_1,    RCC_AHB1Periph_GPIOB,       ENABLE,  true,  },
    { RCC_AHB_1,    RCC_AHB1Periph_GPIOC,       ENABLE,  true,  },
    { RCC_AHB_1,    RCC_AHB1Periph_GPIOD,       ENABLE,  true,  },
    { RCC_AHB_1,    RCC_AHB1Periph_GPIOE,       ENABLE,  true,  },
    { RCC_AHB_1,    RCC_AHB1Periph_GPIOF,       ENABLE,  true,  },
    { RCC_AHB_1,    RCC_AHB1Periph_GPIOG,       ENABLE,  true,  },
    { RCC_AHB_1,    RCC_AHB1Periph_GPIOH,       DISABLE, true,  },
    { RCC_AHB_1,    RCC_AHB1Periph_GPIOI,       DISABLE, true,  },
    { RCC_AHB_1,    RCC_AHB1Periph_CRC,         DISABLE, true,  },
    { RCC_AHB_1,    RCC_AHB1Periph_BKPSRAM,     ENABLE,  false, },              //not a resettable peripheral
    { RCC_AHB_1,    RCC_AHB1Periph_DMA1,        ENABLE,  true,  },
    { RCC_AHB_1,    RCC_AHB1Periph_DMA2,        ENABLE,  true,  },
    { RCC_AHB_1,    RCC_AHB1Periph_ETH_MAC,     DISABLE, true,  },
    { RCC_AHB_1,    RCC_AHB1Periph_ETH_MAC_Tx,  DISABLE, false, },              //not a resettable peripheral
    { RCC_AHB_1,    RCC_AHB1Periph_ETH_MAC_Rx,  DISABLE, false, },              //not a resettable peripheral
    { RCC_AHB_1,    RCC_AHB1Periph_ETH_MAC_PTP, DISABLE, false, },              //not a resettable peripheral
    { RCC_AHB_1,    RCC_AHB1Periph_OTG_HS,      DISABLE, true,  },
    { RCC_AHB_1,    RCC_AHB1Periph_OTG_HS_ULPI, DISABLE, false, },              //not a resettable peripheral

    { RCC_AHB_2,    RCC_AHB2Periph_DCMI,        DISABLE, true,  },
    { RCC_AHB_2,    RCC_AHB2Periph_CRYP,        DISABLE, true,  },
    { RCC_AHB_2,    RCC_AHB2Periph_HASH,        DISABLE, true,  },
    { RCC_AHB_2,    RCC_AHB2Periph_RNG,         DISABLE, true,  },
    { RCC_AHB_2,    RCC_AHB2Periph_OTG_FS,      ENABLE,  true,  },  // used by USB

    { RCC_AHB_3,    RCC_AHB3Periph_FSMC,        DISABLE, true,  },

    { 0, 0, 0, false }
};

const EXTI_LINE_CONFIG_T EXTI_InitTable[] =
{
    //Soft   Port                  PinSource            Line            Mode                    Edge                    State       Comment
    { true,  0,                    0,                   EXTI_Line22,    EXTI_Mode_Interrupt,    EXTI_Trigger_Rising,    ENABLE  },  // Interrupt for RTC_WKUP_IRQn
    { false, EXTI_PortSourceGPIOG, EXTI_PinSource10,    EXTI_Line10,    EXTI_Mode_Interrupt,    EXTI_Trigger_Falling,   ENABLE  },  // LUX2.8VFault ISL POWER FAIL
    { false, EXTI_PortSourceGPIOG, EXTI_PinSource11,    EXTI_Line11,    EXTI_Mode_Interrupt,    EXTI_Trigger_Falling,   ENABLE  },  // GPS RF FAULT
    { false, EXTI_PortSourceGPIOG, EXTI_PinSource12,    EXTI_Line12,    EXTI_Mode_Interrupt,    EXTI_Trigger_Falling,   ENABLE  },  // GPS VCC FAULT
    { false, EXTI_PortSourceGPIOG, EXTI_PinSource15,    EXTI_Line15,    EXTI_Mode_Interrupt,    EXTI_Trigger_Rising,    ENABLE  },  // GPS 1PPS
    { false, EXTI_PortSourceGPIOA, EXTI_PinSource8,     EXTI_Line8,     EXTI_Mode_Interrupt,    EXTI_Trigger_Falling,   DISABLE },  // SVINFO
    { false, 0, 0, 0, 0, 0, DISABLE} // End Marker
};

const NVIC_InitTypeDef NVIC_InitTable[] =
{
    //Channel                   PreemptionPriority                          SubPriority     Command
    { USART1_IRQn,              configLIBRARY_KERNEL_INTERRUPT_PRIORITY,    0,              ENABLE  },
    { USART2_IRQn,              configLIBRARY_KERNEL_INTERRUPT_PRIORITY,    0,              ENABLE  },
    { USART3_IRQn,              configLIBRARY_KERNEL_INTERRUPT_PRIORITY,    0,              ENABLE  },
    { UART4_IRQn,               configLIBRARY_KERNEL_INTERRUPT_PRIORITY,    0,              ENABLE  },
    { UART5_IRQn,               configLIBRARY_KERNEL_INTERRUPT_PRIORITY,    0,              ENABLE  },
    { RTC_WKUP_IRQn,            configLIBRARY_KERNEL_INTERRUPT_PRIORITY,    0,              ENABLE  },
    { EXTI9_5_IRQn,             configLIBRARY_KERNEL_INTERRUPT_PRIORITY,    0,              ENABLE  },
    { EXTI15_10_IRQn,           configLIBRARY_KERNEL_INTERRUPT_PRIORITY,    0,              ENABLE  },
    { ADC_IRQn,                 13,                                         0,              ENABLE  },
    { TIM1_CC_IRQn,             13,                                         0,              ENABLE  },  // SVInfo
    { TIM2_IRQn,                13,                                         0,              ENABLE  },
    { TIM3_IRQn,                13,                                         0,              DISABLE },
    { TIM4_IRQn,                13,                                         0,              ENABLE  },
    { TIM1_BRK_TIM9_IRQn,       13,                                         0,              ENABLE  },
    { TIM6_DAC_IRQn,            13,                                         0,              ENABLE  },
    { SPI1_IRQn,                13,                                         0,              ENABLE  },
    { SPI2_IRQn,                13,                                         0,              ENABLE  },
    { I2C1_EV_IRQn,             13,                                         0,              ENABLE  },
    { I2C1_ER_IRQn,             14 + 1,                                     0,              ENABLE  },
    { OTG_FS_IRQn,              14,                                         0,              DISABLE },

#ifdef HWP_CAN_MASTER
    { CAN1_TX_IRQn,             13,                                         0,              ENABLE  },
    { CAN1_RX0_IRQn,            13,                                         0,              ENABLE  },
    { CAN1_RX1_IRQn,            13,                                         0,              DISABLE },
    { CAN1_SCE_IRQn,            13,                                         0,              ENABLE  },
#endif

    { 0, 0, 0, 0 }
};



/* Implementations ------------------------------------------------------------------*/

void RCC_Configuration(void)
{
    uint8_t idx = 0;

    while (RCC_InitTable[idx].APB != 0)
    {
        switch (RCC_InitTable[idx].APB)
        {
        case RCC_APB_1:
            if (RCC_InitTable[idx].bReset)
                RCC_APB1PeriphResetCmd(RCC_InitTable[idx].Periph, ENABLE);  // enable reset to force a reset of the selected component
            RCC_APB1PeriphClockCmd(RCC_InitTable[idx].Periph, RCC_InitTable[idx].NewState);
            if (RCC_InitTable[idx].bReset)
                RCC_APB1PeriphResetCmd(RCC_InitTable[idx].Periph, DISABLE);  // disable reset
            break;
        case RCC_APB_2:
            if (RCC_InitTable[idx].bReset)
                RCC_APB2PeriphResetCmd(RCC_InitTable[idx].Periph, ENABLE);  // enable reset to force a reset of the selected component
            RCC_APB2PeriphClockCmd(RCC_InitTable[idx].Periph, RCC_InitTable[idx].NewState);
            if (RCC_InitTable[idx].bReset)
                RCC_APB2PeriphResetCmd(RCC_InitTable[idx].Periph, DISABLE);  // disable reset
            break;
        case RCC_AHB_1:
            if (RCC_InitTable[idx].bReset)
                RCC_AHB1PeriphResetCmd(RCC_InitTable[idx].Periph, ENABLE);
            RCC_AHB1PeriphClockCmd(RCC_InitTable[idx].Periph, RCC_InitTable[idx].NewState);
            if (RCC_InitTable[idx].bReset)
                RCC_AHB1PeriphResetCmd(RCC_InitTable[idx].Periph, DISABLE);
            break;
        case RCC_AHB_2:
            if (RCC_InitTable[idx].bReset)
                RCC_AHB2PeriphResetCmd(RCC_InitTable[idx].Periph, ENABLE);
            RCC_AHB2PeriphClockCmd(RCC_InitTable[idx].Periph, RCC_InitTable[idx].NewState);
            if (RCC_InitTable[idx].bReset)
                RCC_AHB2PeriphResetCmd(RCC_InitTable[idx].Periph, DISABLE);
            break;
        case RCC_AHB_3:
            if (RCC_InitTable[idx].bReset)
                RCC_AHB3PeriphResetCmd(RCC_InitTable[idx].Periph, ENABLE);
            RCC_AHB3PeriphClockCmd(RCC_InitTable[idx].Periph, RCC_InitTable[idx].NewState);
            if (RCC_InitTable[idx].bReset)
                RCC_AHB3PeriphResetCmd(RCC_InitTable[idx].Periph, DISABLE);
            break;
        }
        idx++;
    }
}

void EXTI_Configuration(void)
{
    uint8_t idx = 0;
    EXTI_InitTypeDef EXTI_InitStructure;

    while (EXTI_InitTable[idx].EXTI_Line != 0)
    {
        if (EXTI_InitTable[idx].bSoftInterrupt == false)
            SYSCFG_EXTILineConfig(EXTI_InitTable[idx].GPIO_PortSource, EXTI_InitTable[idx].GPIO_PinSource);

        //Clear any possibly pending interrupts
        EXTI_ClearITPendingBit(EXTI_InitTable[idx].EXTI_Line);

        EXTI_InitStructure.EXTI_Line = EXTI_InitTable[idx].EXTI_Line;
        EXTI_InitStructure.EXTI_Mode = EXTI_InitTable[idx].EXTI_Mode;
        EXTI_InitStructure.EXTI_Trigger = EXTI_InitTable[idx].EXTI_Trigger;
        EXTI_InitStructure.EXTI_LineCmd = EXTI_InitTable[idx].EXTI_LineCmd;
        EXTI_Init(&EXTI_InitStructure);

        idx++;
    }
}

void NVIC_Configuration(void)
{
    uint8_t idx = 0;

    // Configure the NVIC Preemption Priority Bits
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);

    while (!(NVIC_InitTable[idx].NVIC_IRQChannel == 0 && NVIC_InitTable[idx].NVIC_IRQChannelPreemptionPriority == 0
             && NVIC_InitTable[idx].NVIC_IRQChannelSubPriority == 0 && NVIC_InitTable[idx].NVIC_IRQChannelCmd == 0))
    {
        NVIC_Init((NVIC_InitTypeDef*)&NVIC_InitTable[idx]);
        idx++;
    }
}

void GPIO_Configuration(void)
{
    uint8_t idx = 0;

    while (GPIO_InitTable[idx].GPIO_Port != 0)
    {
        if (GPIO_InitTable[idx].GPIO_DefaultState == GPIO_LOW)
            GPIO_WriteBit(GPIO_InitTable[idx].GPIO_Port, GPIO_InitTable[idx].GPIO_InitStructure.GPIO_Pin, Bit_RESET);
        else if (GPIO_InitTable[idx].GPIO_DefaultState == GPIO_HIGH)
            GPIO_WriteBit(GPIO_InitTable[idx].GPIO_Port, GPIO_InitTable[idx].GPIO_InitStructure.GPIO_Pin, Bit_SET);

        GPIO_Init(GPIO_InitTable[idx].GPIO_Port, (GPIO_InitTypeDef*)&GPIO_InitTable[idx].GPIO_InitStructure);

        idx++;
    }
}

void GPIO_PinUnInit(GPIO_TypeDef * port, uint16_t pin)
{
    GPIO_InitTypeDef GPIO_InitValue;
    GPIO_StructInit(&GPIO_InitValue);

    GPIO_InitValue.GPIO_Pin = pin;

    GPIO_Init(port, &GPIO_InitValue);
}

bool GPIO_PinInit(GPIO_TypeDef * port, uint16_t pin)
{
    uint8_t idx = 0;

    while (GPIO_InitTable[idx].GPIO_Port != 0)
    {
        if (GPIO_InitTable[idx].GPIO_Port == port && GPIO_InitTable[idx].GPIO_InitStructure.GPIO_Pin == pin)
        {
            if (GPIO_InitTable[idx].GPIO_DefaultState == GPIO_LOW)
                GPIO_WriteBit(GPIO_InitTable[idx].GPIO_Port, GPIO_InitTable[idx].GPIO_InitStructure.GPIO_Pin, Bit_RESET);
            else if (GPIO_InitTable[idx].GPIO_DefaultState == GPIO_HIGH)
                GPIO_WriteBit(GPIO_InitTable[idx].GPIO_Port, GPIO_InitTable[idx].GPIO_InitStructure.GPIO_Pin, Bit_SET);

            GPIO_Init(GPIO_InitTable[idx].GPIO_Port, (GPIO_InitTypeDef*)&GPIO_InitTable[idx].GPIO_InitStructure);

            return TRUE;
        }

        idx++;
    }

    return FALSE;
}

void IO_Remap(void)
{
    uint8_t idx = 0;

    while (IO_RemapTable[idx].GPIOx != 0)
    {
        GPIO_PinAFConfig(IO_RemapTable[idx].GPIOx, IO_RemapTable[idx].GPIO_PinSource, IO_RemapTable[idx].GPIO_AF);
        idx++;
    }
}

#if (VERSION_IS_DEBUG == 0)
__attribute__ ((section (".ramfunc"))) static FLASH_Status HWCONFIG_Init_ob_launch(void)
{
    /* Set the OPTSTRT bit in OPTCR register */
    *(__IO uint8_t *)OPTCR_BYTE0_ADDRESS |= FLASH_OPTCR_OPTSTRT;

    /*
     * Wait for last operation to be completed without using any code living in flash.
     * Also reset the watchdog as this operation takes several hundreds of milliseconds
     */
    while ((FLASH->SR & FLASH_FLAG_BSY) == FLASH_FLAG_BSY)
        IWDG->KR = (uint16_t)0xAAAA;

    return FLASH_GetStatus();
}
#endif

void HWCONFIG_Init(void)
{
#if (VERSION_IS_DEBUG == 0)
    // Option bytes have to be set
    /*
        uint8_t FLASH_OB_GetUser(void);
        void FLASH_OB_UserConfig(uint8_t OB_IWDG, uint8_t OB_STOP, uint8_t OB_STDBY);
     */
    // Set flash readout protection if not already set.
    // Activate brownout reset if not already activated.
    bool b_enable_bor = FALSE;
    bool b_enable_rdp = FALSE;

    if (FLASH_OB_GetBOR() != OB_BOR_LEVEL3)
    {
        b_enable_bor = TRUE;
    }

    if (FLASH_OB_GetRDP() == RESET)
    {
        b_enable_rdp = TRUE;
    }

    if (b_enable_bor || b_enable_rdp)
    {
        FLASH_OB_Unlock();  // unlock flash

        if (b_enable_bor)
        {
            // Enable brownout reset
            FLASH_OB_BORConfig(OB_BOR_LEVEL3);
        }

        if (b_enable_rdp)
        {
            // Enable readout protection
            FLASH_OB_RDPConfig(OB_RDP_Level_1);
        }

        HWCONFIG_Init_ob_launch();  // start programming of new settings
        FLASH_OB_Lock();  // lock flash
    }
#endif

    // System Clocks Configuration
    RCC_Configuration();

    // NVIC configuration
    NVIC_Configuration();

    // Configure the GPIO ports
    GPIO_Configuration();

    // Configure external interrupt lines
    EXTI_Configuration();

    // Remap IOs
    IO_Remap();
}
