
#ifndef FIFO_H
#define FIFO_H


#define FIFO_SIZE		200

typedef struct fifo_s
{
    volatile uint8_t data[FIFO_SIZE];
    volatile uint16_t nInput;
    volatile uint16_t nOutput;
    volatile uint16_t nSize;
    volatile uint16_t nMaxSize;
} FIFO_T;

void fifo_put(FIFO_T *pFifo, uint8_t byte);
uint8_t fifo_get(FIFO_T *pFifo);
uint8_t fifo_size(FIFO_T *pFifo);
void fifo_clear(FIFO_T *pFifo);


#endif

