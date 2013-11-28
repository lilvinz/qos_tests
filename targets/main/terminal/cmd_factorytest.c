/*
 * cmd_factorytest.c
 *
 *  Created on: 22.11.2011
 *      Author: cba
 */

#include "stdheader.h"
#include "config.h"
#include "freegetopt.h"
#include "vt102.h"
#include "hw_configuration.h"
#include "cli.h"
#include "printf-stdarg.h"

#include "system.h"
#include "version.h"
#include "indication.h"
#include "mesh_testmode.h"
#include "sequence.h"
#include "gpssync.h"

#include "sensor.h"
#include "sensors.h"
#include "gloom.h"
#include "adc.h"
#include "svinfo.h"

#include "primestruct.h"

#include "cmd_factorytest.h"


/************** cli specific definitions ***********/

const char CMD_CommandString_Factorytest[] = "factorytest";
const char CMD_HelpString_Factorytest[]
= "factorytest <options>     - Engage factory tests";
const char CMD_HelpLongString_Factorytest[]
= "factorytest <options>\n"\
  "Engage factory tests\n"\
  "   Possible options:\n"\
  "   -a    Perform all tests\n"\
  "   -s    Sensor test\n"\
  "   -i    Indication test\n"\
  "   -l    LED test\n"\
  "   -c    Radio HF carrier test\n"\
  "   -g    GPS test\n";


/************** local variables **************/

/************** local functions **************/

/************** local definitions **************/
#define TEST_SENSORS    0x0001
#define TEST_INDICATION 0x0002
#define TEST_LED_RINGS  0x0004
#define TEST_GPS        0x0010
#define TEST_MESH     0x0020
#define TEST_HF_CARRIER 0x0040

#define TEST_ALL        0xFFFF

const CMD_FACTORYTEST_INDICATION_TEST_TABLE_ENTRY_T indication_test_table[CMD_FACTORYTEST_INDICATION_TEST_TABLE_NUM_ENTRIES] =
{
    //  text                    state relais a  state relais b  expected result
    {"\n 1->on  2=off  |   ",   TRUE,           FALSE,          TRUE},   // measure time to open normally closed contact of relais a
    {"\n 1->off 2=off  |   ",   FALSE,          FALSE,          FALSE},  // measure time to close normally closed contact of relais a
    {"\n 1=off  2->on  |   ",   FALSE,          TRUE,           TRUE},   // measure time to open normally closed contact of relais b
    {"\n 1->on  2=on   |   ",   TRUE,           TRUE,           FALSE},  // measure time to close normally open contact of relais a
    {"\n 1=on   2->off |   ",   TRUE,           FALSE,          TRUE},   // measure time to open normally open contact of relais b
    {"\n 1=on   2->on  |   ",   TRUE,           TRUE,           FALSE},  // measure time to close normally open contact of relais b
    {"\n 1->off 2=on   |   ",   FALSE,          TRUE,           TRUE},   // measure time to open normally open contact of relais a
    {"\n 1=off  2->off |   ",   FALSE,          FALSE,          FALSE},  // measure time to close normally closed contact of relais b
};


void testdelay(uint16_t ms)
{
    vTaskDelay(ms);
}


// ******** TEST INDICATION AND SVINFO *********

int16_t measurerelaisswitchdelay(bool expectedstate)
{
#define RELAISSWITCHDELAY_TIMEOUT 1000

    uint16_t    uTime = 0;

    do
    {
        if (expectedstate == SVINFO_STATE())
        {
            printf("%u", uTime);
            return uTime;
        }
        testdelay(1);
        uTime++;
    }
    while (uTime < RELAISSWITCHDELAY_TIMEOUT);

    printf("timeout!");
    return -1;
}

