#ifndef STATISTICS_H
#define STATISTICS_H


typedef struct sensors_temperature_statistics_s
{
    float       min;  // minimum value
    float       max;  // maximum value
    double      sum;  // sum for average value
    uint32_t    num;  // number of values for average value
} STATISTICS_SENSORS_T;

typedef enum
{
    STATISTICS_SENSORS_TEMPERATURE_MAIN = 0,
    STATISTICS_SENSORS_TEMPERATURE_LED,
    STATISTICS_SENSORS_SUPPLY_VOLTAGE,
    STATISTICS_SENSORS_VISIBILITY,
    STATISTICS_SENSORS_COUNT,
} STATISTICS_SENSORS_E;

void STATISTICS_AutoSetValue(STATISTICS_SENSORS_E type, float value);
void STATISTICS_SetValue(STATISTICS_SENSORS_E type, float min, float max, double sum);
STATISTICS_SENSORS_T STATISTICS_GetValue(STATISTICS_SENSORS_E type);
void STATISTICS_Store(void);

#endif
