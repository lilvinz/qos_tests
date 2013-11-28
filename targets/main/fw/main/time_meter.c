#include "stdheader.h"
#include "time_meter.h"

#include "clock.h"
#include "fram_mirror.h"
#include "meshcom.h"

static bool b_wait_counter_set;
static uint32_t time_meter_counter_system;
static uint32_t time_meter_counter_network;
static uint32_t time_meter_counter_led[DIM_LED_COUNT];
static uint32_t time_meter_counter_led_compensated[DIM_LED_COUNT];
static uint32_t led_ms_ctr[DIM_LED_COUNT];
static uint32_t led_ms_ctr_compensated[DIM_LED_COUNT];
static uint32_t value_to_set;
static TIME_METER_COUNTER_T timer_to_set;


/**
 * @brief  Initialize module time_meter.
 * @param  none
 * @retval none
 */
void TIME_METER_Init(void)
{
    FRAM_MIRROR_Read32(FRAM_TIME_METER_SYSTEM, &time_meter_counter_system);
    FRAM_MIRROR_Read32(FRAM_TIME_METER_NETWORK, &time_meter_counter_network);

    uint16_t address = FRAM_TIME_METER_LED_WHITE;

    for (DIM_LED_T idx = 0; idx < DIM_LED_COUNT; idx++)
    {
        FRAM_MIRROR_Read32(address, &time_meter_counter_led[idx]);
        address += 4;
        led_ms_ctr[idx] = 0;
    }

    address = FRAM_TIME_METER_COMPENSATED_LED_WHITE;

    for (DIM_LED_T idx = 0; idx < DIM_LED_COUNT; idx++)
    {
        FRAM_MIRROR_Read32(address, &time_meter_counter_led_compensated[idx]);
        address += 4;
        led_ms_ctr_compensated[idx] = 0;
    }

    timer_to_set = TIME_METER_COUNTER_LED_NONE;
}

/**
 * @brief  Collect operating time.
 * @param  none
 * @retval none
 */
void TIME_METER_Tick(void)
{
    time_meter_counter_system++;  // increment counter for system operating time
    if  (_meshcom_node_status.nodeStatus == MESHCOM_NODE_STATUS_IN_NETWORK_STATE)
        time_meter_counter_network++;  // increment counter for network operating time

    for (DIM_LED_T idx = 0; idx < DIM_LED_COUNT; idx++)
    {
        // simple counter
        led_ms_ctr[idx] += DIM_GetLedOperatingTime(idx);
        time_meter_counter_led[idx] += led_ms_ctr[idx] / 1000;
        led_ms_ctr[idx] = led_ms_ctr[idx] % 1000;
    }

    for (DIM_LED_T idx = 0; idx < DIM_LED_COUNT; idx++)
    {
        // compensated counter
        led_ms_ctr_compensated[idx] += DIM_GetCompensatedLedOperatingTime(idx);
        time_meter_counter_led_compensated[idx] += led_ms_ctr_compensated[idx] / 1000;
        led_ms_ctr_compensated[idx] = led_ms_ctr_compensated[idx] % 1000;
    }

    switch (timer_to_set)
    {
    case TIME_METER_COUNTER_LED_WHITE:
    case TIME_METER_COUNTER_LED_RED_1:
    case TIME_METER_COUNTER_LED_RED_2:
    case TIME_METER_COUNTER_LED_RED_3:
    case TIME_METER_COUNTER_LED_RED_4:
    case TIME_METER_COUNTER_LED_RED_C:
        time_meter_counter_led[timer_to_set - 1] = value_to_set;
        break;
    case TIME_METER_COUNTER_LED_WHITE_COMP:
    case TIME_METER_COUNTER_LED_RED_1_COMP:
    case TIME_METER_COUNTER_LED_RED_2_COMP:
    case TIME_METER_COUNTER_LED_RED_3_COMP:
    case TIME_METER_COUNTER_LED_RED_4_COMP:
    case TIME_METER_COUNTER_LED_RED_C_COMP:
        time_meter_counter_led_compensated[timer_to_set - DIM_LED_COUNT - 1] = value_to_set;
        break;
    case TIME_METER_COUNTER_SYSTEM:
        time_meter_counter_system = value_to_set;
        break;
    case TIME_METER_COUNTER_NETWORK:
        time_meter_counter_network = value_to_set;
        break;
    case TIME_METER_COUNTER_LED_NONE:
    default:
        break;
    }

    FRAM_MIRROR_Write32(FRAM_TIME_METER_SYSTEM, &time_meter_counter_system);
    FRAM_MIRROR_Write32(FRAM_TIME_METER_NETWORK, &time_meter_counter_network);

    uint16_t address = FRAM_TIME_METER_LED_WHITE;
    for (DIM_LED_T idx = 0; idx < DIM_LED_COUNT; idx++)
    {
        FRAM_MIRROR_Write32(address, &time_meter_counter_led[idx]);
        address += 4;
    }

    address = FRAM_TIME_METER_COMPENSATED_LED_WHITE;
    for (DIM_LED_T idx = 0; idx < DIM_LED_COUNT; idx++)
    {
        FRAM_MIRROR_Write32(address, &time_meter_counter_led_compensated[idx]);
        address += 4;
    }

    if (timer_to_set != TIME_METER_COUNTER_LED_NONE)
    {
        timer_to_set = TIME_METER_COUNTER_LED_NONE;
        b_wait_counter_set = FALSE;
    }
}

/**
 * @brief  Get operating time of the system.
 * @param  none
 * @retval system operating time in s
 */
uint32_t TIME_METER_GetSystemOperatingTime(void)
{
    return time_meter_counter_system;
}

/**
 * @brief  Get operating time of the network.
 * @param  none
 * @retval network time in s
 */
uint32_t TIME_METER_GetNetworkOperatingTime(void)
{
    return time_meter_counter_network;
}

/**
 * @brief  Get operating time of selected led.
 * @param  led
 * @retval system operating time in s
 */
uint32_t TIME_METER_GetLedOverallOperatingTime(DIM_LED_T led)
{
    if (led >= DIM_LED_COUNT)
        return 0;

    return time_meter_counter_led[led];
}

/**
 * @brief  Get operating time of selected led.
 * @param  led
 * @retval system operating time in s
 */
uint32_t TIME_METER_GetLedCompensatedOverallOperatingTime(DIM_LED_T led)
{
    if (led >= DIM_LED_COUNT)
        return 0;

    return time_meter_counter_led_compensated[led];
}

/**
 * @brief  Get operating time of selected led.
 * @param  timer
 * @param  new value
 * @retval system operating time in s
 */
bool TIME_METER_SetOperatingTime(TIME_METER_COUNTER_T timer, uint32_t value)
{
    if (timer >= TIME_METER_COUNTER_COUNT)
        return FALSE;

    b_wait_counter_set = TRUE;
    value_to_set = value;
    timer_to_set = timer;

    while (b_wait_counter_set)
        vTaskDelay(10);

    return TRUE;
}
