
/* own */
#include "lfsr.h"

/**
 * @brief   Initializes a lfsr context structure.
 *
 * @param[in] ctxp          pointer to a @p lfsr_ctx_t object
 * @param[in] poly          desired generator polynominal
 * @param[in] key           desired encryption key
 *
 */
void lfsr_init(lfsr_ctx_t* ctxp, uint32_t poly, uint32_t key)
{
    ctxp->poly = poly;
    ctxp->key = key;
}

/**
 * @brief   Decrypts a chunk of data.
 *
 * @param[in] ctxp      pointer to a @p lfsr_ctx_t object
 * @param[in] n         number of bytes to process
 * @param[io] buffer    data buffer
 *
 */
void lfsr_decrypt(lfsr_ctx_t* ctxp, size_t n, uint8_t* buffer)
{
    for (uint16_t iByte = 0; iByte < n; ++iByte)
    {
        for (uint8_t iBit = 0; iBit < 8; ++iBit)
        {
            uint32_t temp = ctxp->key & ctxp->poly;
            temp ^= temp >> 1;
            temp ^= temp >> 2;
            temp ^= temp >> 4;
            temp ^= temp >> 8;
            temp ^= temp >> 16;
            temp &= 1;
            ctxp->key <<= 1;
            ctxp->key |= (uint8_t)temp;
        }
        *buffer++ ^= (ctxp->key & 0xff);
    }
}
