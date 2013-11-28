#include "stdheader.h"
#include "factorytest.h"


#include "printf-stdarg.h"

#include "rtc.h"
#include "clock.h"

#include "cli.h"
#include "cli_commands.h"
#include "state.h"
#include "version.h"
#include "system.h"

#include "log.h"
#include "sensors.h"
#include "gloom.h"
#include "adc.h"
#include "aguard.h"
#include "gpssync.h"
#include "svinfo.h"
#include "detector.h"

#include "indication.h"
#include "dim.h"
#include "sequence.h"



static const char screen_factorytest_1[] = "Memory:\r\n";
static const char screen_factorytest_2[] = "   External FRAM\r\n";
static const char screen_factorytest_3[] = "   External Flash\r\n";
static const char screen_factorytest_4[] = "\r\nLED Stage:\r\n";
static const char screen_factorytest_5[] = "   LED  voltage  state\r\n";
static const char screen_factorytest_6[] = "\r\nSensors:\r\n   Supply Volt\r\n   Fault Detect\r\n";
static const char screen_factorytest_7[] = "   Temperature    Main    LED 1   LED 2   LED 3   LED 4\r\n\r\n";
static const char screen_factorytest_8[] = "   Light\r\n\r\n";
static const char screen_factorytest_9[] = "Indicator:\r\n   Relais state\r\n   SVInfo state\r\n\r\n";
static const char screen_factorytest_10[] = "GPS:\r\n   GPS Time and Date\r\n   GPS Time Status\r\n   Satellites in fix\r\n";
static const char screen_factorytest_11[] = "   Satellite ID List\r\n   Position Fixtype\r\n   Position Latitude\r\n   Position Longitude\r\n   Position Altitude\r\n\r\n";
static const char screen_factorytest_12[] = "   Ch  SVID  State     C/No\r\n";
static const char screen_factorytest_13[] = "\r\nLOG:\r\n   Total number of log entries:\r\n";
static const char screen_factorytest_14[] = "   Last Log Entries:\r\n";

static uint8_t relais_out;

void FACTORYTEST_Init(void)
{
    relais_out = 0;
    printf("%s", screen_factorytest_1);
    printf("%s", screen_factorytest_2);
    printf("%s", screen_factorytest_3);
    printf("%s", screen_factorytest_4);
    printf("%s", screen_factorytest_5);
    for (ADC_LED_CH_INDEX channel = ADC_LED_CH_INDEX_FIRST; channel <= ADC_LED_CH_INDEX_LAST; channel++)
    {
        printf(ESC_CLR_NL);
    }
    printf("%s", screen_factorytest_6);
    printf("%s", screen_factorytest_7);
    printf("%s", screen_factorytest_8);
    printf("%s", screen_factorytest_9);
    printf("%s", screen_factorytest_10);
    printf("%s", screen_factorytest_11);
    printf("%s", screen_factorytest_12);
    VT_DOWN(12);
    printf("%s", screen_factorytest_13);
    printf("%s", screen_factorytest_14);
}

