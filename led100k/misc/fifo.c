
#include "stdheader.h"
#include "fifo.h"

void fifo_put(FIFO_T *pFifo, uint8_t byte)
{
    CRITICAL_SECTION_ENTER();
    if (!pFifo || pFifo->nSize >= FIFO_SIZE)
    {
        CRITICAL_SECTION_LEAVE();
        return;
    }
    pFifo->data[pFifo->nInput++] = byte;
    if (pFifo->nInput == FIFO_SIZE)
        pFifo->nInput = 0;
    pFifo->nSize++;
    if (pFifo->nSize > pFifo->nMaxSize)

        pFifo->nMaxSize = pFifo->nSize;
    CRITICAL_SECTION_LEAVE();
}

uint8_t fifo_get(FIFO_T *pFifo)
{
    CRITICAL_SECTION_ENTER();
    if (!pFifo || !pFifo->nSize)
    {
        CRITICAL_SECTION_LEAVE();
        return 0x00;
    }
    uint8_t byte = pFifo->data[pFifo->nOutput++];
    if (pFifo->nOutput == FIFO_SIZE)
        pFifo->nOutput = 0;
    pFifo->nSize--;
    CRITICAL_SECTION_LEAVE();
    return byte;
}

uint8_t fifo_size(FIFO_T *pFifo)
{
    CRITICAL_SECTION_ENTER();
    if (!pFifo)
    {
        CRITICAL_SECTION_LEAVE();
        return 0;
    }
    uint8_t size = pFifo->nSize;
    CRITICAL_SECTION_LEAVE();
    return size;
}

void fifo_clear(FIFO_T *pFifo)
{
    CRITICAL_SECTION_ENTER();
    if (!pFifo)
    {
        CRITICAL_SECTION_LEAVE();
        return;
    }
    pFifo->nInput = 0;
    pFifo->nOutput = 0;
    pFifo->nSize = 0;
    CRITICAL_SECTION_LEAVE();
}

