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
}

/* Flash file emulation. */
NVMFileDriver nvmfile_extfram;

/* Flash mirror. */
NVMMirrorDriver nvmmirror_test;

static const NVMFileConfig nvmfilecfg_extfram =
{
    // file_name
    "extfram.bin",
    // sector_size
    512,
    // sector_num
    64,
};

static const NVMMirrorConfig nvmmirrorcfg_test =
{
    // nvmp
    (BaseNVMDevice*)&nvmfile_extfram,
    // sector_header_num
    1,
};

class NVMMirror: public ::testing::Test
{
protected:

    virtual void SetUp()
    {
        nvmfileObjectInit(&nvmfile_extfram);
        nvmmirrorObjectInit(&nvmmirror_test);

        nvmfileStart(&nvmfile_extfram, &nvmfilecfg_extfram);
        nvmmirrorStart(&nvmmirror_test, &nvmmirrorcfg_test);
    }

    virtual void TearDown()
    {
        nvmmirrorSync(&nvmmirror_test);
        nvmmirrorStop(&nvmmirror_test);

        nvmfileSync(&nvmfile_extfram);
        nvmfileStop(&nvmfile_extfram);
    }
};

TEST_F(NVMMirror, nvmmirrorGetInfo)
{
    NVMDeviceInfo info;
    EXPECT_EQ(nvmmirrorGetInfo(&nvmmirror_test, &info), HAL_SUCCESS);

    EXPECT_EQ(info.sector_size, nvmfile_extfram.config->sector_size);

    EXPECT_EQ(info.sector_num, (nvmfile_extfram.config->sector_num - nvmmirror_test.config->sector_header_num) / 2);
}

TEST_F(NVMMirror, nvmmirrorErase)
{
    EXPECT_EQ(nvmmirrorErase(&nvmmirror_test, 0, nvmfilecfg_extfram.sector_size), HAL_SUCCESS);
}

TEST_F(NVMMirror, nvmmirrorWrite)
{
    uint8_t buffer[nvmfilecfg_extfram.sector_size];

    EXPECT_EQ(nvmmirrorWrite(&nvmmirror_test, 0, sizeof(buffer), buffer), HAL_SUCCESS);
}

TEST_F(NVMMirror, nvmmirrorRead)
{
    uint8_t buffer[nvmfilecfg_extfram.sector_size];

    EXPECT_EQ(nvmmirrorRead(&nvmmirror_test, 0, sizeof(buffer), buffer), HAL_SUCCESS);
}

TEST_F(NVMMirror, EraseWriteReadVerify)
{
    NVMDeviceInfo info;
    EXPECT_EQ(nvmmirrorGetInfo(&nvmmirror_test, &info), HAL_SUCCESS);

    uint8_t buffer[info.sector_size];

    uint32_t addr = 0;

    while (true)
    {
        uint32_t block_length = info.sector_size;
        if (addr + block_length > info.sector_num * info.sector_size)
            block_length = info.sector_num * info.sector_size - addr;

        uint8_t pattern = 0xa5 ^ (addr / block_length);
        pattern = 0;

        for (size_t i = 0; i < block_length; ++i)
            buffer[i] = pattern ^ i;

        EXPECT_EQ(nvmErase(&nvmmirror_test, addr, block_length), HAL_SUCCESS);

        EXPECT_EQ(nvmWrite(&nvmmirror_test, addr, block_length, buffer), HAL_SUCCESS);

        EXPECT_EQ(nvmRead(&nvmmirror_test, addr, block_length, buffer), HAL_SUCCESS);

        for (size_t i = 0; i < block_length; ++i)
            EXPECT_EQ((uint8_t)(pattern ^ i), buffer[i]);

        // go to next block
        addr += block_length;
        if (addr >= info.sector_size * info.sector_num)
        {
            break;
        }
    }
}


