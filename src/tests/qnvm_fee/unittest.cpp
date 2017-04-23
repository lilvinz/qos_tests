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

static uint8_t memory_test[512 * 64];

/* Flash file emulation. */
NVMMemoryDriver nvmmemory_test;

/* Flash eeprom emulation. */
NVMFeeDriver nvmfee_test;

static const NVMMemoryConfig nvmmemorycfg_test =
{
    // buffer
    memory_test,
    // sector_size
    512,
    // sector_num
    64,
};

static const NVMFeeConfig nvmfeecfg_test =
{
    // nvmp
    (BaseNVMDevice*)&nvmmemory_test,
    // sector_header_num
    1,
};

class NVMFee: public ::testing::Test
{
protected:

    virtual void SetUp()
    {
        memset(memory_test, 0, sizeof(memory_test));

        nvmmemoryObjectInit(&nvmmemory_test);
        nvmfeeObjectInit(&nvmfee_test);

        nvmmemoryStart(&nvmmemory_test, &nvmmemorycfg_test);
        nvmfeeStart(&nvmfee_test, &nvmfeecfg_test);
    }

    virtual void TearDown()
    {
        nvmfeeSync(&nvmfee_test);
        nvmfeeStop(&nvmfee_test);

        nvmmemorySync(&nvmmemory_test);
        nvmmemoryStop(&nvmmemory_test);
    }
};

TEST_F(NVMFee, nvmfeeGetInfo)
{
    NVMDeviceInfo info;
    EXPECT_EQ(nvmfeeGetInfo(&nvmfee_test, &info), HAL_SUCCESS);

    EXPECT_EQ(info.sector_size, (uint32_t)NVM_FEE_SLOT_PAYLOAD_SIZE);

    EXPECT_EQ(info.sector_num,
            ((nvmmemory_test.config->sector_num * nvmmemory_test.config->sector_size / 2) - 32) / (8 + NVM_FEE_SLOT_PAYLOAD_SIZE));
}

TEST_F(NVMFee, nvmfeeMassErase)
{
    EXPECT_EQ(nvmfeeMassErase(&nvmfee_test), HAL_SUCCESS);
}

TEST_F(NVMFee, nvmfeeErase)
{
    const char temp[] = "Dies ist ein Test!";

    EXPECT_EQ(nvmWrite(&nvmfee_test, 0x00000003, sizeof(temp), (const uint8_t*)temp), HAL_SUCCESS);

    EXPECT_EQ(nvmfeeErase(&nvmfee_test, 0x12, 2), HAL_SUCCESS);

    char temp3[sizeof(temp)] = "Dies ist ein Te\xff\xff!";

    char temp2[sizeof(temp)];

    EXPECT_EQ(nvmRead(&nvmfee_test, 0x00000003, sizeof(temp2), (uint8_t*)temp2), HAL_SUCCESS);

    EXPECT_EQ(memcmp(temp2, temp3, sizeof(temp2)), 0);
}

TEST_F(NVMFee, nvmfeeWrite)
{
    uint8_t buffer[nvmmemorycfg_test.sector_size];

    EXPECT_EQ(nvmfeeWrite(&nvmfee_test, 0, sizeof(buffer), buffer), HAL_SUCCESS);
}

TEST_F(NVMFee, nvmfeeRead)
{
    uint8_t buffer[nvmmemorycfg_test.sector_size];

    EXPECT_EQ(nvmfeeRead(&nvmfee_test, 0, sizeof(buffer), buffer), HAL_SUCCESS);
}

