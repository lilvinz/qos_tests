#include "gtest/gtest.h"

extern "C"
{
#include "ch.h"
#include "qhal.h"
#include "qchprintf.h"
}

static SerialVirtualDriver sdvirtual_a;
static SerialVirtualDriver sdvirtual_b;
static SerialVirtualDriver sdvirtual_loop;

static const SerialVirtualConfig sdvirtual_a_cfg =
{
    /* Pointer to the far end. */
    &sdvirtual_b,
};

static const SerialVirtualConfig sdvirtual_b_cfg =
{
    /* Pointer to the far end. */
    &sdvirtual_a,
};

static const SerialVirtualConfig sdvirtual_loop_cfg =
{
    /* Pointer to the far end. */
    &sdvirtual_loop,
};

class SerialVirtual: public ::testing::Test
{
protected:

    virtual void SetUp()
    {
        qhalInit();
        chSysInit();

        sdvirtualObjectInit(&sdvirtual_a);
        sdvirtualStart(&sdvirtual_a, &sdvirtual_a_cfg);

        sdvirtualObjectInit(&sdvirtual_b);
        sdvirtualStart(&sdvirtual_b, &sdvirtual_b_cfg);

        sdvirtualObjectInit(&sdvirtual_loop);
        sdvirtualStart(&sdvirtual_loop, &sdvirtual_loop_cfg);
    }

    virtual void TearDown()
    {
        sdvirtualStop(&sdvirtual_loop);
        sdvirtualStop(&sdvirtual_b);
        sdvirtualStop(&sdvirtual_a);

        chSysDisable();
    }
};

TEST_F(SerialVirtual, test_a_to_b)
{
    qchprintf((BaseSequentialStream*)&sdvirtual_a, "Test234\n");

    char temp[200];
    memset(temp, 0, sizeof(temp));

    chnReadTimeout(&sdvirtual_b, (uint8_t*)temp, sizeof(temp), TIME_IMMEDIATE);

    EXPECT_STREQ("Test234\n", temp);
}

TEST_F(SerialVirtual, test_b_to_a)
{
    qchprintf((BaseSequentialStream*)&sdvirtual_b, "Test234\n");

    char temp[200];
    memset(temp, 0, sizeof(temp));

    chnReadTimeout(&sdvirtual_a, (uint8_t*)temp, sizeof(temp), TIME_IMMEDIATE);

    EXPECT_STREQ("Test234\n", temp);
}

TEST_F(SerialVirtual, test_loop)
{
    qchprintf((BaseSequentialStream*)&sdvirtual_loop, "Test234\n");

    char temp[200];
    memset(temp, 0, sizeof(temp));

    chnReadTimeout(&sdvirtual_loop, (uint8_t*)temp, sizeof(temp),
            TIME_IMMEDIATE);

    EXPECT_STREQ("Test234\n", temp);
}

/*
 * Worker thread moving data from sdvirtual_a to sdvirtual_b and back.
 */
static msg_t loop_a_pump_worker(void *arg)
{
    (void) arg;

    chRegSetThreadName("loop_a_pump_worker");

    EventListener listener_b;
    static const uint32_t event_b_id = 1;
    chEvtRegister(chnGetEventSource((BaseAsynchronousChannel*)&sdvirtual_b),
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
                        (BaseAsynchronousChannel*)&sdvirtual_b, TIME_IMMEDIATE))
                        >= Q_OK)
                {
                    if (c == 'q')
                    {
                        chEvtUnregister(
                                chnGetEventSource((BaseAsynchronousChannel*)&sdvirtual_b),
                                &listener_b);

                        return 0;
                    }
                    else
                    {
                        chSequentialStreamPut((BaseSequentialStream*)&sdvirtual_b, c);
                    }
                }
            }
        }
    }
}

TEST_F(SerialVirtual, test_events)
{
    static WORKING_AREA(wa_loop_a_pump_worker, 1024);
    Thread* loop_a_pumpp = chThdCreateStatic(wa_loop_a_pump_worker,
            sizeof(wa_loop_a_pump_worker),
            HIGHPRIO, loop_a_pump_worker, NULL);

    /* Force context switch to give loop_a_pump a chance to register events. */
    chSysLock();
    chSchRescheduleS();
    chSysUnlock();

    qchprintf((BaseSequentialStream*)&sdvirtual_a, "Test234\n");

    char temp[200];
    memset(temp, 0, sizeof(temp));

    chnReadTimeout(&sdvirtual_a, (uint8_t*)temp, 8, S2ST(1));

    EXPECT_STREQ("Test234\n", temp);

    qchprintf((BaseSequentialStream*)&sdvirtual_a, "q");

    /* Wait for thread to exit. */
    chThdWait(loop_a_pumpp);
}
