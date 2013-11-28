#ifndef VISIBILITY_H
#define VISIBILITY_H

#include "sensor.h"
#include "settings.h"

#define VISIBILITY_SENSOR_TYPE      _settings_list_ram[SETTINGS_VISIBILITY_SENSOR_TYPE]
#define VISIBILITY_CLUSTER_ID       _settings_list_ram[SETTINGS_VISIBILITY_CLUSTER_ID]
#define VISIBILITY_CLUSTER_ID_MASK  _settings_list_ram[SETTINGS_VISIBILITY_CLUSTER_ID_MASK]

#define VISI_ACQUIRE_MAX_ERRCNT 4

#define VISI_STATE_TIMEOUT_INIT         3
#define VISI_STATE_INTERVAL_ACQUIRE     3
#define VISI_STATE_TIMEOUT_FAILED       6

typedef enum
{
    VISIBILITY_SENSOR_STATUS_SENSOR_NOT_AVAILABLE,
    VISIBILITY_SENSOR_STATUS_OK,
    VISIBILITY_SENSOR_STATUS_HARDWARE_ERROR,
    VISIBILITY_SENSOR_STATUS_HARDWARE_WARNING,
    VISIBILITY_SENSOR_STATUS_BACKSCATTER_ALARM,
    VISIBILITY_SENSOR_STATUS_BACKSCATTER_WARNING,
    VISIBILITY_SENSOR_STATUS_UNKNOWN,
} VISIBILITY_SENSOR_STATUS_T;

typedef struct visibility_sensor_s
{
    uint32_t                    value_rms_1min;
    uint32_t                    value_rms_10min;
    VISIBILITY_SENSOR_STATUS_T  status;
    SENSOR_STATUS_T             state;
} VISIBILITY_SENSOR_T;

typedef void (Visibility_Data_In_Callback_t)(VISIBILITY_SENSOR_T *p_data);


void VISIBILITY_GetVisibility(VISIBILITY_SENSOR_T *p_data);
void VISIBILITY_SetVisibility(VISIBILITY_SENSOR_T *p_data);
void VISIBILITY_Enable(void);
void VISIBILITY_Disable(void);
void VISIBILITY_DeInit(void);
void VISIBILITY_Init(void);
bool VISIBILITY_Acquire(void);

#endif
