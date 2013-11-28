#ifndef UART_H
#define UART_H

//initial baudrate values
#define UART5_DEFAULT_BAUDRATE  38400
#define UART5_DEFAULT_CTRL      UART_CTRL_8N1

#define RS485_2_DIR_PORT        GPIOF
#define RS485_2_DIR_PIN         GPIO_Pin_5

#define CHARACTER_TIMEOUT       0x8FFFF

void UART5_PutChar(char c);
void UART5_SendString(const char *c);
bool UART5_GetChar(char *c);
bool UART5_Receive(uint8_t *ui);

void UART_Init(void);

#endif
