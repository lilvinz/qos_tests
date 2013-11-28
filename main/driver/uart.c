#include "stdheader.h"
#include "uart.h"

#include "projdefs.h"
#include "driver.h"
#include "hw_configuration.h"

#include "debug.h"

/* Private variables ---------------------------------------------------------*/
static volatile xQueueHandle xQueueUARTTX[UART_CONFIG_CHANNEL_COUNT];
static volatile xQueueHandle xQueueUARTRX[UART_CONFIG_CHANNEL_COUNT];
static volatile UART_ChannelConfig_t UartChannelConfig[UART_CONFIG_CHANNEL_COUNT];

const UART_ChannelHardwareTypeDef UartChannelHardware[UART_CONFIG_CHANNEL_COUNT] =
{
    //  TX PORT         TX PIN          RX PORT         RX PIN
    {   UART1_PORT,     UART1_TX_PIN,   UART1_PORT,     UART1_RX_PIN    },
    {   UART2_PORT,     UART2_TX_PIN,   UART2_PORT,     UART2_RX_PIN    },
    {   UART3_PORT,     UART3_TX_PIN,   UART3_PORT,     UART3_RX_PIN    },
    {   UART4_PORT,     UART4_TX_PIN,   UART4_PORT,     UART4_RX_PIN    },
    {   UART5_TX_PORT,  UART5_TX_PIN,   UART5_RX_PORT,  UART5_RX_PIN    }
};




/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/

/**
 * @brief  This function handles USARTy global interrupt request.
 * @param  None
 * @retval None
 */
