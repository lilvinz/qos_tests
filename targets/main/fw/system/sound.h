/*
 * sound.h
 *
 *  Created on: 21.07.2010
 *      Author: Carsten Bartels
 */

#ifndef SOUND_H_
#define SOUND_H_


typedef enum
{
    HELLO_MELODY,
    GOODBYE_MELODY,
    DAY_MELODY,
    NIGHT_MELODY,
    GPS_SYNCED_MELODY,
    MESH_INNWK_MELODY,
    MELODY_COUNT
} MELODY_T;


void SOUND_PlayMelody(MELODY_T melody);
void SOUND_Task(void *pvParameters);

#endif /* SOUND_H_ */
