
#include "stdheader.h"

#include "timeout.h"
#include "clock.h"

#include "uart.h"
#include "mesh.h"
#include "meshcom_conn.h"
#include "debug.h"

#include "mesh_testmode.h"

#define PUTBYTE(x) UART2_PutChar(x)
#define MESHCOM_UART_CHANNEL UART_CHANNEL_2


#define FACTORY_TEST_ENTER_TIMEOUT_S 6

#define FACTORYTEST_MAGIC_REQUEST 0xfa12b8f8
#define FACTORYTEST_MAGIC_RESPONSE 0xb52ab27b


static xSemaphoreHandle semTmCommandOk;


// *************** local function declarations ****************

static void meshradio_tm_magic_in(char c);
static bool meshradio_testmode_enter(MESH_TESTMODE_T mode);
static void meshradio_tm_data_in(char c);
static bool meshradio_send_command(uint8_t cmd, uint16_t val);


// *************** global variable declarations ****************

static uint32_t received_dword;


// *************** local function implementations ****************

static void meshradio_tm_magic_in(char c)
{
    received_dword <<= 8;
    received_dword &= 0xFFFFFF00;
    received_dword |= (uint8_t)c;
}

static bool meshradio_testmode_enter(MESH_TESTMODE_T mode)
{
    static uint32_t u32temp;

    received_dword = 0;

    for (uint8_t i = 0; i <= FACTORY_TEST_ENTER_TIMEOUT_S * 4; i++)
    {
        u32temp = FACTORYTEST_MAGIC_REQUEST;
        PUTBYTE((u32temp >> 24) & 0xFF);
        PUTBYTE((u32temp >> 16) & 0xFF);
        PUTBYTE((u32temp >> 8) & 0xFF);
        PUTBYTE((u32temp) & 0xFF);

        vTaskDelay(250);

        if (received_dword == FACTORYTEST_MAGIC_RESPONSE)
            return TRUE;
    }

    return FALSE;
}

static void meshradio_tm_data_in(char c)
{
    static char rx_ok[3];

    rx_ok[0] = rx_ok[1];
    rx_ok[1] = rx_ok[2];
    rx_ok[2] = c;

    if (rx_ok[0] == 'O' && rx_ok[1] == 'K' && rx_ok[2] == 0x0D)
        xSemaphoreGive(semTmCommandOk);
}

static bool meshradio_send_command(uint8_t cmd, uint16_t val)
{
    char aValue[5];
    sprintf(aValue, "%04x", val);

    PUTBYTE('F');
    PUTBYTE('T');
    PUTBYTE(cmd);
    PUTBYTE(aValue[0]);
    PUTBYTE(aValue[1]);
    PUTBYTE(aValue[2]);
    PUTBYTE(aValue[3]);
    PUTBYTE(0x0D);

    if (xSemaphoreTake(semTmCommandOk, 5000) != pdTRUE)
        return FALSE;

    return TRUE;
}


// *************** global function implementations ****************

bool MESH_TestmodeStart(MESH_TESTMODE_T mode)
{
    UART_ChannelInitTypeDef UART_ChannelInitStruct;

    if (semTmCommandOk == NULL)
        vSemaphoreCreateBinary(semTmCommandOk);

    xSemaphoreTake(semTmCommandOk, 0);

    UART_ChannelInitStruct.baudrate = 115200;
    UART_ChannelInitStruct.ctrl_flags = UART_CTRL_8N1;
    UART_ChannelInitStruct.rx_callback_func = meshradio_tm_magic_in;

    UART_ChannelInit(MESHCOM_UART_CHANNEL, &UART_ChannelInitStruct);

    // perform reset
    MESHCOM_UnInit();
    MESH_ResetModule();

    if (meshradio_testmode_enter(mode) == FALSE)
        return FALSE;

    UART_ChannelInitStruct.rx_callback_func = meshradio_tm_data_in;
    UART_ChannelInit(MESHCOM_UART_CHANNEL, &UART_ChannelInitStruct);

    return TRUE;
}

void MESH_TestmodeStop(void)
{
    MESH_ResetModule();
    MESHCOM_Init();
}

bool MESH_TestmodeSetAntenna(uint8_t ant)
{
    return meshradio_send_command('A', ant);
}

bool MESH_TestmodeSetTxPower(uint8_t pwr)
{
    return meshradio_send_command('P', pwr);
}

bool MESH_TestmodeSetTrim(uint8_t trim)
{
    return meshradio_send_command('T', trim);
}

bool MESH_TestmodeSetChannel(uint8_t chan)
{
    return meshradio_send_command('C', chan);
}

bool MESH_TestmodeSetFrameBufferPattern(uint8_t pat)
{
    return meshradio_send_command('B', pat);
}

bool MESH_TestmodeSaveSettings(void)
{
    return meshradio_send_command('S', 0);
}

