/*
 * vt102.h
 *
 *  Created on: 03.05.2011
 *      Author: cba
 */


/* This VT102 definition list is totally incomplete */


#ifndef VT102_H_
#define VT102_H_


#define CSI                 "\033["

#define RESET_TERMINAL      printf(CSI"c")

#define ESC_CRS_ULC         CSI"H"        // Cursor will move to the home position, at the upper left of the screen.

#define ESC_CLR             CSI"K"        // Erases from the current cursor position to the end of the current line.
#define ESC_CLR_NL          CSI"K\n"
#define ESC_ERASE_DOWN      CSI"J"        // Erases the screen from the current line down to the bottom of the screen.

#define ESC_BOLD            CSI"1m"
#define ESC_INVERSE         CSI"7m"
#define ESC_ATTRIBUTE_OFF   CSI"0m"
#define ESC_COLOR_BLACK     CSI"30m"
#define ESC_COLOR_RED       CSI"31m"
#define ESC_COLOR_GREEN     CSI"32m"
#define ESC_COLOR_YELLOW    CSI"33m"
#define ESC_COLOR_BLUE      CSI"34m"
#define ESC_COLOR_MAGENTA   CSI"35m"
#define ESC_COLOR_CYAN      CSI"36m"
#define ESC_COLOR_WHITE     CSI"37m"
#define ESC_BGCOLOR_BLACK   CSI"40m"
#define ESC_BGCOLOR_RED     CSI"41m"
#define ESC_BGCOLOR_GREEN   CSI"42m"
#define ESC_BGCOLOR_YELLOW  CSI"43m"
#define ESC_BGCOLOR_BLUE    CSI"44m"
#define ESC_BGCOLOR_MAGENTA CSI"45m"
#define ESC_BGCOLOR_CYAN    CSI"46m"
#define ESC_BGCOLOR_WHITE   CSI"47m"

#define ESC_ECHO_OFF        CSI"12h"
#define ESC_ECHO_ON         CSI"12l"


#define VT_SET(escapesequence) printf("%s", escapesequence)


#define VT_BOLD                printf(CSI"1m")
#define VT_UNDERSCORE          printf(CSI"4m")
#define VT_BLINK               printf(CSI"5m")
#define VT_INVERSE             printf(CSI"7m")
#define VT_BOLD_INVERSE        printf(CSI"1;7m")
#define VT_RED_ON_WHITE        printf(CSI"31;47m")
#define VT_ATTRIBUTE_OFF       printf(CSI"0m")
#define VT_CLEAR_SCREEN        printf(CSI";H\033[2J")
#define VT_CURSOR_HOME         printf(CSI";H")
#define VT_DISABLE_LINE_WRAP   printf(CSI"7l")


#define VT_ATTRIBUTE(attr)     printf(CSI"%dm"), attr)
#define VT_UP(count)           printf(CSI"%dA", count)
#define VT_DOWN(count)         printf(CSI"%dB", count)
#define VT_RIGHT(count)        printf(CSI"%dC", count)
#define VT_LEFT(count)         printf(CSI"%dD", count)
#define VT_COLUMN(n)           printf(CSI"%dG", n)
#define VT_CLR_TOEND(x)        printf(CSI"K")
#define VT_CLR_NL(x)           printf(CSI"K\n")
#define VT_POSITION(r,c)       printf(CSI"%d;%dH", r, c)
#define VT_COLOR(color)        printf(CSI"3%dm", color)
#define VT_BGCOLOR(color)      printf(CSI"4%dm", color)


#define VT_COLOR_BLACK     0
#define VT_COLOR_RED       1
#define VT_COLOR_GREEN     2
#define VT_COLOR_YELLOW    3
#define VT_COLOR_BLUE      4
#define VT_COLOR_MAGENTA   5
#define VT_COLOR_CYAN      6
#define VT_COLOR_WHITE     7

#endif /* VT102_H_ */
