// Quantec Networks GmbH
// created 20.05.2010
//
// this module evaluates the internal and external gloom values to decide if the leds have to be switched on.

#include "stdheader.h"
#include "gloom_manager.h"

#include "gloom.h"
#include "clock.h"
#include "settings.h"
#include "bmm.h"
#include "syndication.h"
#include "meshcom_bc.h"
#include "log.h"
#include "debug.h"
#include "watchdog.h"
#include "debug_freertos.h"

static bool     b_local_led_on_off_state = false;       // this variable represents the state of this device, true: leds have to be turned on, false: leds have to be turned off
static bool     b_turn_leds_on_msg_received = false;    // flag, turn on message received
static bool     b_turn_leds_off_msg_received = false;   // flag, turn off message received
static float    global_gloom_value_avg = 0;             // this value is calculated on each device with the received gloom values from other devices
static uint32_t keep_leds_on_msg_received_time = 0;     // time stamp
static uint32_t switch_led_uptime_time = 0;             // time at which a led switch message was received, used only for backup
static uint32_t timestamp_last_tx = 0;                  // time stamp
static RTCTIME  switch_led_on_gps_time = 0;             // time at which led flash mode should be turned on
static RTCTIME  switch_led_off_gps_time = 0;            // time at which led flash mode should be turned off
static GLOOM_MANAGER_LED_STATE_T                manual_day_night = GLOOM_MANAGER_LED_STATE_NOT_SET;       // flag, day night controlled by modbus
static SENSOR_STATUS_T                          my_reliability = SENSOR_WAIT_FOR_INIT;
static GLOOM_MANAGER_GLOBAL_LED_ON_OFF_STATE_T  my_global_led_on_off_opinion = GLOOM_MANAGER_GLOBAL_LED_STATE_NO_DECISION;
static xQueueHandle xQueueSwitchCommandReceived = 0;
static xQueueHandle xQueueExternalDayNight;
/**
 * \brief initialize variables
 *
 * \param none
 * \return none
*/
void GLOOM_MANAGER_Init(GLOOM_MANAGER_DATA_STRUCT_T *p_gloom_mng_ds)
{
    p_gloom_mng_ds->b_repeat_turn_on_message    = false;
    p_gloom_mng_ds->b_repeat_turn_off_message   = false;
    p_gloom_mng_ds->my_gloom_state              = SENSOR_WAIT_FOR_INIT;
    p_gloom_mng_ds->my_global_gloom_value       = 0;
    p_gloom_mng_ds->my_gloom_value_ema          = 0;
    p_gloom_mng_ds->switch_on_off_time          = 0;

    b_local_led_on_off_state                    = false;
    b_turn_leds_on_msg_received                 = false;
    b_turn_leds_off_msg_received                = false;
    manual_day_night                            = GLOOM_MANAGER_LED_STATE_NOT_SET;
    my_reliability                              = SENSOR_WAIT_FOR_INIT;
    my_global_led_on_off_opinion                = GLOOM_MANAGER_GLOBAL_LED_STATE_NO_DECISION;
    global_gloom_value_avg                      = 0;
    keep_leds_on_msg_received_time              = 0;
    switch_led_uptime_time                      = 0;
    timestamp_last_tx                           = 0;
    switch_led_on_gps_time                      = 0;
    switch_led_off_gps_time                     = 0;
}

/**
 * \brief set message information
 *
 * \param   on_or_off_command   : type of received command, true --> command to switch leds on, false --> command to switch leds off
 *          switch_time         : time to switch state, have to be in future
 * \return none
*/
void GLOOM_MANAGER_SetOnOffCommandReceivedState(bool on_or_off_command, RTCTIME switch_time)
{
    // Warning!
    // If sender or receiver has wrong time information (maybe because gps module is not initialized or damaged),
    // this command may not work. Therefor a second timer is implemented which is based on the uptime and switches
    // the leds on after a constant time.

    GLOOM_MANAGER_SWITCH_COMMAND_DATA_STRUCT_T switch_command;
    switch_command.on_or_off_command = on_or_off_command;
    switch_command.switch_time = switch_time;
    xQueueSend(xQueueSwitchCommandReceived, &switch_command, 0);
}

