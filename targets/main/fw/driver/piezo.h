/*
 * sound.h
 *
 *  Created on: 16.07.2010
 *      Author: Carsten Bartels
 */

#ifndef PIEZO_H_
#define PIEZO_H_


enum
{
    C0, Db0, D0, Eb0, E0, F0, Gb0, G0, Ab0, A0, Bb0, B0,
    C1, Db1, D1, Eb1, E1, F1, Gb1, G1, Ab1, A1, Bb1, B1,
    C2, Db2, D2, Eb2, E2, F2, Gb2, G2, Ab2, A2, Bb2, B2,
    C3, Db3, D3, Eb3, E3, F3, Gb3, G3, Ab3, A3, Bb3, B3,
    C4, Db4, D4, Eb4, E4, F4, Gb4, G4, Ab4, A4, Bb4, B4,
    NOTE_COUNT
};

typedef struct
{
    uint8_t note;
    uint8_t volume;
    uint16_t attack;
    uint16_t decay;
    uint16_t sustain;
    uint16_t release;
    uint16_t duration;
} PIEZO_TONE_T;

void PIEZO_Play(const PIEZO_TONE_T *tone);
void PIEZO_Stop(void);

void PIEZO_Init(void);

#endif /* PIEZO_H_ */
