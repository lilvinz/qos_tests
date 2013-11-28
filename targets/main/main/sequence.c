
#include "stdheader.h"
#include "sequence.h"

#include "config.h"
#include "settings.h"
#include "detector.h"

#include "bmm.h"
#include "rtc.h"
#include "timer.h"
#include "watchdog.h"

#include "measureport.h"
#include "can_manager.h"


/*-------------- Local Type Definitions ------------*/

typedef enum
{
    SEQ_STATE_STOP,
    SEQ_STATE_TESTDRIVE_START,
    SEQ_STATE_TESTDRIVE,
    SEQ_STATE_IDLE,
    SEQ_STATE_DELAY,
    SEQ_STATE_RUN_ONCE,
    SEQ_STATE_UNDEFINED = 0xFF
} SEQUENCE_STATE_T;

typedef struct
{
    SEQUENCE_STATE_T                State;
    const DIM_BMM_DIM_PARAMETER_T   *pDimParameter;
    uint8_t                         ActiveEntry;
    const SEQUENCE_T *              pSequence;
    uint32_t                        uSequenceOverallTime;
    uint32_t                        uSequenceDelay;
    uint32_t                        uTestDriveStepTimeMs;
    SEQUENCE_STATE_T                StateRequest;
} SEQUENCE_LOCAL_T;


/*-------------- Local Variables ------------*/

static SEQUENCE_LOCAL_T sSequence;
static volatile xSemaphoreHandle xSequenceBSemaphore = NULL;
static TIMER_T sequencetimer;



/*-------------- Local Functions ------------*/

bool seq_TestDrive(bool bInit);
void seq_Disable(void);
void seq_Enable(void);
void seq_BMMCall(void);

void seq_Disable(void)
{
    sSequence.StateRequest = SEQ_STATE_STOP;
}

void seq_Enable(void)
{
    sSequence.StateRequest = SEQ_STATE_IDLE;
}

#ifdef HWP_LOCAL_LEDS
void seq_FlatOnTimeoutCallback(uint32_t parameter)
{
    DIM_Reset();
    seq_Enable();
}

/*-------------- Global Functions ------------*/

void SEQUENCE_RTC_TickHandlerFromISR(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if (xSequenceBSemaphore != NULL)
    {
        xSemaphoreGiveFromISR(xSequenceBSemaphore, &xHigherPriorityTaskWoken);
        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
    }
}

void SEQUENCE_FlatOn(uint16_t timeout)
{
    TIMER_Stop(&sequencetimer);
    sequencetimer.callback = seq_FlatOnTimeoutCallback;
    sequencetimer.callback_parameter = 0;
    sequencetimer.interval = timeout * 1000;
    sequencetimer.mode = TIMER_ONE_SHOT_MODE;
    TIMER_Start(&sequencetimer);
    seq_Disable();
}
#endif


void SEQUENCE_KickStart(void)
{
    // Start blink sequence as fast as possible.
    seq_BMMCall();
    SEQUENCE_Init();
    sSequence.StateRequest = SEQ_STATE_RUN_ONCE;
}

void SEQUENCE_Init(void)
{
    TIMER_Stop(&sequencetimer);
    DIM_Reset();
    sSequence.StateRequest = SEQ_STATE_IDLE;
}


void seq_BMMCall(void)
{
    const uint16_t *reduction;
    bool bLedsAreAlwaysOn = TRUE;
    BMM_GetActualProfile(&sSequence.pSequence, &sSequence.pDimParameter, &reduction);
    sSequence.uSequenceOverallTime = 0;
    for (int i=0; i < sSequence.pSequence->length; i++)
    {
        sSequence.uSequenceOverallTime += sSequence.pSequence->table[i].timespan;
        if ((sSequence.pSequence->table[i].dim_value == 0) && (sSequence.pSequence->table[i].timespan >= (SEQUENCE_GLOOM_SENSOR_CAPTURE_TIME)))
            bLedsAreAlwaysOn = FALSE;
    }

    if ((sSequence.uSequenceOverallTime % 1000) != 0)
    {
        // avoid cutting off parts of the sequence
        sSequence.uSequenceOverallTime /= 1000;
        sSequence.uSequenceOverallTime++;
    }
    else
        sSequence.uSequenceOverallTime /= 1000;

    if (sSequence.uSequenceOverallTime > 0)
        sSequence.uSequenceDelay = ((sSequence.uSequenceOverallTime * 1000) - (sSequence.pSequence->table[0].dim_time / 2) + sSequence.pSequence->delay) % (sSequence.uSequenceOverallTime * 1000);
    else
        sSequence.uSequenceDelay = 0;

#ifdef HWP_LOCAL_LEDS
    DIM_SetParameter(sSequence.pDimParameter, reduction, bLedsAreAlwaysOn);
#endif
#ifdef HWP_CAN_MASTER
#ifdef HWP_LED_WHITE_100K
    CAN_MANAGER_SetSlavePWM(sSequence.pDimParameter->led_powerlevel[DIM_LED_WHITE]);
    bLedsAreAlwaysOn = bLedsAreAlwaysOn;
#endif
#endif
}

void SEQUENCE_TestDriveStart(uint32_t uStepTimeMs)
{
    // todo [medium] change parameter / add new parameter to select different operation modes (led sequence and time)
    sSequence.uTestDriveStepTimeMs = uStepTimeMs;
    sSequence.StateRequest = SEQ_STATE_TESTDRIVE_START;
}