void USART1_IRQHandler(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    portCHAR cChar;

    // ATTENTION:
    // The ORE-FLAG will generate an interrupt if RXNEIE is set. But "USART_GetITStatus(USART1, USART_IT_ORE) != RESET)"
    // needs the EIE to be set to work!
    // ==>> Do not use "USART_GetITStatus(USART1, USART_IT_ORE)" in IRQHandler!
    // ==>> Use "USART_GetFlagStatus(USART1, USART_FLAG_ORE)" instead!
    if ((USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) || (USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET) || (USART_GetITStatus(
                USART1, USART_IT_FE) != RESET) || (USART_GetITStatus(USART1, USART_IT_NE) != RESET))
    {
        /* Read one byte from the receive data register */
        cChar = USART_ReceiveData(USART1);
        if (UartChannelConfig[UART_CHANNEL_1].ClearParity7Bit == TRUE)
            cChar &= 0x7f;
        xQueueSendFromISR(xQueueUARTRX[UART_CHANNEL_1], &cChar, &xHigherPriorityTaskWoken);
        DRIVER_IO_REQUEST();
    }

    if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
    {
        if (xQueueReceiveFromISR(xQueueUARTTX[UART_CHANNEL_1], (void*) &cChar, &xHigherPriorityTaskWoken) == pdTRUE)
        {
            /* Write one byte to the transmit data register */
            USART_SendData(USART1, cChar);
        }
        else
        {
            /* Disable the USART1 Transmit interrupt */
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
        }
    }

    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

void USART2_IRQHandler(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    portCHAR cChar;

    // ATTENTION:
    // The ORE-FLAG will generate an interrupt if RXNEIE is set. But "USART_GetITStatus(USART2, USART_IT_ORE) != RESET)"
    // needs the EIE to be set to work!
    // ==>> Do not use "USART_GetITStatus(USART2, USART_IT_ORE)" in IRQHandler!
    // ==>> Use "USART_GetFlagStatus(USART2, USART_FLAG_ORE)" instead!
    if ((USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) || (USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET) || (USART_GetITStatus(
                USART2, USART_IT_FE) != RESET) || (USART_GetITStatus(USART2, USART_IT_NE) != RESET))
    {
        /* Read one byte from the receive data register */
        cChar = USART_ReceiveData(USART2);
        if (UartChannelConfig[UART_CHANNEL_2].ClearParity7Bit == TRUE)
            cChar &= 0x7f;
        xQueueSendFromISR(xQueueUARTRX[UART_CHANNEL_2], &cChar, &xHigherPriorityTaskWoken);
        DRIVER_IO_REQUEST();
    }

    if (USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
    {
        if (xQueueReceiveFromISR(xQueueUARTTX[UART_CHANNEL_2], (void*) &cChar, &xHigherPriorityTaskWoken) == pdTRUE)
        {
            /* Write one byte to the transmit data register */
            USART_SendData(USART2, cChar);
        }
        else
        {
            /* Disable the USART2 Transmit interrupt */
            USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
        }
    }

    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

void USART3_IRQHandler(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    portCHAR cChar;

    // ATTENTION:
    // The ORE-FLAG will generate an interrupt if RXNEIE is set. But "USART_GetITStatus(USART3, USART_IT_ORE) != RESET)"
    // needs the EIE to be set to work!
    // ==>> Do not use "USART_GetITStatus(USART3, USART_IT_ORE)" in IRQHandler!
    // ==>> Use "USART_GetFlagStatus(USART3, USART_FLAG_ORE)" instead!
    if ((USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) || (USART_GetFlagStatus(USART3, USART_FLAG_ORE) != RESET) || (USART_GetITStatus(
                USART3, USART_IT_FE) != RESET) || (USART_GetITStatus(USART3, USART_IT_NE) != RESET))
    {
        /* Read one byte from the receive data register */
        cChar = USART_ReceiveData(USART3);
        if (UartChannelConfig[UART_CHANNEL_3].ClearParity7Bit == TRUE)
            cChar &= 0x7f;
        xQueueSendFromISR(xQueueUARTRX[UART_CHANNEL_3], &cChar, &xHigherPriorityTaskWoken);
        DRIVER_IO_REQUEST();
    }

    if (USART_GetITStatus(USART3, USART_IT_TXE) != RESET)
    {
        if (xQueueReceiveFromISR(xQueueUARTTX[UART_CHANNEL_3], (void*) &cChar, &xHigherPriorityTaskWoken) == pdTRUE)
        {
            /* Write one byte to the transmit data register */
            USART_SendData(USART3, cChar);
        }
        else
        {
            /* Disable the USART3 Transmit interrupt */
            USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
        }
    }

    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

void UART4_IRQHandler(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    portCHAR cChar;

    // ATTENTION:
    // The ORE-FLAG will generate an interrupt if RXNEIE is set. But "USART_GetITStatus(UART4, USART_IT_ORE) != RESET)"
    // needs the EIE to be set to work!
    // ==>> Do not use "USART_GetITStatus(UART4, USART_IT_ORE)" in IRQHandler!
    // ==>> Use "USART_GetFlagStatus(UART4, USART_FLAG_ORE)" instead!
    if ((USART_GetITStatus(UART4, USART_IT_RXNE) != RESET) || (USART_GetFlagStatus(UART4, USART_FLAG_ORE) != RESET))
    {
        /* Read one byte from the receive data register */
        cChar = USART_ReceiveData(UART4);
        if (UartChannelConfig[UART_CHANNEL_4].ClearParity7Bit == TRUE)
            cChar &= 0x7f;
        xQueueSendFromISR(xQueueUARTRX[UART_CHANNEL_4], &cChar, &xHigherPriorityTaskWoken);
        DRIVER_IO_REQUEST();
    }

    if (USART_GetITStatus(UART4, USART_IT_TXE) != RESET)
    {
        if (xQueueReceiveFromISR(xQueueUARTTX[UART_CHANNEL_4], (void*) &cChar, &xHigherPriorityTaskWoken) == pdTRUE)
        {
            USART_ITConfig(UART4, USART_IT_TC, DISABLE);
            //USART_ClearITPendingBit(UART4, USART_IT_TC);
            /* Write one byte to the transmit data register */
            USART_SendData(UART4, cChar);
        }
        else
        {
            /* Disable the UART4 Transmit interrupt */
            USART_ITConfig(UART4, USART_IT_TXE, DISABLE);
            USART_ITConfig(UART4, USART_IT_TC, ENABLE);
        }
    }

    if (USART_GetITStatus(UART4, USART_IT_TC) != RESET)
    {
        if (xQueueIsQueueEmptyFromISR(xQueueUARTTX[UART_CHANNEL_4]))
        {
            // nothing to send. switch to RX mode
            cChar = USART_ReceiveData(UART4); // clear pending interrupt
            GPIO_WriteBit(RS485_1_DIR_PORT, RS485_1_DIR_PIN, Bit_RESET); // activate receive mode
            // ToDo: Disable DIR Pin after half byte tx time using a timer to avoid gaps in uart4<->5 passtrough mode
            UART4->CR1 |= USART_Mode_Rx;  // enable receive
            USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
        }
        else
        {
            // whops, new data arrived in the meantime. enable TX again.
            USART_ITConfig(UART4, USART_IT_TXE, ENABLE);
        }
        USART_ITConfig(UART4, USART_IT_TC, DISABLE);
    }

    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

void UART5_IRQHandler(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    portCHAR cChar;

    // ATTENTION:
    // The ORE-FLAG will generate an interrupt if RXNEIE is set. But "USART_GetITStatus(UART5, USART_IT_ORE) != RESET)"
    // needs the EIE to be set to work!
    // ==>> Do not use "USART_GetITStatus(UART5, USART_IT_ORE)" in IRQHandler!
    // ==>> Use "USART_GetFlagStatus(UART5, USART_FLAG_ORE)" instead!
    if ((USART_GetITStatus(UART5, USART_IT_RXNE) != RESET) || (USART_GetFlagStatus(UART5, USART_FLAG_ORE) != RESET))
    {
        /* Read one byte from the receive data register */
        cChar = USART_ReceiveData(UART5);
        if (UartChannelConfig[UART_CHANNEL_5].ClearParity7Bit == TRUE)
            cChar &= 0x7f;
        xQueueSendFromISR(xQueueUARTRX[UART_CHANNEL_5], &cChar, &xHigherPriorityTaskWoken);
        DRIVER_IO_REQUEST();
    }

    if (USART_GetITStatus(UART5, USART_IT_TXE) != RESET)
    {
        if (xQueueReceiveFromISR(xQueueUARTTX[UART_CHANNEL_5], (void*) &cChar, &xHigherPriorityTaskWoken) == pdTRUE)
        {
            USART_ITConfig(UART5, USART_IT_TC, DISABLE);
            //USART_ClearITPendingBit(UART5, USART_IT_TC);
            /* Write one byte to the transmit data register */
            USART_SendData(UART5, cChar);
        }
        else
        {
            /* Disable the UART5 Transmit interrupt */
            USART_ITConfig(UART5, USART_IT_TXE, DISABLE);
            USART_ITConfig(UART5, USART_IT_TC, ENABLE);
        }
    }

    if (USART_GetITStatus(UART5, USART_IT_TC) != RESET)
    {
        if (xQueueIsQueueEmptyFromISR(xQueueUARTTX[UART_CHANNEL_5]))
        {
            // nothing to send. switch to RX mode
            cChar = USART_ReceiveData(UART5); // clear pending interrupt
            GPIO_WriteBit(RS485_2_DIR_PORT, RS485_2_DIR_PIN, Bit_RESET); // activate receive mode
            // ToDo: Disable DIR Pin after half byte tx time using a timer to avoid gaps in uart4<->5 passtrough mode
            UART5->CR1 |= USART_Mode_Rx;  // enable receive
            USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
        }
        else
        {
            // whops, new data arrived in the meantime. enable TX again.
            USART_ITConfig(UART5, USART_IT_TXE, ENABLE);
        }
        USART_ITConfig(UART5, USART_IT_TC, DISABLE);
    }

    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

int UART1_PutChar(char c)
{
    if (xQueueSendToBack(xQueueUARTTX[UART_CHANNEL_1], (void *) &c, (portTickType) 100) == pdPASS)
    {
        USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
    }
    return c;
}

int UART1_GetChar(void)
{
    char c;
    if (pdTRUE == xQueueReceive(xQueueUARTRX[UART_CHANNEL_1], (void *) &c, portMAX_DELAY))
        return (int) c;
    return -1;
}

int UART2_PutChar(char c)
{
    if (xQueueSendToBack(xQueueUARTTX[UART_CHANNEL_2], (void *) &c, (portTickType) 100) == pdPASS)
    {
        USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
    }
    return c;
}

int UART2_GetChar(void)
{
    char c;
    if (pdTRUE == xQueueReceive(xQueueUARTRX[UART_CHANNEL_2], (void *) &c, portMAX_DELAY))
    {
        return (int) c;
    }
    return -1;
}

int UART3_PutChar(char c)
{
    if (xQueueSendToBack(xQueueUARTTX[UART_CHANNEL_3], (void *) &c, (portTickType) 100) == pdPASS)
    {
        USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
    }
    return c;
}

int UART3_GetChar(void)
{
    char c;
    if (pdTRUE == xQueueReceive(xQueueUARTRX[UART_CHANNEL_3], (void *) &c, portMAX_DELAY))
        return (int) c;
    return -1;
}

int UART4_PutChar(char c)
{
    if (xQueueSendToBack(xQueueUARTTX[UART_CHANNEL_4], (void *) &c, (portTickType) 100) == pdPASS)
    {
        CRITICAL_SECTION_ENTER();
        GPIO_WriteBit(RS485_1_DIR_PORT, RS485_1_DIR_PIN, Bit_SET); // activate send mode
        USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);
        UART4->CR1 &= (~USART_Mode_Rx);  // disable receive
        USART_ITConfig(UART4, USART_IT_TXE, ENABLE);
        CRITICAL_SECTION_LEAVE();
    }
    return c;
}

int UART4_GetChar(void)
{
    char c;
    if (pdTRUE == xQueueReceive(xQueueUARTRX[UART_CHANNEL_4], (void *) &c, portMAX_DELAY))
        return (int) c;
    return -1;
}

int UART5_PutChar(char c)
{
    if (xQueueSendToBack(xQueueUARTTX[UART_CHANNEL_5], (void *) &c, (portTickType) 100) == pdPASS)
    {
        CRITICAL_SECTION_ENTER();
        GPIO_WriteBit(RS485_2_DIR_PORT, RS485_2_DIR_PIN, Bit_SET); // activate send mode
        USART_ITConfig(UART5, USART_IT_RXNE, DISABLE);
        UART5->CR1 &= (~USART_Mode_Rx);  // disable receive
        USART_ITConfig(UART5, USART_IT_TXE, ENABLE);
        CRITICAL_SECTION_LEAVE();
    }
    return c;
}

int UART5_GetChar(void)
{
    char c;
    if (pdTRUE == xQueueReceive(xQueueUARTRX[UART_CHANNEL_5], (void *) &c, portMAX_DELAY))
        return (int) c;
    return -1;
}

bool UART1_CharAvail(void)
{
    if (0 < uxQueueMessagesWaiting(xQueueUARTRX[UART_CHANNEL_1]))
        return TRUE;
    return FALSE;
}

bool UART2_CharAvail(void)
{
    if (0 < uxQueueMessagesWaiting(xQueueUARTRX[UART_CHANNEL_2]))
        return TRUE;
    return FALSE;
}

bool UART3_CharAvail(void)
{
    if (0 < uxQueueMessagesWaiting(xQueueUARTRX[UART_CHANNEL_3]))
        return TRUE;
    return FALSE;
}

bool UART4_CharAvail(void)
{
    if (0 < uxQueueMessagesWaiting(xQueueUARTRX[UART_CHANNEL_4]))
        return TRUE;
    return FALSE;
}

bool UART5_CharAvail(void)
{
    if (0 < uxQueueMessagesWaiting(xQueueUARTRX[UART_CHANNEL_5]))
        return TRUE;
    return FALSE;
}

void UART_ChannelInit(uint8_t channel, UART_ChannelInitTypeDef *InitStruct)
{
    USART_InitTypeDef USART_InitStructure;
    USART_StructInit(&USART_InitStructure);

    USART_InitStructure.USART_BaudRate = InitStruct->baudrate;

    if (channel >= UART_CONFIG_CHANNEL_COUNT)
        return;

    GPIO_PinInit(UartChannelHardware[channel].TX_Port, UartChannelHardware[channel].TX_Pin);
    GPIO_PinInit(UartChannelHardware[channel].RX_Port, UartChannelHardware[channel].RX_Pin);

    UartChannelConfig[channel].UartRXHandler = InitStruct->rx_callback_func;
    UartChannelConfig[channel].UartRXQueue = InitStruct->rx_queue;
    UartChannelConfig[channel].ClearParity7Bit = FALSE;

    /* FLUSH Queues -------------------------------------------------------------*/
    char c;

    while(xQueueReceive(xQueueUARTTX[channel], (void*) &c, 1) == pdTRUE);       // Flush TX Queue
    while(xQueueReceive(xQueueUARTRX[channel], (void*) &c, 1) == pdTRUE);       // Flush RX Queue

    /* USART configuration ------------------------------------------------------*/

    switch (InitStruct->ctrl_flags)
    {
    case UART_CTRL_8N1:
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
        USART_InitStructure.USART_Parity = USART_Parity_No;
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
        break;
    case UART_CTRL_8E1:
        // attention: it is confusing but parity bit is one bit of wordlength. thus this equals in 8E1
        USART_InitStructure.USART_WordLength = USART_WordLength_9b;
        USART_InitStructure.USART_Parity = USART_Parity_Even;
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
        break;
    case UART_CTRL_7E1:
        // attention: it is confusing but parity bit is one bit of wordlength. thus this equals in 7E1
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
        USART_InitStructure.USART_Parity = USART_Parity_Even;
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
        // Parity Bit (Bit7) has to be cleared manually on reception.
        UartChannelConfig[channel].ClearParity7Bit = TRUE;
        break;
    }

    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    switch (channel)
    {
    case UART_CHANNEL_1:
        USART_Init(USART1, &USART_InitStructure);
        break;
    case UART_CHANNEL_2:
        USART_Init(USART2, &USART_InitStructure);
        break;
    case UART_CHANNEL_3:
        USART_Init(USART3, &USART_InitStructure);
        break;
    case UART_CHANNEL_4:
        USART_Init(UART4, &USART_InitStructure);
        break;
    case UART_CHANNEL_5:
        USART_Init(UART5, &USART_InitStructure);
        break;
    default:
        break;
    }
}

void UART_ChannelUnInit(uint8_t channel)
{
    if (channel >= UART_CONFIG_CHANNEL_COUNT)
        return;

    char c;
    while(xQueueReceive(xQueueUARTTX[channel], (void*) &c, 1) == pdTRUE);       // Flush TX Queue
    while(xQueueReceive(xQueueUARTRX[channel], (void*) &c, 1) == pdTRUE);       // Flush RX Queue

    GPIO_PinUnInit(UartChannelHardware[channel].TX_Port, UartChannelHardware[channel].TX_Pin);
    GPIO_PinUnInit(UartChannelHardware[channel].RX_Port, UartChannelHardware[channel].RX_Pin);
    UartChannelConfig[channel].UartRXHandler = NULL;
    UartChannelConfig[channel].UartRXQueue = NULL;
}

void UART_Init(void)
{
    /* Create IO-Queues */
    xQueueUARTTX[UART_CHANNEL_1] = xQueueCreate(64, sizeof(char));
    xQueueUARTRX[UART_CHANNEL_1] = xQueueCreate(64, sizeof(char));
    xQueueUARTTX[UART_CHANNEL_2] = xQueueCreate(128, sizeof(char));  // Mesh Buffer must be this size
    xQueueUARTRX[UART_CHANNEL_2] = xQueueCreate(256, sizeof(char));  // Mesh Buffer must be this size
    xQueueUARTTX[UART_CHANNEL_3] = xQueueCreate(64, sizeof(char));
    xQueueUARTRX[UART_CHANNEL_3] = xQueueCreate(64, sizeof(char));
    xQueueUARTTX[UART_CHANNEL_4] = xQueueCreate(64, sizeof(char));
    xQueueUARTRX[UART_CHANNEL_4] = xQueueCreate(64, sizeof(char));
    xQueueUARTTX[UART_CHANNEL_5] = xQueueCreate(64, sizeof(char));
    xQueueUARTRX[UART_CHANNEL_5] = xQueueCreate(64, sizeof(char));

    vQueueAddToRegistry(xQueueUARTTX[UART_CHANNEL_1], (signed char*)"TX QueueUART_CHANNEL_1");
    vQueueAddToRegistry(xQueueUARTRX[UART_CHANNEL_1], (signed char*)"RX QueueUART_CHANNEL_1");
    vQueueAddToRegistry(xQueueUARTTX[UART_CHANNEL_2], (signed char*)"TX QueueUART_CHANNEL_2");
    vQueueAddToRegistry(xQueueUARTRX[UART_CHANNEL_2], (signed char*)"RX QueueUART_CHANNEL_2");
    vQueueAddToRegistry(xQueueUARTTX[UART_CHANNEL_3], (signed char*)"TX QueueUART_CHANNEL_3");
    vQueueAddToRegistry(xQueueUARTRX[UART_CHANNEL_3], (signed char*)"RX QueueUART_CHANNEL_3");
    vQueueAddToRegistry(xQueueUARTTX[UART_CHANNEL_4], (signed char*)"TX QueueUART_CHANNEL_4");
    vQueueAddToRegistry(xQueueUARTRX[UART_CHANNEL_4], (signed char*)"RX QueueUART_CHANNEL_4");
    vQueueAddToRegistry(xQueueUARTTX[UART_CHANNEL_5], (signed char*)"TX QueueUART_CHANNEL_5");
    vQueueAddToRegistry(xQueueUARTRX[UART_CHANNEL_5], (signed char*)"RX QueueUART_CHANNEL_5");

    assert_param(xQueueUARTTX[UART_CHANNEL_1] != NULL);
    assert_param(xQueueUARTRX[UART_CHANNEL_1] != NULL);
    assert_param(xQueueUARTTX[UART_CHANNEL_2] != NULL);
    assert_param(xQueueUARTRX[UART_CHANNEL_2] != NULL);
    assert_param(xQueueUARTTX[UART_CHANNEL_3] != NULL);
    assert_param(xQueueUARTRX[UART_CHANNEL_3] != NULL);
    assert_param(xQueueUARTTX[UART_CHANNEL_4] != NULL);
    assert_param(xQueueUARTRX[UART_CHANNEL_4] != NULL);
    assert_param(xQueueUARTTX[UART_CHANNEL_5] != NULL);
    assert_param(xQueueUARTRX[UART_CHANNEL_5] != NULL);

    /* UART Configuration */
    UART_ChannelInitTypeDef UartChannelInitStruct;

    UartChannelInitStruct.baudrate = UART1_DEFAULT_BAUDRATE;
    UartChannelInitStruct.ctrl_flags = UART1_DEFAULT_CTRL;
    UartChannelInitStruct.rx_callback_func = NULL;
    UART_ChannelInit(UART_CHANNEL_1, &UartChannelInitStruct);

    UartChannelInitStruct.baudrate = UART2_DEFAULT_BAUDRATE;
    UartChannelInitStruct.ctrl_flags = UART2_DEFAULT_CTRL;
    UartChannelInitStruct.rx_callback_func = NULL;
    UART_ChannelInit(UART_CHANNEL_2, &UartChannelInitStruct);

    UartChannelInitStruct.baudrate = UART3_DEFAULT_BAUDRATE;
    UartChannelInitStruct.ctrl_flags = UART3_DEFAULT_CTRL;
    UartChannelInitStruct.rx_callback_func = NULL;
    UART_ChannelInit(UART_CHANNEL_3, &UartChannelInitStruct);

    UartChannelInitStruct.baudrate = UART4_DEFAULT_BAUDRATE;
    UartChannelInitStruct.ctrl_flags = UART4_DEFAULT_CTRL;
    UartChannelInitStruct.rx_callback_func = NULL;
    UART_ChannelInit(UART_CHANNEL_4, &UartChannelInitStruct);

    UartChannelInitStruct.baudrate = UART5_DEFAULT_BAUDRATE;
    UartChannelInitStruct.ctrl_flags = UART5_DEFAULT_CTRL;
    UartChannelInitStruct.rx_callback_func = NULL;
    UART_ChannelInit(UART_CHANNEL_5, &UartChannelInitStruct);

    /* Enable USART Receive interrupts */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
    USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);

    /* Enable the USARTs */
    USART_Cmd(USART1, ENABLE);
    USART_Cmd(USART2, ENABLE);
    USART_Cmd(USART3, ENABLE);
    USART_Cmd(UART4, ENABLE);
    USART_Cmd(UART5, ENABLE);
}

void UART_Tick(void)
{
    char c;

    for (int channel = 0; channel < UART_CONFIG_CHANNEL_COUNT; channel++)
    {
        if (UartChannelConfig[channel].UartRXHandler != NULL && 0 < uxQueueMessagesWaiting(xQueueUARTRX[channel]))
        {
            while(pdTRUE == xQueueReceive(xQueueUARTRX[channel], (void *) &c, 0))
                (UartChannelConfig[channel].UartRXHandler)(c);
        }
    }
}
