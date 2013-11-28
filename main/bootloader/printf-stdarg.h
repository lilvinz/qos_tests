/*
 * printf-stdarg.h
 *
 *  Created on: 05.05.2010
 *      Author: Administrator
 */

#ifndef PRINTFSTDARG_H_
#define PRINTFSTDARG_H_


int printf(const char *format, ...);
int sprintf(char *out, const char *format, ...);
int snprintf( char *buf, unsigned int count, const char *format, ... );
int vprintf(const char *format, va_list args);

#endif /* PRINTFSTDARG_H_ */
