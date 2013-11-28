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
        { LED_ENABLE_PORT,              { LED_ENABLE_SV1_PIN,                   GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // LED ENABLE 1
        { LED_ENABLE_PORT,              { LED_ENABLE_SV2_PIN,                   GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // LED ENABLE 2
        { LED_ENABLE_PORT,              { LED_ENABLE_SV3_PIN,                   GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // LED ENABLE 3
        { LED_ENABLE_PORT,              { LED_ENABLE_SV4_PIN,                   GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // LED ENABLE 4
        { LED_ENABLE_PORT,              { LED_ENABLE_SV5_PIN,                   GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // LED ENABLE 5
        { LED_ENABLE_PORT,              { LED_ENABLE_SV6_PIN,                   GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // LED ENABLE 6
        { LED_ENABLE_PORT,              { LED_ENABLE_SV7_PIN,                   GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // LED ENABLE 7
        { LED_ENABLE_PORT,              { LED_ENABLE_SV8_PIN,                   GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // LED ENABLE 8
        { LED_ENABLE_PORT,              { LED_ENABLE_SV9_PIN,                   GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // LED ENABLE 9
        { LED_ENABLE_PORT,              { LED_ENABLE_SV10_PIN,                  GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // LED ENABLE 10
        { LED_ENABLE_PORT,              { LED_ENABLE_SV11_PIN,                  GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // LED ENABLE 11
        { LED_ENABLE_PORT,              { LED_ENABLE_SV12_PIN,                  GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // LED ENABLE 12
        { LED_ENABLE_PORT,              { LED_ENABLE_SV13_PIN,                  GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // LED ENABLE 13
        { LED_ENABLE_PORT,              { LED_ENABLE_SV14_PIN,                  GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // LED ENABLE 14
        { GPIOB,                        { GPIO_Pin_0,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // ADC1/ADC2 CH 8
        { GPIOB,                        { GPIO_Pin_1,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // ADC1/ADC2 CH 9
        { GPIOC,                        { GPIO_Pin_0,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // ADC1/ADC2 CH 10
        { GPIOC,                        { GPIO_Pin_1,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // ADC1/ADC2 CH 11
        { GPIOC,                        { GPIO_Pin_2,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // ADC1/ADC2 CH 12
        { GPIOC,                        { GPIO_Pin_3,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // ADC1/ADC2 CH 13
        { GPIOC,                        { GPIO_Pin_4,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // ADC1/ADC2 CH 14
        { GPIOC,                        { GPIO_Pin_5,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // ADC1/ADC2 CH 15
        { GPIOA,                        { GPIO_Pin_0,                           GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_HIGH   }, // TIM2 CH1 PartialRemap2
        { GPIOD,                        { GPIO_Pin_0,                           GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // CAN RX
        { GPIOD,                        { GPIO_Pin_1,                           GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // CAN TX
        { GPIOF,                        { GPIO_Pin_11,                          GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_HIGH   }, // EN_SENS  //ToDo [medium] auf sinnvollen Ausgangswert setzen
        { GPIOG,                        { GPIO_Pin_10,                          GPIO_Mode_IN,    GPIO_Speed_50MHz,   /*GPIO_Mode_IPU,*/           GPIO_OType_PP,   GPIO_PuPd_UP,       },    0           }, // SENSFault
        { GPIOC,                        { GPIO_Pin_10,                          GPIO_Mode_IN,    GPIO_Speed_50MHz,   /*GPIO_Mode_IPU,*/           GPIO_OType_PP,   GPIO_PuPd_UP,       },    0           }, // S1_1
        { GPIOC,                        { GPIO_Pin_11,                          GPIO_Mode_IN,    GPIO_Speed_50MHz,   /*GPIO_Mode_IPU,*/           GPIO_OType_PP,   GPIO_PuPd_UP,       },    0           }, // S1_2
        { GPIOC,                        { GPIO_Pin_12,                          GPIO_Mode_IN,    GPIO_Speed_50MHz,   /*GPIO_Mode_IPU,*/           GPIO_OType_PP,   GPIO_PuPd_UP,       },    0           }, // S1_3
        { GPIOG,                        { GPIO_Pin_0,                           GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // SW1
        { GPIOG,                        { GPIO_Pin_1,                           GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // SW2
        { GPIOG,                        { GPIO_Pin_2,                           GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // SW3
        { GPIOG,                        { GPIO_Pin_3,                           GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // SW4
        { STATUS_LED_PORT,              { STATUS_LED_RED_PIN,                   GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // STATUS LED RED
        { STATUS_LED_PORT,              { STATUS_LED_GREEN_PIN,                 GPIO_Mode_OUT,   GPIO_Speed_50MHz,   /*GPIO_Mode_Out_PP,*/        GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    GPIO_LOW    }, // STATUS LED GREEN
        { UART3_PORT,                   { UART3_TX_PIN,                         GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // UART3 TX
        { UART3_PORT,                   { UART3_RX_PIN,                         GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_IN_FLOATING,*/   GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // UART3 RX
        { UART3_PORT,                   { UART3_CK_PIN,                         GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // UART3 CL
        { UART3_PORT,                   { UART3_RTS_PIN,                        GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_AF_PP,*/         GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // UART3 RTS
        { UART3_PORT,                   { UART3_CTS_PIN,                        GPIO_Mode_AF,    GPIO_Speed_50MHz,   /*GPIO_Mode_IN_FLOATING,*/   GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // UART3 CTS
        { GPIOA,                        { GPIO_Pin_4,                           GPIO_Mode_AN,    GPIO_Speed_50MHz,   /*GPIO_Mode_AIN,*/           GPIO_OType_PP,   GPIO_PuPd_NOPULL,   },    0           }, // DAC TRACE Output TP21
        { 0, { 0, 0, 0, 0, 0 }, 0 } // End Marker
};

const SETUP_IO_REMAP_T IO_RemapTable[] =
{
        //Port           PinSource               AF

        { GPIOA,         GPIO_PinSource0,        GPIO_AF_TIM2   },  // Map TIM 2 Channel 1 to GPIOA Pin 0

        { GPIOD,         GPIO_PinSource0,        GPIO_AF_CAN1   },  // Map CAN 1 to GPIOD Pin 0
        { GPIOD,         GPIO_PinSource1,        GPIO_AF_CAN1   },  // Map CAN 1 to GPIOD Pin 1

        { UART3_PORT,    UART3_TX_PINSOURCE,     GPIO_AF_USART3 },  // Map USART 3 TX to GPIOD Pin 8
        { UART3_PORT,    UART3_RX_PINSOURCE,     GPIO_AF_USART3 },  // Map USART 3 RX to GPIOD Pin 9
        { UART3_PORT,    UART3_CK_PINSOURCE,     GPIO_AF_USART3 },  // Map USART 3 RTS to GPIOD Pin 10
        { UART3_PORT,    UART3_RTS_PINSOURCE,    GPIO_AF_USART3 },  // Map USART 3 RTS to GPIOD Pin 12
        { UART3_PORT,    UART3_CTS_PINSOURCE,    GPIO_AF_USART3 },  // Map USART 3 CTS to GPIOD Pin 11

        { 0, 0, 0 }
};

const SETUP_RCC_T RCC_InitTable[] =
{
        //APB           Peripheral                  State    Reset
        { RCC_APB_1,    RCC_APB1Periph_TIM2,        ENABLE,  true,  },
        { RCC_APB_1,    RCC_APB1Periph_TIM3,        ENABLE,  true,  },  // todo check
        { RCC_APB_1,    RCC_APB1Periph_TIM4,        ENABLE,  true,  },
        { RCC_APB_1,    RCC_APB1Periph_TIM5,        ENABLE,  true,  },  // todo check
        { RCC_APB_1,    RCC_APB1Periph_TIM6,        ENABLE,  true,  },  // todo check
        { RCC_APB_1,    RCC_APB1Periph_TIM7,        DISABLE, true,  },
        { RCC_APB_1,    RCC_APB1Periph_TIM12,       DISABLE, true,  },
        { RCC_APB_1,    RCC_APB1Periph_TIM13,       DISABLE, true,  },
        { RCC_APB_1,    RCC_APB1Periph_TIM14,       DISABLE, true,  },
        { RCC_APB_1,    RCC_APB1Periph_WWDG,        DISABLE, true,  },
        { RCC_APB_1,    RCC_APB1Periph_SPI2,        DISABLE, true,  },
        { RCC_APB_1,    RCC_APB1Periph_SPI3,        DISABLE, true,  },
        { RCC_APB_1,    RCC_APB1Periph_USART2,      DISABLE, true,  },
        { RCC_APB_1,    RCC_APB1Periph_USART3,      ENABLE,  true,  },
        { RCC_APB_1,    RCC_APB1Periph_UART4,       DISABLE, true,  },
        { RCC_APB_1,    RCC_APB1Periph_UART5,       DISABLE, true,  },
        { RCC_APB_1,    RCC_APB1Periph_I2C1,        DISABLE, true,  },
        { RCC_APB_1,    RCC_APB1Periph_I2C2,        DISABLE, true,  },
        { RCC_APB_1,    RCC_APB1Periph_I2C3,        DISABLE, true,  },
        { RCC_APB_1,    RCC_APB1Periph_CAN1,        ENABLE,  true,  },
        { RCC_APB_1,    RCC_APB1Periph_CAN2,        DISABLE, true,  },
        { RCC_APB_1,    RCC_APB1Periph_PWR,         ENABLE,  false, },  // may not be reset to preserve RTC function
        { RCC_APB_1,    RCC_APB1Periph_DAC,         ENABLE,  true,  },  // todo check

        { RCC_APB_2,    RCC_APB2Periph_TIM1,        ENABLE,  true,  },
        { RCC_APB_2,    RCC_APB2Periph_TIM8,        DISABLE, true,  },
        { RCC_APB_2,    RCC_APB2Periph_USART1,      DISABLE, true,  },
        { RCC_APB_2,    RCC_APB2Periph_USART6,      DISABLE, true,  },
        { RCC_APB_2,    RCC_APB2Periph_ADC1,        ENABLE,  true,  },
        { RCC_APB_2,    RCC_APB2Periph_ADC2,        ENABLE,  false, },  // not a resettable peripheral (is reset via ADC1)
        { RCC_APB_2,    RCC_APB2Periph_ADC3,        DISABLE, false, },  // not a resettable peripheral (is reset via ADC1)
        { RCC_APB_2,    RCC_APB2Periph_SDIO,        DISABLE, true,  },
        { RCC_APB_2,    RCC_APB2Periph_SPI1,        DISABLE, true,  },
        { RCC_APB_2,    RCC_APB2Periph_SYSCFG,      ENABLE,  true,  },  // used by EXTI and USB
        { RCC_APB_2,    RCC_APB2Periph_TIM9,        ENABLE,  true,  },  // todo check
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
        { RCC_AHB_1,    RCC_AHB1Periph_BKPSRAM,     ENABLE,  false, },  // not a resettable peripheral
        { RCC_AHB_1,    RCC_AHB1Periph_DMA1,        ENABLE,  true,  },
        { RCC_AHB_1,    RCC_AHB1Periph_DMA2,        ENABLE,  true,  },
        { RCC_AHB_1,    RCC_AHB1Periph_ETH_MAC,     DISABLE, true,  },
        { RCC_AHB_1,    RCC_AHB1Periph_ETH_MAC_Tx,  DISABLE, false, },  // not a resettable peripheral
        { RCC_AHB_1,    RCC_AHB1Periph_ETH_MAC_Rx,  DISABLE, false, },  // not a resettable peripheral
        { RCC_AHB_1,    RCC_AHB1Periph_ETH_MAC_PTP, DISABLE, false, },  // not a resettable peripheral
        { RCC_AHB_1,    RCC_AHB1Periph_OTG_HS,      DISABLE, true,  },
        { RCC_AHB_1,    RCC_AHB1Periph_OTG_HS_ULPI, DISABLE, false, },  // not a resettable peripheral

        { RCC_AHB_2,    RCC_AHB2Periph_DCMI,        DISABLE, true,  },
        { RCC_AHB_2,    RCC_AHB2Periph_CRYP,        DISABLE, true,  },
        { RCC_AHB_2,    RCC_AHB2Periph_HASH,        DISABLE, true,  },
        { RCC_AHB_2,    RCC_AHB2Periph_RNG,         DISABLE, true,  },
        { RCC_AHB_2,    RCC_AHB2Periph_OTG_FS,      DISABLE, true,  },  // used by USB

        { RCC_AHB_3,    RCC_AHB3Periph_FSMC,        DISABLE, true,  },

        { 0, 0, 0, false }
};

const EXTI_LINE_CONFIG_T EXTI_InitTable[] =
{
        //Soft   Port                  PinSource            Line            Mode                    Edge                    State       Comment
        { true,  0,                    0,                   EXTI_Line22,    EXTI_Mode_Interrupt,    EXTI_Trigger_Rising,    ENABLE },   // Interrupt for RTC_WKUP_IRQn
        { false, EXTI_PortSourceGPIOG, EXTI_PinSource10,    EXTI_Line10,    EXTI_Mode_Interrupt,    EXTI_Trigger_Falling,   ENABLE },   // SENSFault
        { false, 0, 0, 0, 0, 0, DISABLE} // End Marker
};

const NVIC_InitTypeDef NVIC_InitTable[] =
{
        //Channel                   PreemptionPriority                          SubPriority     Command
        { USART1_IRQn,              configLIBRARY_KERNEL_INTERRUPT_PRIORITY,    0,              DISABLE },
        { USART2_IRQn,              configLIBRARY_KERNEL_INTERRUPT_PRIORITY,    0,              DISABLE },
        { USART3_IRQn,              configLIBRARY_KERNEL_INTERRUPT_PRIORITY,    0,              ENABLE  },
        { UART4_IRQn,               configLIBRARY_KERNEL_INTERRUPT_PRIORITY,    0,              DISABLE },
        { UART5_IRQn,               configLIBRARY_KERNEL_INTERRUPT_PRIORITY,    0,              DISABLE },
        { RTC_WKUP_IRQn,            configLIBRARY_KERNEL_INTERRUPT_PRIORITY,    0,              ENABLE  },
        { EXTI9_5_IRQn,             configLIBRARY_KERNEL_INTERRUPT_PRIORITY,    0,              ENABLE  },  // todo check
        { EXTI15_10_IRQn,           configLIBRARY_KERNEL_INTERRUPT_PRIORITY,    0,              ENABLE  },
        { ADC_IRQn,                 13,                                         0,              ENABLE  },
        { CAN1_TX_IRQn,             13,                                         0,              ENABLE  },
        { CAN1_RX0_IRQn,            13,                                         0,              ENABLE  },
        { CAN1_RX1_IRQn,            13,                                         0,              DISABLE },
        { CAN1_SCE_IRQn,            13,                                         0,              ENABLE  },
        { TIM1_CC_IRQn,             13,                                         0,              ENABLE  },  // SVInfo
        { TIM2_IRQn,                13,                                         0,              ENABLE  },
        { TIM3_IRQn,                13,                                         0,              DISABLE },
        { TIM4_IRQn,                13,                                         0,              ENABLE  },
        { TIM1_BRK_TIM9_IRQn,       13,                                         0,              ENABLE  },  // todo check
        { TIM6_DAC_IRQn,            13,                                         0,              ENABLE  },  // todo check
        { SPI1_IRQn,                13,                                         0,              DISABLE },
        { SPI2_IRQn,                13,                                         0,              DISABLE },
        { I2C1_EV_IRQn,             13,                                         0,              DISABLE },
        { I2C1_ER_IRQn,             14 + 1,                                     0,              DISABLE },
        { OTG_FS_IRQn,              14,                                         0,              DISABLE },
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

void HWCONFIG_Init(void)
{
    // Option bytes have to be set       // todo [stm32f2xx]
    /*
        void FLASH_OB_Unlock(void);
        void FLASH_OB_Lock(void);
        void FLASH_OB_WRPConfig(uint32_t OB_WRP, FunctionalState NewState);
        void FLASH_OB_RDPConfig(uint8_t OB_RDP);
        void FLASH_OB_UserConfig(uint8_t OB_IWDG, uint8_t OB_STOP, uint8_t OB_STDBY);
        void FLASH_OB_BORConfig(uint8_t OB_BOR);
        FLASH_Status FLASH_OB_Launch(void);
        uint8_t FLASH_OB_GetUser(void);
        uint16_t FLASH_OB_GetWRP(void);
        FlagStatus FLASH_OB_GetRDP(void);
        uint8_t FLASH_OB_GetBOR(void);
     */

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