void FACTORYTEST_ActivateLed(FACTORYTEST_LED_T selected_led)
{
    DIM_LED_POWERLEVEL powerlevel;
    //uint16_t uTimeout;

    switch (selected_led)
    {
    case FACTORYTEST_LED_NONE:
        powerlevel[DIM_LED_RED_1] = 0;
        powerlevel[DIM_LED_RED_2] = 0;
        powerlevel[DIM_LED_RED_3] = 0;
        powerlevel[DIM_LED_RED_4] = 0;
        powerlevel[DIM_LED_RED_C] = 0;
        powerlevel[DIM_LED_WHITE] = 0;
        break;
    case FACTORYTEST_LED_WHITE:
        powerlevel[DIM_LED_RED_1] = 0;
        powerlevel[DIM_LED_RED_2] = 0;
        powerlevel[DIM_LED_RED_3] = 0;
        powerlevel[DIM_LED_RED_4] = 0;
        powerlevel[DIM_LED_RED_C] = 0;
        powerlevel[DIM_LED_WHITE] = FACTORYTEST_LEDPOWER_WHITE;
        break;
    case FACTORYTEST_LED_RED:
        powerlevel[DIM_LED_RED_1] = FACTORYTEST_LEDPOWER_RED;
        powerlevel[DIM_LED_RED_2] = FACTORYTEST_LEDPOWER_RED;
        powerlevel[DIM_LED_RED_3] = FACTORYTEST_LEDPOWER_RED;
        powerlevel[DIM_LED_RED_4] = FACTORYTEST_LEDPOWER_RED;
        powerlevel[DIM_LED_RED_C] = FACTORYTEST_LEDPOWER_RED;
        powerlevel[DIM_LED_WHITE] = 0;
        break;
    default:
        powerlevel[DIM_LED_RED_1] = 0;
        powerlevel[DIM_LED_RED_2] = 0;
        powerlevel[DIM_LED_RED_3] = 0;
        powerlevel[DIM_LED_RED_4] = 0;
        powerlevel[DIM_LED_RED_C] = 0;
        powerlevel[DIM_LED_WHITE] = 0;
        break;
    }
    DIM_SetNow(&powerlevel, FALSE, FALSE);
    SEQUENCE_FlatOn(FACTORYTEST_FLATON_TIMEOUT);
}

