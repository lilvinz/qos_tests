#ifndef HELPERS_H
#define HELPERS_H

#include <inttypes.h>
#include <stdbool.h>

#define TRUE true
#define FALSE false

#define UINT8_T_MIN		0x00
#define UINT8_T_MAX		UINT8_MAX
#define UINT16_T_MIN	0x0000
#define UINT16_T_MAX	UINT16_MAX
#define UINT32_T_MIN	0x00000000L
#define UINT32_T_MAX	UINT32_MAX
#define INT8_T_MIN		INT8_MIN
#define INT8_T_MAX		INT8_MAX
#define INT16_T_MIN		INT16_MIN
#define INT16_T_MAX		INT16_MAX
#define INT32_T_MIN		INT32_MIN
#define INT32_T_MAX		INT32_MAX

// wait about nCount * 2us
// todo check timing
#define DELAY_2US(nCount) \
    { \
        volatile uint32_t i_ctr_1 = nCount; \
        volatile uint32_t i_ctr_2; \
        for(; i_ctr_1 != 0; i_ctr_1--) \
        { \
            i_ctr_2 = 11; \
            for(; i_ctr_2 != 0; i_ctr_2--); \
        } \
    }

// wait about nCount * 1ms
#define DELAY_MS(nCount) \
    { \
        volatile uint32_t i_ctr_1 = nCount; \
        volatile uint32_t i_ctr_2; \
        for(; i_ctr_1 != 0; i_ctr_1--) \
        { \
            i_ctr_2 = 5500; \
            for(; i_ctr_2 != 0; i_ctr_2--); \
        } \
    }
#if 0
#ifndef NULL
#define NULL		0
#endif

#define STR(x)			STR_HELPER(x)
#define STR_HELPER(x)	#x

#ifndef QT
#define PACK __attribute__((__packed__))
#else
#define PACK
#endif

// Host<->Network converter for little endian system
#define HTONS(x)	( ( (((uint16_t)(x)) >>8) & 0xff) | \
                      ((((uint16_t)(x)) & 0xff)<<8) )
#define NTOHS(x)	( ( (((uint16_t)(x)) >>8) & 0xff) | \
                      ((((uint16_t)(x)) & 0xff)<<8) )
#define HTONL(x)	((((x)>>24) & 0xffL) | (((x)>>8) & 0xff00L) | \
                     (((x)<<8) & 0xff0000L) | (((x)<<24) & 0xff000000L))
#define NTOHL(x)	((((x)>>24) & 0xffL) | (((x)>>8) & 0xff00L) | \
                     (((x)<<8) & 0xff0000L) | (((x)<<24) & 0xff000000L))

#endif
#endif
