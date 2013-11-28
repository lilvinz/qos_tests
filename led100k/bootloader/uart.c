
#include "stdheader.h"
#include "uart.h"


void UART5_PutChar(char c)
{
	UART5->CR1 &= (~USART_Mode_Rx);  // disable receive
	GPIO_WriteBit(RS485_2_DIR_PORT, RS485_2_DIR_PIN, Bit_SET);  // activate send mode
    /* Write one byte to the transmit data register */
    USART_SendData(UART5, c);
    /* Loop until USART5 DR register is empty */
    while (USART_GetFlagStatus(UART5, USART_FLAG_TC) == RESET)
    {
        asm("nop");
    }
    GPIO_WriteBit(RS485_2_DIR_PORT, RS485_2_DIR_PIN, Bit_RESET);  // activate receive mode
    UART5->CR1 |= USART_Mode_Rx;  // enable receive
}

void UART5_SendString(const char *c)
{
    while (*c != '\0')
    {
        UART5_PutChar(*c);
        c++;
    }
}

bool UART5_GetChar(char *c)
{
    /* Read one byte from the receive data register */
    if (    (USART_GetFlagStatus(UART5, USART_FLAG_RXNE) != RESET)
        ||  (USART_GetFlagStatus(UART5, USART_FLAG_ORE)  != RESET)
        ||  (USART_GetFlagStatus(UART5, USART_FLAG_FE)   != RESET)
        ||  (USART_GetFlagStatus(UART5, USART_FLAG_NE)   != RESET))
    {
        *c = USART_ReceiveData(UART5);
        return TRUE;
    }
    else
        return FALSE;
}

bool UART5_Receive(uint8_t *ui)
{
    for (uint32_t iWait = 0; iWait < CHARACTER_TIMEOUT; ++iWait)
    {
        if (UART5_GetChar((char*)ui))
            return true;
    }
    return false;
}

void UART_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);

    /* Enable GPIO clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_GPIOC, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_GPIOD, DISABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_GPIOF, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_UART5, DISABLE);

    // Connect PXx to USARTx_Tx
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_UART5);
    // Connect PXx to USARTx_Rx
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource2, GPIO_AF_UART5);

    // 485RE2 (RS485-2) (PC)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    // Configure UART5 TX as alternate function push-pull (RS485-2) (PC)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // Configure UART5 RX as input floating UART5 RX (RS485-2) (PC)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStructure);


    /* USART configuration ------------------------------------------------------*/
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = UART5_DEFAULT_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(UART5, &USART_InitStructure);

    /* Enable the USARTs */
    USART_Cmd(UART5, ENABLE);
}
