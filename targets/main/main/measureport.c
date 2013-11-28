/*
 * measureport.c
 *
 *  Created on: 28.11.2011
 *      Author: cba
 */

#include "stdheader.h"
#include "printf-stdarg.h"
#include "ascii.h"
#include "uart.h"
#include "settings.h"

#include "sensor.h"
#include "sensors.h"
#include "lm75.h"
#include "primestruct.h"
#include "heater.h"

#include "measureport.h"


void MEASUREPORT_PutS(char * s)
{
    while (*s != 0)
    {
        MEASUREPORT_PUTCHAR(*s);
        s++;
    }
}


void MEASUREPORT_Init(void)
{
    UART_ChannelInitTypeDef UartInitStruct;

    UartInitStruct.baudrate = 115200;
    UartInitStruct.ctrl_flags = UART_CTRL_8N1;
    UartInitStruct.rx_callback_func = NULL;

    UART_ChannelInit(UART_CHANNEL_4, &UartInitStruct);

    if (MEASUREPORT_USAGE == SETTINGS_RS485_PORT2_FUNCTION_MEASUREMENTS)
    {
        MEASUREPORT_PutS("T_main;T_led1;T_led2;T_led3;T_led4;E;U_24;");
        for (uint8_t i = 0; i < PS_ANALOG_LED_COUNT; i++)
        {
            char buffer[10];
            sprintf(buffer, "U_led%u;", i);
            MEASUREPORT_PutS(buffer);
        }
        MEASUREPORT_PutS("B_heater;");
        MEASUREPORT_PUTCHAR(ASCII_CR);
        MEASUREPORT_PUTCHAR(ASCII_LF);
    }
}


void MEASUREPORT_Tick(void)
{
    char buffer[20];
    static CLOCK_S_T   timestamp;

    uint32_t u32_temp;

    if (MEASUREPORT_USAGE != SETTINGS_RS485_PORT2_FUNCTION_MEASUREMENTS)
        return;

    if (timestamp == CLOCK_GetTimestamp())
        return;

    timestamp = CLOCK_GetTimestamp();

    if (timestamp % MEASUREPORT_INTERVAL == 0)
    {
        for (uint8_t idx = 0; idx < TEMPERATURE_COUNT; idx++)
        {
            TEMPERATURE_SENSOR_T temperature;
            SENSORS_GetTemperature(idx, &temperature);
            if (temperature.state == SENSOR_OK)
            {
                sprintf(buffer, "%d.%d", (int)temperature.value, (int)(temperature.value * 1000.0f) % 1000);
                MEASUREPORT_PutS(buffer);
            }
            MEASUREPORT_PUTCHAR(';');
        }

        u32_temp = (uint32_t)(1000.0f * _primestruct.sensors.analog.supply[PS_ANALOG_VOLTAGE_MAIN_24V].value);
        sprintf(buffer, "%3u.%03u;", u32_temp / 1000, u32_temp % 1000);
        MEASUREPORT_PutS(buffer);

        for (uint8_t ch = 0; ch < PS_ANALOG_LED_COUNT ; ch++)
        {
            u32_temp = (uint32_t)(1000.0f * _primestruct.sensors.analog.led[ch].value);

            sprintf(buffer, "%2u.%03u;", (uint16_t)(u32_temp / 1000), (uint16_t)(u32_temp % 1000));
            MEASUREPORT_PutS(buffer);
        }

        if (HEATER_GetState() == HEATER_ON)
            MEASUREPORT_PutS("1;");
        else
            MEASUREPORT_PutS("0;");

        MEASUREPORT_PUTCHAR(ASCII_CR);
        MEASUREPORT_PUTCHAR(ASCII_LF);
    }
}


void MEASUREPORT_SendSync(void)
{
    if (MEASUREPORT_USAGE != SETTINGS_RS485_PORT2_FUNCTION_SYNC)
        return;

    MEASUREPORT_PUTCHAR('S');
    MEASUREPORT_PUTCHAR(ASCII_CR);
    MEASUREPORT_PUTCHAR(ASCII_LF);
}
