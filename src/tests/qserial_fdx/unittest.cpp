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

static SerialFdxDriver sdfdx_master;
static SerialFdxDriver sdfdx_slave;

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

static const SerialFdxConfig sdfdx_master_cfg =
{
    /* Pointer to the far end. */
    (BaseAsynchronousChannel*)&sdvirtual_a, SFDXD_MASTER,
};

static const SerialFdxConfig sdfdx_slave_cfg =
{
    /* Pointer to the far end. */
    (BaseAsynchronousChannel*)&sdvirtual_b, SFDXD_SLAVE,
};

class SerialFdx: public ::testing::Test
{
protected:

    virtual void SetUp()
    {
        halInit();

        sdvirtualObjectInit(&sdvirtual_a);
        sdvirtualObjectInit(&sdvirtual_b);
        sfdxdObjectInit(&sdfdx_master);
        sfdxdObjectInit(&sdfdx_slave);

        chSysInit();

        sdvirtualStart(&sdvirtual_a, &sdvirtual_a_cfg);
        sdvirtualStart(&sdvirtual_b, &sdvirtual_b_cfg);
        sfdxdStart(&sdfdx_master, &sdfdx_master_cfg);
        sfdxdStart(&sdfdx_slave, &sdfdx_slave_cfg);
    }

    virtual void TearDown()
    {
        sfdxdStop(&sdfdx_slave);
        sfdxdStop(&sdfdx_master);

        sdvirtualStop(&sdvirtual_b);
        sdvirtualStop(&sdvirtual_a);

        chSysDisable();
    }
};

TEST_F(SerialFdx, test_a_to_b)
{
    /* wait to make sure master and slave are connected */
    chThdSleepMilliseconds(10);
    EXPECT_TRUE(sfdxdConnected(&sdfdx_master));
    EXPECT_TRUE(sfdxdConnected(&sdfdx_slave));

    qchprintf((BaseSequentialStream*)&sdfdx_master, "Test234\n");

    char temp[200];
    memset(temp, 0, sizeof(temp));

    chnReadTimeout(&sdfdx_slave, (uint8_t*)temp, sizeof(temp), S2ST(1));

    EXPECT_STREQ("Test234\n", temp);
}

TEST_F(SerialFdx, test_b_to_a)
{
    /* wait to make sure master and slave are connected */
    chThdSleepMilliseconds(10);
    EXPECT_TRUE(sfdxdConnected(&sdfdx_master));
    EXPECT_TRUE(sfdxdConnected(&sdfdx_slave));

    qchprintf((BaseSequentialStream*)&sdfdx_slave, "Test234\n");

    char temp[200];
    memset(temp, 0, sizeof(temp));

    chnReadTimeout(&sdfdx_master, (uint8_t*)temp, sizeof(temp), S2ST(1));

    EXPECT_STREQ("Test234\n", temp);
}

TEST_F(SerialFdx, test_a_to_b_over_mtu)
{
    /* wait to make sure master and slave are connected */
    chThdSleepMilliseconds(10);
    EXPECT_TRUE(sfdxdConnected(&sdfdx_master));
    EXPECT_TRUE(sfdxdConnected(&sdfdx_slave));

    char src[SERIAL_FDX_MTU * 2];
    memset((void*)src, 'a', sizeof(src) - 1);
    src[sizeof(src) - 2] = '\n';
    src[sizeof(src) - 1] = 0;

    qchprintf((BaseSequentialStream*)&sdfdx_master, src);

    char temp[SERIAL_FDX_MTU * 3];
    memset(temp, 0, sizeof(temp));

    chnReadTimeout(&sdfdx_slave, (uint8_t*)temp, sizeof(temp), S2ST(1));

    EXPECT_STREQ(src, temp);
}

/*
 * Worker thread moving data from sdfdx_a to sdfdx_b and back.
 */
static void loop_a_pump_worker(void *arg)
{
    (void) arg;

    chRegSetThreadName("loop_a_pump_worker");

    event_listener_t listener_slave;
    static const uint32_t event_b_id = 1;
    chEvtRegister(chnGetEventSource((BaseAsynchronousChannel*)&sdfdx_slave),
            &listener_slave, event_b_id);

    while (true)
    {
        uint32_t events = chEvtWaitOne(EVENT_MASK(event_b_id));

        if (events & EVENT_MASK(event_b_id))
        {
            eventflags_t flags = chEvtGetAndClearFlags(&listener_slave);
            if (flags & CHN_INPUT_AVAILABLE)
            {
                msg_t c;
                while ((c = chnGetTimeout((BaseAsynchronousChannel*)&sdfdx_slave, TIME_IMMEDIATE)) >= Q_OK)
                {
                    if (c == 'q')
                    {
                        chEvtUnregister(chnGetEventSource((BaseAsynchronousChannel*)&sdfdx_slave),
                                &listener_slave);

                        return;
                    }
                    else
                    {
                        chnPutTimeout((BaseAsynchronousChannel*)&sdfdx_slave, c, S2ST(1));
                    }
                }
            }
        }
    }
}

