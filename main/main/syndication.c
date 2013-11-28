#include "stdheader.h"
#include "syndication.h"

#include "clock.h"
#include "rtc.h"
#include "visibility.h"
#include "gloom.h"
#include "system.h"
#include "watchdog.h"
#include "debug_freertos.h"
//#include "debug.h"


//Note Client_ID: ID 0 is our own ID, thus our values have to be there.

static xQueueHandle        xQueueSyndication;
// Mutex to protect data of this module
static xSemaphoreHandle xSYNDICATION_AssimilatedDataMutex;
static xSemaphoreHandle xSYNDICATION_SyndicateDataMutex;
static SDC_SYNDICATE_T syndicate;
static SDC_ASSIMILATE_T assimilated;

bool SYNDICATION_GetNextClientGloomInfo(uint16_t *client, SDC_GLOOM_T *p_data)
{
    bool b_result = FALSE;
    // try to get mutex
    if (xSemaphoreTake(xSYNDICATION_SyndicateDataMutex, SDC_GET_ASSIMILATED_DATA_MUTEX_TIMEOUT_MS) != pdTRUE)
        return FALSE;  // return if mutex could not be obtained

    while((syndicate.client[*client].TimeToLive < SDC_CLIENT_VALUE_TIMEOUT) && (*client < SDC_MAX_NO_OF_CLIENTS))
    {
        (*client)++;  // search next client
    }

    if (*client != SDC_MAX_NO_OF_CLIENTS)
    {
        p_data->value = syndicate.client[*client].opinion.gloom.value;
        p_data->sensor_state = syndicate.client[*client].opinion.gloom.sensor_state;

        (*client)++;
        b_result = TRUE;
    }

    xSemaphoreGive(xSYNDICATION_SyndicateDataMutex);  // give mutex back

    return b_result;
}

/**
  * @brief  Get syndicate client data using mutex.
  * @param  pdata: Pointer where the returned data should be stored.
  * @retval boolean: - true: success
  *                  - false: timeout, mutex could not be obtained
  */
bool SYNDICATION_GetSyndicateClientData(uint16_t client, SDC_CLIENT_S *pdata)
{
    if (client >= SDC_MAX_NO_OF_CLIENTS)
        return FALSE;  // index out of boundary

    // try to get mutex
    if (xSemaphoreTake(xSYNDICATION_SyndicateDataMutex, SDC_GET_SYNDICATE_DATA_MUTEX_TIMEOUT_MS) == pdFALSE)
        return FALSE;  // return if mutex could not be obtained

    memcpy(pdata, &syndicate.client[client], sizeof(SDC_CLIENT_S));
    xSemaphoreGive(xSYNDICATION_SyndicateDataMutex);
    return TRUE;
}

/**
  * @brief  Get get syndicate client count using mutex.
  * @param  pdata: Pointer where the returned data should be stored.
  * @retval boolean: - true: success
  *                  - false: timeout, mutex could not be obtained
  */
bool SYNDICATION_GetSyndicateClientCount(uint16_t *pdata)
{
    // try to get mutex
    if (xSemaphoreTake(xSYNDICATION_SyndicateDataMutex, SDC_GET_SYNDICATE_DATA_MUTEX_TIMEOUT_MS) == pdFALSE)
        return FALSE;  // return if mutex could not be obtained

    *pdata = syndicate.client_count;

    xSemaphoreGive(xSYNDICATION_SyndicateDataMutex);
    return TRUE;
}

int32_t SYNDICATION_GetSpot(uint32_t serial_number)
{
    uint16_t idx;
    uint16_t free_spot;

    idx = 0;
    free_spot = UINT16_T_MAX;
    do
    {
        if ((free_spot == UINT16_T_MAX) && (syndicate.client[idx].TimeToLive == 0))
            free_spot = idx;

        if (syndicate.client[idx].serial_number == serial_number)
            return idx;
        idx++;
    }
    while (idx < SDC_MAX_NO_OF_CLIENTS);

    if (free_spot == UINT16_T_MAX)
        return -1;      //client not found and no space left to add new. leave.

    //reserve and init spot and return spot index;
    syndicate.client[free_spot].serial_number = serial_number;

    return free_spot;
}

/**
  * @brief  Set assimilated gloom using mutex.
  * @param  pdata: Pointer to memory with data which should be stored.
  * @retval None
  */
