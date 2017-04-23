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

static const NVMFileConfig nvmfilecfg =
{
    // file_name
    "extflash.bin",
    // sector_size
    1024 * 4,
    // sector_num
    8192,
};

class NVMFile: public ::testing::Test
{
protected:

    virtual void SetUp()
    {
        nvmfileObjectInit(&_nvmfile);
        nvmfileStart(&_nvmfile, &nvmfilecfg);
    }

    virtual void TearDown()
    {
        nvmfileSync(&_nvmfile);
        nvmfileStop(&_nvmfile);
    }

    /* Flash file emulation. */
    NVMFileDriver _nvmfile;
};

TEST_F(NVMFile, nvmfileGetInfo)
{
    NVMDeviceInfo info;
    EXPECT_EQ(nvmGetInfo(&_nvmfile, &info), HAL_SUCCESS);

    EXPECT_EQ(info.sector_size, nvmfilecfg.sector_size);

    EXPECT_EQ(info.sector_num, nvmfilecfg.sector_num);
}

TEST_F(NVMFile, nvmfileMassErase)
{
    EXPECT_EQ(nvmfileMassErase(&_nvmfile), HAL_SUCCESS);
}

TEST_F(NVMFile, nvmfileWrite)
{
    uint8_t buffer[nvmfilecfg.sector_size];
    EXPECT_EQ(nvmfileWrite(&_nvmfile, 0, sizeof(buffer), buffer), HAL_SUCCESS);

    EXPECT_EQ(nvmfileWrite(&_nvmfile, nvmfilecfg.sector_num * nvmfilecfg.sector_size - sizeof(*buffer), sizeof(*buffer), buffer), HAL_SUCCESS);
}

TEST_F(NVMFile, nvmfileRead)
{
    uint8_t buffer[nvmfilecfg.sector_size];
    EXPECT_EQ(nvmfileRead(&_nvmfile, 0, sizeof(buffer), buffer), HAL_SUCCESS);

    EXPECT_EQ(nvmfileRead(&_nvmfile, nvmfilecfg.sector_num * nvmfilecfg.sector_size - sizeof(*buffer), sizeof(*buffer), buffer), HAL_SUCCESS);
}

