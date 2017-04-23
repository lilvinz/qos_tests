/*
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

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
        halInit();

        sdvirtualObjectInit(&sdvirtual_a);
        sdvirtualObjectInit(&sdvirtual_b);
        sdvirtualObjectInit(&sdvirtual_loop);

        chSysInit();

        sdvirtualStart(&sdvirtual_a, &sdvirtual_a_cfg);
        sdvirtualStart(&sdvirtual_b, &sdvirtual_b_cfg);
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
static void loop_a_pump_worker(void *arg)
{
    (void) arg;

    chRegSetThreadName("loop_a_pump_worker");

    event_listener_t listener_b;
    static const uint32_t event_b_id = 1;
    chEvtRegister(chnGetEventSource((BaseAsynchronousChannel*)&sdvirtual_b),
            &listener_b, event_b_id);

    while (true)
    {
        uint32_t events = chEvtWaitOne(EVENT_MASK(event_b_id));

        if (events & EVENT_MASK(event_b_id))
        {
            eventflags_t flags = chEvtGetAndClearFlags(&listener_b);
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

                        return;
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
    static THD_WORKING_AREA(wa_loop_a_pump_worker, 1024);
    thread_reference_t loop_a_pumpp = chThdCreateStatic(wa_loop_a_pump_worker,
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

/*
 * Worker thread reading data from sdvirtual_b. Data will be lost.
 */
static void loop_a_read_worker(void *arg)
{
    (void) arg;

    chRegSetThreadName("loop_a_read_worker");

    event_listener_t listener_b;
    static const uint32_t event_b_id = 1;
    chEvtRegister(chnGetEventSource((BaseAsynchronousChannel*)&sdvirtual_b),
            &listener_b, event_b_id);

    size_t* bytes_r = (size_t*)arg;
    while (true)
    {
        uint32_t events = chEvtWaitOne(EVENT_MASK(event_b_id));

        if (events & EVENT_MASK(event_b_id))
        {
            eventflags_t flags = chEvtGetAndClearFlags(&listener_b);
            if (flags & CHN_INPUT_AVAILABLE)
            {
                msg_t c;
                while ((c = chnGetTimeout(
                        (BaseAsynchronousChannel*)&sdvirtual_b, TIME_IMMEDIATE))
                        >= Q_OK)
                {
                    ++(*bytes_r);
                    if (c == 'q')
                    {
                        chEvtUnregister(
                                chnGetEventSource((BaseAsynchronousChannel*)&sdvirtual_b),
                                &listener_b);

                        return;
                    }
                }
            }
        }
    }
}


static void fill_buffer(uint8_t* buffer, int n)
{
    for (int i = 0; i < n; i++)
    {
        buffer[i] = ((i % 10) & 0xff);
    }
}

TEST_F(SerialVirtual, test_write_large_buffer)
{
    size_t bytes_r = 0;
    static THD_WORKING_AREA(wa_loop_a_read_worker, 1024);
    thread_reference_t loop_a_pumpp = chThdCreateStatic(wa_loop_a_read_worker,
            sizeof(wa_loop_a_read_worker),
            HIGHPRIO, loop_a_read_worker, (void*)&bytes_r);

    /* Force context switch to give loop_a_read_worker a chance to register events. */
    chSysLock();
    chSchRescheduleS();
    chSysUnlock();

    /* create a buffer larger as configured serial virutal buffer */
    uint8_t temp[SERIAL_VIRTUAL_BUFFER_SIZE * 2];
    fill_buffer(temp, sizeof(temp));

    size_t bytes_w = chnWriteTimeout(&sdvirtual_a, (uint8_t*)temp, sizeof(temp), TIME_INFINITE);
    EXPECT_EQ(sizeof(temp), bytes_w);


    int print_i = qchprintf((BaseSequentialStream*)&sdvirtual_a, "q");
    EXPECT_EQ(1, print_i);

    /* Wait for thread to exit. */
    chThdWait(loop_a_pumpp);

    EXPECT_EQ(sizeof(temp) + 1, bytes_r);
}
