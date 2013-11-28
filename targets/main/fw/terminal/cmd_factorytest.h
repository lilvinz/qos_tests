/*
 * cmd_factorytest.h
 *
 *  Created on: 22.11.2011
 *      Author: cba
 */

#ifndef CMD_FACTORYTEST_H_
#define CMD_FACTORYTEST_H_

#define FT_TERMINAL_WIDTH   100

#define FT_PRINT_HEADER(x) \
    {\
        printf("\n"ESC_INVERSE);\
        for (uint8_t col=0; col < ((FT_TERMINAL_WIDTH - strlen(x) - 16) / 2); col++)\
            printf(" ");\
        printf("* * *   %s   * * *", x);\
        for (uint8_t col=0; col < ((FT_TERMINAL_WIDTH - strlen(x) - 16) / 2); col++)\
            printf(" ");\
        printf(ESC_ATTRIBUTE_OFF"\n\n");\
    }
#define FT_PRINT_SUCCESS() printf("\n\n   "ESC_BGCOLOR_GREEN ESC_COLOR_BLACK"   >>> DONE <<<   "ESC_ATTRIBUTE_OFF"\n")
#define FT_PRINT_FAIL() printf("\n\n   "ESC_BGCOLOR_RED ESC_COLOR_BLACK"   >>> FAILED <<<   "ESC_ATTRIBUTE_OFF"\n")

typedef struct
{
    char * text;
    bool state_relais_a;
    bool state_relais_b;
    bool expected_result;
} CMD_FACTORYTEST_INDICATION_TEST_TABLE_ENTRY_T;

#define CMD_FACTORYTEST_INDICATION_TEST_TABLE_NUM_ENTRIES   8

extern const char CMD_CommandString_Factorytest[];
extern const char CMD_HelpString_Factorytest[];
extern const char CMD_HelpLongString_Factorytest[];



int CMDFactorytest(int argc, char * argv[]);

#endif /* CMD_FACTORYTEST_H_ */
