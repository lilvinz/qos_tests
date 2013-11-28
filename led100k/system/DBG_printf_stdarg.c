/*
	Copyright 2001, 2002 Georges Menie (www.menie.org)
	stdarg version contributed by Christian Ettinger

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
	DBG_putchar is the only external dependency for this file,
	if you have a working DBG_putchar, leave it commented out.
	If not, uncomment the define below and
	replace outbyte(c) by your own function call.

#define putchar(c) outbyte(c)
*/

#include "stdheader.h"
#include "DBG_printf_stdarg.h"

#include "uart.h"

#define REPLACE_NL_BY_DYNAMIC  1

#define DBG_putchar UART3_PutChar

DBG_LINE_END_MODE_T line_end_mode;

static void DBG_printchar(char **str, int c)
{
//	extern int DBG_putchar(int c);

	if (str) {
		**str = c;
		++(*str);
	}
	else (void)DBG_putchar(c);
}

void DBG_PRINTF_set_line_end_mode(DBG_LINE_END_MODE_T mode)
{
    line_end_mode = mode;
}

int DBG_send_end_character(char **out)
{
    //send line end character
    switch(line_end_mode)
    {
    case DBG_TELNET_CMD_END_MODE_CR:
        DBG_printchar (out, '\r');
        DBG_printchar (out, '\n');
        return 1;
    case DBG_TELNET_CMD_END_MODE_LF:
        DBG_printchar (out, '\n');
        return 1;
    case DBG_TELNET_CMD_END_MODE_CR_LF:
        DBG_printchar (out, '\r');
        DBG_printchar (out, '\n');
        return 2;
    }
    return 0;
}

#define PAD_RIGHT 1
#define PAD_ZERO 2

static int DBG_prints(char **out, const char *string, int width, int pad)
{
	register int pc = 0, padchar = ' ';

	if (width > 0) {
		register int len = 0;
		register const char *ptr;
		for (ptr = string; *ptr; ++ptr) ++len;
		if (len >= width) width = 0;
		else width -= len;
		if (pad & PAD_ZERO) padchar = '0';
	}
	if (!(pad & PAD_RIGHT)) {
		for ( ; width > 0; --width) {
		    DBG_printchar (out, padchar);
			++pc;
		}
	}
	for ( ; *string ; ++string) {
#if (REPLACE_NL_BY_DYNAMIC== 1)
        if (*string == '\n')
        {
             pc += DBG_send_end_character(out);
        }
        else
#endif
        {
            DBG_printchar (out, *string);
            ++pc;
        }
	}
	for ( ; width > 0; --width) {
	    DBG_printchar (out, padchar);
		++pc;
	}

	return pc;
}

/* the following should be enough for 32 bit int */
#define DBG_PRINT_BUF_LEN 12

