#include "stdheader.h"
#include "visibility.h"
#include "visibility_vaisala_pwd20.h"

#include "state.h"
#include "ascii.h"
#include "systemconfig.h"
#include "log.h"

static VISIBILITY_SENSOR_T  visibility;
static uint8_t visibility_acquire_errcnt = 0;
static LOG_DUPVAR_T visi_logdupvar;
static LOG_DUPVAR_T visi_logstatusdupvar;

void visibility_data_in(VISIBILITY_SENSOR_T *p_data);


void visibility_data_in(VISIBILITY_SENSOR_T *p_data)
{
    LOG(LOG_ID_VISIBILITY_COMM_STATUS_OK, 0, p_data->value_rms_1min, p_data->value_rms_10min, &visi_logdupvar);

    if (visibility_acquire_errcnt)
        visibility_acquire_errcnt--;

    VISIBILITY_SetVisibility(p_data);

    switch (p_data->status)
    {
    case VISIBILITY_SENSOR_STATUS_OK:
        LOG(LOG_ID_VISIBILITY_SENSOR_STATUS_OK, p_data->status, 0, 0, &visi_logstatusdupvar);
        break;
    case VISIBILITY_SENSOR_STATUS_HARDWARE_ERROR:
        LOG(LOG_ID_VISIBILITY_SENSOR_STATUS_HARDWARE_ERROR, p_data->status, 0, 0, &visi_logstatusdupvar);
        break;
    case VISIBILITY_SENSOR_STATUS_HARDWARE_WARNING:
        LOG(LOG_ID_VISIBILITY_SENSOR_STATUS_HARDWARE_WARNING, p_data->status, 0, 0, &visi_logstatusdupvar);
        break;
    case VISIBILITY_SENSOR_STATUS_BACKSCATTER_ALARM:
        LOG(LOG_ID_VISIBILITY_SENSOR_STATUS_BACKSCATTER_ALARM, p_data->status, 0, 0, &visi_logstatusdupvar);
        break;
    case VISIBILITY_SENSOR_STATUS_BACKSCATTER_WARNING:
        LOG(LOG_ID_VISIBILITY_SENSOR_STATUS_BACKSCATTER_WARNING, p_data->status, 0, 0, &visi_logstatusdupvar);
        break;
    default:
        break;
    }

    if (p_data->state == SENSOR_OK)
        systemconfig_update_visibility();
}

void VISIBILITY_GetVisibility(VISIBILITY_SENSOR_T *p_data)
{
    taskENTER_CRITICAL();
    memcpy(p_data, &visibility, sizeof(VISIBILITY_SENSOR_T));
    taskEXIT_CRITICAL();
}

void VISIBILITY_SetVisibility(VISIBILITY_SENSOR_T *p_data)
{
    taskENTER_CRITICAL();
    memcpy(&visibility, p_data, sizeof(VISIBILITY_SENSOR_T));
    taskEXIT_CRITICAL();
}

bool VISIBILITY_Acquire(void)
{
    bool bRet;

    switch (VISIBILITY_SENSOR_TYPE)
    {
    case SETTINGS_VISIBILITY_SENSOR_TYPE_NONE:
        bRet = FALSE;
        break;
    case SETTINGS_VISIBILITY_SENSOR_TYPE_VAISALA_PWD20:
        bRet = VISIBILITY_VAISALA_PWD20_Acquire(visibility_data_in);
        break;
    default:
        bRet = FALSE;
        break;
    }

    if (bRet == FALSE)
        visibility_acquire_errcnt++;

    if (visibility_acquire_errcnt >= VISI_ACQUIRE_MAX_ERRCNT)
    {
        // to many errors
        visibility_acquire_errcnt = 0;
        //visi_state.current = VISI_STATE_FAILED;
        LOG(LOG_ID_VISIBILITY_COMM_STATUS_FAILED, 0, 0, 0, &visi_logdupvar);
        VISIBILITY_SENSOR_T local_visibility;
        local_visibility.state = SENSOR_NOT_AVAILABLE;
        local_visibility.value_rms_10min = UINT16_T_MAX;
        local_visibility.value_rms_1min = UINT16_T_MAX;
        local_visibility.status = VISIBILITY_SENSOR_STATUS_SENSOR_NOT_AVAILABLE;
        VISIBILITY_SetVisibility(&local_visibility);
    }

    return bRet;
}

void VISIBILITY_Init(void)
{
    taskENTER_CRITICAL();
    visibility.state = SENSOR_NOT_AVAILABLE;
    visibility.status = VISIBILITY_SENSOR_STATUS_SENSOR_NOT_AVAILABLE;
    visibility.value_rms_10min = UINT16_T_MAX;
    visibility.value_rms_1min = UINT16_T_MAX;
    taskEXIT_CRITICAL();

    visibility_acquire_errcnt = 0;

    switch (VISIBILITY_SENSOR_TYPE)
    {
    case SETTINGS_VISIBILITY_SENSOR_TYPE_NONE:
        VISIBILITY_DeInit();
        break;
    case SETTINGS_VISIBILITY_SENSOR_TYPE_VAISALA_PWD20:
        VISIBILITY_VAISALA_PWD20_Init();
        break;
    default:
        break;
    }
}

void VISIBILITY_DeInit(void)
{
    switch (VISIBILITY_SENSOR_TYPE)
    {
    case SETTINGS_VISIBILITY_SENSOR_TYPE_NONE:
        break;
    case SETTINGS_VISIBILITY_SENSOR_TYPE_VAISALA_PWD20:
        VISIBILITY_VAISALA_PWD20_DeInit();
        break;
    default:
        break;
    }

    taskENTER_CRITICAL();
    visibility.state = SENSOR_NOT_AVAILABLE;
    visibility.status = VISIBILITY_SENSOR_STATUS_SENSOR_NOT_AVAILABLE;
    visibility.value_rms_10min = UINT16_T_MAX;
    visibility.value_rms_1min = UINT16_T_MAX;
    taskEXIT_CRITICAL();
}