void SEQUENCE_TestDriveStartBlocking(uint32_t uStepTimeMs)
{
    SEQUENCE_TestDriveStart(uStepTimeMs);

    while (((sSequence.StateRequest == SEQ_STATE_TESTDRIVE_START)
            ||(sSequence.State == SEQ_STATE_TESTDRIVE_START)
            ||(sSequence.State == SEQ_STATE_TESTDRIVE)))
        vTaskDelay(10);
}

bool seq_TestDrive(bool bInit)
{
    static uint8_t ledchannel;
    DIM_LED_POWERLEVEL powerlevel[DIM_LED_COUNT];

    if (bInit == TRUE)
    {
        DIM_Reset();
        ledchannel = 0;
        return FALSE;
    }

    for (uint8_t ch = 0; ch < DIM_LED_COUNT; ch++)
        powerlevel[ch] = 0;


    if (ledchannel < DIM_LED_COUNT)
    {
        powerlevel[ledchannel] = SEQUENCE_TESTDRIVE_LEDPOWER;
        DIM_SetNow(powerlevel, FALSE, FALSE);
        ledchannel++;
        return FALSE;
    }

    DIM_Reset();
    return TRUE;
}


void SEQUENCE_Task(void *pvParameters)
{
    sSequence.StateRequest = SEQ_STATE_UNDEFINED;
    sSequence.State = SEQ_STATE_IDLE;

    uint8_t last_state = 0;
    bool bStateIsNew = FALSE;

    vSemaphoreCreateBinary(xSequenceBSemaphore);

    // at initialization run sequence once.
    seq_BMMCall();

    SEQUENCE_Init();

    // at initialization run the test sequence
    SEQUENCE_TestDriveStart(2000);

    while(1)
    {
        WATCHDOG_Heartbeat(WATCHDOG_TASK_ID_SEQUENCE);

        if (sSequence.StateRequest != SEQ_STATE_UNDEFINED)
        {
            sSequence.State = sSequence.StateRequest;
            sSequence.StateRequest = SEQ_STATE_UNDEFINED;
        }

        if (sSequence.State != last_state)
        {
            last_state = sSequence.State;
            bStateIsNew = TRUE;
        }
        else
        {
            bStateIsNew = FALSE;
        }

        switch (sSequence.State)
        {
        case SEQ_STATE_STOP:
            vTaskDelay(100);
            // Refresh output power levels e.g. recalculate temperature compensation
            DIM_SetNowRefresh();
            break;
        case SEQ_STATE_TESTDRIVE_START:
            seq_TestDrive(TRUE);
            sSequence.State = SEQ_STATE_TESTDRIVE;
            break;
        case SEQ_STATE_TESTDRIVE:
            if (seq_TestDrive(FALSE))
            {
                xSemaphoreTake(xSequenceBSemaphore, 0); // clear any pending ticks
                sSequence.State = SEQ_STATE_IDLE;
                break;
            }
            vTaskDelay(sSequence.uTestDriveStepTimeMs);
            break;
        case SEQ_STATE_IDLE:
            sSequence.ActiveEntry = 0;
            seq_BMMCall();

            if (pdTRUE == xSemaphoreTake(xSequenceBSemaphore, WATCHDOG_TASK_WAIT_TIME_MS))
            {
                // Start sequences with 0 duration every tick. This is necessary for compensation recalculations
                if (sSequence.uSequenceOverallTime == 0)
                    sSequence.State = SEQ_STATE_RUN_ONCE;
                // Start sequences with no delay on multiplies of overall time
                else if ((sSequence.pSequence->delay == 0) && (RTC_GetUnixTime() % sSequence.uSequenceOverallTime == 0))
                    sSequence.State = SEQ_STATE_RUN_ONCE;
                // Sequences with delay have to delayed for full seconds now and fractions in second step.
                else if ((RTC_GetUnixTime() - (sSequence.uSequenceDelay / 1000)) % sSequence.uSequenceOverallTime == 0)
                    sSequence.State = SEQ_STATE_DELAY;
            }
            break;
        case SEQ_STATE_DELAY:
            vTaskDelay((portTickType)sSequence.uSequenceDelay % 1000);
            sSequence.State = SEQ_STATE_RUN_ONCE;
            break;
        case SEQ_STATE_RUN_ONCE:
        {
            if (bStateIsNew)
                MEASUREPORT_SendSync();

            if (sSequence.pSequence->length > 0 && sSequence.ActiveEntry + 1  < sSequence.pSequence->length)
            {
                //set dim.
#ifdef HWP_LOCAL_LEDS
                DIM_Set(sSequence.pSequence->table[sSequence.ActiveEntry].dim_value, sSequence.pSequence->table[sSequence.ActiveEntry].dim_time);
#endif
#ifdef HWP_CAN_MASTER
                if (sSequence.pSequence->table[sSequence.ActiveEntry].dim_value > 0)
                    CAN_MANAGER_StartSlaveSequence(sSequence.pSequence->table[sSequence.ActiveEntry].timespan);
#endif
                vTaskDelay((portTickType)sSequence.pSequence->table[sSequence.ActiveEntry].timespan);
                sSequence.ActiveEntry++;
            }
            else
            {
                // set dim according to last entry and prepare for next start.
#ifdef HWP_LOCAL_LEDS
                DIM_Set(sSequence.pSequence->table[sSequence.ActiveEntry].dim_value, sSequence.pSequence->table[sSequence.ActiveEntry].dim_time);
#endif
                sSequence.State = SEQ_STATE_IDLE;
            }
        }
        break;
        default:
            sSequence.State = SEQ_STATE_IDLE;
            break;
        }
    }
}

