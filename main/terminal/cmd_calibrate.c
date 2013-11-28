/*
 * cmd_factorytest.c
 *
 *  Created on: 22.11.2011
 *      Author: cba
 */

#include "stdheader.h"
#include "freegetopt.h"

#include "vt102.h"
#include "cli.h"
#include "printf-stdarg.h"

#include "sequence.h"

#include "sensor.h"
#include "sensors.h"
#include "adc.h"
#include "aguard.h"

#include "cmd_calibrate.h"


/************** cli specific definitions ***********/

const char CMD_CommandString_Calibrate[] = "calibrate";
const char CMD_HelpString_Calibrate[]
= "calibrate <options> - Calibrate components";
const char CMD_HelpLongString_Calibrate[]
= "calibrate <options>\n"\
  "Calibrate components\n"\
  "   Possible options:\n"\
  "   -a    Run all available calibrations\n"\
  "   -l    Calibrate LEDs\n"\
  "   -v    View all calibration values\n";


/************** local variables **************/

/************** local functions **************/

/************** local definitions **************/
#define CALIBRATE_LEDS      0x0001

#define CALIBRATE_VIEW      0x0100

#define CALIBRATE_ALL       0xFFFF


void caldelay(uint16_t ms)
{
    vTaskDelay(ms);
}


// ******** TEST SENSORS *********

bool cal_print_float(float value, float average, uint8_t tolerance)
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

void cal_print_float_color(float value, uint8_t vtcolor)
{
    VT_VGBGCOLOR(VT_COLOR_BLACK, vtcolor);
    printf(" %4d.%03d "ESC_ATTRIBUTE_OFF" ", (int)value, (int)(value * 1000.0f) % 1000);
}

bool calibrate_leds(void)
{
#if 0   //ToDo Port this shit to new primestruct
// todo[when this code should be activated] change / move fram section to store calibration data, the current section is too small to keep the data for all leds (100k hw)

#define CAL_SENSORS_TEMPERATURE_TOLERANCE_PERCENT   10      // percent
#define CAL_SENSORS_24V_TOLERANCE_PERCENT            5      // percent
#define CAL_SENSORS_LED_TOLERANCE_PERCENT            5      // percent
#define CAL_SENSORS_LED_RED_VOLTAGE      52      // Volt
#define CAL_SENSORS_LED_WHITE_VOLTAGE    43      // Volt
#define CAL_LEDPOWER_WHITE               10000   // PWM Value
#define CAL_LEDPOWER_RED                 10000   // PWM Value

#define CAL_MEASUREMENTS_PER_SECOND     100
#define CAL_OVERALL_TIME_IN_SECONDS     10
#define CAL_MAX_LED_VPP                 1.0     // Maximum Voltage peak to peak

    TEMPERATURE_SENSOR_T temperature;
    uint8_t valuecount = 0;
    float valuesum = 0;
    uint16_t errcount = 0;

    printf("\nMaximum Sensor Tolerance: %u percent\n\n", CAL_SENSORS_TEMPERATURE_TOLERANCE_PERCENT);

    printf("Temperatur sensors comparison\n");
    printf("         Tmain      Tled1      Tled2      Tled3      Tled4\n         ");

    valuesum = 0.0;
    valuecount = 0;

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
            if (!cal_print_float(temperature.value, valuesum / valuecount, CAL_SENSORS_TEMPERATURE_TOLERANCE_PERCENT))
                errcount++;
        }
        else
        {
            printf(ESC_BGCOLOR_RED ESC_COLOR_BLACK"  <FAIL>  "ESC_ATTRIBUTE_OFF" ");
            errcount++;
        }
    }


    printf("\n\n*** WARNING ***\nLEDs will be switched on in 3 seconds...\n");
    caldelay(3000);

//    printf("Maximum 24V Tolerance: %u percent\n", CAL_SENSORS_24V_TOLERANCE_PERCENT);
    //printf("Maximum LED Tolerance: %u percent\n", CAL_SENSORS_LED_TOLERANCE_PERCENT);

    DIM_LED_POWERLEVEL powerlevel[DIM_LED_COUNT];
#ifdef HWP_LED_RED_UNI
    powerlevel[DIM_LED_RED_1] = CAL_LEDPOWER_RED;
    powerlevel[DIM_LED_RED_2] = CAL_LEDPOWER_RED;
    powerlevel[DIM_LED_RED_3] = CAL_LEDPOWER_RED;
    powerlevel[DIM_LED_RED_4] = CAL_LEDPOWER_RED;