TEST_F(SerialFdx, test_events)
{
    static THD_WORKING_AREA(wa_loop_a_pump_worker, 1024);
    thread_reference_t loop_a_pumpp = chThdCreateStatic(wa_loop_a_pump_worker,
            sizeof(wa_loop_a_pump_worker),
            HIGHPRIO, loop_a_pump_worker, NULL);

    /* wait to make sure master and slave are connected */
    chThdSleepMilliseconds(10);
    EXPECT_TRUE(sfdxdConnected(&sdfdx_master));
    EXPECT_TRUE(sfdxdConnected(&sdfdx_slave));

    qchprintf((BaseSequentialStream*)&sdfdx_master, "Test234\n");

    char temp[200];
    memset(temp, 0, sizeof(temp));

    chnReadTimeout(&sdfdx_master, (uint8_t*)temp, 8, S2ST(1));

    EXPECT_STREQ("Test234\n", temp);

    qchprintf((BaseSequentialStream*)&sdfdx_master, "q");

    /* Wait for thread to exit. */
    chThdWait(loop_a_pumpp);
}

/*
 * Worker thread to send a message after a connection event occurred
 */
static void connectionevents_a_pump_worker(void *arg)
{
    (void)arg;

    chRegSetThreadName("connectionevents_a_pump_worker");

    event_listener_t listener_slave;
    static const uint32_t event_b_id = 1;
    chEvtRegister(chnGetEventSource((BaseAsynchronousChannel*)&sdfdx_slave),
            &listener_slave, event_b_id);

    while (true)
    {
        uint32_t events = chEvtWaitOneTimeout(EVENT_MASK(event_b_id), S2ST(2));

        if (events & EVENT_MASK(event_b_id))
        {
            eventflags_t flags = chEvtGetAndClearFlags(&listener_slave);
            if (flags & CHN_CONNECTED)
            {
                qchprintf((BaseSequentialStream*)&sdfdx_slave, "Test234\n");
                chEvtUnregister(chnGetEventSource((BaseAsynchronousChannel*)&sdfdx_slave),
                        &listener_slave);

                return;
            }
        }
    }
}

TEST_F(SerialFdx, test_connected_event)
{
    static THD_WORKING_AREA(wa_connectionevents_a_pump_worker, 1024);
    thread_reference_t loop_a_pumpp = chThdCreateStatic(wa_connectionevents_a_pump_worker,
            sizeof(wa_connectionevents_a_pump_worker),
            HIGHPRIO, connectionevents_a_pump_worker, NULL);

    /* wait to make sure master and slave are connected */
    chThdSleepMilliseconds(10);
    EXPECT_TRUE(sfdxdConnected(&sdfdx_master));
    EXPECT_TRUE(sfdxdConnected(&sdfdx_slave));

    char temp[200];
    memset(temp, 0, sizeof(temp));

    chnReadTimeout(&sdfdx_master, (uint8_t*)temp, 8, MS2ST(1));

    EXPECT_STREQ("Test234\n", temp);

    /* Wait for thread to exit. */
    chThdWait(loop_a_pumpp);
}

TEST_F(SerialFdx, test_connected)
{
    /* wait to make sure master and slave are connected */
    chThdSleepMilliseconds(10);
    EXPECT_TRUE(sfdxdConnected(&sdfdx_master));
    EXPECT_TRUE(sfdxdConnected(&sdfdx_slave));

    /*
     * stop slave to initiate a disconnect
     * master should handle timeout
     */
    sfdxdStop(&sdfdx_slave);
    chThdSleepMilliseconds(2000);
    EXPECT_FALSE(sfdxdConnected(&sdfdx_master));
    EXPECT_FALSE(sfdxdConnected(&sdfdx_slave));

    /* start slave again to reconnect */
    sfdxdStart(&sdfdx_slave, &sdfdx_slave_cfg);

    chThdSleepMilliseconds(10);
    EXPECT_TRUE(sfdxdConnected(&sdfdx_master));
    EXPECT_TRUE(sfdxdConnected(&sdfdx_slave));
}

TEST_F(SerialFdx, test_escaping)
{
    /* wait to make sure master and slave are connected */
    chThdSleepMilliseconds(10);
    EXPECT_TRUE(sfdxdConnected(&sdfdx_master));
    EXPECT_TRUE(sfdxdConnected(&sdfdx_slave));

    /* use special characters as message */
    char src[] = {SFDX_FRAME_BEGIN, SFDX_FRAME_END, SFDX_BYTE_ESC, 0};

    chnWrite(&sdfdx_master, (uint8_t*)src, sizeof(src));

    char temp[200];
    memset(temp, 0, sizeof(temp));

    msg_t rb = chnReadTimeout(&sdfdx_slave, (uint8_t*)temp, sizeof(src), S2ST(4));

    EXPECT_STREQ(src, temp);
    EXPECT_EQ(4, rb);
}