/**
 * \brief set message information (intern, this function is used only in gloom_manager task)
 *
 * \param   on_or_off_command   : type of received command, true --> command to switch leds on, false --> command to switch leds off
 *          switch_time         : time to switch state, have to be in future
 * \return none
*/
void GLOOM_MANAGER_SetOnOffCommandReceivedStateIntern(bool on_or_off_command, RTCTIME switch_time)
{
    // Warning!
    // If sender or receiver has wrong time information (maybe because gps module is not initialized or damaged),
    // this command may not work. Therefor a second timer is implemented which is based on the uptime and switches
    // the leds on after a constant time.

    if (on_or_off_command)
    {
        b_turn_leds_off_msg_received    = FALSE;
        keep_leds_on_msg_received_time  = CLOCK_GetTimestamp();
        if (!b_turn_leds_on_msg_received)
        {
            // set flag to handle turn leds on command
            b_turn_leds_on_msg_received     = TRUE;
            switch_led_on_gps_time          = switch_time;
            switch_led_uptime_time          = CLOCK_GetTimestamp();
            LOG(LOG_ID_GLOOMMANAGER_RECEIVED_ON_OFF_COMMAND, GLOOM_MANAGER_SWITCH_LEDS_ON, switch_time, 0, NULL);
        }
        else
        {
            // switch on message already received ==> set the time to switch the operating mode to the nearest time
            if ((switch_time > CLOCK_GetTimestamp()) && (switch_time < switch_led_on_gps_time))
            {
                switch_led_on_gps_time = switch_time;
                LOG(LOG_ID_GLOOMMANAGER_RECEIVED_ON_OFF_COMMAND, GLOOM_MANAGER_SWITCH_LEDS_ON, switch_time, 0, NULL);
            }
        }
    }
    else
    {
        if ((!b_turn_leds_off_msg_received) && (!b_turn_leds_on_msg_received))
        {
            // set flag to handle turn leds off command
            b_turn_leds_off_msg_received    = TRUE;
            switch_led_off_gps_time         = switch_time;
            switch_led_uptime_time          = CLOCK_GetTimestamp();
            LOG(LOG_ID_GLOOMMANAGER_RECEIVED_ON_OFF_COMMAND, GLOOM_MANAGER_SWITCH_LEDS_OFF, switch_time, 0, NULL);
        }
    }

    DPRINT2(DEBUG_GLOOMMANAGER, "GLOOM MANAGER received on_off_message state: %u switch time: %u\n", on_or_off_command, switch_time);
}

/**
 * \brief set network on / off opinion
 *
 * \param   on_off_opinion  :
 * \return none
*/
void GLOOM_MANAGER_SetNetworkLedOnOffOpinion(GLOOM_MANAGER_GLOBAL_LED_ON_OFF_STATE_T on_off_opinion)
{
    // only GLOOM_MANAGER_GLOBAL_LED_STATE_ON is considered at this time
    if (on_off_opinion == GLOOM_MANAGER_GLOBAL_LED_STATE_ON)
        keep_leds_on_msg_received_time = CLOCK_GetTimestamp();

    DPRINT1(DEBUG_GLOOMMANAGER, "GLOOM MANAGER set_network_led_on_off_opinion: %u\n", on_off_opinion);
}

/**
 * \brief get decision if own leds should turned on or off
 *
 * \param none
 * \return bool - GLOOM_MANAGER_LED_STATE_NIGHT : activate night mode
 *              - GLOOM_MANAGER_LED_STATE_DAY : activate day mode
*/
GLOOM_MANAGER_LED_STATE_T GLOOM_MANAGER_GetLocalLedState(void)
{
    if (manual_day_night != GLOOM_MANAGER_LED_STATE_NOT_SET)
        return manual_day_night;

    if (b_local_led_on_off_state)
        return GLOOM_MANAGER_LED_STATE_NIGHT;
    else
        return GLOOM_MANAGER_LED_STATE_DAY;
}

/**
 * \brief   get global calculated global average of gloom value
 *
 * \param none
 * \return gloom value
*/
float GLOOM_MANAGER_GetGlobalGloomValueAvg(void)
{
    return global_gloom_value_avg;
}

/**
 * \brief   get decision if global leds should turned on or off
 *          needed for status message
 *          every device which thinks the lamps should be on sends this information
 *          with the status message
 *          the turn on command will be send in an own message from the first device which detects darkness
 *
 * \param none
 * \return GLOOM_MANAGER_GLOBAL_LED_STATE
*/
GLOOM_MANAGER_GLOBAL_LED_ON_OFF_STATE_T GLOOM_MANAGER_GetGlobalLedOnOffOpinion(void)
{
    return my_global_led_on_off_opinion;
}

/**
 * \brief   get reliability
 *
 * \param none
 * \return SENSOR_STATE_E
*/
SENSOR_STATUS_T GLOOM_MANAGER_GetReliability(void)
{
    return my_reliability;
}

/**
 * \brief   set day night manual
 *
 * \param SENSOR_STATE_E
 * \return none
*/
void GLOOM_MANAGER_SetDayNightManual(GLOOM_MANAGER_LED_STATE_T day_night)
{
    if (xQueueExternalDayNight != NULL)
        xQueueSendToBack(xQueueExternalDayNight, (void*)&day_night, (portTickType)100);
}