TEST_F(NVMFile, nvmfileErase)
{
    EXPECT_EQ(nvmfileErase(&_nvmfile, 0, nvmfilecfg.sector_size), HAL_SUCCESS);

    EXPECT_EQ(nvmfileErase(&_nvmfile, nvmfilecfg.sector_num * nvmfilecfg.sector_size - nvmfilecfg.sector_size, nvmfilecfg.sector_size), HAL_SUCCESS);


    static const char magic[] = "magic string marker\0";
    char buffer[100];

    /* Verify bounds of erase 1. */
    EXPECT_EQ(nvmfileWrite(&_nvmfile, 1 * nvmfilecfg.sector_size, sizeof(magic), (const uint8_t*)magic), HAL_SUCCESS);
    EXPECT_EQ(nvmfileErase(&_nvmfile, 0, nvmfilecfg.sector_size), HAL_SUCCESS);
    EXPECT_EQ(nvmfileRead(&_nvmfile, 1 * nvmfilecfg.sector_size, sizeof(buffer), (uint8_t*)buffer), HAL_SUCCESS);
    EXPECT_STREQ(magic, buffer);

    /* Verify bounds of erase 2. */
    EXPECT_EQ(nvmfileWrite(&_nvmfile, 2 * nvmfilecfg.sector_size, sizeof(magic), (const uint8_t*)magic), HAL_SUCCESS);
    EXPECT_EQ(nvmfileErase(&_nvmfile, 1, nvmfilecfg.sector_size), HAL_SUCCESS);
    EXPECT_EQ(nvmfileErase(&_nvmfile, 1, nvmfilecfg.sector_size - 1), HAL_SUCCESS);
    EXPECT_EQ(nvmfileErase(&_nvmfile, 0, nvmfilecfg.sector_size * 2), HAL_SUCCESS);
    EXPECT_EQ(nvmfileErase(&_nvmfile, 0, nvmfilecfg.sector_size * 2 - 1), HAL_SUCCESS);
    EXPECT_EQ(nvmfileErase(&_nvmfile, 0, nvmfilecfg.sector_size * 1 + 1), HAL_SUCCESS);
    EXPECT_EQ(nvmfileRead(&_nvmfile, 2 * nvmfilecfg.sector_size, sizeof(buffer), (uint8_t*)buffer), HAL_SUCCESS);
    EXPECT_STREQ(magic, buffer);

    /* Verify bounds of erase 3. */
    EXPECT_EQ(nvmfileWrite(&_nvmfile, 1 * nvmfilecfg.sector_size, sizeof(magic), (const uint8_t*)magic), HAL_SUCCESS);
    EXPECT_EQ(nvmfileErase(&_nvmfile, 1, nvmfilecfg.sector_size), HAL_SUCCESS);
    EXPECT_EQ(nvmfileRead(&_nvmfile, 1 * nvmfilecfg.sector_size, sizeof(buffer), (uint8_t*)buffer), HAL_SUCCESS);
    EXPECT_STRNE(magic, buffer);

    /* Verify bounds of erase 4. */
    EXPECT_EQ(nvmfileWrite(&_nvmfile, 1 * nvmfilecfg.sector_size, sizeof(magic), (const uint8_t*)magic), HAL_SUCCESS);
    EXPECT_EQ(nvmfileErase(&_nvmfile, nvmfilecfg.sector_size - 1, nvmfilecfg.sector_size), HAL_SUCCESS);
    EXPECT_EQ(nvmfileRead(&_nvmfile, 1 * nvmfilecfg.sector_size, sizeof(buffer), (uint8_t*)buffer), HAL_SUCCESS);
    EXPECT_STRNE(magic, buffer);

    /* Verify bounds of erase 5. */
    EXPECT_EQ(nvmfileWrite(&_nvmfile, 1 * nvmfilecfg.sector_size, sizeof(magic), (const uint8_t*)magic), HAL_SUCCESS);
    EXPECT_EQ(nvmfileErase(&_nvmfile, nvmfilecfg.sector_size - 1, 1), HAL_SUCCESS);
    EXPECT_EQ(nvmfileRead(&_nvmfile, 1 * nvmfilecfg.sector_size, sizeof(buffer), (uint8_t*)buffer), HAL_SUCCESS);
    EXPECT_STREQ(magic, buffer);

    /* Verify bounds of erase 6. */
    EXPECT_EQ(nvmfileWrite(&_nvmfile, 1 * nvmfilecfg.sector_size, sizeof(magic), (const uint8_t*)magic), HAL_SUCCESS);
    EXPECT_EQ(nvmfileErase(&_nvmfile, nvmfilecfg.sector_size - 1, 2), HAL_SUCCESS);
    EXPECT_EQ(nvmfileRead(&_nvmfile, 1 * nvmfilecfg.sector_size, sizeof(buffer), (uint8_t*)buffer), HAL_SUCCESS);
    EXPECT_STRNE(magic, buffer);
}

TEST_F(NVMFile, EraseWriteReadVerify)
{
    uint8_t buffer[nvmfilecfg.sector_size];

    uint32_t addr = 0;

    while (true)
    {
        uint32_t block_length = nvmfilecfg.sector_size;
        if (addr + block_length > nvmfilecfg.sector_num * nvmfilecfg.sector_size)
            block_length = nvmfilecfg.sector_num * nvmfilecfg.sector_size - addr;

        uint8_t pattern = 0xa5 ^ (addr / block_length);
        pattern = 0;

        for (size_t i = 0; i < block_length; ++i)
            buffer[i] = pattern ^ i;

        EXPECT_EQ(nvmErase(&_nvmfile, addr, block_length), HAL_SUCCESS);
        EXPECT_EQ(nvmSync(&_nvmfile), HAL_SUCCESS);

        EXPECT_EQ(nvmWrite(&_nvmfile, addr, block_length, buffer), HAL_SUCCESS);
        EXPECT_EQ(nvmSync(&_nvmfile), HAL_SUCCESS);

        EXPECT_EQ(nvmRead(&_nvmfile, addr, block_length, buffer), HAL_SUCCESS);
        EXPECT_EQ(nvmSync(&_nvmfile), HAL_SUCCESS);

        for (size_t i = 0; i < block_length; ++i)
            EXPECT_EQ((uint8_t)(pattern ^ i), buffer[i]);

        /* go to next block */
        addr += block_length;
        if (addr >= nvmfilecfg.sector_size * nvmfilecfg.sector_num)
        {
            break;
        }
    }
}

