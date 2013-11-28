#ifndef MESH_H
#define MESH_H

#include "uart.h"
#include "settings.h"
#include "hw_configuration.h"

#define MESH_SETTING_USE                             _settings_list_ram[SETTINGS_MESH_USE]

#define MESH_DONT_INTERRUPT_DEBUG 0

#define MESH_UART_RTS_PIN     UART2_RTS_PIN
#define MESH_UART_RTS_PORT    UART2_PORT


#define MESH_RESET_RELEASE    GPIO_WriteBit(MESH_RESET_PORT, MESH_RESET_PIN, Bit_SET);
#define MESH_POWER_ON         GPIO_WriteBit(MESH_VCC_EN_PORT, MESH_VCC_EN_PIN, Bit_SET);

#if (MESH_DONT_INTERRUPT_DEBUG == 1)
INFO(WARNING: MESH_DONT_INTERRUPT_DEBUG enabled!)

#define MESH_RESET_PULL
#define MESH_POWER_OFF
#else
#define MESH_RESET_PULL       GPIO_WriteBit(MESH_RESET_PORT, MESH_RESET_PIN, Bit_RESET);
#define MESH_POWER_OFF        GPIO_WriteBit(MESH_VCC_EN_PORT, MESH_VCC_EN_PIN, Bit_RESET);
#endif

#define MESH_RTS_SET          GPIO_WriteBit(MESH_UART_RTS_PORT, MESH_UART_RTS_PIN, Bit_SET);
#define MESH_RTS_CLEAR        GPIO_WriteBit(MESH_UART_RTS_PORT, MESH_UART_RTS_PIN, Bit_RESET);


#define MESH_RESET_HOLD_DURATION          1000
#define MESH_POWER_OFF_HOLD_DURATION      5000

#define MESH_STATE_FAIL_TIMEOUT         2000
#define MESH_STATE_BROKEN_TIMEOUT           (24*(1000UL*60*60))

#define MESH_UART_DEFAULT_BAUDRATE      115200
#define MESH_UART_FORMAT                    UART_CTRL_8N1

enum
{
    MESH_STATE_HALT,
    MESH_STATE_POWER_OFF,
    MESH_STATE_RESET,
    MESH_STATE_POWERTOGGLE,
    MESH_STATE_TESTMODE,
    MESH_STATE_INIT,
    MESH_STATE_IDLE,
    MESH_STATE_FAIL,
    MESH_STATE_FIRMWAREUPGRADE,
    MESH_STATE_BROKEN,
    MESH_STATE_UNDEFINED
};

typedef enum
{
    MESH_FW_OK,
    MESH_FW_VIRGIN,
    MESH_FW_UPDATE_REQUEST,
    MESH_FW_UPDATED,
    MESH_FW_UPDATE_FAILED,
    MESH_FW_ERROR,
} MESH_FW_T;

void MESH_BroadcastShutUp(uint16_t uTimeout);
bool MESH_NetworkingIsAvailable(void);
void MESH_ResetModule(void);
void MESH_PowerToggleModule(void);
void MESH_EnterTestMode(void);



void MESH_Init(void);
void MESH_Uninit(void);
void MESH_Tick(void);

#endif
