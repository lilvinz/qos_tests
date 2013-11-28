// Quantec Networks GmbH
// created 23.10.2009
#ifndef GLOOM_MANAGER_H
#define GLOOM_MANAGER_H

#include "rtc.h"
#include "sensors.h"

typedef enum gloom_manager_global_led_on_off_state_e
{
    GLOOM_MANAGER_GLOBAL_LED_STATE_NO_DECISION,
    GLOOM_MANAGER_GLOBAL_LED_STATE_OFF,
    GLOOM_MANAGER_GLOBAL_LED_STATE_ON
} GLOOM_MANAGER_GLOBAL_LED_ON_OFF_STATE_T;

#define GLOOM_MANAGER_GLOOM_AVG_CALC_METHOD             (_settings_list_ram[SETTINGS_GLOOM_AVG_CALC_METHOD_GL])
#define GLOOM_MANAGER_MIN_NUM_RELIABLE_CLIENTS          (_settings_list_ram[SETTINGS_ADVANCED_MIN_NUM_RELIABLE_CL])
#define GLOOM_MANAGER_EMA_FACTOR                        (_settings_list_ram[SETTINGS_ADVANCED_EMA_FACTOR])
#define GLOOM_MANAGER_TOLERANCE_FACTOR                  (_settings_list_ram[SETTINGS_ADVANCED_TOL_FACTOR])
#define GLOOM_MANAGER_LIGHT_CONSIDERATION               (_settings_list_ram[SETTINGS_GLOOM_LIGHT_CONSIDERATION])
#define GLOOM_MANAGER_GLOOM_SENSOR_USE                  (_settings_list_ram[SETTINGS_GLOOM_SENSOR_USE])
#define GLOOM_MANAGER_SWITCH_LED_ON_OFF_DELAY           (_settings_list_ram[SETTINGS_ADVANCED_SWITCH_LED_ON_OFF_DELAY])  // delay in seconds
#define KEEP_LEDS_ON_MSG_RECEIVED_TIMEOUT_SEC           (_settings_list_ram[SETTINGS_ADVANCED_KEEP_ON_MSG_TIMEOUT])  // timeout in seconds for keep-leds-on-message

#define GLOOM_MANAGER_MAX_REPEAT_ON_OFF_MSG             2
#define GLOOM_MANAGER_REPEAT_ON_OFF_MSG_INTERVAL        2 // time to wait between two messages in s
#define GLOOM_MANAGER_MIN_NUMBER_RECEIVED_GLOOM_VALUES  20      // number of values which have to be received (catch all) before filtering and selected calculation mode of ema will be considered
#define GLOOM_MANAGER_TOLERANCE_COUNTER_MAX             50      //
#define GLOOM_MANAGER_EMA_DIVISOR                       100
#define GLOOM_MANAGER_EMA_MIN_DIFFERENCE                (GLOOM_MANAGER_EMA_DIVISOR / GLOOM_MANAGER_EMA_FACTOR)  // attention: divisor % multiplier has to be 0 otherwise GLOOM_EMA_MIN_DIFFERENCE has to be incremented by one
#define GLOOM_MANAGER_ADD_VALUE_STARTUP_FACTOR          5
#define GLOOM_MANAGER_ADD_VALUE_STARTUP_DIVISOR         10
#define GLOOM_MANAGER_ADD_VALUE_STARTUP_MIN_DIFFERENCE  (GLOOM_MANAGER_ADD_VALUE_STARTUP_DIVISOR / GLOOM_MANAGER_ADD_VALUE_STARTUP_FACTOR)  // attention: divisor % multiplier has to be 0 otherwise GLOOM_EMA_MIN_DIFFERENCE has to be incremented by one

#define GLOOM_MANAGER_MESSAGE_RECEIVED_FACTOR           5     // factor for number of messages

#define GLOOM_MANAGER_GRACE_TIME_SEC                    20   // give system time to start and initialize subsystems
#define GLOOM_MANAGER_TASK_DELAY_TIME                   100  // run task with higher speed to check if there is something to do
#define GLOOM_MANAGER_UPDATE_VALUES_INTERVAL            2    // update values every 2 seconds to save cpu time
#define GLOOM_MANAGER_LENGTH_OF_PERIOD_FOR_LOWEST_VALUE 300  // in s