void SYNDICATION_SetAssimilatedGloom(SDC_A_GLOOM_T *pdata)
{
    xSemaphoreTake(xSYNDICATION_AssimilatedDataMutex, portMAX_DELAY);  // get mutex, do not access assimilated without mutex
    memcpy(&assimilated.gloom, pdata, sizeof(SDC_A_GLOOM_T));
    xSemaphoreGive(xSYNDICATION_AssimilatedDataMutex);  // give mutex back
}

/**
  * @brief  Get assimilated gloom using mutex.
  * @param  pdata: Pointer where the returned data should be stored.
  * @retval boolean: - true: success
  *                  - false: timeout, mutex could not be obtained
  */
bool SYNDICATION_GetAssimilatedGloom(SDC_A_GLOOM_T *pdata)
{
    // try to get mutex
    if (xSemaphoreTake(xSYNDICATION_AssimilatedDataMutex, SDC_GET_ASSIMILATED_DATA_MUTEX_TIMEOUT_MS) == pdFALSE)
        return FALSE;  // return if mutex could not be obtained

    memcpy(pdata, &assimilated.gloom, sizeof(SDC_A_GLOOM_T));
    xSemaphoreGive(xSYNDICATION_AssimilatedDataMutex);
    return TRUE;
}

/**
  * @brief  Set assimilated temperature using mutex.
  * @param  pdata: Pointer to memory with data which should be stored.
  * @retval None
  */
void SYNDICATION_SetAssimilatedTemperature(SDC_A_TEMPERATURE_T *pdata)
{
    xSemaphoreTake(xSYNDICATION_AssimilatedDataMutex, portMAX_DELAY);  // get mutex, do not access assimilated without mutex
    memcpy(&assimilated.temperature, pdata, sizeof(SDC_A_TEMPERATURE_T));
    xSemaphoreGive(xSYNDICATION_AssimilatedDataMutex);  // give mutex back
}

/**
  * @brief  Get assimilated temperature using mutex.
  * @param  pdata: Pointer where the returned data should be stored.
  * @retval boolean: - true: success
  *                  - false: timeout, mutex could not be obtained
  */
bool SYNDICATION_GetAssimilatedTemperature(SDC_A_TEMPERATURE_T *pdata)
{
    // try to get mutex
    if (xSemaphoreTake(xSYNDICATION_AssimilatedDataMutex, SDC_GET_ASSIMILATED_DATA_MUTEX_TIMEOUT_MS) == pdFALSE)
        return FALSE;  // return if mutex could not be obtained

    memcpy(pdata, &assimilated.temperature, sizeof(SDC_A_TEMPERATURE_T));
    xSemaphoreGive(xSYNDICATION_AssimilatedDataMutex);
    return TRUE;
}

/**
  * @brief  Set assimilated visibility using mutex.
  * @param  pdata: Pointer to memory with data which should be stored.
  * @retval None
  */
void SYNDICATION_SetAssimilatedVisibility(SDC_A_VISIBILITY_T *pdata)
{
    xSemaphoreTake(xSYNDICATION_AssimilatedDataMutex, portMAX_DELAY);  // get mutex, do not access assimilated without mutex
    memcpy(&assimilated.visibility, pdata, sizeof(SDC_A_VISIBILITY_T));
    xSemaphoreGive(xSYNDICATION_AssimilatedDataMutex);  // give mutex back
}

/**
  * @brief  Get assimilated visibility using mutex.
  * @param  pdata: Pointer where the returned data should be stored.
  * @retval boolean: - true: success
  *                  - false: timeout, mutex could not be obtained
  */
bool SYNDICATION_GetAssimilatedVisibility(SDC_A_VISIBILITY_T *pdata)
{
    // try to get mutex
    if (xSemaphoreTake(xSYNDICATION_AssimilatedDataMutex, SDC_GET_ASSIMILATED_DATA_MUTEX_TIMEOUT_MS) == pdFALSE)
        return FALSE;  // return if mutex could not be obtained

    memcpy(pdata, &assimilated.visibility, sizeof(SDC_A_VISIBILITY_T));
    xSemaphoreGive(xSYNDICATION_AssimilatedDataMutex);
    return TRUE;
}

void SYNDICATION_Submit(SDC_CLIENT_DATA_T *client_data)
{
    xQueueSend(xQueueSyndication, client_data, SYNDICATION_QUEUE_TIMEOUT_PUT);
}