static int DBG_printi(char **out, int i, int b, int sg, int width, int pad, int letbase)
{
	char DBG_print_buf[DBG_PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = i;

	if (i == 0) {
	    DBG_print_buf[0] = '0';
	    DBG_print_buf[1] = '\0';
		return DBG_prints (out, DBG_print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s = DBG_print_buf + DBG_PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = u % b;
		if ( t >= 10 )
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if (neg) {
		if ( width && (pad & PAD_ZERO) ) {
		    DBG_printchar (out, '-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}

	return pc + DBG_prints (out, s, width, pad);
}

int DBG_print( char **out, const char *format, va_list args )
{
	register int width, pad;
	register int pc = 0;
	char scr[2];

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0') break;
			if (*format == '%') goto out;
			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}
			for ( ; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}
			if ( *format == 's' ) {
				register char *s = (char *)va_arg( args, int );
				pc += DBG_prints (out, s?s:"(null)", width, pad);
				continue;
			}
			if ( *format == 'd' ) {
				pc += DBG_printi (out, va_arg( args, int ), 10, 1, width, pad, 'a');
				continue;
			}
			if ( *format == 'x' ) {
				pc += DBG_printi (out, va_arg( args, int ), 16, 0, width, pad, 'a');
				continue;
			}
			if ( *format == 'X' ) {
				pc += DBG_printi (out, va_arg( args, int ), 16, 0, width, pad, 'A');
				continue;
			}
			if ( *format == 'u' ) {
				pc += DBG_printi (out, va_arg( args, int ), 10, 0, width, pad, 'a');
				continue;
			}
			if ( *format == 'c' ) {
				/* char are converted to int then pushed on the stack */
				scr[0] = (char)va_arg( args, int );
				scr[1] = '\0';
				pc += DBG_prints (out, scr, width, pad);
				continue;
			}
		}
#if (REPLACE_NL_BY_DYNAMIC == 1)
		else if (*format == '\n')
        {
		    pc += DBG_send_end_character(out);
        }
#endif
		else {
		out:
		DBG_printchar (out, *format);
			++pc;
		}
	}
	if (out) **out = '\0';
	va_end( args );
	return pc;
}

int DBG_printf(const char *format, ...)
{
        va_list args;

        va_start( args, format );
        return DBG_print( 0, format, args );
}

int DBG_sprintf(char *out, const char *format, ...)
{
        va_list args;

        va_start( args, format );
        return DBG_print( &out, format, args );
}


int DBG_snprintf(char *buf, unsigned int count, const char *format, ... )
{
        va_list args;

        ( void ) count;

        va_start( args, format );
        return DBG_print( &buf, format, args );
}

int DBG_vprintf(const char *format, va_list args)
{
    return DBG_print(0, format, args);
}

int DBG_vsprintf(char *buf, const char *format, va_list args)
{
    return DBG_print(&buf, format, args);
}

#ifdef TEST_PRINTF
int main(void)
{
	char *ptr = "Hello world!";
	char *np = 0;
	int i = 5;
	unsigned int bs = sizeof(int)*8;
	int mi;
	char buf[80];

	mi = (1 << (bs-1)) + 1;
	printf("%s\n", ptr);
	printf("printf test\n");
	printf("%s is null pointer\n", np);
	printf("%d = 5\n", i);
	printf("%d = - max int\n", mi);
	printf("char %c = 'a'\n", 'a');
	printf("hex %x = ff\n", 0xff);
	printf("hex %02x = 00\n", 0);
	printf("signed %d = unsigned %u = hex %x\n", -3, -3, -3);
	printf("%d %s(s)%", 0, "message");
	printf("\n");
	printf("%d %s(s) with %%\n", 0, "message");
	sprintf(buf, "justif: \"%-10s\"\n", "left"); printf("%s", buf);
	sprintf(buf, "justif: \"%10s\"\n", "right"); printf("%s", buf);
	sprintf(buf, " 3: %04d zero padded\n", 3); printf("%s", buf);
	sprintf(buf, " 3: %-4d left justif.\n", 3); printf("%s", buf);
	sprintf(buf, " 3: %4d right justif.\n", 3); printf("%s", buf);
	sprintf(buf, "-3: %04d zero padded\n", -3); printf("%s", buf);
	sprintf(buf, "-3: %-4d left justif.\n", -3); printf("%s", buf);
	sprintf(buf, "-3: %4d right justif.\n", -3); printf("%s", buf);

	return 0;
}

/*
 * if you compile this file with
 *   gcc -Wall $(YOUR_C_OPTIONS) -DTEST_PRINTF -c printf.c
 * you will get a normal warning:
 *   printf.c:214: warning: spurious trailing `%' in format
 * this line is testing an invalid % at the end of the format string.
 *
 * this should display (on 32bit int machine) :
 *
 * Hello world!
 * printf test
 * (null) is null pointer
 * 5 = 5
 * -2147483647 = - max int
 * char a = 'a'
 * hex ff = ff
 * hex 00 = 00
 * signed -3 = unsigned 4294967293 = hex fffffffd
 * 0 message(s)
 * 0 message(s) with %
 * justif: "left      "
 * justif: "     right"
 *  3: 0003 zero padded
 *  3: 3    left justif.
 *  3:    3 right justif.
 * -3: -003 zero padded
 * -3: -3   left justif.
 * -3:   -3 right justif.
 */

#endif


