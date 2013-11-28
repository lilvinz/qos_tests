#ifndef TIMER_H_
#define TIMER_H_


typedef enum
{
    TIMER_ONE_SHOT_MODE,
    TIMER_REPEAT_MODE
} TIMER_MODE_T;

typedef struct _Timer_t TIMER_T;

struct _Timer_t
{
    struct
    {
        TIMER_T *next;
        uint32_t sysTimeLabel;
    } service;
    uint32_t interval;
    TIMER_MODE_T mode;
    void (*callback)(uint32_t parameter);
    uint32_t callback_parameter;
};



int TIMER_Start(TIMER_T *Timer);
int TIMER_Stop(TIMER_T *Timer);
int TIMER_Restart(TIMER_T *Timer);

void TIMER_Task(void *pvParameters);


#endif /* TIMER_H_ */
