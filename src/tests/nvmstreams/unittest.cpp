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
#include "nvmstreams.h"
#include "chprintf.h"
}

/* Flash memory emulation. */
uint8_t _nvmmemory_buf[512 * 2];
static const NVMMemoryConfig _nvmmemory_cfg =
{
    // buffer
    _nvmmemory_buf,
    // sector_size
    512,
    // sector_num
    2,
};

class Test_NVMStream: public ::testing::Test
{
protected:
    /* Flash memory emulation. */
    NVMMemoryDriver _nvmmemory;
    NVMStream _nvmstream;

    virtual void SetUp()
    {
        halInit();

        nvmmemoryObjectInit(&_nvmmemory);

        chSysInit();

        nvmmemoryStart(&_nvmmemory, &_nvmmemory_cfg);

        nvmsObjectInit(&_nvmstream, (BaseNVMDevice*)&_nvmmemory, 0);
    }

    virtual void TearDown()
    {
        nvmmemorySync(&_nvmmemory);
        nvmmemoryStop(&_nvmmemory);

        chSysDisable();
    }
};

TEST_F(Test_NVMStream, chprintf)
{
    BaseSequentialStream *streamp = (BaseSequentialStream*)&_nvmstream;

    chprintf(streamp, "Test234\0");

    EXPECT_STREQ("Test234", (const char*)_nvmmemory_buf);
}

TEST_F(Test_NVMStream, chSequentialStreamWrite)
{
    BaseSequentialStream *streamp = (BaseSequentialStream*)&_nvmstream;

    chSequentialStreamWrite(streamp, (const uint8_t*)"Test234\0", 8);

    EXPECT_STREQ("Test234", (const char*)_nvmmemory_buf);
}

TEST_F(Test_NVMStream, chSequentialStreamPut)
{
    BaseSequentialStream *streamp = (BaseSequentialStream*)&_nvmstream;

    chSequentialStreamPut(streamp, 'T');
    chSequentialStreamPut(streamp, 'e');
    chSequentialStreamPut(streamp, 's');
    chSequentialStreamPut(streamp, 't');
    chSequentialStreamPut(streamp, '2');
    chSequentialStreamPut(streamp, '3');
    chSequentialStreamPut(streamp, '4');
    chSequentialStreamPut(streamp, '\0');

    EXPECT_STREQ("Test234", (const char*)_nvmmemory_buf);
}

TEST_F(Test_NVMStream, multiple_chprintf)
{
    BaseSequentialStream *streamp = (BaseSequentialStream*)&_nvmstream;

    chprintf(streamp, "Test234");
    chprintf(streamp, "ZweiterTest");

    EXPECT_STREQ("Test234ZweiterTest", (const char*)_nvmmemory_buf);
}

TEST_F(Test_NVMStream, overflow)
{
    BaseSequentialStream *streamp = (BaseSequentialStream*)&_nvmstream;

    for (size_t i = 0; i < sizeof(_nvmmemory_buf); ++i)
        EXPECT_EQ(chSequentialStreamPut(streamp, (uint8_t)i), Q_OK);

    EXPECT_EQ(chSequentialStreamPut(streamp, 0x00), Q_RESET);
}

TEST_F(Test_NVMStream, chSequentialStreamRead)
{
    BaseSequentialStream *streamp = (BaseSequentialStream*)&_nvmstream;
    uint8_t temp[8];

    chSequentialStreamWrite(streamp, (const uint8_t*)"Test234\0", 8);

    chSequentialStreamRead(streamp, temp, 8);

    chSequentialStreamWrite(streamp, (const uint8_t*)"Test567\0", 8);

    chSequentialStreamRead(streamp, temp, 8);

    EXPECT_STREQ((const char*)temp, (const char*)_nvmmemory_buf + 8);
}