#endif
#ifdef HWP_LED_RED_2K
    powerlevel[DIM_LED_RED_C] = CAL_LEDPOWER_RED;
#endif
#ifdef HWP_LED_WHITE_20K
    powerlevel[DIM_LED_WHITE] = CAL_LEDPOWER_WHITE;
#endif


    DIM_SetNow(powerlevel, FALSE, FALSE);
    SEQUENCE_FlatOn(CAL_OVERALL_TIME_IN_SECONDS + 3);
    caldelay(2000);

    printf("\nRunning measurements for %u seconds...", CAL_OVERALL_TIME_IN_SECONDS);

    AGUARD_EXTREMES_T supplyvoltage;
    uint64_t supplyvoltage_sum = 0;
    AGUARD_LED_CALIBRATION_T led[ADC_LED_CH_INDEX_LAST + 1];
    uint64_t led_sum[ADC_LED_CH_INDEX_LAST + 1];

    uint16_t sum_count = 0;

    for (uint8_t channel = ADC_LED_CH_INDEX_FIRST; channel <= ADC_LED_CH_INDEX_LAST; channel++)
    {
        led[channel].value.min = UINT16_T_MAX;
        led[channel].value.max = 0;
        led[channel].value.average = 0;
        led_sum[channel] = 0;
    }

    supplyvoltage.pwr24v.min = UINT16_T_MAX;
    supplyvoltage.pwr24v.max = 0;
    supplyvoltage.pwr24v.average = 0;


    //////////////////////////////
    // capture values
    while (sum_count < (CAL_OVERALL_TIME_IN_SECONDS * CAL_MEASUREMENTS_PER_SECOND))
    {
        for (uint8_t channel = ADC_LED_CH_INDEX_FIRST; channel <= ADC_LED_CH_INDEX_LAST; channel++)
        {
            if (adc_values.values.led.array[channel] > led[channel].value.max)
                led[channel].value.max = adc_values.values.led.array[channel];
            if (adc_values.values.led.array[channel] < led[channel].value.min)
                led[channel].value.min = adc_values.values.led.array[channel];

            led_sum[channel] += adc_values.values.led.array[channel];
        }

        if (ADC_CURRENT_PWR24V_VALUE > supplyvoltage.pwr24v.max)
            supplyvoltage.pwr24v.max = ADC_CURRENT_PWR24V_VALUE;
        if (ADC_CURRENT_PWR24V_VALUE < supplyvoltage.pwr24v.min)
            supplyvoltage.pwr24v.min = ADC_CURRENT_PWR24V_VALUE;

        supplyvoltage_sum += ADC_CURRENT_PWR24V_VALUE;

        sum_count++;

        caldelay(1000 / CAL_MEASUREMENTS_PER_SECOND);
    }

    //////////////////////////////
    // calculate average values

    supplyvoltage.pwr24v.average = supplyvoltage_sum / sum_count;

    for (uint8_t channel = ADC_LED_CH_INDEX_FIRST; channel <= ADC_LED_CH_INDEX_LAST; channel++)
    {
        led[channel].value.average = led_sum[channel] / sum_count;
    }


    //////////////////////////////
    // display values

    printf("\n\nVoltage sensors\n");
    printf("             Vcal_min   Vmin       Vavrg      Vmax       Vcal_max   Vpp\n");
    printf("   24V\n\n   White A\n   White B\n   White C\n   White D\n\n");
    printf("   Red 1\n   Red 2\n   Red 3\n   Red 4\n   Red C1\n   Red C2\n");

    VT_UP(13);
    VT_COLUMN(25);

    cal_print_float((float)supplyvoltage.pwr24v.min / 1000, 24.0, CAL_SENSORS_24V_TOLERANCE_PERCENT);
    cal_print_float((float)supplyvoltage.pwr24v.average / 1000, 24.0, CAL_SENSORS_24V_TOLERANCE_PERCENT);
    cal_print_float((float)supplyvoltage.pwr24v.max / 1000, 24.0, CAL_SENSORS_24V_TOLERANCE_PERCENT);

    VT_DOWN(1);

    for (uint8_t channel = ADC_LED_CH_INDEX_FIRST; channel <= ADC_LED_CH_INDEX_LAST; channel++)
    {
        VT_DOWN(1);
        VT_COLUMN(14);

        if (channel == ADC_LED_CH_INDEX_RED_1)
            VT_DOWN(1);

        AGUARD_LED_CALIBRATION_T    led_current_calibration;
        AGUARD_GetLEDCalibration (channel, &led_current_calibration);

        float average = (float)led[channel].value.average / 1000;
        float minimum = (float)led[channel].value.min / 1000;
        float maximum = (float)led[channel].value.max / 1000;

        float lower_limit = average - (average * (float)CAL_SENSORS_LED_TOLERANCE_PERCENT / 100);
        float upper_limit = average + (average * (float)CAL_SENSORS_LED_TOLERANCE_PERCENT / 100);

        // calibrated lower limit
        cal_print_float_color(lower_limit, VT_COLOR_WHITE);

        //measured values
        if (minimum < lower_limit)
            cal_print_float_color(minimum, VT_COLOR_RED);
        else
            cal_print_float_color(minimum, VT_COLOR_YELLOW);

        cal_print_float_color(average, VT_COLOR_GREEN);

        if (maximum > upper_limit)
            cal_print_float_color(maximum, VT_COLOR_RED);
        else
            cal_print_float_color(maximum, VT_COLOR_YELLOW);

        // calibrated upper limit
        cal_print_float_color(upper_limit, VT_COLOR_WHITE);

        // delta
        if ((maximum - minimum) > CAL_MAX_LED_VPP)
            cal_print_float_color(maximum - minimum, VT_COLOR_RED);
        else
            cal_print_float_color(maximum - minimum, VT_COLOR_GREEN);

        // ToDo Save calibration temperature within values!
        // ToDo In AGUARD do respect the LED Vf voltage drift caused by temperature.

        // set upper and lower boundary for storage
        led[channel].value.min = lower_limit * 1000;
        led[channel].value.max = upper_limit * 1000;

        AGUARD_SetLEDCalibration(channel, &led[channel]);

    }

    AGUARD_SaveLEDCalibration();
    printf("\nCalibration saved.\n\n");