bool test_indication(void)
{
    printf("\n");
    printf("      Relais 1         Relais 2\n");
    printf("           o-------------o\n");
    printf("\n");
    printf(" +24V----o                 o----SVInfo\n");
    printf("          \\               /\n");
    printf("           o-------------o\n");

    printf("\nRelais         |Delay [ms]");
    printf("\n---------------|---------------");

    SVINFO_Deinit();

    INDICATION_OVERRIDE_Set(IND_A, FALSE, 30);
    INDICATION_OVERRIDE_Set(IND_B, FALSE, 30);
    testdelay(500);

    bool b_result = true;

    for (uint8_t i = 0; ((i < CMD_FACTORYTEST_INDICATION_TEST_TABLE_NUM_ENTRIES) && b_result); i++)
    {
        printf(indication_test_table[i].text);
        INDICATION_OVERRIDE_Set(IND_A, indication_test_table[i].state_relais_a, 30);
        INDICATION_OVERRIDE_Set(IND_B, indication_test_table[i].state_relais_b, 30);
        if (measurerelaisswitchdelay(indication_test_table[i].expected_result) == -1)
            b_result = FALSE;
        testdelay(500);
    }

    SVINFO_Init();

    return b_result;
}


// ******** TEST SENSORS *********

bool ft_print_float(float value, float average, uint8_t tolerance)
{
    bool bRet = FALSE;

    if ((value < average * (1.0f - ((float)tolerance / 100))) || (value > average * (1.0f + ((float)tolerance / 100))))
        printf(ESC_BGCOLOR_RED ESC_COLOR_BLACK);
    else
    {
        printf(ESC_BGCOLOR_GREEN ESC_COLOR_BLACK);
        bRet = TRUE;
    }

    printf(" %4d.%03d "ESC_ATTRIBUTE_OFF" ", (int)value, (int)(value * 1000.0f) % 1000);

    return bRet;
}