/**
 * \brief   set reliability
 *
 * \param SENSOR_STATE_E
 * \return none
*/
void GLOOM_MANAGER_SetReliability(SENSOR_STATUS_T reliability)
{
    my_reliability = reliability;  // todo[medium] change function, function is used by another task to change internal variable
}


/**
 * \brief calculate state for local and global leds, overwrite reliability if necessary
 *
 * \param none
 * \return none
*/
void calc_new_led_on_off_state(GLOOM_MANAGER_DATA_STRUCT_T *p_gloom_mng_ds)
{
    // ********************************************************************************
    // parameter GLOOM_MANAGER_LIGHT_CONSIDERATION is used to select different modes for
    // calculating led state. This is usefull to turn leds on or off manually for
    // exsample in case of an error
    // ********************************************************************************
    switch (GLOOM_MANAGER_LIGHT_CONSIDERATION)
    {
    case GLOOM_MANAGER_LIGHT_CONSIDERATION_ALWAYS_DAY:
    {
        // always off
        // overwrite reliability
        // set global led on off opinion to neutral
        // turn local leds off
        my_reliability                  = p_gloom_mng_ds->my_gloom_state;
        my_global_led_on_off_opinion    = GLOOM_MANAGER_GLOBAL_LED_STATE_NO_DECISION;
        b_local_led_on_off_state        = FALSE;
        break;
    }
    case GLOOM_MANAGER_LIGHT_CONSIDERATION_ALWAYS_NIGHT:
    {
        // always on
        // overwrite reliability
        // set global led on off opinion to neutral
        // turn local leds on
        my_reliability                  = p_gloom_mng_ds->my_gloom_state;
        my_global_led_on_off_opinion    = GLOOM_MANAGER_GLOBAL_LED_STATE_NO_DECISION;
        b_local_led_on_off_state        = TRUE;
        break;
    }
    case GLOOM_MANAGER_LIGHT_CONSIDERATION_NORMAL:
    {
        // normal operation
        bool b_opinion;

        switch(SETTINGS_FAIR_MODE)
        {
        case SETTINGS_FAIR_MODE_PASSIVE:    // fair mode (passive), local led on off state is controlled by network, opinion is always "NO OPINION"
            my_reliability                  = p_gloom_mng_ds->my_gloom_state;
            my_global_led_on_off_opinion    = GLOOM_MANAGER_GLOBAL_LED_STATE_NO_DECISION;
            return;
        case SETTINGS_FAIR_MODE_ACTIVE:    // fair mode (active), global led on off state is controlled by this device, only local gloom value is used
            b_opinion = BMM_CheckDarkness(p_gloom_mng_ds->my_gloom_value_ema);
            if (p_gloom_mng_ds->my_gloom_state == SENSOR_RUNAWAY_VALUE)
                my_reliability = SENSOR_OK;  // overwrite state
            else
                my_reliability = p_gloom_mng_ds->my_gloom_state;
            break;
        default:    // fair mode disabled
            b_opinion = BMM_CheckDarkness(p_gloom_mng_ds->my_global_gloom_value);
            break;
        }

        // decide what to do
        if (b_local_led_on_off_state)
        {
            // actual leds are on
            if (b_opinion)
            {
                // keep leds on
                if (p_gloom_mng_ds->my_gloom_state >= SENSOR_RUNAWAY_VALUE)
                {
                    my_global_led_on_off_opinion = GLOOM_MANAGER_GLOBAL_LED_STATE_ON;  // set opinion only to on if own sensor is ok
                    if (b_turn_leds_off_msg_received)
                    {
                        p_gloom_mng_ds->switch_on_off_time = RTC_GetUnixTime();  // get actual time
                        p_gloom_mng_ds->switch_on_off_time += GLOOM_MANAGER_SWITCH_LED_ON_OFF_DELAY;
                        LOG(LOG_ID_GLOOMMANAGER_SEND_SWITCH_ON_COMMAND, p_gloom_mng_ds->my_global_gloom_value, 0, 0, NULL);
                        GLOOM_MANAGER_SetOnOffCommandReceivedStateIntern(GLOOM_MANAGER_SWITCH_LEDS_ON, p_gloom_mng_ds->switch_on_off_time);
                        if (my_reliability >= SENSOR_OK)  // do not send switch on or off messages if own sensor is unreliable
                            MESHCOM_SendSwitchOnOffMessage(GLOOM_MANAGER_SWITCH_LEDS_ON, p_gloom_mng_ds->switch_on_off_time, NULL);
                        timestamp_last_tx = CLOCK_GetTimestamp();
                        p_gloom_mng_ds->b_repeat_turn_on_message = TRUE;
                        DPRINT1(DEBUG_GLOOMMANAGER, "GLOOM MANAGER calc_new_led_on_off_state send switch_on_message switch time: %u\n", p_gloom_mng_ds->switch_on_off_time);
                    }
                }
                else
                    my_global_led_on_off_opinion = GLOOM_MANAGER_GLOBAL_LED_STATE_NO_DECISION;
            }
            else
            {
                if (CLOCK_GetTimestamp() < (keep_leds_on_msg_received_time + KEEP_LEDS_ON_MSG_RECEIVED_TIMEOUT_SEC))
                    my_global_led_on_off_opinion = GLOOM_MANAGER_GLOBAL_LED_STATE_NO_DECISION;  // no decision / neutral
                else
                {
                    // send switch-off-message if no turn on or turn off message was received
                    if ((!b_turn_leds_on_msg_received) && (!b_turn_leds_off_msg_received))
                    {
                        p_gloom_mng_ds->switch_on_off_time = RTC_GetUnixTime();  // get actual time
                        p_gloom_mng_ds->switch_on_off_time += GLOOM_MANAGER_SWITCH_LED_ON_OFF_DELAY;
                        my_global_led_on_off_opinion = GLOOM_MANAGER_GLOBAL_LED_STATE_NO_DECISION;
                        LOG(LOG_ID_GLOOMMANAGER_SEND_SWITCH_OFF_COMMAND, p_gloom_mng_ds->my_global_gloom_value, 0, 0, NULL);
                        GLOOM_MANAGER_SetOnOffCommandReceivedStateIntern(GLOOM_MANAGER_SWITCH_LEDS_OFF, p_gloom_mng_ds->switch_on_off_time);
                        if (my_reliability >= SENSOR_OK)  // do not send switch on or off messages if own sensor is unreliable
                            MESHCOM_SendSwitchOnOffMessage(GLOOM_MANAGER_SWITCH_LEDS_OFF, p_gloom_mng_ds->switch_on_off_time, NULL);
                        timestamp_last_tx = CLOCK_GetTimestamp();
                        p_gloom_mng_ds->b_repeat_turn_off_message = TRUE;
                        DPRINT1(DEBUG_GLOOMMANAGER, "GLOOM MANAGER calc_new_led_on_off_state send switch_off_message switch time: %u\n", p_gloom_mng_ds->switch_on_off_time);
                    }
                }
            }
        }
        else
        {
            // actual leds are off
            if (b_opinion)
            {
                // send switch-on-message if no switch-on-message was received
                if (!b_turn_leds_on_msg_received)
                {
                    p_gloom_mng_ds->switch_on_off_time = RTC_GetUnixTime();  // get actual time
                    p_gloom_mng_ds->switch_on_off_time += GLOOM_MANAGER_SWITCH_LED_ON_OFF_DELAY;
                    my_global_led_on_off_opinion = GLOOM_MANAGER_GLOBAL_LED_STATE_NO_DECISION;
                    LOG(LOG_ID_GLOOMMANAGER_SEND_SWITCH_ON_COMMAND, p_gloom_mng_ds->my_global_gloom_value, 0, 0, NULL);
                    GLOOM_MANAGER_SetOnOffCommandReceivedStateIntern(GLOOM_MANAGER_SWITCH_LEDS_ON, p_gloom_mng_ds->switch_on_off_time);
                    if (my_reliability >= SENSOR_OK)  // do not send switch on or off messages if own sensor is unreliable
                        MESHCOM_SendSwitchOnOffMessage(GLOOM_MANAGER_SWITCH_LEDS_ON, p_gloom_mng_ds->switch_on_off_time, NULL);
                    timestamp_last_tx = CLOCK_GetTimestamp();
                    p_gloom_mng_ds->b_repeat_turn_on_message = TRUE;
                    DPRINT1(DEBUG_GLOOMMANAGER, "GLOOM MANAGER calc_new_led_on_off_state send switch_on_message switch time: %u\n", p_gloom_mng_ds->switch_on_off_time);
                }
            }
            else
                my_global_led_on_off_opinion = GLOOM_MANAGER_GLOBAL_LED_STATE_OFF;  // keep leds off
        }
        break;
    }
    case GLOOM_MANAGER_LIGHT_CONSIDERATION_LOCAL:
    {
        // use only local value
        // overwrite reliability
        // set global led on off opinion to neutral
        // turn local leds off
        my_reliability                  = p_gloom_mng_ds->my_gloom_state;
        my_global_led_on_off_opinion    = GLOOM_MANAGER_GLOBAL_LED_STATE_NO_DECISION;
        b_local_led_on_off_state        = BMM_CheckDarkness(p_gloom_mng_ds->my_gloom_value_ema);
        break;
    }
    case GLOOM_MANAGER_LIGHT_CONSIDERATION_NETWORK:
    {
        // use only global value
        // overwrite reliability
        // set global led on off opinion to neutral
        // turn local leds off
        my_reliability                  = p_gloom_mng_ds->my_gloom_state;
        my_global_led_on_off_opinion    = GLOOM_MANAGER_GLOBAL_LED_STATE_NO_DECISION;
        b_local_led_on_off_state        = BMM_CheckDarkness(global_gloom_value_avg);
        break;
    }
    }
}

