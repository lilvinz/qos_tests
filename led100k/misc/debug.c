
#include "stdheader.h"
#include "debug.h"

#include "rtc.h"
#include "DBG_printf_stdarg.h"



#if (VERSION_IS_DEBUG > 0)

static uint8_t debug_module_mask;
static uint8_t debug_options;

typedef int (putchar_func_t)(char c);
putchar_func_t *putchar_func;

void DEBUG_PutS(char * s)
{
    if (!putchar_func)
        return;

    while (*s != 0)
    {
        putchar_func(*s);
        s++;
    }
}

void DEBUG_Init(void)
{
    putchar_func = DEBUG_PUTCHAR_INTERNAL;

    DEBUG_PutS("\r\n\r\n*****************************************\r\n");
    DEBUG_PutS("Sirius " VERSION_MAJOR_STR "." VERSION_MINOR_STR " (Build: " __DATE__ " " __TIME__")\r\n");
    DEBUG_PutS("Debug Output Console\r\n\r\n");

    // setup affected port pins to output
    debug_module_mask   = 0xFF;
    debug_options       =  0;
}

void DEBUG_PrintSet(uint8_t module_mask, uint8_t options, bool b_internal)
{
    debug_module_mask = module_mask;
    debug_options = options;
    if (b_internal)
        putchar_func = DEBUG_PUTCHAR_INTERNAL;  // internal
    else
        putchar_func = DEBUG_PUTCHAR_EXTERNAL;  // external
}

void DEBUG_Printf_module(uint8_t module, char* fmt, ...)
{
    if ((debug_module_mask & module) == 0)
        return;

    char buffer[200];

    if (debug_options & DEBUG_OPTION_TIMESTAMP)
    {
        RTCTM datetime;

        RTC_GetLocalTime(&datetime);

        DBG_sprintf(buffer, "%02u.%02u.%04u %02u:%02u:%02u   ",
                datetime.tm_mday, datetime.tm_mon + 1, (uint16_t)datetime.tm_year + RTC_BASEYEAR,
                datetime.tm_hour, datetime.tm_min, datetime.tm_sec);
        DEBUG_PutS(buffer);
    }

    if (debug_options & DEBUG_OPTION_MODULENAME)
    {
        switch (module)
        {
        case DEBUG_SYSTEM:          DEBUG_PutS("SYSTEM: ");     break;
        case DEBUG_ZIGBEE:          DEBUG_PutS("ZIGBEE: ");     break;
        case DEBUG_GLOOMMANAGER:    DEBUG_PutS("GLOOMMGR: ");   break;
        case DEBUG_LOG:             DEBUG_PutS("LOG: ");        break;
        case DEBUG_GPSSYNC:         DEBUG_PutS("GPSSYNC: ");    break;
        case DEBUG_CLI:             DEBUG_PutS("CLI: ");        break;
        default:                    DEBUG_PutS("UNKNOWN: ");    break;
        }
    }

    va_list args;
    va_start(args, fmt);
    DBG_vsprintf(buffer, fmt, args);
    va_end(args);

    DEBUG_PutS(buffer);
}

void DEBUG_Printf_continue(uint8_t module, char* fmt, ...)
{
    if ((debug_module_mask & module) == 0)
        return;

    char buffer[200];

    va_list args;
    va_start(args, fmt);
    DBG_vsprintf(buffer, fmt, args);
    va_end(args);

    DEBUG_PutS(buffer);
}

void DEBUG_Printf(char* fmt, ...)
{
    char buffer[200];

    va_list args;
    va_start(args, fmt);
    DBG_vsprintf(buffer, fmt, args);
    va_end(args);

    DEBUG_PutS(buffer);
}

void DEBUG_Printf_naked(uint8_t module, char* fmt, ...)
{
    if ((debug_module_mask & module) == 0)
        return;

    char buffer[200];

    va_list args;
    va_start(args, fmt);
    DBG_vsprintf(buffer, fmt, args);
    va_end(args);

    DEBUG_PutS(buffer);
}


#else
void DEBUG_Init(void) {};
void DEBUG_PrintSet(uint8_t module_mask, uint8_t options, bool b_internal)
{
    module_mask = module_mask;
}

void DEBUG_Printf_module(uint8_t module, char* fmt, ...)
{
    module = module;
    *fmt = *fmt;
}


#endif

