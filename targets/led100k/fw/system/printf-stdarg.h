/*
 * printf-stdarg.h
 *
 *  Created on: 05.05.2010
 *      Author: Administrator
 */

#ifndef PRINTFSTDARG_H_
#define PRINTFSTDARG_H_


#pragma GCC diagnostic ignored "-Wformat"

typedef enum
{
    TELNET_CMD_END_MODE_CR = 0,
    TELNET_CMD_END_MODE_LF,
    TELNET_CMD_END_MODE_CR_LF
} LINE_END_MODE_T;


#if 0
int printf(const char *format, ...);
int sprintf(char *out, const char *format, ...);
int snprintf( char *buf, unsigned int count, const char *format, ... );
int vprintf(const char *format, va_list args);
#endif

void PRINTF_set_line_end_mode(LINE_END_MODE_T mode);
#endif /* PRINTFSTDARG_H_ */
