/*
 * vt102.h
 *
 *  Created on: 03.05.2011
 *      Author: cba
 */


/* This VT102 definition list is totally incomplete */


#ifndef VT102_H_
#define VT102_H_

/* common/common */
#include "stringify.h"

#define CSI                 "\033["

#define RESET_TERMINAL      chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, "%s", CSI"c")

#define ESC_CRS_ULC         CSI"H"        // Cursor will move to the home position, at the upper left of the screen.

#define ESC_CLR             CSI"K"        // Erases from the current cursor position to the end of the current line.
#define ESC_CLR_NL          CSI"K\r\n"
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

#define ESC_DOWN(n)         CSI STRINGIFY(n) "B"
#define ESC_RIGHT(n)        CSI STRINGIFY(n) "C"
#define ESC_COLUMN(n)       CSI STRINGIFY(n) "G"


#define VT_SET(escapesequence) chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, "%s", escapesequence)


#define VT_BOLD                chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, "%s", CSI"1m")
#define VT_UNDERSCORE          chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, "%s", CSI"4m")
#define VT_BLINK               chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, "%s", CSI"5m")
#define VT_INVERSE             chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, "%s", CSI"7m")
#define VT_BOLD_INVERSE        chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, "%s", CSI"1;7m")
#define VT_RED_ON_WHITE        chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, "%s", CSI"31;47m")
#define VT_ATTRIBUTE_OFF       chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, "%s", CSI"0m")
#define VT_CLEAR_SCREEN        chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, "%s", CSI";H\033[2J")
#define VT_CURSOR_HOME         chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, "%s", CSI";H")
#define VT_DISABLE_LINE_WRAP   chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, "%s", CSI"7l")


#define VT_ATTRIBUTE(attr)     chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, CSI"%dm"), attr)
#define VT_UP(count)           chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, CSI"%dA", count)
#define VT_DOWN(count)         chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, CSI"%dB", count)
#define VT_RIGHT(count)        chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, CSI"%dC", count)
#define VT_LEFT(count)         chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, CSI"%dD", count)
#define VT_COLUMN(n)           chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, CSI"%dG", n)
#define VT_CLR_TOEND(x)        chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, "%s", CSI"K")
#define VT_CLR_NL(x)           chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, "%s", CSI"K\r\n")
#define VT_POSITION(r,c)       chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, CSI"%d;%dH", r, c)
#define VT_COLOR(color)        chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, CSI"3%dm", color)
#define VT_BGCOLOR(color)      chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, CSI"4%dm", color)
#define VT_VGBGCOLOR(vg,bg)    chprintft(STDIO_CHANNEL, STDIO_CHAR_TIMEOUT, CSI"3%dm"CSI"4%dm", vg, bg)


#define VT_COLOR_BLACK     0
#define VT_COLOR_RED       1
#define VT_COLOR_GREEN     2
#define VT_COLOR_YELLOW    3
#define VT_COLOR_BLUE      4
#define VT_COLOR_MAGENTA   5
#define VT_COLOR_CYAN      6
#define VT_COLOR_WHITE     7

#endif /* VT102_H_ */
