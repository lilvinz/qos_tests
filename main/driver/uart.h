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
    UART_CHANNEL_1 = 0,
    UART_CHANNEL_2,
    UART_CHANNEL_3,
    UART_CHANNEL_4,
    UART_CHANNEL_5,
    UART_CONFIG_CHANNEL_COUNT
};

//initial baudrate values
#define UART1_DEFAULT_BAUDRATE  4800
#define UART1_DEFAULT_CTRL      UART_CTRL_8N1
#define UART2_DEFAULT_BAUDRATE  115200
#define UART2_DEFAULT_CTRL      UART_CTRL_8N1
#define UART3_DEFAULT_BAUDRATE  115200
#define UART3_DEFAULT_CTRL      UART_CTRL_8N1
#define UART4_DEFAULT_BAUDRATE  38400
#define UART4_DEFAULT_CTRL      UART_CTRL_8N1
#define UART5_DEFAULT_BAUDRATE  38400
#define UART5_DEFAULT_CTRL      UART_CTRL_8N1

#define uart_getchar(x) UART5_GetChar(x)
#define uart_putchar(x) UART5_PutChar(x)
#define uart_cavail(x)  UART5_CharAvail(x)

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


int UART1_PutChar(char c);
int UART2_PutChar(char c);
int UART3_PutChar(char c);
int UART4_PutChar(char c);
int UART5_PutChar(char c);

int UART1_GetChar(void);
int UART2_GetChar(void);
int UART3_GetChar(void);
int UART4_GetChar(void);
int UART5_GetChar(void);

bool UART1_CharAvail(void);
bool UART2_CharAvail(void);
bool UART3_CharAvail(void);
bool UART4_CharAvail(void);
bool UART5_CharAvail(void);

void UART_ChannelInit(uint8_t channel, UART_ChannelInitTypeDef *InitStruct);
void UART_ChannelUnInit(uint8_t channel);

void UART_Init(void);
void UART_Tick(void);

#endif