/**
 * \brief calculate state for local leds
 *
 * \param none
 * \return none
*/
void calc_new_local_led_on_off_state(GLOOM_MANAGER_DATA_STRUCT_T *p_gloom_mng_ds)
{
    // ********************************************************************************
    // parameter GLOOM_MANAGER_LIGHT_CONSIDERATION is used to select different modes for
    // calculating led state. This is usefull to turn leds on or off manually for
    // exsample in case of an error
    // ********************************************************************************
    switch (GLOOM_MANAGER_LIGHT_CONSIDERATION)
    {
    case GLOOM_MANAGER_LIGHT_CONSIDERATION_ALWAYS_DAY:
    {
        // always off
        b_local_led_on_off_state = FALSE;
        break;
    }
    case GLOOM_MANAGER_LIGHT_CONSIDERATION_ALWAYS_NIGHT:
    {
        // always on
        b_local_led_on_off_state = TRUE;
        break;
    }
    case GLOOM_MANAGER_LIGHT_CONSIDERATION_NORMAL:  // normal operation (without network only the local sensor can be used)
    case GLOOM_MANAGER_LIGHT_CONSIDERATION_LOCAL:  // use only local value
    case GLOOM_MANAGER_LIGHT_CONSIDERATION_NETWORK:  // use only global value but network without network clients can not work
    default:
    {
        // switch leds on or off using my own gloom value for decision
        b_local_led_on_off_state = BMM_CheckDarkness(p_gloom_mng_ds->my_gloom_value_ema) || (p_gloom_mng_ds->my_gloom_state < SENSOR_OK);
        break;
    }
    }
}

