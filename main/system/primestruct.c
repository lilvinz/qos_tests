#include "primestruct.h"


PRIMESTRUCT_T _primestruct;

/**
  * @brief  Initializes primestruct.
  * @param  None
  * @retval None
  */
void PRIMESTRUCT_Init(void)
{
    for (uint32_t i = 0; i < PS_ANALOG_LED_COUNT; i++)
    {
        _primestruct.sensors.analog.led[i].value = PRIMESTRUCT_INIT_VOLTAGE_VALUE;
        _primestruct.sensors.analog.led[i].status = SENSOR_WAIT_FOR_INIT;
    }
    for (uint32_t i = 0; i < ADC_LED_CH_INDEX_COUNT; i++)
    {
        _primestruct.sensors.analog.local_led_voltages_min_max[i].min = PS_MINIMUM_RESET_VALUE_1000_VOLT;
        _primestruct.sensors.analog.local_led_voltages_min_max[i].max = 0;
        _primestruct.sensors.analog.local_led_voltages_min_max[i].average = 0;
    }
    for (uint32_t i = 0; i < PS_ANALOG_VOLTAGE_COUNT; i++)
    {
        _primestruct.sensors.analog.supply[i].value = PRIMESTRUCT_INIT_VOLTAGE_VALUE;
        _primestruct.sensors.analog.supply[i].status = SENSOR_WAIT_FOR_INIT;
    }
    for (uint32_t i = 0; i < PS_LED_TEMPERATURE_COUNT; i++)
    {
        _primestruct.sensors.led_temperature[i].value = PRIMESTRUCT_INIT_TEMPERATURE_VALUE;
        _primestruct.sensors.led_temperature[i].status = SENSOR_WAIT_FOR_INIT;
    }
    for (uint32_t i = 0; i < PS_BOARD_TEMPERATURE_COUNT; i++)
    {
        _primestruct.sensors.board_temperature[i].value = PRIMESTRUCT_INIT_TEMPERATURE_VALUE;
        _primestruct.sensors.board_temperature[i].status = SENSOR_WAIT_FOR_INIT;
    }
    for (uint32_t i = 0; i < PS_LED_OVERVOLTAGE_BM_COUNT; i++)
    {
        _primestruct.flags.led_overvoltage_bm[i] = 0;
        _primestruct.flags.led_voltage_ok_bm[i] = 0;
    }
}
