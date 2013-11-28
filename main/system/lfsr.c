
#include "stdheader.h"
#include "lfsr.h"


static uint32_t lfsr_poly;
static uint32_t lfsr_key;

void LFSR_Init(void)
{
    lfsr_poly = LFSR_GENERATOR_POLY;
    lfsr_key = LFSR_KEY;
}


void LFSR_Decrypt(uint8_t *pData, uint16_t length)
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



