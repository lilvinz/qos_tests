#ifndef LFSR_H
#define LFSR_H

#include <stdint.h>
#include <stdlib.h>

typedef struct
{
    uint32_t poly;
    uint32_t key;
} lfsr_ctx_t;

void lfsr_init(lfsr_ctx_t* ctxp, uint32_t poly, uint32_t key);
void lfsr_decrypt(lfsr_ctx_t* ctxp, size_t n, uint8_t* buffer);

#endif
