
#include "stdheader.h"
#include "mesh.h"

#include "version.h"
#include "clock.h"
#include "state.h"
#include "log.h"
#include "network.h"
#include "mesh_fw_manager.h"
#include "meshcom.h"
#include "meshcom_conn.h"


STATE_T mesh_state;

static bool b_tried_mesh_fw_update_this_powercycle = FALSE;
static uint8_t mesh_StateRequest = MESH_STATE_UNDEFINED;

void MESH_ResetModule(void)
{
    mesh_StateRequest = MESH_STATE_RESET;
}

void MESH_PowerToggleModule(void)
{
    mesh_StateRequest = MESH_STATE_POWERTOGGLE;
}

void MESH_EnterTestMode(void)
{
    mesh_StateRequest = MESH_STATE_TESTMODE;
}

void MESH_Uninit(void)
{
    mesh_StateRequest = MESH_STATE_POWER_OFF;
}

void MESH_Init(void)
{
    if (MESH_SETTING_USE)
    {
        b_tried_mesh_fw_update_this_powercycle = FALSE;  // reset flag to allow mesh fw update
        mesh_StateRequest = MESH_STATE_POWERTOGGLE;
    }
    else
    {
        mesh_state.current = MESH_STATE_POWER_OFF;
        MESH_RESET_PULL;
        MESH_POWER_OFF;
    }
}


void MESH_Tick(void)
{
    if (MESH_SETTING_USE == 0)
        return;

    if (mesh_StateRequest != MESH_STATE_UNDEFINED)
    {
        mesh_state.current = mesh_StateRequest;
        mesh_StateRequest = MESH_STATE_UNDEFINED;
    }

    switch (mesh_state.current)
    {
    case MESH_STATE_HALT:
        break;

    case MESH_STATE_POWER_OFF:
    {
        if (state_is_new(&mesh_state))
        {
            MESH_RESET_PULL;
            MESH_POWER_OFF;
        }
        break;
    }

    case MESH_STATE_POWERTOGGLE:
    {
        if (state_is_new(&mesh_state))
        {
            MESH_RESET_PULL;
            MESH_POWER_OFF;
        }
        if (state_timeout(&mesh_state, MESH_POWER_OFF_HOLD_DURATION))
        {
            MESH_POWER_ON;
            mesh_state.current = MESH_STATE_RESET; // next state is reset for release delay.
        }
        break;
    }

    case MESH_STATE_RESET:
    {
        if (state_is_new(&mesh_state))
        {
            MESH_RTS_SET;
            MESH_RESET_PULL;
        }
        if (state_timeout(&mesh_state, MESH_RESET_HOLD_DURATION))
        {
            MESH_RESET_RELEASE;
            mesh_state.current = MESH_STATE_INIT;
        }
        break;
    }

    case MESH_STATE_TESTMODE:
        // ToDo Implement testmode
        break;

    case MESH_STATE_INIT:

        if (state_is_new(&mesh_state))
        {
        }
        mesh_state.current = MESH_STATE_IDLE;

        break;

    case MESH_STATE_IDLE:

        if (state_is_new(&mesh_state))
        {
        }

        break;

    case MESH_STATE_FAIL:

        if (state_is_new(&mesh_state))
            MESHCOM_UnInit();

        if (state_timeout(&mesh_state, MESH_STATE_FAIL_TIMEOUT))
            MESH_Init();

        break;

    case MESH_STATE_BROKEN:

        if (state_is_new(&mesh_state))
        {
            // MeshRadio Module seems to be broken.
            LOG(LOG_ID_MESHCOM_FW_STATUS_SWITCH_OFF, 0, 0, 0, NULL);
            MESH_RESET_PULL;
            MESH_POWER_OFF;
        }

        if (state_timeout(&mesh_state, MESH_STATE_BROKEN_TIMEOUT))
            mesh_state.current = MESH_STATE_POWERTOGGLE;

        break;

    default:
        break;
    }
}

