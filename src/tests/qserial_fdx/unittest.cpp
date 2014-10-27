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
    (BaseAsynchronousChannel*)&sdvirtual_a,
    SFDXD_MASTER,
};

static const SerialFdxConfig sdfdx_slave_cfg =
{
    /* Pointer to the far end. */
	(BaseAsynchronousChannel*)&sdvirtual_b,
    SFDXD_SLAVE,
};

class SerialFdx: public ::testing::Test
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

        sfdxdObjectInit(&sdfdx_master);
		sfdxdStart(&sdfdx_master, &sdfdx_master_cfg);

		sfdxdObjectInit(&sdfdx_slave);
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
    qchprintf((BaseSequentialStream*)&sdfdx_master, "Test234\n");

    char temp[200];
    memset(temp, 0, sizeof(temp));

    chnReadTimeout(&sdfdx_slave, (uint8_t*)temp, sizeof(temp), S2ST(1));

    EXPECT_STREQ("Test234\n", temp);
}

TEST_F(SerialFdx, test_b_to_a)
{
    qchprintf((BaseSequentialStream*)&sdfdx_slave, "Test234\n");

    char temp[200];
    memset(temp, 0, sizeof(temp));

    chnReadTimeout(&sdfdx_master, (uint8_t*)temp, sizeof(temp), S2ST(1));

    EXPECT_STREQ("Test234\n", temp);
}

TEST_F(SerialFdx, test_a_to_b_over_mtu)
{
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
static msg_t loop_a_pump_worker(void *arg)
{
    (void) arg;

    chRegSetThreadName("loop_a_pump_worker");

    EventListener listener_slave;
    static const uint32_t event_b_id = 1;
    chEvtRegister(chnGetEventSource((BaseAsynchronousChannel*)&sdfdx_slave),
            &listener_slave, event_b_id);

    while (true)
    {
        uint32_t events = chEvtWaitOne(EVENT_MASK(event_b_id));

        if (events & EVENT_MASK(event_b_id))
        {
            flagsmask_t flags = chEvtGetAndClearFlags(&listener_slave);
            if (flags & CHN_INPUT_AVAILABLE)
            {
                msg_t c;
                while ((c = chnGetTimeout(
                        (BaseAsynchronousChannel*)&sdfdx_slave, TIME_IMMEDIATE))
                        >= Q_OK)
                {
                    if (c == 'q')
                    {
                        chEvtUnregister(
                                chnGetEventSource((BaseAsynchronousChannel*)&sdfdx_slave),
                                &listener_slave);

                        return 0;
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
    static WORKING_AREA(wa_loop_a_pump_worker, 1024);
    Thread* loop_a_pumpp = chThdCreateStatic(wa_loop_a_pump_worker,
            sizeof(wa_loop_a_pump_worker),
            HIGHPRIO, loop_a_pump_worker, NULL);

    /* Force context switch to give loop_a_pump a chance to register events. */
    chSysLock();
    chSchRescheduleS();
    chSysUnlock();

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
 * Worker thread moving data from sdfdx_a to sdfdx_b and back.
 */
static msg_t connectionevents_a_pump_worker(void *arg)
{
    (void) arg;

    chRegSetThreadName("connectionevents_a_pump_worker");

    EventListener listener_slave;
    static const uint32_t event_b_id = 1;
    chEvtRegister(chnGetEventSource((BaseAsynchronousChannel*)&sdfdx_slave),
            &listener_slave, event_b_id);

    while (true)
    {
        uint32_t events = chEvtWaitOneTimeout(EVENT_MASK(event_b_id), S2ST(2));

        if (events & EVENT_MASK(event_b_id))
        {
            flagsmask_t flags = chEvtGetAndClearFlags(&listener_slave);
            if (flags & CHN_CONNECTED)
            {
            	 qchprintf((BaseSequentialStream*)&sdfdx_slave, "Test234\n");
            	 chEvtUnregister(
					 chnGetEventSource((BaseAsynchronousChannel*)&sdfdx_slave),
					 &listener_slave);

            	 return 0;
            }
        }
    }
}
TEST_F(SerialFdx, test_connected_event)
{
	static WORKING_AREA(wa_connectionevents_a_pump_worker, 1024);
	Thread* loop_a_pumpp = chThdCreateStatic(wa_connectionevents_a_pump_worker,
			sizeof(wa_connectionevents_a_pump_worker),
			HIGHPRIO, connectionevents_a_pump_worker, NULL);

	/* Force context switch to give loop_a_pump a chance to register events. */
	chSysLock();
	chSchRescheduleS();
	chSysUnlock();

	char temp[200];
	memset(temp, 0, sizeof(temp));

	chnReadTimeout(&sdfdx_master, (uint8_t*)temp, 8, S2ST(5));

	EXPECT_STREQ("Test234\n", temp);

	/* Wait for thread to exit. */
	chThdWait(loop_a_pumpp);
}

TEST_F(SerialFdx, test_connected)
{
	chThdSleepMilliseconds(1000);
	EXPECT_TRUE(sdfdx_master.connected);
	EXPECT_TRUE(sdfdx_slave.connected);

	//stop slave to initiate a disconnect
	//master should handle timeout
	sfdxdStop(&sdfdx_slave);
	chThdSleepMilliseconds(2000);
	EXPECT_FALSE(sdfdx_master.connected);
	EXPECT_FALSE(sdfdx_slave.connected);

	//start slave again to reconnect
	sfdxdStart(&sdfdx_slave, &sdfdx_slave_cfg);

	chThdSleepMilliseconds(2000);
	EXPECT_TRUE(sdfdx_master.connected);
	EXPECT_TRUE(sdfdx_slave.connected);

}

TEST_F(SerialFdx, test_escaping)
{
	char src[4];
	src[0] = SFDX_FRAME_BEGIN;
	src[1] = SFDX_FRAME_END;
	src[2] = SFDX_BYTE_ESC;
	src[3] = 0;

	qchprintf((BaseSequentialStream*)&sdfdx_master, src);

	char temp[200];
	memset(temp, 0, sizeof(temp));

	chnReadTimeout(&sdfdx_slave, (uint8_t*)temp, sizeof(temp), S2ST(1));

	EXPECT_STREQ(src, temp);


}
