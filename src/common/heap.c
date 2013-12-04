/*
 * heap.c
 *
 *  Created on: 04.12.2013
 *      Author: vke
 */

#include "heap.h"
#include "irq.h"
#include <stddef.h>


#if !defined(FREERTOS)

// default heap size to 32k if not defined
#if !defined(HEAP_SIZE)
    #define HEAP_SIZE (32 * 1024)
#endif

// default alignment to 8 bytes if not defined
#if !defined(HEAP_ALIGNMENT)
    #define HEAP_ALIGNMENT 8
#endif

#if HEAP_ALIGNMENT == 8
    #define HEAP_ALIGNMENT_MASK ( 0x0007 )
#else
    #error "Unhandled heap alignment"
#endif

#ifndef portPOINTER_SIZE_TYPE
    #define portPOINTER_SIZE_TYPE unsigned long
#endif

/* A few bytes might be lost to byte aligning the heap start address. */
#define ADJUSTED_HEAP_SIZE    ( HEAP_SIZE - HEAP_ALIGNMENT )

/* Allocate the memory for the heap. */
static unsigned char ucHeap[HEAP_SIZE];
static size_t xNextFreeByte = (size_t)0;

static void* pvPortMalloc(size_t xWantedSize)
{
    void* pvReturn = NULL;
    static unsigned char* pucAlignedHeap = NULL;

    /* Ensure that blocks are always aligned to the required number of bytes. */
#if HEAP_ALIGNMENT != 1
    if (xWantedSize & HEAP_ALIGNMENT_MASK)
    {
        /* Byte alignment required. */
        xWantedSize += (HEAP_ALIGNMENT
                - (xWantedSize & HEAP_ALIGNMENT_MASK));
    }
#endif

    IRQ_Disable();
    {
        if (pucAlignedHeap == NULL )
        {
            /* Ensure the heap starts on a correctly aligned boundary. */
            pucAlignedHeap = (unsigned char *) (((portPOINTER_SIZE_TYPE)
                    & ucHeap[HEAP_ALIGNMENT])
                    & ((portPOINTER_SIZE_TYPE) ~HEAP_ALIGNMENT_MASK));
        }

        /* Check there is enough room left for the allocation. */
        if (((xNextFreeByte + xWantedSize) < ADJUSTED_HEAP_SIZE)&&
                ( ( xNextFreeByte + xWantedSize ) > xNextFreeByte ) )/* Check for overflow. */
        {
            /* Return the next free byte then increment the index past this
               block. */
            pvReturn = pucAlignedHeap + xNextFreeByte;
            xNextFreeByte += xWantedSize;
        }
    }
    IRQ_Enable();

    return pvReturn;
}

static void vPortInitialiseBlocks(void)
{
    /* Only required when static memory is not cleared. */
    xNextFreeByte = (size_t) 0;
}

#else

extern void* pvPortMalloc(size_t xWantedSize);

#endif

void Heap_Init(void)
{
#if defined(FREERTOS)
    // nothing to do here as FreeRTOS kernel calls vPortInitialiseBlocks
#else
    vPortInitialiseBlocks();
#endif
}

// this overwrites malloc from newlibc
void* malloc(size_t size)
{
    return pvPortMalloc(size);
}
