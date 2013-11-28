/*
 * delay.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 02.09.2011
 *      Author: DHA
 *
 * Purpose: Provide a function to delay program execution.
 *
 */
#ifndef DELAY_H_
#define DELAY_H_


void DELAY_Init(void);
void DELAY_uS(uint32_t nCount);
// wait about nCount * 1ms
#define DELAY_MS(nCount) \
    { \
        uint32_t i_ctr_1 = nCount; \
        for(; i_ctr_1 != 0; i_ctr_1--) \
        { \
            DELAY_uS(1000); \
        } \
    }
#endif