/**
 * \brief function to calculate minimum and maximum values
 *
 * \param float *average_value, float new_value
 * \return none
*/
void GLOOM_MANAGER_Add_New_Val_AVG(float *average_value, float new_value)
{
    float difference;
    difference = (float) new_value - *average_value;
    // check if the absolute value of difference is not to small for following operations
    if (abs(difference) < abs(GLOOM_MANAGER_EMA_MIN_DIFFERENCE))
    {
        // the absolute value is to small for normal calculation -> add or subtract 1
        if (difference != 0)
        {
            // do not change average if difference is 0!
            if (difference > 0)
                *average_value += 1;  // add 1
            else
                *average_value -= 1;  // subtract 1
        }
    }
    else
    {
        // difference is big enough -> normal calculation
        difference *= GLOOM_MANAGER_EMA_FACTOR;
        difference /= GLOOM_MANAGER_EMA_DIVISOR;
        *average_value += difference;
    }

}

/**
 * \brief function to calculate minimum and maximum values
 *
 * \param float average_value, float *min_value, float *max_value
 * \return none
*/
void GLOOM_MANAGER_Calc_Min_Max(float average_value, float *min_value, float *max_value)
{
    // calculate max value
    *max_value = (float)(average_value) * GLOOM_MANAGER_TOLERANCE_FACTOR / 100;
    if ((*max_value - average_value) < GLOOM_MIN_GLOOM_TOLERANCE)
        *max_value = (uint16_t)(average_value + GLOOM_MIN_GLOOM_TOLERANCE);  // avoid to small values for tolerance

    // calculate min value
    float tolerance;
    tolerance = average_value - ((average_value * 100) / GLOOM_MANAGER_TOLERANCE_FACTOR);
    // avoid too small values for tolerance
    if (tolerance < GLOOM_MIN_GLOOM_TOLERANCE)
    {
        if (average_value > GLOOM_MIN_GLOOM_TOLERANCE)
            *min_value = average_value - GLOOM_MIN_GLOOM_TOLERANCE;
        else
            *min_value = 0;  // avoid underflow
    }
    else
        *min_value = (uint16_t)((average_value - tolerance));
}

