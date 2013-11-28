/*
 * sound.c
 *
 *  Created on: 21.07.2010
 *      Author: Carsten Bartels
 */

#include "stdheader.h"
#include "sound.h"
#include "piezo.h"
#include "timeout.h"

static xQueueHandle xQueueMelody = NULL;

#define MELODY_END {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}

const PIEZO_TONE_T Hello_Melody[] =
{
    //note vol att dec sus rel
    { C2,  255,  0,  100,  0,  100, 100   },
    { C4,  255,  0,  100,  0,  1000, 1400   },
    MELODY_END
};

const PIEZO_TONE_T Goodbye_Melody[] =
{
    //note vol att dec sus rel
    { C3,  50,  0,  100,  0,  100, 200   },
    { C2,  50,  100,  100,  0,  200, 200   },
    { C1,  50,  100,  100,  0,  200, 400   },
    { C0,  50,  0,  100,  0,  300, 400   },
    MELODY_END
};

const PIEZO_TONE_T Day_Melody[] =
{
    { C3,  255,  0,  100,  0,  100, 50   },
    { D3,  255,  0,  100,  0,  100, 50   },
    { E3,  255,  0,  100,  0,  100, 50   },
    { F3,  255,  0,  100,  0,  100, 50   },
    { G3,  255,  0,  100,  0,  100, 50   },
    { A3,  255,  0,  100,  0,  100, 50   },
    { B3,  255,  0,  100,  0,  100, 50   },
    MELODY_END
};

const PIEZO_TONE_T Night_Melody[] =
{
    { B3,  255,  0,  100,  0,  100, 50   },
    { A3,  255,  0,  100,  0,  100, 50   },
    { G3,  255,  0,  100,  0,  100, 50   },
    { F3,  255,  0,  100,  0,  100, 50   },
    { E3,  255,  0,  100,  0,  100, 50   },
    { D3,  255,  0,  100,  0,  100, 50   },
    { C3,  255,  0,  100,  0,  100, 50   },
    MELODY_END
};

const PIEZO_TONE_T GPS_Synced_Melody[] =
{
    //note vol att dec sus rel
    { C1,  255,  0,  100,  0,  600, 1000   },
    MELODY_END
};

const PIEZO_TONE_T Mesh_InNwk_Melody[] =
{
    //note vol att dec sus rel
    { C2,  255,  0,  100,  0,  100, 200   },
    { C2,  255,  0,  100,  0,  100, 200   },
    { C2,  255,  0,  100,  0,  100, 200   },
    MELODY_END
};

const PIEZO_TONE_T* const Melody_Table[MELODY_COUNT] =
{
    Hello_Melody,
    Goodbye_Melody,
    Day_Melody,
    Night_Melody,
    GPS_Synced_Melody,
    Mesh_InNwk_Melody,
};

void SOUND_Task(void *pvParameters)
{
    MELODY_T nextMelody;
    const PIEZO_TONE_T *melody;
    uint16_t step;
    TIMEOUT_MS_T MelodyTimeout;

    xQueueMelody = xQueueCreate(5, sizeof(MELODY_T));

    while (xQueueReceive(xQueueMelody, (void *) &nextMelody, portMAX_DELAY) == pdPASS)
    {
        melody = Melody_Table[nextMelody];
        step = 0;

        while(melody[step].note != 0xFF)
        {
            PIEZO_Play(&melody[step]);

            TIMEOUT_Reset_ms(MelodyTimeout);

            while(!TIMEOUT_Check_ms(MelodyTimeout, melody[step].duration))
            {
                vTaskDelay(1);
            }
            step++;
        }
        PIEZO_Stop();
    }
}

void SOUND_PlayMelody(MELODY_T melody)
{
    if (xQueueMelody != NULL)
        xQueueSendToBack(xQueueMelody, (void *) &melody, (portTickType) 100);
}



