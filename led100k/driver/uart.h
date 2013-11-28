#ifndef UART_H
#define UART_H


enum
{
    UART_CTRL_8N1,
    UART_CTRL_8E1,
    UART_CTRL_7E1
};

enum
{
    UART_CHANNEL_3 = 0,
    UART_CONFIG_CHANNEL_COUNT
};

//initial baudrate values
#define UART3_DEFAULT_BAUDRATE  115200
#define UART3_DEFAULT_CTRL      UART_CTRL_8N1

typedef void (UartRxCallBackHandler_t)(char c);

typedef struct
{
    UartRxCallBackHandler_t *UartRXHandler;
    xQueueHandle            UartRXQueue;
    bool                    ClearParity7Bit;
} UART_ChannelConfig_t;

typedef struct
{
    int                baudrate;
    int                 ctrl_flags;
    UartRxCallBackHandler_t *rx_callback_func;
    xQueueHandle            rx_queue;
} UART_ChannelInitTypeDef;

typedef struct
{
    GPIO_TypeDef *    TX_Port;
    uint16_t    TX_Pin;
    GPIO_TypeDef *    RX_Port;
    uint16_t    RX_Pin;
} UART_ChannelHardwareTypeDef;



int UART3_PutChar(char c);
int UART3_GetChar(void);
bool UART3_CharAvail(void);

void UART_ChannelInit(uint8_t channel, UART_ChannelInitTypeDef *InitStruct);
void UART_ChannelUnInit(uint8_t channel);

void UART_Init(void);
void UART_Tick(void);

#endif