bool test_sensors(void)
{
#define FT_SENSORS_TEMPERATURE_TOLERANCE_PERCENT    10      // percent
#define FT_SENSORS_24V_TOLERANCE_PERCENT            10      // percent
#define FT_SENSORS_LED_TOLERANCE_PERCENT            15      // percent
#define FT_SENSORS_LED_FET_VOLTAGE      2       // Volt ToDo: check for multi and medium_white
#define FT_SENSORS_LED_WHITE_VOLTAGE    43      // Volt
#define FT_FLATON_TIMEOUT               10      // seconds
#define FT_LEDPOWER_WHITE               10000   // PWM Value
#define FT_LEDPOWER_RED                 10000   // PWM Value

    TEMPERATURE_SENSOR_T temperature;
    GLOOM_SENSOR_T gloom;

    uint8_t valuecount = 0;
    float valuesum = 0;
    uint32_t errcount = 0;

    printf("\nMaximum Sensor Tolerance: %u percent\n\n", FT_SENSORS_TEMPERATURE_TOLERANCE_PERCENT);

    printf("Temperatur sensors comparison\n");
    printf("         Tmain      Tled1      Tled2      Tled3      Tled4\n         ");


    for (uint8_t idx = 0; idx < TEMPERATURE_COUNT; idx++)
    {
        SENSORS_GetTemperature(idx, &temperature);
        if (temperature.state == SENSOR_OK)
        {
            valuesum += temperature.value;
            valuecount++;
        }
    }

    for (uint8_t idx = 0; idx < TEMPERATURE_COUNT; idx++)
    {
        SENSORS_GetTemperature(idx, &temperature);
        if (temperature.state == SENSOR_OK)
        {
            if (!ft_print_float(temperature.value, valuesum / valuecount, FT_SENSORS_TEMPERATURE_TOLERANCE_PERCENT))
                errcount++;
        }
        else
        {
            printf(ESC_BGCOLOR_RED ESC_COLOR_BLACK"  <FAIL>  "ESC_ATTRIBUTE_OFF" ");
            errcount++;
        }
    }

    printf("\n\n\nGloom sensor value change test\n");
    printf("Challenge: You have to let gloom double or half its value!\n");

    SENSORS_GetGloom(&gloom);
    valuesum = gloom.value;
    printf("\nStart Value: %5u.%u\n", (int)valuesum, (int)(valuesum * 10.0f) % 10);
    printf("\nCurrent Value: ");
    while(1)
    {
        SENSORS_GetGloom(&gloom);
        switch (gloom.state)
        {
        case SENSOR_WAIT_FOR_INIT:
            printf(ESC_BGCOLOR_RED ESC_COLOR_BLACK"  <INIT> "ESC_ATTRIBUTE_OFF);
            break;
        case SENSOR_OK:
            printf(ESC_BGCOLOR_GREEN ESC_COLOR_BLACK" %5u.%u "ESC_ATTRIBUTE_OFF, (int)gloom.value, (int)(gloom.value * 10.0f) % 10);
            break;
        case SENSOR_LOWER_LIMIT:
        case SENSOR_UPPER_LIMIT:
            printf(ESC_BGCOLOR_RED ESC_COLOR_BLACK" <RANGE> "ESC_ATTRIBUTE_OFF);
            break;
        case SENSOR_FAILED:
        default:
            printf(ESC_BGCOLOR_RED ESC_COLOR_BLACK"  <FAIL> "ESC_ATTRIBUTE_OFF);
            break;
        }

        if (gloom.value < (valuesum / 2) || gloom.value > (valuesum * 2))
        {
            printf("\n\n  >> IMPRESSIVE! <<\n");
            break;
        }

        testdelay(1000);

        if (cli_check_abort())
        {
            printf("\nTest aborted!\n");
            return FALSE;
        }

        VT_LEFT(9);
    }

    {
        printf("\n\n*** WARNING ***\nLEDs will be switched on in 3 seconds...\n");
        testdelay(3000);

        printf("\n");

        printf("Maximum 24V Tolerance: %u percent\n", FT_SENSORS_24V_TOLERANCE_PERCENT);
        printf("Maximum LED-FET Tolerance: %u percent\n", FT_SENSORS_LED_TOLERANCE_PERCENT);

        printf("\n\nVoltage sensors\n");

        DIM_LED_POWERLEVEL powerlevel[DIM_LED_COUNT];

#ifdef HWP_LED_RED_UNI
        powerlevel[DIM_LED_RED_1] = FT_LEDPOWER_RED;
        powerlevel[DIM_LED_RED_2] = FT_LEDPOWER_RED;
        powerlevel[DIM_LED_RED_3] = FT_LEDPOWER_RED;
        powerlevel[DIM_LED_RED_4] = FT_LEDPOWER_RED;
#endif
#ifdef HWP_LED_RED_2K
        powerlevel[DIM_LED_RED_C] = FT_LEDPOWER_RED;
#endif
#ifdef HWP_LED_WHITE_20K
        powerlevel[DIM_LED_WHITE] = FT_LEDPOWER_WHITE;
#endif
        DIM_SetNow(powerlevel, FALSE, FALSE);
#ifdef HWP_LOCAL_LEDS
        SEQUENCE_FlatOn(FT_FLATON_TIMEOUT);
#endif

        // output values for 10 times and wait 1/10 of FLATON_TIMEOUT each
        for (uint8_t cnt = 0; cnt < 10; cnt++)
        {
            printf("24 V:");
            VT_COLUMN(10);
            ft_print_float(_primestruct.sensors.analog.supply[PS_ANALOG_VOLTAGE_MAIN_24V].value, 24.0f, FT_SENSORS_24V_TOLERANCE_PERCENT);
            printf("\n");

            for (uint8_t channel = ADC_LED_CH_INDEX_FIRST; channel <= ADC_LED_CH_INDEX_LAST; channel++)
            {
                printf("%s:", adc_channel_list[channel].name);
                VT_COLUMN(10);

                if (adc_channel_list[channel].led_color == LED_COLOR_WHITE)
                {
                    if (!ft_print_float(_primestruct.sensors.analog.led[channel].value, FT_SENSORS_LED_WHITE_VOLTAGE, FT_SENSORS_LED_TOLERANCE_PERCENT))
                        errcount++;
                }
                else if (adc_channel_list[channel].led_color == LED_COLOR_RED)
                {
                    if(!ft_print_float(_primestruct.sensors.analog.led[channel].value, FT_SENSORS_LED_FET_VOLTAGE, FT_SENSORS_LED_TOLERANCE_PERCENT))
                        errcount++;
                }
                printf("\n");
            }

            testdelay(FT_FLATON_TIMEOUT * 1000 / 10);
            VT_UP(ADC_LED_CH_INDEX_COUNT + 1);
        }

        VT_DOWN(ADC_LED_CH_INDEX_COUNT + 1);
    }

    if (errcount == 0)
        return TRUE;

    return FALSE;
}


// ******** TEST GPS *********

