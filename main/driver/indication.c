
#include "stdheader.h"
#include "indication.h"
#include "detector.h"

#include "rtc.h"
#include "hw_configuration.h"
#include "clock.h"
#include "log.h"
#include "watchdog.h"

static IND_STATE_T  indication_override_state[IND_RELAIS_COUNT];
static uint8_t u_indication_override_count;

IND_STATE_T _indication_state[IND_RELAIS_COUNT];


/**
  * @brief  Set relais.
  * @param  ind: selected relais
  * @param  state: state relais should be set to. Negative logic: FALSE = Relais on, TRUE = Relais off
  * @retval None
  */
void INDICATION_Relais(IND_RELAIS_T ind, bool state)
{
    // set relais state. negative logic. FALSE = Relais on, TRUE = Relais off
    switch (ind)
    {
    case IND_A:
        if (state == FALSE)
            GPIO_WriteBit(INDICATION_RELAIS_A_PORT, INDICATION_RELAIS_A_BIT , Bit_SET);
        else
            GPIO_WriteBit(INDICATION_RELAIS_A_PORT, INDICATION_RELAIS_A_BIT , Bit_RESET);
        break;
    case IND_B:
        if (state == FALSE)
            GPIO_WriteBit(INDICATION_RELAIS_B_PORT, INDICATION_RELAIS_B_BIT , Bit_SET);
        else
            GPIO_WriteBit(INDICATION_RELAIS_B_PORT, INDICATION_RELAIS_B_BIT , Bit_RESET);
        break;
    default:
        break;
    }
}

/**
  * @brief  Activate indication on the selected channel / relais.
  * @param  ind: selected relais
  * @param  timeout: timeout in 100 ms
  * @retval None
  */
void INDICATION_Set(IND_RELAIS_T ind, IND_TIMEOUT_T timeout)
{
    if (timeout <= 0)
        _indication_state[ind].timeout = INDICATION_NO_TIMEOUT;
    else
        _indication_state[ind].timeout = timeout;

    _indication_state[ind].state = TRUE;
}

/**
  * @brief  Deactivate indication on the selected channel / relais.
  * @param  ind: selected relais
  * @retval None
  */
void INDICATION_Reset(IND_RELAIS_T ind)
{
    _indication_state[ind].state = FALSE;
    _indication_state[ind].timeout = 0;
}

/**
  * @brief  Override indication.
  * @param  ind: selected relais
  * @param  timeout: timeout in 100 ms
  * @retval None
  */
void INDICATION_OVERRIDE_Set(IND_RELAIS_T ind, bool state, IND_TIMEOUT_T timeout)
{
    if (timeout <= 0)
        return;

    indication_override_state[ind].timeout = timeout;
    indication_override_state[ind].state = state;
    u_indication_override_count++;
}

/**
  * @brief  Indication task.
  * @param  pvParameters
  * @retval None
  */
void INDICATION_Task(void *pvParameters)
{
    static LOG_DUPVAR_T indication_logdupvar[IND_RELAIS_COUNT];

    // turn relais 1 on
    GPIO_WriteBit(INDICATION_RELAIS_A_PORT, INDICATION_RELAIS_A_BIT , Bit_RESET);
    // turn relais 2 on
    GPIO_WriteBit(INDICATION_RELAIS_B_PORT, INDICATION_RELAIS_B_BIT , Bit_RESET);

    for (uint8_t idx = 0; idx < IND_RELAIS_COUNT; idx++)
    {
        _indication_state[idx].state = FALSE;
        _indication_state[idx].timeout = 0;
        indication_override_state[idx].state = FALSE;
        indication_override_state[idx].timeout = 0;
    }

    u_indication_override_count = 0;

    while(1)
    {
        WATCHDOG_Heartbeat(WATCHDOG_TASK_ID_INDICATION);

        // indication override mostly used by factory tests
        if (u_indication_override_count > 0)
        {
            u_indication_override_count = 0;

            for (uint8_t idx = 0; idx < IND_RELAIS_COUNT; idx++)
            {
                if (indication_override_state[idx].timeout > 0)
                {
                    indication_override_state[idx].timeout--;
                    u_indication_override_count++;
                }
                if (indication_override_state[idx].timeout == 0)
                    indication_override_state[idx].state = FALSE;
            }

            // set relais
            for (uint8_t idx = 0; idx < IND_RELAIS_COUNT; idx++)
            {
                INDICATION_Relais(idx, indication_override_state[idx].state);
            }
        }
        else
        {
            // set relais
            for (uint8_t idx = 0; idx < IND_RELAIS_COUNT; idx++)
            {
                INDICATION_Relais(idx, _indication_state[idx].state);

                switch (idx)
                {
                case IND_A:
                    if (_indication_state[idx].state != 0)
                        LOG(LOG_ID_INDICATION_WARNING, 0, _detector_status.uWarningFlags, 0, &indication_logdupvar[idx]);
                    else
                        LOG(LOG_ID_INDICATION_WARNING_CLEARED, 0, 0, 0, &indication_logdupvar[idx]);
                    break;
                case IND_B:
                    if (_indication_state[idx].state != 0)
                        LOG(LOG_ID_INDICATION_ERROR,  _detector_status.uErrorFlags, 0, 0, &indication_logdupvar[idx]);
                    else
                        LOG(LOG_ID_INDICATION_ERROR_CLEARED, 0, 0, 0, &indication_logdupvar[idx]);
                    break;
                default:
                    break;
                }
            }
        }

        for (uint8_t idx = 0; idx < IND_RELAIS_COUNT; idx++)
        {
            if (_indication_state[idx].timeout > 0)
                _indication_state[idx].timeout--;
            if (_indication_state[idx].timeout == 0)
                _indication_state[idx].state = FALSE;
        }
        vTaskDelay(INDICATION_WAIT_TIME);
    }
}