TEST_F(NVMFee, EraseWriteReadVerify)
{
    NVMDeviceInfo info;
    EXPECT_EQ(nvmfeeGetInfo(&nvmfee_test, &info), HAL_SUCCESS);

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

        EXPECT_EQ(nvmErase(&nvmfee_test, addr, block_length), HAL_SUCCESS);

        EXPECT_EQ(nvmWrite(&nvmfee_test, addr, block_length, buffer), HAL_SUCCESS);

        EXPECT_EQ(nvmRead(&nvmfee_test, addr, block_length, buffer), HAL_SUCCESS);

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

TEST_F(NVMFee, OutOfBoundsDetection)
{
    NVMDeviceInfo info;
    EXPECT_EQ(nvmfeeGetInfo(&nvmfee_test, &info), HAL_SUCCESS);

    const uint8_t temp = 0xaa;

    EXPECT_EQ(nvmWrite(&nvmfee_test, info.sector_num * info.sector_size - 1, sizeof(temp), &temp), HAL_SUCCESS);
}

TEST_F(NVMFee, ReadWrite)
{
    {
        const uint8_t temp = 0xaa;

        EXPECT_EQ(nvmWrite(&nvmfee_test, 0x00000000, sizeof(temp), &temp), HAL_SUCCESS);
        EXPECT_EQ(nvmWrite(&nvmfee_test, 0x00000001, sizeof(temp), &temp), HAL_SUCCESS);
        EXPECT_EQ(nvmWrite(&nvmfee_test, 0x00000002, sizeof(temp), &temp), HAL_SUCCESS);
        EXPECT_EQ(nvmWrite(&nvmfee_test, 0x00000003, sizeof(temp), &temp), HAL_SUCCESS);
        EXPECT_EQ(nvmWrite(&nvmfee_test, 0x00000004, sizeof(temp), &temp), HAL_SUCCESS);
        EXPECT_EQ(nvmWrite(&nvmfee_test, 0x00000005, sizeof(temp), &temp), HAL_SUCCESS);
        EXPECT_EQ(nvmWrite(&nvmfee_test, 0x00000006, sizeof(temp), &temp), HAL_SUCCESS);
        EXPECT_EQ(nvmWrite(&nvmfee_test, 0x00000007, sizeof(temp), &temp), HAL_SUCCESS);
    }

    {
        const char temp[] = "Dies ist ein Test!";

        EXPECT_EQ(nvmWrite(&nvmfee_test, 0x00000003, sizeof(temp), (const uint8_t*)temp), HAL_SUCCESS);

        char temp2[sizeof(temp)];

        EXPECT_EQ(nvmRead(&nvmfee_test, 0x00000003, sizeof(temp2), (uint8_t*)temp2), HAL_SUCCESS);

        EXPECT_STREQ(temp, temp2);
    }
}

TEST_F(NVMFee, MaximumWrite)
{
    NVMDeviceInfo info;
    EXPECT_EQ(nvmfeeGetInfo(&nvmfee_test, &info), HAL_SUCCESS);

    const uint8_t temp = 0xaa;

    EXPECT_EQ(nvmWrite(&nvmfee_test, info.sector_num * info.sector_size - 1, sizeof(temp), &temp), HAL_SUCCESS);

    for (uint32_t i = 0; i < info.sector_num * info.sector_size; ++i)
        EXPECT_EQ(nvmWrite(&nvmfee_test, i, sizeof(temp), &temp), HAL_SUCCESS);
}

TEST_F(NVMFee, Overwrite)
{
    uint32_t temp;
    uint32_t temp2;

    temp = 9600;
    EXPECT_EQ(nvmWrite(&nvmfee_test, 0x230, sizeof(temp), (const uint8_t*)&temp), HAL_SUCCESS);
    EXPECT_EQ(nvmRead(&nvmfee_test, 0x230, sizeof(temp2), (uint8_t*)&temp2), HAL_SUCCESS);
    EXPECT_EQ(temp, temp2);

    temp = 19200;
    EXPECT_EQ(nvmWrite(&nvmfee_test, 0x230, sizeof(temp), (const uint8_t*)&temp), HAL_SUCCESS);
    EXPECT_EQ(nvmRead(&nvmfee_test, 0x230, sizeof(temp2), (uint8_t*)&temp2), HAL_SUCCESS);
    EXPECT_EQ(temp, temp2);

    temp = 38400;
    EXPECT_EQ(nvmWrite(&nvmfee_test, 0x230, sizeof(temp), (const uint8_t*)&temp), HAL_SUCCESS);
    EXPECT_EQ(nvmRead(&nvmfee_test, 0x230, sizeof(temp2), (uint8_t*)&temp2), HAL_SUCCESS);
    EXPECT_EQ(temp, temp2);
}

TEST_F(NVMFee, Reload)
{
    uint32_t temp;
    uint32_t temp2;

    temp = 9600;
    EXPECT_EQ(nvmWrite(&nvmfee_test, 0x230, sizeof(temp), (const uint8_t*)&temp), HAL_SUCCESS);
    EXPECT_EQ(nvmRead(&nvmfee_test, 0x230, sizeof(temp2), (uint8_t*)&temp2), HAL_SUCCESS);
    EXPECT_EQ(temp, temp2);

    temp = 19200;
    EXPECT_EQ(nvmWrite(&nvmfee_test, 0x230, sizeof(temp), (const uint8_t*)&temp), HAL_SUCCESS);
    EXPECT_EQ(nvmRead(&nvmfee_test, 0x230, sizeof(temp2), (uint8_t*)&temp2), HAL_SUCCESS);
    EXPECT_EQ(temp, temp2);

    temp = 38400;
    EXPECT_EQ(nvmWrite(&nvmfee_test, 0x230, sizeof(temp), (const uint8_t*)&temp), HAL_SUCCESS);
    EXPECT_EQ(nvmRead(&nvmfee_test, 0x230, sizeof(temp2), (uint8_t*)&temp2), HAL_SUCCESS);
    EXPECT_EQ(temp, temp2);

    nvmfeeStop(&nvmfee_test);
    nvmfeeStart(&nvmfee_test, &nvmfeecfg_test);

    temp = 38400;
    EXPECT_EQ(nvmRead(&nvmfee_test, 0x230, sizeof(temp2), (uint8_t*)&temp2), HAL_SUCCESS);
    EXPECT_EQ(temp, temp2);
}