void SYNDICATION_SubmitIntern(SDC_CLIENT_DATA_T *client_data)
{
    int32_t idx;
    xSemaphoreTake(xSYNDICATION_SyndicateDataMutex, portMAX_DELAY);

    idx = SYNDICATION_GetSpot(client_data->serial_number);

    if (idx < 0)
        return;  // client not found and list full

    syndicate.client[idx].serial_number = client_data->serial_number;
    memcpy(&syndicate.client[idx].opinion.temperature, &client_data->temperature, sizeof(SDC_TEMPERATURE_T));
    memcpy(&syndicate.client[idx].opinion.gloom, &client_data->gloom, sizeof(SDC_GLOOM_T));
    memcpy(&syndicate.client[idx].opinion.visibility, &client_data->visibility, sizeof(SDC_VISIBILITY_T));

    syndicate.client[idx].TimeToLive = SDC_SETTING_NODE_TIMEOUT;
    xSemaphoreGive(xSYNDICATION_SyndicateDataMutex);
}

void SYNDICATION_PutLocalValues(void)
{
    SDC_CLIENT_DATA_T my_data;

    my_data.serial_number = SERIAL_NUMBER;

    TEMPERATURE_SENSOR_T temperature;
    SENSORS_GetTemperature(TEMPERATURE_MAIN, &temperature);
    my_data.temperature.value           = (int16_t)temperature.value;
    my_data.temperature.sensor_state    = temperature.state;

    my_data.gloom.value                 = SENSORS_GetGloomEMA();
    my_data.gloom.sensor_state          = GLOOM_MANAGER_GetReliability();

    VISIBILITY_SENSOR_T visibility;
    VISIBILITY_GetVisibility(&visibility);

    my_data.visibility.value = visibility.value_rms_1min;
    my_data.visibility.sensor_status = visibility.state;
    my_data.visibility.cluster_id = VISIBILITY_CLUSTER_ID;  // set own ID

    SYNDICATION_Submit(&my_data);
}

/**
  * @brief  Create.
  * @param  None
  * @retval None
  */
void SYNDICATION_Create(void)
{
    if (xSYNDICATION_AssimilatedDataMutex == NULL)
    {
        xSYNDICATION_AssimilatedDataMutex = xSemaphoreCreateMutex();
        assert_param(xSYNDICATION_AssimilatedDataMutex != NULL);
        DQueueAddToRegistry(xSYNDICATION_AssimilatedDataMutex, (const signed char*)"xSYNDICATION_AssimilatedDataMutex");
    }
    else
        assert_param(FALSE);

    if (xSYNDICATION_SyndicateDataMutex == NULL)
    {
        xSYNDICATION_SyndicateDataMutex = xSemaphoreCreateMutex();
        assert_param(xSYNDICATION_SyndicateDataMutex != NULL);
        DQueueAddToRegistry(xSYNDICATION_SyndicateDataMutex, (const signed char*)"xSYNDICATION_SyndicateDataMutex");
    }
    else
        assert_param(FALSE);
}

void SYNDICATION_Init(void)
{
    xSemaphoreTake(xSYNDICATION_SyndicateDataMutex, portMAX_DELAY);
    for (uint16_t i = 0; i < SDC_MAX_NO_OF_CLIENTS; i++)
    {
        syndicate.client[i].TimeToLive = 0;
    }
    assimilated.time = 0;
    xSemaphoreGive(xSYNDICATION_SyndicateDataMutex);
}