#endif

    return TRUE;
}

void calibrate_show_leds(void)
{
    // display values

    printf("\n\nVoltage sensors\n");
    printf("             cal_min    center     cal_max\n");
    printf("   White A\n   White B\n   White C\n   White D\n\n");
    printf("   Red 1\n   Red 2\n   Red 3\n   Red 4\n   Red C1\n   Red C2\n");

    VT_UP(12);
#if 0   //ToDo Port this shit to new primestruct
    for (uint8_t channel = ADC_LED_CH_INDEX_FIRST; channel <= ADC_LED_CH_INDEX_LAST; channel++)
    {
        VT_COLUMN(14);
        VT_DOWN(1);

        if (channel == ADC_LED_CH_INDEX_RED_1)
            VT_DOWN(1);

        AGUARD_LED_CALIBRATION_T    led_current_calibration;
        AGUARD_GetLEDCalibration (channel, &led_current_calibration);

        // calibrated lower limit
        cal_print_float_color((float)led_current_calibration.value.min / 1000, VT_COLOR_YELLOW);
        cal_print_float_color((float)led_current_calibration.value.average / 1000, VT_COLOR_GREEN);
        cal_print_float_color((float)led_current_calibration.value.max / 1000, VT_COLOR_YELLOW);
    }
#endif
    printf("\n\n");
}



bool cal_QuestionContinueRedo(void)
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

int CMDCalibrate(int argc, char * argv[])
{
    signed char c;
    uint16_t uCalibrationMask = 0;

    while((c = freegetopt(argc, argv, "alv")) != -1)
    {
        switch(c)
        {
        case 'a':
            uCalibrationMask = CALIBRATE_ALL;
            break;
        case 'l':
            uCalibrationMask |= CALIBRATE_LEDS;
            break;
        case 'v':
            uCalibrationMask |= CALIBRATE_VIEW;
            break;
        case ':':
            printf("\nOption -%c needs operant\n", optopt);
            return -1;
        case '?':
            printf("\nUnknown option -%c\n", optopt);
            return -1;
        }
    }

    if (uCalibrationMask == 0)
        uCalibrationMask = CALIBRATE_ALL;

    // Display test preamble
//    printf("\nSirius " VERSION_MAJOR_STR "." VERSION_MINOR_STR " VCS Rev. " VCS_REVISION_STR " (" __DATE__ " " __TIME__ " ) S/N: %010u\n", SERIAL_NUMBER);


    if (uCalibrationMask & CALIBRATE_LEDS)
    {
        //   FT_PRINT_HEADER("SENSOR TEST");
        calibrate_leds();
    }

    if (uCalibrationMask & CALIBRATE_VIEW)
    {
        calibrate_show_leds();
    }

    return 0;
}
