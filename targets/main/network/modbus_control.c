/*
 * modbus_control.c
 *
 *  Created on: 04.08.2010
 *      Author: Carsten Bartels
 */

#include "stdheader.h"
#include "modbus_control.h"

#include "settings.h"
#include "bmm.h"
#include "gloom_manager.h"
#include "indication.h"


static uint16_t current_digin = 0;
static uint16_t current_digout = 0;
static bool digin_init;


void MODBUS_CONTROL_DigitalInput(uint16_t digin)
{
    static uint16_t digin_last = 0xFFFF;
    uint16_t digin_changed_bits;

    // if module has been initialized, all input bits have new states
    if (digin_init == TRUE)
    {
        digin_changed_bits = 0xFFFF;
        digin_init = FALSE;
    }
    else
    {
        digin_changed_bits = digin ^ digin_last;
    }

    digin_last = digin;
    current_digin = digin;

    if ((_settings_list_ram[SETTINGS_TERMINAL_CONTROL_MODBUS_ON_OFF] > 0)
        && (digin_changed_bits & MODBUS_CONTROL_DI_BIT_ONOFF))
    {
        if (digin & MODBUS_CONTROL_DI_BIT_ONOFF)
            BMM_SetLedStageOff(TRUE);
        else
            BMM_SetLedStageOff(FALSE);
    }

    if ((_settings_list_ram[SETTINGS_TERMINAL_CONTROL_MODBUS_DAY_NIGHT] > 0)
        && (digin_changed_bits & (MODBUS_CONTROL_DI_BIT_DAYNIGHT | MODBUS_CONTROL_DI_BIT_TWILIGHT)))
    {
        if (digin & MODBUS_CONTROL_DI_BIT_DAYNIGHT)
            GLOOM_MANAGER_SetDayNightManual(GLOOM_MANAGER_LED_STATE_DAY);
        else if (digin & MODBUS_CONTROL_DI_BIT_TWILIGHT)
            GLOOM_MANAGER_SetDayNightManual(GLOOM_MANAGER_LED_STATE_TWILIGHT);
        else
            GLOOM_MANAGER_SetDayNightManual(GLOOM_MANAGER_LED_STATE_NIGHT);
    }

    if ((_settings_list_ram[SETTINGS_TERMINAL_CONTROL_MODBUS_REDUCTION] > 0)
        && (digin_changed_bits & (MODBUS_CONTROL_DI_BIT_VISIBILITY_REDUCTION_1 | MODBUS_CONTROL_DI_BIT_VISIBILITY_REDUCTION_2)))
    {
        if (digin & MODBUS_CONTROL_DI_BIT_VISIBILITY_REDUCTION_2)
            BMM_SetVisibilityRange(BMM_VISI_DISTANCE_2);
        else if (digin & MODBUS_CONTROL_DI_BIT_VISIBILITY_REDUCTION_1)
            BMM_SetVisibilityRange(BMM_VISI_DISTANCE_1);
        else
            BMM_SetVisibilityRange(BMM_VISI_DISTANCE_0);
    }
}

void MODBUS_CONTROL_DigitalInputFailed(void)
{
    if (_settings_list_ram[SETTINGS_TERMINAL_CONTROL_MODBUS_ON_OFF] > 0)
        BMM_SetLedStageOff(FALSE);

    if (_settings_list_ram[SETTINGS_TERMINAL_CONTROL_MODBUS_DAY_NIGHT] > 0)
        GLOOM_MANAGER_SetDayNightManual(GLOOM_MANAGER_LED_STATE_NOT_SET);

    if (_settings_list_ram[SETTINGS_TERMINAL_CONTROL_MODBUS_REDUCTION] > 0)
        BMM_SetVisibilityRange(BMM_VISI_DISTANCE_0);
}

uint16_t MODBUS_CONTROL_DigitalOutput(void)
{
    uint16_t digout;

    digout = 0;

    switch(GLOOM_MANAGER_GetLocalLedState())
    {
    case GLOOM_MANAGER_LED_STATE_DAY:
        digout |= MODBUS_CONTROL_DO_BIT_DAYNIGHT;
        break;
    case GLOOM_MANAGER_LED_STATE_NIGHT:
        break;
    case GLOOM_MANAGER_LED_STATE_TWILIGHT:
        digout |= MODBUS_CONTROL_DO_BIT_TWILIGHT;
        break;
    default:
        break;
    }
    if (BMM_GetLedStageOff() == TRUE)
        digout |= MODBUS_CONTROL_DO_BIT_ONOFF;

    switch(BMM_GetVisibilityRange())
    {
    case BMM_VISI_DISTANCE_0:
        break;
    case BMM_VISI_DISTANCE_1:
        digout |= MODBUS_CONTROL_DO_BIT_VISIBILITY_REDUCTION_1;
        break;
    case BMM_VISI_DISTANCE_2:
        digout |= MODBUS_CONTROL_DO_BIT_VISIBILITY_REDUCTION_2;
        break;
    default:
        break;
    }

    if (_indication_state[IND_A].state == TRUE)
        digout |= MODBUS_CONTROL_DO_BIT_WARNING;

    if (_indication_state[IND_B].state == TRUE)
        digout |= MODBUS_CONTROL_DO_BIT_ERROR;

    current_digout = digout;

    return digout;
}


uint16_t MODBUS_CONTROL_GetDigitalInputValue(void)
{
    return current_digin;
}


uint16_t MODBUS_CONTROL_GetDigitalOutputValue(void)
{
    return current_digout;
}

void MODBUS_CONTROL_Init(void)
{
    digin_init = TRUE;

    // reset all controlled targets to normal state
    BMM_SetLedStageOff(FALSE);
    GLOOM_MANAGER_SetDayNightManual(GLOOM_MANAGER_LED_STATE_NOT_SET);
    BMM_SetVisibilityRange(BMM_VISI_DISTANCE_0);
}
