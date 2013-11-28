#ifndef INDICATION_H
#define INDICATION_H

#define INDICATION_NO_TIMEOUT (-1)

#define INDICATION_WAIT_TIME 100  // wait 100 ms

typedef int16_t IND_TIMEOUT_T;

typedef enum
{
    IND_A,
    IND_B,
    IND_RELAIS_COUNT
} IND_RELAIS_T;

typedef struct ind_relais_s
{
    bool            state;
    IND_TIMEOUT_T   timeout;
} IND_STATE_T;

extern IND_STATE_T  _indication_state[IND_RELAIS_COUNT];


void INDICATION_Set(IND_RELAIS_T ind, IND_TIMEOUT_T timeout);   // timeout value in seconds
void INDICATION_Reset(IND_RELAIS_T ind);
void INDICATION_OVERRIDE_Set(IND_RELAIS_T ind, bool state, IND_TIMEOUT_T timeout);
void INDICATION_Task(void *pvParameters);


#endif