#define SWITCH_LED_ON_NO_GPS_TIMEOUT                    300  // timeout for no gps time in ms
#define GLOOM_MANAGER_MANUAL_DAY_NIGHT_TIMEOUT_MS       5000 // timeout for manual external day/night state request

#define GLOOM_MANAGER_SWITCH_LEDS_ON                    TRUE
#define GLOOM_MANAGER_SWITCH_LEDS_OFF                   FALSE

#define GLOOM_MANAGER_SWITCH_COMMAND_QUEUE_SIZE         22  // todo[low] check size

typedef enum gloom_manager_led_state_e
{
    GLOOM_MANAGER_LED_STATE_DAY,
    GLOOM_MANAGER_LED_STATE_NIGHT,
    GLOOM_MANAGER_LED_STATE_TWILIGHT,
    GLOOM_MANAGER_LED_STATE_NOT_SET
} GLOOM_MANAGER_LED_STATE_T;

typedef enum gloom_manager_ema_calculation_modes_e
{
    GLOOM_MANAGER_SIMPLE_EMA,                                   // Every value is used to calculate the ema
    GLOOM_MANAGER_EMA_WITHOUT_RUNAWAY_VALUES,                   // runaway values are ignored for the ema
    GLOOM_MANAGER_LOWEST_VALUE_IN_PERIOD                        // only the lowest value in a given period is used
} GLOOM_MANAGER_EMA_CALCULATION_MODES_T;

typedef enum gloom_manager_light_consideration_e
{
    GLOOM_MANAGER_LIGHT_CONSIDERATION_NORMAL,
    GLOOM_MANAGER_LIGHT_CONSIDERATION_LOCAL,
    GLOOM_MANAGER_LIGHT_CONSIDERATION_NETWORK,
    GLOOM_MANAGER_LIGHT_CONSIDERATION_ALWAYS_DAY,
    GLOOM_MANAGER_LIGHT_CONSIDERATION_ALWAYS_NIGHT,
    GLOOM_MANAGER_LIGHT_CONSIDERATION_ALWAYS_TWILIGHT
} GLOOM_MANAGER_LIGHT_CONSIDERATION_T;

typedef struct
{
    bool            b_repeat_turn_on_message;   // flag, send turn on message
    bool            b_repeat_turn_off_message;  // flag, send turn off message
    SENSOR_STATUS_T     my_gloom_state;
    float           my_global_gloom_value;      // calculated global gloom value for this client
    float           my_gloom_value_ema;
    RTCTIME         switch_on_off_time;
} GLOOM_MANAGER_DATA_STRUCT_T;

typedef struct
{
    bool on_or_off_command;  // type of received command, true --> command to switch leds on, false --> command to switch leds off
    RTCTIME switch_time;  // time to switch state, has to be in the future
} GLOOM_MANAGER_SWITCH_COMMAND_DATA_STRUCT_T;

GLOOM_MANAGER_LED_STATE_T GLOOM_MANAGER_GetLocalLedState(void);
SENSOR_STATUS_T GLOOM_MANAGER_GetReliability(void);
void GLOOM_MANAGER_SetReliability(SENSOR_STATUS_T reliability);
void GLOOM_MANAGER_SetDayNightManual(GLOOM_MANAGER_LED_STATE_T day_night);
GLOOM_MANAGER_GLOBAL_LED_ON_OFF_STATE_T GLOOM_MANAGER_GetGlobalLedOnOffOpinion(void);
float GLOOM_MANAGER_GetGlobalGloomValueAvg(void);
void GLOOM_MANAGER_SetNetworkLedOnOffOpinion(GLOOM_MANAGER_GLOBAL_LED_ON_OFF_STATE_T on_off_opinion);
void GLOOM_MANAGER_SetOnOffCommandReceivedState(bool on_or_off_command, RTCTIME switch_time);
void GLOOM_MANAGER_Task(void *pvParameters);

#endif //GLOOM_MANAGER_H
