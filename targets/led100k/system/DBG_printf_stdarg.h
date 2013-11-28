/*
 * DBG_printf_stdarg.h
 *
 *  Created on: 22.10.2010
 *      Author: Administrator
 */

#ifndef DBG_PRINTFSTDARG_H_
#define DBG_PRINTFSTDARG_H_


typedef enum
{
    DBG_TELNET_CMD_END_MODE_CR = 0,
    DBG_TELNET_CMD_END_MODE_LF,
    DBG_TELNET_CMD_END_MODE_CR_LF
} DBG_LINE_END_MODE_T;


int DBG_printf(const char *format, ...);
int DBG_sprintf(char *out, const char *format, ...);
int DBG_snprintf( char *buf, unsigned int count, const char *format, ... );
int DBG_vprintf(const char *format, va_list args);
int DBG_vsprintf(char *buf, const char *format, va_list args);

void DBG_PRINTF_set_line_end_mode(DBG_LINE_END_MODE_T mode);
#endif /* PRINTFSTDARG_H_ */