bool test_gps(void)
{
    uint8_t sat_locked = 0;
    uint8_t sat_tracking = 0;
    uint16_t runtime = 0;

#define GPS_MIN_NUM_LOCKED          3
#define GPS_WAIT_LOCK_TIMEOUT_S     600

    printf("Waiting for %u Satellites to be locked...\n\n", GPS_MIN_NUM_LOCKED);
    while (1)
    {
        sat_locked = 0;
        sat_tracking = 0;

        for (uint8_t i = 0; i < GPSSYNC_GPS_MAX_NO_OF_CH; i++)
        {
            printf("   %02u", i + 1);
            if (_gpssync.svmeasurements[i].state > GPSSYNC_MEASUREMENTS_STATE_LOST)
            {
                printf("   %02u", _gpssync.svmeasurements[i].svid);

                if (_gpssync.svmeasurements[i].state == GPSSYNC_MEASUREMENTS_STATE_LOCKED)
                {
                    printf("   LOCKED  ");
                    sat_locked++;
                }
                else if (_gpssync.svmeasurements[i].state == GPSSYNC_MEASUREMENTS_STATE_TRACKED)
                {
                    printf("   tracking");
                    sat_tracking++;
                }
                else
                    printf("   ???     ");

                printf("  %02u  [", _gpssync.svmeasurements[i].cno);
                for (uint8_t sig = 0; sig < (_gpssync.svmeasurements[i].cno / 4); sig++)
                    printf("#");
                printf(ESC_CLR"\033[%uC]", 25 - _gpssync.svmeasurements[i].cno / 4);
            }
            printf(ESC_CLR_NL);
        }

        printf (ESC_CLR_NL"Tracking: %u Locked: %u TimeTillFail: %u"ESC_CLR_NL, sat_tracking, sat_locked, GPS_WAIT_LOCK_TIMEOUT_S - runtime);

        if (sat_locked >= GPS_MIN_NUM_LOCKED)
            return TRUE;

        if (runtime >= GPS_WAIT_LOCK_TIMEOUT_S)
        {
            printf("\nNo %u locks after %u seconds\n", GPS_MIN_NUM_LOCKED, GPS_WAIT_LOCK_TIMEOUT_S);
            return FALSE;
        }

        testdelay(1000);
        runtime++;

        if (cli_check_abort())
        {
            printf("\nTest aborted!\n");
            return FALSE;
        }

        //move cursor up to beginning of output
        VT_UP(GPSSYNC_GPS_MAX_NO_OF_CH + 2);
    }

    return FALSE;
}


bool test_meshradio(void)
{
    uint8_t uAntenna = 0;
    uint8_t uTrim = 7;
    uint8_t uTxPower = 0;
    uint8_t uChannel = 11;
    uint8_t uPattern = 0x00;
    bool bEndTest = FALSE;

    if (MESH_TestmodeStart(MESH_TESTMODE_CCTM) == FALSE)
    {
        printf("\nERROR: HF Test Mode activation failed\nABORT\n");
        return FALSE;
    }

    MESH_TestmodeSetAntenna(uAntenna);
    MESH_TestmodeSetTrim(uTrim);
    MESH_TestmodeSetTxPower(uTxPower);
    MESH_TestmodeSetChannel(uChannel);
    MESH_TestmodeSetFrameBufferPattern(uPattern);

    printf("\n Interactive HF part test\n\n"\
           "Press key to change test parameter:\n"\
           "   [a]     Antenna\n"\
           "   [w]/[s] XTAL Trim\n"\
           "   [e]/[d] TX Power\n"\
           "   [r]/[f] Channel\n"\
           "   [t]     Frame buffer pattern (0x00 or 0xFF)\n"\
           "   [c]     Quit HF part test\n"\
           "\n\n");

    do
    {
        printf("\rAnt: %u   Trm: 0x%02x   Pwr: 0x%02x   Ch: %u   Frm: 0x%02x", uAntenna, uTrim, uTxPower, uChannel, uPattern);

        char c = CLI_GETCHAR();

        switch (c)
        {
        case 'a':
            uAntenna = uAntenna ? 0 : 1;
            if (MESH_TestmodeSetAntenna(uAntenna) == FALSE)
                uAntenna = uAntenna ? 0 : 1;
            break;
        case 'w':
            if (uTrim < 0x0f)
                if (MESH_TestmodeSetTrim(++uTrim) == FALSE)
                    --uTrim;
            break;
        case 's':
            if (uTrim > 0x00)
                if (MESH_TestmodeSetTrim(--uTrim) == FALSE)
                    ++uTrim;
            break;
        case 'e':
            if (uTxPower < 0x0f)
                if (MESH_TestmodeSetTxPower(++uTxPower) == FALSE)
                    --uTxPower;
            break;
        case 'd':
            if (uTxPower > 0x00)
                if (MESH_TestmodeSetTxPower(--uTxPower) == FALSE)
                    ++uTxPower;
            break;
        case 'r':
            if (uChannel < 26)
                if (MESH_TestmodeSetChannel(++uChannel) == FALSE)
                    --uTxPower;
            break;
        case 'f':
            if (uChannel > 11)
                if (MESH_TestmodeSetChannel(--uChannel) == FALSE)
                    ++uChannel;
            break;
        case 't':
            uPattern = uPattern ? 0x00 : 0xFF;
            if (MESH_TestmodeSetFrameBufferPattern(uPattern) == FALSE)
                uPattern = uPattern ? 0x00 : 0xFF;
            break;
        case 'c':
            bEndTest = TRUE;
            break;
        default:
            break;
        }
    } while (bEndTest == FALSE);

    printf("\n\nSaving setting to mesh module... ");
    if (MESH_TestmodeSaveSettings() == TRUE)
        printf("OK\n\n");
    else
        printf("FAILED\n\n");

    MESH_TestmodeStop();

    return TRUE;
}

