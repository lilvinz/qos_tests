

#include "lfsr.h"


uint32_t lfsr_poly;
uint32_t lfsr_key;

void lfsr_init(void)
{
    lfsr_poly = LFSR_GENERATOR_POLY;
    lfsr_key = LFSR_KEY;
}


void lfsr_decrypt(uint8_t *pData, uint16_t length)
{
    for (uint16_t iByte = 0; iByte < length; ++iByte)
    {
        for (uint8_t iBit = 0; iBit < 8; ++iBit)
        {
            uint32_t temp = lfsr_key & lfsr_poly;
            temp ^= temp >> 1;
            temp ^= temp >> 2;
            temp ^= temp >> 4;
            temp ^= temp >> 8;
            temp ^= temp >> 16;
            temp &= 1;
            lfsr_key <<= 1;
            lfsr_key |= (uint8_t)temp;
        }
        *pData++ ^= (lfsr_key & 0xff);
    }
}