/**
 * \brief Task function
 *
 * \param none
 * \return none
*/
void GLOOM_MANAGER_Task(void *pvParameters)
{
    static uint8_t i_considered_gloom_values = 0;
    static uint8_t i_global_value_out_of_tolerance_ctr = 0;  // number of values out of tolerance, used to reset ema, used in mode "ignore runaway values" only
    uint8_t i_turn_on_off_message_ctr = 0;  // number of
    static uint32_t timestamp_last_calculation = 0; // time stamp
    static float nw_gloom_without_runaway = 0;
    GLOOM_SENSOR_T gloom_sensor;
    gloom_sensor.state = SENSOR_WAIT_FOR_INIT;
    gloom_sensor.value = 0;
    GLOOM_MANAGER_DATA_STRUCT_T gloom_mng_ds;

    xQueueSwitchCommandReceived = xQueueCreate(GLOOM_MANAGER_SWITCH_COMMAND_QUEUE_SIZE, sizeof(GLOOM_MANAGER_SWITCH_COMMAND_DATA_STRUCT_T));
    DQueueAddToRegistry(xQueueSwitchCommandReceived, "xQueueSwitchCommandReceived");
    assert_param(xQueueSwitchCommandReceived != NULL);

    xQueueExternalDayNight = xQueueCreate(1, sizeof(GLOOM_MANAGER_LED_STATE_T));
    assert_param(xQueueExternalDayNight != NULL);

    GLOOM_MANAGER_Init(&gloom_mng_ds);  // initialize this module

    // give system time to initialize subsystems at startup
    vTaskDelay(GLOOM_MANAGER_GRACE_TIME_SEC);

    while(1)
    {
        WATCHDOG_Heartbeat(WATCHDOG_TASK_ID_GLOOM_MANAGER);

        while(uxQueueMessagesWaiting(xQueueSwitchCommandReceived) > 0)
        {
            GLOOM_MANAGER_SWITCH_COMMAND_DATA_STRUCT_T switch_command;
            xQueueReceive(xQueueSwitchCommandReceived, &switch_command, 0);
            GLOOM_MANAGER_SetOnOffCommandReceivedStateIntern(switch_command.on_or_off_command, switch_command.switch_time);
        }

        // check manual day/night request queue.
        xQueueReceive(xQueueExternalDayNight, (void*)&manual_day_night, 0);

        // check if a led-turn-on-message was received
        if (b_turn_leds_on_msg_received  && ((RTC_GetUnixTime() > switch_led_on_gps_time) || (CLOCK_GetTimestamp() > (switch_led_uptime_time + SWITCH_LED_ON_NO_GPS_TIMEOUT))))
        {
            // no further checks
            // it is better to turn on a lamp which should be off than
            // to turn off a lamp which should be on
            b_local_led_on_off_state        = TRUE;
            b_turn_leds_on_msg_received     = FALSE;
            b_turn_leds_off_msg_received    = FALSE;
        }

        // check if a led-turn-off-message was received
        if (b_turn_leds_off_msg_received && ((RTC_GetUnixTime() > switch_led_off_gps_time) || (CLOCK_GetTimestamp() > (switch_led_uptime_time + SWITCH_LED_ON_NO_GPS_TIMEOUT))))
        {
            b_turn_leds_off_msg_received    = FALSE;

            // check if lamp can be turned off
            // to turn lamp off the following conditions have to be met
            // no network turn on request
            // no turn on opinion when sensor is ok
            if (!b_turn_leds_on_msg_received)
                b_local_led_on_off_state        = FALSE;
        }

        // check if led_turn_on_message should be send
        if ((gloom_mng_ds.b_repeat_turn_on_message) && (CLOCK_GetTimestamp() > (timestamp_last_tx + GLOOM_MANAGER_REPEAT_ON_OFF_MSG_INTERVAL)))
        {
            // send message
            i_turn_on_off_message_ctr++;
            MESHCOM_SendSwitchOnOffMessage(GLOOM_MANAGER_SWITCH_LEDS_ON, gloom_mng_ds.switch_on_off_time, NULL);
            //LOG(0, LOG_TYPE_INFO, LOG_MODULE_GLOOM, LOG_TEXT_MODULE_GLOOM_SEND_ON_OFF_COMMAND, GLOOM_MANAGER_SWITCH_LEDS_ON, 0, 0, NULL);
            DPRINT1(DEBUG_GLOOMMANAGER, "GLOOM MANAGER repeated switch_on_message switch time: %u\n", gloom_mng_ds.switch_on_off_time);
            timestamp_last_tx = CLOCK_GetTimestamp();
            if (i_turn_on_off_message_ctr >= GLOOM_MANAGER_MAX_REPEAT_ON_OFF_MSG)
            {
                gloom_mng_ds.b_repeat_turn_on_message    = FALSE;
                i_turn_on_off_message_ctr   = 0;
            }
        }

        // check if led_turn_off_message should be send
        if ((gloom_mng_ds.b_repeat_turn_off_message) && (CLOCK_GetTimestamp() > (timestamp_last_tx + GLOOM_MANAGER_REPEAT_ON_OFF_MSG_INTERVAL)))
        {
            // send message
            i_turn_on_off_message_ctr++;
            MESHCOM_SendSwitchOnOffMessage(GLOOM_MANAGER_SWITCH_LEDS_OFF, gloom_mng_ds.switch_on_off_time, NULL);
            //LOG(0, LOG_TYPE_INFO, LOG_MODULE_GLOOM, LOG_TEXT_MODULE_GLOOM_SEND_ON_OFF_COMMAND, GLOOM_MANAGER_SWITCH_LEDS_OFF, 0, 0, NULL);
            DPRINT1(DEBUG_GLOOMMANAGER, "GLOOM MANAGER repeated switch_on_message switch time: %u\n", gloom_mng_ds.switch_on_off_time);
            timestamp_last_tx = CLOCK_GetTimestamp();
            if (i_turn_on_off_message_ctr >= GLOOM_MANAGER_MAX_REPEAT_ON_OFF_MSG)
            {
                gloom_mng_ds.b_repeat_turn_off_message   = FALSE;
                i_turn_on_off_message_ctr   = 0;
            }
        }

        // check if new calculation should be started
        if (CLOCK_GetTimestamp() >= (timestamp_last_calculation + GLOOM_MANAGER_UPDATE_VALUES_INTERVAL))
        {
            timestamp_last_calculation = CLOCK_GetTimestamp();
            // collect data from other modules
            gloom_mng_ds.my_gloom_value_ema = SENSORS_GetGloomEMA();
            SENSORS_GetGloom(&gloom_sensor);
            if (GLOOM_MANAGER_GLOOM_SENSOR_USE)
                gloom_mng_ds.my_gloom_state = gloom_sensor.state;  // use sensor
            else
                gloom_mng_ds.my_gloom_state = SENSOR_NOT_AVAILABLE;  // do not use sensor

            // determine gloom and reliability
            uint16_t syndicate_client_count = 0;
            SYNDICATION_GetSyndicateClientCount(&syndicate_client_count);  // return value is not evaluated because variable is initialized to 0

            if (syndicate_client_count <= 1)
            {
                // no network devices available --> stand alone operation
                // set reliability
                my_reliability = gloom_mng_ds.my_gloom_state;

                calc_new_local_led_on_off_state(&gloom_mng_ds);
            }
            else
            {
                // network devices available
                // collect data from syndication
                uint16_t    i_num_reliabel_clients      = 0;
                uint16_t    i_num_unreliabel_clients    = 0;
                uint16_t    client = 0;
                float       nw_gloom_lowest_value       = UINT16_T_MAX;
                double      nw_gloom_value_sum          = 0;
                double      nw_gloom_value_avg          = 0;
                SDC_GLOOM_T client_gloom_data;

                while(SYNDICATION_GetNextClientGloomInfo(&client, &client_gloom_data) == TRUE)
                {
                    // client available
                    if (client_gloom_data.sensor_state < SENSOR_OK)
                        i_num_unreliabel_clients++;  // client is not reliable
                    else
                    {
                        // client is reliable
                        i_num_reliabel_clients++;
                        nw_gloom_value_sum += (double)client_gloom_data.value;  // average
                        if (client_gloom_data.value < nw_gloom_lowest_value)
                            nw_gloom_lowest_value = client_gloom_data.value;  // lowest value

                        // average without runaway values
                        if (i_considered_gloom_values < GLOOM_MANAGER_MIN_NUMBER_RECEIVED_GLOOM_VALUES)
                        {
                            // if we are just powered up or to many values have been ignored accept all values
                            i_considered_gloom_values++;
                            GLOOM_MANAGER_Add_New_Val_AVG(&nw_gloom_without_runaway, client_gloom_data.value);
                        }
                        else
                        {
                            float min_value;
                            float max_value;
                            GLOOM_MANAGER_Calc_Min_Max(nw_gloom_without_runaway, &min_value, &max_value);
                            if ((client_gloom_data.value >= min_value) && (client_gloom_data.value <= max_value))
                            {
                                // value is within tolerance --> consider value
                                // note: used >= and <= to avoid wrong decisions if value is 0
                                GLOOM_MANAGER_Add_New_Val_AVG(&nw_gloom_without_runaway, client_gloom_data.value);
                                if (i_global_value_out_of_tolerance_ctr > 0)
                                    i_global_value_out_of_tolerance_ctr--;
                            }
                            else
                            {
                                // value is out of tolerance --> ignore value, increment error counter
                                if (i_global_value_out_of_tolerance_ctr < UINT8_T_MAX)
                                    i_global_value_out_of_tolerance_ctr++;
                                if (i_global_value_out_of_tolerance_ctr > GLOOM_MANAGER_TOLERANCE_COUNTER_MAX)
                                {
                                    // too many values out of tolerance --> ema doesn't represent measured values any more
                                    // set the ema to 0
                                    nw_gloom_without_runaway = client_gloom_data.value;
                                    i_considered_gloom_values = 0;
                                    i_global_value_out_of_tolerance_ctr = 0;
                                }
                            }
                        }
                    }
                }
                if (i_num_reliabel_clients != 0)
                    nw_gloom_value_avg = nw_gloom_value_sum / i_num_reliabel_clients;
                else
                    nw_gloom_value_avg = 0;

                // check if own value matches global value and local sensor is ok
                // do this check only if enough reliable network clients are available
                if (i_num_reliabel_clients < (uint16_t) GLOOM_MANAGER_MIN_NUM_RELIABLE_CLIENTS)
                    my_reliability = gloom_mng_ds.my_gloom_state;  // not enough reliable clients get own reliability from sensor state
                else
                {
                    float min_value;
                    float max_value;
                    if (GLOOM_MANAGER_GLOOM_AVG_CALC_METHOD == GLOOM_MANAGER_EMA_WITHOUT_RUNAWAY_VALUES)
                        GLOOM_MANAGER_Calc_Min_Max(nw_gloom_without_runaway, &min_value, &max_value);
                    else
                        GLOOM_MANAGER_Calc_Min_Max((float)nw_gloom_value_avg, &min_value, &max_value);
                    if ((gloom_mng_ds.my_gloom_value_ema >= min_value) && (gloom_mng_ds.my_gloom_value_ema <= max_value) && (gloom_mng_ds.my_gloom_state >= SENSOR_OK))
                    {
                        // own value is within tolerance --> consider value
                        // note: used >= and <= to avoid wrong decisions if gloom.value_ema is 0
                        my_reliability = gloom_mng_ds.my_gloom_state;
                    }
                    else
                    {
                        // value is out of tolerance and / or sensor is not ok
                        if (gloom_mng_ds.my_gloom_state < SENSOR_OK)
                            my_reliability = gloom_mng_ds.my_gloom_state;  // sensor is not ok
                        else
                            my_reliability = SENSOR_RUNAWAY_VALUE;  // sensor thinks it is ok but values do not match
                    }
                }

                // three different ways to calculate the global gloom value (at least for development)
                // the first way is to accept all values
                // the second way is to accept only values in the near of the actual average
                // if a value is to far away the value is not considered for the average
                switch (GLOOM_MANAGER_GLOOM_AVG_CALC_METHOD)
                {
                case GLOOM_MANAGER_SIMPLE_EMA:
                {
                    global_gloom_value_avg = (float)nw_gloom_value_avg;
                    gloom_mng_ds.my_global_gloom_value  = (float)nw_gloom_value_avg;
                    if (my_reliability >= SENSOR_OK)
                        GLOOM_MANAGER_Add_New_Val_AVG(&gloom_mng_ds.my_global_gloom_value, gloom_mng_ds.my_gloom_value_ema);
                    break;
                }
                case GLOOM_MANAGER_LOWEST_VALUE_IN_PERIOD:
                {
                    if ((nw_gloom_lowest_value < client_gloom_data.value) && (i_num_reliabel_clients > 0))
                        gloom_mng_ds.my_global_gloom_value = nw_gloom_lowest_value;
                    else
                        gloom_mng_ds.my_global_gloom_value = client_gloom_data.value;
                    global_gloom_value_avg = nw_gloom_lowest_value;
                    break;
                }
                case GLOOM_MANAGER_EMA_WITHOUT_RUNAWAY_VALUES:
                {
                    // ignore runaway values
                    global_gloom_value_avg = nw_gloom_without_runaway;
                    gloom_mng_ds.my_global_gloom_value = nw_gloom_without_runaway;
                    if (my_reliability >= SENSOR_OK)
                        GLOOM_MANAGER_Add_New_Val_AVG(&gloom_mng_ds.my_global_gloom_value, nw_gloom_without_runaway);
                    break;
                }
                }
                // determine own opinion if leds should be on or off and set states and send message
                calc_new_led_on_off_state(&gloom_mng_ds);
            }
        }
        vTaskDelay(GLOOM_MANAGER_TASK_DELAY_TIME);
    }
}