void SYNDICATION_Task(void *pvParameters)
{
    static SDC_ASSIMILATE_T absorbtion;

    static uint16_t     client_count;
    static int32_t      sum_temperature;
    static float        sum_gloom;
    SDC_A_VISIBILITY_T visibility[SDC_MAX_NUM_VISIBILITY_DEV];  // todo[medium] maybe this should be moved to make values available on network status page
    float sum_visibility[SDC_MAX_NUM_VISIBILITY_DEV];
    uint16_t idx;

    xQueueSyndication       = xQueueCreate(SYNDICATION_QUEUE_SIZE, sizeof(SDC_CLIENT_DATA_T));
    DQueueAddToRegistry(xQueueSyndication, "xQueueSyndication");

    assert_param(xQueueSyndication != NULL);

    SYNDICATION_Init();

    while(1)
    {
        WATCHDOG_Heartbeat(WATCHDOG_TASK_ID_SYNDICATION);

        // add all received values to array
        SDC_CLIENT_DATA_T   client_data;
        while (pdTRUE == xQueueReceive(xQueueSyndication, &client_data, SYNDICATION_QUEUE_TIMEOUT_GET))
        {
            SYNDICATION_SubmitIntern(&client_data);
        }

        if (CLOCK_GetTimestamp() > assimilated.time)
        {
            // update
            assimilated.time = CLOCK_GetTimestamp();

            // store actual setting for timeout to avoid problems if setting is changed during processing of data
            uint32_t temp_timeout = SDC_SETTING_NODE_TIMEOUT;
            uint16_t actual_node_timeout;

            // check value (setting is 32 bit but we use only 16 bit)
            if (temp_timeout > UINT16_T_MAX)
            {
                actual_node_timeout = UINT16_T_MAX;
            }
            else
            {
                actual_node_timeout = (uint16_t) temp_timeout;
            }

            xSemaphoreTake(xSYNDICATION_SyndicateDataMutex, portMAX_DELAY);
            for (idx = 0; idx < SDC_MAX_NO_OF_CLIENTS; idx++)
            {
                // fix broken entries. can occur if node time out setting is changed by user during normal operation.
                if (syndicate.client[idx].TimeToLive > actual_node_timeout)
                    syndicate.client[idx].TimeToLive = actual_node_timeout;
                // decrease TTL;
                if (syndicate.client[idx].TimeToLive > 0)
                    syndicate.client[idx].TimeToLive--;
            }
            xSemaphoreGive(xSYNDICATION_SyndicateDataMutex);

            absorbtion.visibility.maximum   = UINT16_T_MIN;
            absorbtion.visibility.minimum   = UINT16_T_MAX;
            absorbtion.visibility.count = 0;
            absorbtion.temperature.maximum  = INT16_T_MIN;
            absorbtion.temperature.minimum  = INT16_T_MAX;
            absorbtion.temperature.count = 0;
            absorbtion.gloom.maximum        = UINT16_T_MIN;
            absorbtion.gloom.minimum        = UINT16_T_MAX;
            absorbtion.gloom.count = 0;

            for (uint8_t i = 0; i < SDC_MAX_NUM_VISIBILITY_DEV; i++)
            {
                visibility[i].maximum   = UINT16_T_MIN;
                visibility[i].minimum   = UINT16_T_MAX;
                visibility[i].count = 0;
                sum_visibility[i] = 0;
            }

            sum_temperature = 0;
            sum_gloom = 0;

            SYNDICATION_DEBUG("tick1: absorbtion visi cleared\n\r");
            client_count = 0;

            SYNDICATION_PutLocalValues();

            for(idx = 0; idx < SDC_MAX_NO_OF_CLIENTS; idx++)
            {
                // process sensor values and time to live
                SDC_CLIENT_S syndicate_client_data;

                if (SYNDICATION_GetSyndicateClientData(idx, &syndicate_client_data) == TRUE)
                {
                    uint16_t actual_value_timeout = actual_node_timeout - (actual_node_timeout / 10);
                    if (syndicate_client_data.TimeToLive > actual_value_timeout)
                    {
                        client_count++;

                        // process gloom
                        if (syndicate_client_data.opinion.gloom.sensor_state >= SENSOR_OK)
                        {
                            sum_gloom += syndicate_client_data.opinion.gloom.value;
                            absorbtion.gloom.count++;
                            if (syndicate_client_data.opinion.gloom.value > absorbtion.gloom.maximum)
                                absorbtion.gloom.maximum = syndicate_client_data.opinion.gloom.value;
                            if (syndicate_client_data.opinion.gloom.value < absorbtion.gloom.minimum)
                                absorbtion.gloom.minimum = syndicate_client_data.opinion.gloom.value;
                        }

                        // process temperature
                        if (syndicate_client_data.opinion.temperature.sensor_state >= SENSOR_OK)
                        {
                            sum_temperature += syndicate_client_data.opinion.temperature.value;
                            absorbtion.temperature.count++;
                            if (syndicate_client_data.opinion.temperature.value > absorbtion.temperature.maximum)
                                absorbtion.temperature.maximum = syndicate_client_data.opinion.temperature.value;
                            if (syndicate_client_data.opinion.temperature.value < absorbtion.temperature.minimum)
                                absorbtion.temperature.minimum = syndicate_client_data.opinion.temperature.value;
                        }

                        // process visibility
                        if ((actual_node_timeout - syndicate_client_data.TimeToLive) < SDC_CLIENT_VISIBILITY_TRUST_AGE_MAX)
                        {
                            // we have an extra timeout for visibility
                            // determine values which should be considered
                            if (syndicate_client_data.opinion.visibility.sensor_status >= SENSOR_OK)
                            {
                                // value available
                                for (uint8_t i = 0; i < SDC_MAX_NUM_VISIBILITY_DEV; i++)
                                {
                                    if (syndicate_client_data.opinion.visibility.cluster_id & (1 << i))
                                    {
                                        visibility[i].count++;
                                        sum_visibility[i] += syndicate_client_data.opinion.visibility.value;
                                        if (syndicate_client_data.opinion.visibility.value > visibility[i].maximum)
                                            visibility[i].maximum = syndicate_client_data.opinion.visibility.value;
                                        if (syndicate_client_data.opinion.visibility.value < absorbtion.visibility.minimum)
                                            visibility[i].minimum = syndicate_client_data.opinion.visibility.value;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if (syndicate_client_data.TimeToLive == 0)
                        {
                            //clear entry
                            xSemaphoreTake(xSYNDICATION_SyndicateDataMutex, portMAX_DELAY);
                            syndicate.client[idx].serial_number = 0;
                            xSemaphoreGive(xSYNDICATION_SyndicateDataMutex);
                        }
                    }
                }
            }

            xSemaphoreTake(xSYNDICATION_SyndicateDataMutex, portMAX_DELAY);
            syndicate.client_count = client_count;
            xSemaphoreGive(xSYNDICATION_SyndicateDataMutex);

            //calculate averages
            if (absorbtion.gloom.count > 0)
            {
                SDC_A_GLOOM_T gloom_data;

                gloom_data.count = absorbtion.gloom.count;
                gloom_data.minimum = absorbtion.gloom.minimum;
                gloom_data.maximum = absorbtion.gloom.maximum;
                gloom_data.timestamp = assimilated.time;
                gloom_data.average = sum_gloom / absorbtion.gloom.count;

                SYNDICATION_SetAssimilatedGloom(&gloom_data);
            }

            if (absorbtion.temperature.count > 0)
            {
                SDC_A_TEMPERATURE_T temperature_data;

                temperature_data.count = absorbtion.temperature.count;
                temperature_data.minimum = absorbtion.temperature.minimum;
                temperature_data.maximum = absorbtion.temperature.maximum;
                temperature_data.timestamp = assimilated.time;
                temperature_data.average = sum_temperature / absorbtion.temperature.count;

                SYNDICATION_SetAssimilatedTemperature(&temperature_data);
            }

            float used_visibility_sum = 0;
            for (uint8_t i = 0; i < SDC_MAX_NUM_VISIBILITY_DEV; i++)
            {
                if (VISIBILITY_CLUSTER_ID_MASK & (1 << i))
                {
                    // use only selected devices
                    SYNDICATION_DEBUG4("tick avg1: bit: %hx, visicount: %hx, visimax: %u, visimin: %u\n\r", i, visibility.count, visibility[i].maximum, visibility[i].minimum);
                    if (visibility[i].count > 0)
                    {
                        absorbtion.visibility.count++;
                        if (visibility[i].maximum > absorbtion.visibility.maximum)
                            absorbtion.visibility.maximum = visibility[i].maximum;
                        if (visibility[i].minimum < absorbtion.visibility.minimum)
                            absorbtion.visibility.minimum = visibility[i].minimum;

                        used_visibility_sum += (sum_visibility[i] / visibility[i].count);
                    }
                }
            }

            if (absorbtion.visibility.count > 0)
            {
                SDC_A_VISIBILITY_T visibility_data;

                visibility_data.count = absorbtion.visibility.count;
                visibility_data.minimum = absorbtion.visibility.minimum;
                visibility_data.maximum = absorbtion.visibility.maximum;
                visibility_data.timestamp = assimilated.time;
                visibility_data.average = used_visibility_sum / absorbtion.visibility.count;

                SYNDICATION_SetAssimilatedVisibility(&visibility_data);
                SYNDICATION_DEBUG6("assimi1: count: %hx, min: %u, max: %u, time: %u, avg: %u, trusted: %hx\n\r", assimilated.visibility.count, assimilated.visibility.minimum, assimilated.visibility.maximum, assimilated.visibility.timestamp, assimilated.visibility.average, assimilated.trusted_vid);
            }
        }
    }
}