void testresult(bool bResult)
{
    if (bResult)
        FT_PRINT_SUCCESS();
    else
        FT_PRINT_FAIL();
}

bool QuestionContinueRedo(void)
{
    char c;
    printf("[C]ontinue or [R]edo?");

    while(1)
    {
        c = CLI_GETCHAR();
        if (c == 'c' || c == 'C')
        {
            printf("   >>> Continue\n\n");
            return TRUE;
        }
        if (c == 'r' || c == 'R')
        {
            printf("   >>> Redo\n\n");
            return FALSE;
        }
    }

    return FALSE;
}

int CMDFactorytest(int argc, char * argv[])
{
    signed char c;
    uint16_t uTestMask = 0;

    while((c = freegetopt(argc, argv, "asilzcg")) != -1)
    {
        switch(c)
        {
        case 'a':
            uTestMask = TEST_ALL;
            break;
        case 's':
            uTestMask |= TEST_SENSORS;
            break;
        case 'i':
            uTestMask |= TEST_INDICATION;
            break;
        case 'l':
            uTestMask |= TEST_LED_RINGS;
            break;
        case 'c':
            uTestMask |= TEST_HF_CARRIER;
            break;
        case 'z':
            uTestMask |= TEST_MESH;
            break;
        case 'g':
            uTestMask |= TEST_GPS;
            break;
        case ':':
            printf("\nOption -%c needs operant\n", optopt);
            return -1;
        case '?':
            printf("\nUnknown option -%c\n", optopt);
            return -1;
        }
    }

    if (uTestMask == 0)
        uTestMask = TEST_ALL;

    // Display test preamble
    printf("\nSirius " VERSION_MAJOR_STR "." VERSION_MINOR_STR " VCS Rev. " VCS_REVISION_STR " (" __DATE__ " " __TIME__ " ) S/N: %010u\n", SERIAL_NUMBER);


    if (uTestMask & TEST_SENSORS)
    {
        FT_PRINT_HEADER("SENSOR TEST");
        testresult(test_sensors());
    }

    if (uTestMask & TEST_INDICATION)
    {
        FT_PRINT_HEADER("INDICATION AND SVINFO TEST");
        testresult(test_indication());
    }

    if (uTestMask & TEST_LED_RINGS)
    {
        FT_PRINT_HEADER("LED RING TEST");
        do
        {
            printf("Running sequence...\n");
            SEQUENCE_TestDriveStartBlocking(2000);
        } while (FALSE == QuestionContinueRedo());
        testresult(TRUE);
    }

    if (uTestMask & TEST_HF_CARRIER)
    {
        FT_PRINT_HEADER("RADIO HF CARRIER TEST");
        testresult(test_meshradio());
    }

    if (uTestMask & TEST_MESH)
    {
        FT_PRINT_HEADER("MESH TEST");
        printf("testcase not defined yet\n");
    }

    if (uTestMask & TEST_GPS)
    {
        FT_PRINT_HEADER("GPS TEST");
        testresult(test_gps());
    }


    return 0;
}