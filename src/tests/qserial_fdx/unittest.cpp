#include "gtest/gtest.h"

extern "C"
{
#include "ch.h"
#include "qhal.h"
#include "qchprintf.h"
}

static SerialFdxDriver sdfdx_a;
static SerialFdxDriver sdfdx_b;
static SerialFdxDriver sdfdx_loop;

static const SerialFdxConfig sdfdx_a_cfg =
{
    /* Pointer to the far end. */
    &sdfdx_b,
};

static const SerialFdxConfig sdfdx_b_cfg =
{
    /* Pointer to the far end. */
    &sdfdx_a,
};

static const SerialFdxConfig sdfdx_loop_cfg =
{
    /* Pointer to the far end. */
    &sdfdx_loop,
};

class SerialFdx: public ::testing::Test
{
protected:

    virtual void SetUp()
    {
        qhalInit();
        chSysInit();

        sdfdxObjectInit(&sdfdx_a);
        sdfdxStart(&sdfdx_a, &sdfdx_a_cfg);

        sdfdxObjectInit(&sdfdx_b);
        sdfdxStart(&sdfdx_b, &sdfdx_b_cfg);

        sdfdxObjectInit(&sdfdx_loop);
        sdfdxStart(&sdfdx_loop, &sdfdx_loop_cfg);
    }

    virtual void TearDown()
    {
        sdfdxStop(&sdfdx_loop);
        sdfdxStop(&sdfdx_b);
        sdfdxStop(&sdfdx_a);

        chSysDisable();
    }
};

TEST_F(SerialFdx, test_a_to_b)
{
    qchprintf((BaseSequentialStream*)&sdfdx_a, "Test234\n");

    char temp[200];
    memset(temp, 0, sizeof(temp));

    chnReadTimeout(&sdfdx_b, (uint8_t*)temp, sizeof(temp), TIME_IMMEDIATE);

    EXPECT_STREQ("Test234\n", temp);
}

TEST_F(SerialFdx, test_b_to_a)
{
    qchprintf((BaseSequentialStream*)&sdfdx_b, "Test234\n");

    char temp[200];
    memset(temp, 0, sizeof(temp));

    chnReadTimeout(&sdfdx_a, (uint8_t*)temp, sizeof(temp), TIME_IMMEDIATE);

    EXPECT_STREQ("Test234\n", temp);
}

TEST_F(SerialFdx, test_loop)
{
    qchprintf((BaseSequentialStream*)&sdfdx_loop, "Test234\n");

    char temp[200];
    memset(temp, 0, sizeof(temp));

    chnReadTimeout(&sdfdx_loop, (uint8_t*)temp, sizeof(temp),
            TIME_IMMEDIATE);

    EXPECT_STREQ("Test234\n", temp);
}

/*
 * Worker thread moving data from sdfdx_a to sdfdx_b and back.
 */
static msg_t loop_a_pump_worker(void *arg)
{
    (void) arg;

    chRegSetThreadName("loop_a_pump_worker");

    EventListener listener_b;
    static const uint32_t event_b_id = 1;
    chEvtRegister(chnGetEventSource((BaseAsynchronousChannel*)&sdfdx_b),
            &listener_b, event_b_id);

    while (true)
    {
        uint32_t events = chEvtWaitOne(EVENT_MASK(event_b_id));

        if (events & EVENT_MASK(event_b_id))
        {
            flagsmask_t flags = chEvtGetAndClearFlags(&listener_b);
            if (flags & CHN_INPUT_AVAILABLE)
            {
                msg_t c;
                while ((c = chnGetTimeout(
                        (BaseAsynchronousChannel*)&sdfdx_b, TIME_IMMEDIATE))
                        >= Q_OK)
                {
                    if (c == 'q')
                    {
                        chEvtUnregister(
                                chnGetEventSource((BaseAsynchronousChannel*)&sdfdx_b),
                                &listener_b);

                        return 0;
                    }
                    else
                    {
                        chSequentialStreamPut((BaseSequentialStream*)&sdfdx_b, c);
                    }
                }
            }
        }
    }
}

TEST_F(SerialFdx, test_events)
{
    static WORKING_AREA(wa_loop_a_pump_worker, 1024);
    Thread* loop_a_pumpp = chThdCreateStatic(wa_loop_a_pump_worker,
            sizeof(wa_loop_a_pump_worker),
            HIGHPRIO, loop_a_pump_worker, NULL);

    /* Force context switch to give loop_a_pump a chance to register events. */
    chSysLock();
    chSchRescheduleS();
    chSysUnlock();

    qchprintf((BaseSequentialStream*)&sdfdx_a, "Test234\n");

    char temp[200];
    memset(temp, 0, sizeof(temp));

    chnReadTimeout(&sdfdx_a, (uint8_t*)temp, 8, S2ST(1));

    EXPECT_STREQ("Test234\n", temp);

    qchprintf((BaseSequentialStream*)&sdfdx_a, "q");

    /* Wait for thread to exit. */
    chThdWait(loop_a_pumpp);
}