bool FACTORYTEST_Show(void)
{
    uint8_t num_entries;
    uint8_t num_entries_to_show;
    uint32_t uLogEntry = 0;
    uint32_t i_log_last_entry_index;
    float gloom_ema;
    RTCTM datetime;
    LOG_ENTRY_T log_entry;
    TEMPERATURE_SENSOR_T temperature;
//    AGUARD_EXTREMES_T aguard_extremes;
    GLOOM_SENSOR_T gloom_sensor_data;

    VT_CURSOR_HOME;

    //////// LED Stage ////////
    VT_DOWN(14);
    uint16_t led_stage_fail_bm;
    led_stage_fail_bm = DETECTOR_GetLedStageFailBitmask();

    for (ADC_LED_CH_INDEX channel = ADC_LED_CH_INDEX_FIRST; channel <= ADC_LED_CH_INDEX_LAST; channel++)
    {
        uint32_t temp = (uint32_t) _primestruct.sensors.analog.led[channel].value;

        VT_RIGHT(3);
        printf("%2u  ", channel);
        if (led_stage_fail_bm & (1 << channel))
        {
            VT_INVERSE;
            printf("  %2u.%03u ", (uint16_t)(temp / 1000), (uint16_t)(temp % 1000));
            printf("  FAIL");
            VT_ATTRIBUTE_OFF;
        }
        else
        {
            printf("  %2u.%03u ", (uint16_t)(temp / 1000), (uint16_t)(temp % 1000));
            printf("   OK");
        }

        printf(ESC_CLR_NL);
    }

    //////// Sensors ////////
    VT_DOWN(2);
    VT_RIGHT(18);

    printf("%2u.%03u", ADC_CURRENT_PWR24V_VALUE / 1000, ADC_CURRENT_PWR24V_VALUE % 1000);
    printf(ESC_CLR_NL);
//    aguard_extremes = AGUARD_GetExtremes();
    /*    printf("%2d.%03dV (min: %2d.%03dV max: %2d.%03dV avrg: %2d.%03dV)"ESC_CLR_NL, ADC_CURRENT_PWR24V_VALUE / 1000, ADC_CURRENT_PWR24V_VALUE % 1000,
                (uint16_t)(aguard_extremes.pwr24v.min * ADC_COUNT_TO_PWR24V_FACTOR) / 1000, (uint16_t)(aguard_extremes.pwr24v.min * ADC_COUNT_TO_PWR24V_FACTOR) % 1000,
                (uint16_t)(aguard_extremes.pwr24v.max * ADC_COUNT_TO_PWR24V_FACTOR) / 1000, (uint16_t)(aguard_extremes.pwr24v.max * ADC_COUNT_TO_PWR24V_FACTOR) % 1000,
                (uint16_t)(aguard_extremes.pwr24v.average * ADC_COUNT_TO_PWR24V_FACTOR) / 1000, (uint16_t)(aguard_extremes.pwr24v.average * ADC_COUNT_TO_PWR24V_FACTOR) % 1000
                );*/

    VT_RIGHT(18);

    if (detector_fault_bm == 0)
        printf("All OK!"ESC_CLR_NL);
    else
    {
        VT_BOLD;
        if (detector_fault_bm & DETECTOR_PM_FAULT_BM)       printf("VCC ");
        if (detector_fault_bm & DETECTOR_FAULT_BM_GPS_VCC)  printf("GPS VCC ");
        if (detector_fault_bm & DETECTOR_FAULT_BM_GPS_RF)   printf("GPS RF ");
        if (detector_fault_bm & DETECTOR_FAULT_BM_LUX_2_8_V)    printf("LUX 2.8V ");
        VT_ATTRIBUTE_OFF;
        printf(ESC_CLR_NL);
    }

    VT_DOWN(1);
    VT_RIGHT(18);
    for (TEMPERATURE_T idx = 0; idx < TEMPERATURE_COUNT; idx++)
    {
        SENSORS_GetTemperature(idx, &temperature);
        if (temperature.state == SENSOR_OK)
            printf("%3d.%03d ", (int)temperature.value, (int)(temperature.value * 1000.0) % 1000);
        else
            printf(" <FAIL> ");
    }

    printf(ESC_CLR_NL);
    VT_RIGHT(18);
    SENSORS_GetGloom(&gloom_sensor_data);
    gloom_ema = SENSORS_GetGloomEMA();
    switch (gloom_sensor_data.state)
    {
    case SENSOR_WAIT_FOR_INIT:
        printf("<INIT>"ESC_CLR_NL);
        break;
    case SENSOR_OK:
        printf("%5u.%ulx   EMA: %5u.%ulx"ESC_CLR_NL, (int)gloom_sensor_data.value, (int)(gloom_sensor_data.value * 10.0) % 10, (int)gloom_ema, (int)(gloom_ema * 10.0) % 10);
        break;
    case SENSOR_UPPER_LIMIT:
        printf("<RANGE UPPER LIMIT>"ESC_CLR_NL);
        break;
    case SENSOR_FAILED:
    default:
        printf("<FAILED>"ESC_CLR_NL);
        break;
    }

    //////// Indicator ////////
    VT_DOWN(2);
    VT_RIGHT(16);
    if (relais_out < FACTORYTEST_RELAIS_CYCLE_MAX)
        relais_out++;

    if (relais_out & 0x01)
    {
        INDICATION_OVERRIDE_Set(IND_A, TRUE, FACTORYTEST_RELAIS_CYCLE_TIMEOUT);
        printf("<A>");
    }
    else
    {
        INDICATION_OVERRIDE_Set(IND_A, FALSE, FACTORYTEST_RELAIS_CYCLE_TIMEOUT);
        printf("   ");
    }

    if (relais_out & 0x02)
    {
        INDICATION_OVERRIDE_Set(IND_B, TRUE, FACTORYTEST_RELAIS_CYCLE_TIMEOUT);
        printf("<B>");
    }
    else
        INDICATION_OVERRIDE_Set(IND_B, FALSE, FACTORYTEST_RELAIS_CYCLE_TIMEOUT);

    printf(ESC_CLR_NL);

    VT_RIGHT(16);

    switch (_svinfo_signal)
    {
    case SVINFO_LOW:
        printf("<LOW>"ESC_CLR_NL);
        break;
    case SVINFO_HIGH:
        printf("<HIGH>"ESC_CLR_NL);
        break;
    case SVINFO_FREQ_1:
        printf("<FREQ 1>"ESC_CLR_NL);
        break;
    case SVINFO_FREQ_2:
        printf("<FREQ 2>"ESC_CLR_NL);
        break;
    case SVINFO_FREQ_3:
        printf("<FREQ 3>"ESC_CLR_NL);
        break;
    case SVINFO_FREQ_4:
        printf("<FREQ 4>"ESC_CLR_NL);
        break;
    case SVINFO_FREQ_5:
        printf("<FREQ 5>"ESC_CLR_NL);
        break;
    case SVINFO_FREQ_6:
        printf("<FREQ 6>"ESC_CLR_NL);
        break;
    case SVINFO_INVALID:
    default:
        printf("<INVALID>"ESC_CLR_NL);
        break;
    }

    //////// GPS ////////
    VT_DOWN(2);
    VT_RIGHT(23);
    printf("%02u.%02u.%04u %02u:%02u:%02u"ESC_CLR_NL, _gpssync.datetime.day, _gpssync.datetime.month, _gpssync.datetime.year,
           _gpssync.datetime.hour, _gpssync.datetime.minute, _gpssync.datetime.second);
    VT_RIGHT(23);
    switch (_gpssync.datetime.status)
    {
    case GPSSYNC_DATETIME_INVALID:
        printf("<INVALID>");
        break;
    case GPSSYNC_DATETIME_VALID:
        printf("<VALID>");
        break;
    case GPSSYNC_DATETIME_CONFIRMED:
        printf("<CONFIRMED>");
        break;
    default:
        break;
    }
    printf(ESC_CLR_NL);
    VT_RIGHT(23);
    printf("%u"ESC_CLR_NL, _gpssync.position.num_of_sv_in_fix);
    VT_RIGHT(23);
    for (uint8_t ibit = 0; ibit < 32; ibit++)
    {
        if ((ibit) && ((ibit % 8) == 0)) printf(" ");
        if (_gpssync.position.sat_id_list & (1L << ibit)) printf("X");
        else printf("-");
    }
    printf(ESC_CLR_NL);

    VT_RIGHT(23);
    switch (_gpssync.position.nav_type)
    {
    case GEONAVDATA_NAV_TYPE_FIXTYPE_NONAVFIX:
        printf("no navigation fix");
        break;
    case GEONAVDATA_NAV_TYPE_FIXTYPE_2D:
        printf("2D solution");
        break;
    case GEONAVDATA_NAV_TYPE_FIXTYPE_3D:
        printf("3D solution");
        break;
    default:
        break;
    }
    printf(ESC_CLR_NL);

    if (_gpssync.position.nav_type >= GEONAVDATA_NAV_TYPE_FIXTYPE_2D)
    {
        VT_RIGHT(23);
        printf("%d.%ddeg"ESC_CLR_NL, (int8_t)(_gpssync.position.latitude / GPS_DEGREE_FACTOR), (int32_t)(_gpssync.position.latitude % GPS_DEGREE_FACTOR));
        VT_RIGHT(23);
        printf("%d.%ddeg"ESC_CLR_NL, (int8_t)(_gpssync.position.longitude / GPS_DEGREE_FACTOR), (int32_t)(_gpssync.position.longitude % GPS_DEGREE_FACTOR));
    }
    else
    {
        VT_RIGHT(23);
        printf("%d.%ddeg <outdated>"ESC_CLR_NL, (int8_t)(_gpssync.position.latitude / GPS_DEGREE_FACTOR), (int32_t)(_gpssync.position.latitude % GPS_DEGREE_FACTOR));
        VT_RIGHT(23);
        printf("%d.%ddeg <outdated>"ESC_CLR_NL, (int8_t)(_gpssync.position.longitude / GPS_DEGREE_FACTOR), (int32_t)(_gpssync.position.longitude % GPS_DEGREE_FACTOR));
    }

    VT_RIGHT(23);
    if (_gpssync.position.nav_type >= GEONAVDATA_NAV_TYPE_FIXTYPE_3D)
        printf("%d.%dm AMSL"ESC_CLR_NL, (int32_t)(_gpssync.position.altitude / 100), (int8_t)(abs(_gpssync.position.altitude) % 100));
    else
        printf("%d.%dm AMSL <outdated>"ESC_CLR_NL, (int32_t)(_gpssync.position.altitude / 100), (int8_t)(abs(_gpssync.position.altitude) % 100));
    printf(ESC_CLR_NL);

    VT_DOWN(1);
    for (uint8_t i = 0; i < GPSSYNC_GPS_MAX_NO_OF_CH; i++)
    {
        printf("   %02u", i + 1);
        if (_gpssync.svmeasurements[i].state > GPSSYNC_MEASUREMENTS_STATE_LOST)
        {
            printf("   %02u", _gpssync.svmeasurements[i].svid);

            if (_gpssync.svmeasurements[i].state == GPSSYNC_MEASUREMENTS_STATE_LOCKED)
                printf("   LOCKED  ");
            else
                printf("   tracking");

            printf("  %02u  [", _gpssync.svmeasurements[i].cno);
            for (uint8_t sig = 0; sig < (_gpssync.svmeasurements[i].cno / 4); sig++)
                printf("#");
            printf(ESC_CLR"\033[%uC]", 25 - _gpssync.svmeasurements[i].cno / 4);
        }
        printf(ESC_CLR_NL);
    }

    //////// LiveLog ////////
    VT_DOWN(2);
    VT_RIGHT(32);
    printf("%u", LOG_GetLogSize());
    printf(ESC_CLR_NL);

    VT_DOWN(1);
    // Setup LiveLog view
    i_log_last_entry_index = LOG_GetNextEntryIndex() - 1;

    if (i_log_last_entry_index > FACTORYTEST_NUM_LOG_ENTRIES)
        num_entries_to_show = FACTORYTEST_NUM_LOG_ENTRIES;
    else
        num_entries_to_show = i_log_last_entry_index + 1;

    while(num_entries_to_show)
    {
        uLogEntry = i_log_last_entry_index - num_entries_to_show + 1;
        num_entries = 1;
        if (LOG_GetLogEntry(&uLogEntry, &log_entry, &num_entries, TRUE))
        {
            RTC_LocalTime(&datetime, (RTCTIME*) &log_entry.timestamp);
            printf("   %5u %02u.%02u.%02u %02u:%02u:%02u %s (%u) (%u) (%u)"ESC_CLR_NL,
                   uLogEntry + 1,
                   datetime.tm_mday, datetime.tm_mon + 1, datetime.tm_year % 100,
                   datetime.tm_hour, datetime.tm_min, datetime.tm_sec,
                   getLocalText_log_texts(log_entry.event_id, en),
                   log_entry.argument_1,
                   log_entry.argument_2,
                   log_entry.argument_3);
        }
        else
            printf("Entry not available!"ESC_CLR_NL);
        num_entries_to_show--;
    }

    if (cli_check_abort())
        return FALSE;
    else
        return TRUE;
}

bool FACTORYTEST_Run(void)
{
    FACTORYTEST_LED_T led = FACTORYTEST_LED_NONE;
    CLOCK_MS_T next_led_change = 0;
    CLOCK_MS_T next_show = 0;

    FACTORYTEST_Init();

    SEQUENCE_TestDriveStartBlocking(FACTORYTEST_TEST_LED_INTERVAL);

    while(!cli_check_abort())
    {
        if (CLOCK_GetMillitimestamp() > next_led_change)
        {
            if (led != FACTORYTEST_LED_WHITE)
                led = FACTORYTEST_LED_WHITE;
            else
                led = FACTORYTEST_LED_RED;

            FACTORYTEST_ActivateLed(led);
            next_led_change = CLOCK_GetMillitimestamp() + FACTORYTEST_CHANGE_LED_INTERVAL;
        }

        if (CLOCK_GetMillitimestamp() > next_show)
        {
            FACTORYTEST_Show();
            next_show = CLOCK_GetMillitimestamp() + FACTORYTEST_SHOW_INTERVAL;
        }
    }

    DIM_Reset();

    return 0;
}
