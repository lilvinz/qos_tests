
#include "stdheader.h"
#include "statistics.h"

#include "log.h"

const uint16_t log_ids[STATISTICS_SENSORS_COUNT] =
{
    LOG_ID_STATISTICS_TEMPERATURE_MAIN_STORED,
    LOG_ID_STATISTICS_TEMPERATURE_LED_STORED,
    LOG_ID_STATISTICS_SUPPLY_VOLTAGE_STORED,
    LOG_ID_STATISTICS_VISIBILITY_STORED
};

static STATISTICS_SENSORS_T values[STATISTICS_SENSORS_COUNT];

/**
 * \brief init variables
 *
 * \param none
 * \return none
*/
void STATISTICS_Init(void)
{
    for (uint8_t i = 0; i < STATISTICS_SENSORS_COUNT; i++)
    {
        values[i].min = 1000000;  // init with a high value
        values[i].max = 0;
        values[i].sum = 0;
        values[i].num = 0;
    }
}

/**
 * \brief set value, determine minimum and maximum
 *
 * \param none
 * \return none
*/
void STATISTICS_AutoSetValue(STATISTICS_SENSORS_E type, float value)
{
    if (value < values[type].min)
        values[type].min = value;

    if (value > values[type].max)
        values[type].max = value;

    values[type].sum += value;

    values[type].num++;
}

/**
 * \brief set values
 *
 * \param none
 * \return none
*/
void STATISTICS_SetValue(STATISTICS_SENSORS_E type, float min, float max, double sum)
{
    values[type].min = min;
    values[type].max = max;
    values[type].sum += sum;
    values[type].num++;
}

/**
 * \brief get values
 *
 * \param none
 * \return values of the selected sensor
*/
STATISTICS_SENSORS_T STATISTICS_GetValue(STATISTICS_SENSORS_E type)
{
    if (type < STATISTICS_SENSORS_COUNT)
        return values[type];
    else
        return values[0];
}

/**
 * \brief collect values for statistics and write them to the log file
 *
 * \param none
 * \return none
*/
void STATISTICS_Store(void)
{
    for (uint8_t i = 0; i < STATISTICS_SENSORS_SUPPLY_VOLTAGE; i++)
    {
        if (values[i].num > 0)  // avoid div 0
            values[i].sum /= values[i].num;  // calculate average
        //ToDo If transition to new Log output with inline values occures, remove abs() here for int (+/-) representation
        LOG(log_ids[i], (uint16_t)abs((int)values[i].sum), (uint32_t)abs((int)values[i].min), (uint32_t)abs((int)values[i].max), NULL);
    }
    STATISTICS_Init();
}
