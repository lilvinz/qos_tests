#ifndef DEBUG_H
#define DEBUG_H

#include "version.h"
#include "uart.h"
#include "hw_configuration.h"


#define DEBUG_PUTCHAR_EXTERNAL UART5_PutChar
#define DEBUG_PUTCHAR_INTERNAL UART3_PutChar

#define DEBUG_GENERAL         0x01

#define DEBUG_SYSTEM          0x02
#define DEBUG_MESH          0x04
#define DEBUG_GLOOMMANAGER    0x08
#define DEBUG_LOG             0x10
#define DEBUG_GPSSYNC         0x20
#define DEBUG_CLI             0x40
#define DEBUG_AGUARD          0x80


#if (VERSION_IS_DEBUG > 0)

#define TPIN_ON(x)          GPIO_SetBits(DEBUG_PIN_PORT, 0x01<<x)
#define TPIN_OFF(x)         GPIO_ResetBits(DEBUG_PIN_PORT, 0x01<<x)
#define TPIN_TOGGLE(x)      {\
                                if (GPIO_ReadOutputDataBit(DEBUG_PIN_PORT, 0x01<<x) != Bit_SET) \
                                    GPIO_SetBits(DEBUG_PIN_PORT, 0x01<<x); \
                                else \
                                    GPIO_ResetBits(DEBUG_PIN_PORT, 0x01<<x); \
                            }

#define DPRINT(m,x)                 DEBUG_Printf_module(m,x)
#define DPRINT1(m,x,a)              DEBUG_Printf_module(m,x,a)
#define DPRINT2(m,x,a,b)            DEBUG_Printf_module(m,x,a,b)
#define DPRINT3(m,x,a,b,c)          DEBUG_Printf_module(m,x,a,b,c)
#define DPRINT4(m,x,a,b,c,d)        DEBUG_Printf_module(m,x,a,b,c,d)
#define DPRINT5(m,x,a,b,c,d,e)      DEBUG_Printf_module(m,x,a,b,c,d,e)
#define DPRINT6(m,x,a,b,c,d,e,f)    DEBUG_Printf_module(m,x,a,b,c,d,e,f)
#define DPRINT7(m,x,a,b,c,d,e,f,g)  DEBUG_Printf_module(m,x,a,b,c,d,e,f,g)

#define DPRINTN(m,x)                DEBUG_Printf_continue(m,x)
#define DPRINT1N(m,x,a)             DEBUG_Printf_continue(m,x,a)
#define DPRINT2N(m,x,a,b)           DEBUG_Printf_continue(m,x,a,b)
#define DPRINT3N(m,x,a,b,c)         DEBUG_Printf_continue(m,x,a,b,c)
#define DPRINT4N(m,x,a,b,c,d)       DEBUG_Printf_continue(m,x,a,b,c,d)

#define DCATCH(c,r)                 if (c != r) DEBUG_Printf(#c" in file %s line %u did not return %u\n", __FILE__, __LINE__, r);

#else //(VERSION_IS_DEBUG > 0)

#define TPIN_ON(x)
#define TPIN_OFF(x)
#define TPIN_TOGGLE(x)

#define DPRINT(m,x)
#define DPRINT1(m,x,a)
#define DPRINT2(m,x,a,b)
#define DPRINT3(m,x,a,b,c)
#define DPRINT4(m,x,a,b,c,d)
#define DPRINT5(m,x,a,b,c,d,e)
#define DPRINT6(m,x,a,b,c,d,e,f)
#define DPRINT7(m,x,a,b,c,d,e,f,g)

#define DPRINTN(m,x)
#define DPRINT1N(m,x,a)
#define DPRINT2N(m,x,a,b)
#define DPRINT3N(m,x,a,b,c)
#define DPRINT4N(m,x,a,b,c,d)

#define DCATCH(c,r)               c

#endif //(VERSION_IS_DEBUG > 0)



#define DEBUG_OPTION_NONE       0x00
#define DEBUG_OPTION_TIMESTAMP  0x01
#define DEBUG_OPTION_MODULENAME 0x02


// use the following template for your modules header file and replace "rename"

/*

#define DEBUG_RENAME    1

#if (DEBUG_RENAME == 1)

#include "debug.h"

#define RENAME_TPIN_ON(x)               TPIN_ON(x)
#define RENAME_TPIN_OFF(x)              TPIN_OFF(x)
#define RENAME_TPIN_TOGGLE(x)           TPIN_TOGGLE(x)

#define RENAME_DPRINT(x)                 DPRINT(MODULE_RENAME,x)
#define RENAME_DPRINT1(x,a)              DPRINT1(MODULE_RENAME,x,a)
#define RENAME_DPRINT2(x,a,b)            DPRINT2(MODULE_RENAME,x,a,b)
#define RENAME_DPRINT3(x,a,b,c)          DPRINT3(MODULE_RENAME,x,a,b,c)
#define RENAME_DPRINT4(x,a,b,c,d)        DPRINT4(MODULE_RENAME,x,a,b,c,d)
#define RENAME_DPRINT5(x,a,b,c,d,e)      DPRINT5(MODULE_RENAME,x,a,b,c,d,e)
#define RENAME_DPRINT6(x,a,b,c,d,e,f)    DPRINT6(MODULE_RENAME,x,a,b,c,d,e,f)
#define RENAME_DPRINT7(x,a,b,c,d,e,f,g)  DPRINT7(MODULE_RENAME,x,a,b,c,d,e,f,g)

#else

#define RENAME_TPIN_ON(x)
#define RENAME_TPIN_OFF(x)
#define RENAME_TPIN_TOGGLE(x)

#define RENAME_DPRINT(x)
#define RENAME_DPRINT1(x,a)
#define RENAME_DPRINT2(x,a,b)
#define RENAME_DPRINT3(x,a,b,c)
#define RENAME_DPRINT4(x,a,b,c,d)
#define RENAME_DPRINT5(x,a,b,c,d,e)
#define RENAME_DPRINT6(x,a,b,c,d,e,f)
#define RENAME_DPRINT7(x,a,b,c,d,e,f,g)

#endif

*/




void DEBUG_Init(void);
void DEBUG_PrintSet(uint8_t module_mask, uint8_t options, bool b_internal);
void DEBUG_Printf(char* fmt, ...);
void DEBUG_Printf_module(uint8_t module, char* fmt, ...);
void DEBUG_Printf_continue(uint8_t module, char* fmt, ...);
void DEBUG_PutS(char * s);

#endif
