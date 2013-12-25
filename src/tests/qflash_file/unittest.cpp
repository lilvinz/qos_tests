#include "gtest/gtest.h"

extern "C"
{
#include "ch.h"
#include "qhal.h"
}

static const FlashFileConfig flashfilecfg =
{
    // file_name
    "extflash.bin",
    // sector_size
    1024 * 4,
    // sector_num
    8192,
};

class FlashFile: public ::testing::Test
{
protected:

    virtual void SetUp()
    {
        ffileObjectInit(&_flashfile);
        ffileStart(&_flashfile, &flashfilecfg);
    }

    virtual void TearDown()
    {
        ffileSync(&_flashfile);
        ffileStop(&_flashfile);
    }

    /* Flash file emulation. */
    FlashFileDriver _flashfile;
};

TEST_F(FlashFile, ffileGetInfo)
{
    FlashDeviceInfo info;
    EXPECT_EQ(ffileGetInfo(&_flashfile, &info), CH_SUCCESS);

    EXPECT_EQ(info.sector_size, flashfilecfg.sector_size);

    EXPECT_EQ(info.sector_num, flashfilecfg.sector_num);
}

TEST_F(FlashFile, ffileMassErase)
{
    EXPECT_EQ(ffileMassErase(&_flashfile), CH_SUCCESS);
}

TEST_F(FlashFile, ffileWrite)
{
    uint8_t buffer[flashfilecfg.sector_size];
    EXPECT_EQ(ffileWrite(&_flashfile, 0, sizeof(*buffer), buffer), CH_SUCCESS);

    EXPECT_EQ(ffileWrite(&_flashfile, flashfilecfg.sector_num * flashfilecfg.sector_size - sizeof(*buffer), sizeof(*buffer), buffer), CH_SUCCESS);
}

TEST_F(FlashFile, ffileRead)
{
    uint8_t buffer[flashfilecfg.sector_size];
    EXPECT_EQ(ffileRead(&_flashfile, 0, sizeof(*buffer), buffer), CH_SUCCESS);

    EXPECT_EQ(ffileRead(&_flashfile, flashfilecfg.sector_num * flashfilecfg.sector_size - sizeof(*buffer), sizeof(*buffer), buffer), CH_SUCCESS);
}

TEST_F(FlashFile, ffileErase)
{
    EXPECT_EQ(ffileErase(&_flashfile, 0, flashfilecfg.sector_size), CH_SUCCESS);

    EXPECT_EQ(ffileErase(&_flashfile, flashfilecfg.sector_num * flashfilecfg.sector_size - flashfilecfg.sector_size, flashfilecfg.sector_size), CH_SUCCESS);
}

TEST_F(FlashFile, EraseWriteReadVerify)
{
    uint8_t buffer[flashfilecfg.sector_size];

    uint32_t addr = 0;

    while (true)
    {
        uint32_t block_length = flashfilecfg.sector_size;
        if (addr + block_length > flashfilecfg.sector_num * flashfilecfg.sector_size)
            block_length = flashfilecfg.sector_num * flashfilecfg.sector_size - addr;

        uint8_t pattern = 0xa5 ^ (addr / block_length);
        pattern = 0;

        for (size_t i = 0; i < block_length; ++i)
            buffer[i] = pattern ^ i;

        EXPECT_EQ(flashErase(&_flashfile, addr, block_length), CH_SUCCESS);
        EXPECT_EQ(flashSync(&_flashfile), CH_SUCCESS);

        EXPECT_EQ(flashWrite(&_flashfile, addr, block_length, buffer), CH_SUCCESS);
        EXPECT_EQ(flashSync(&_flashfile), CH_SUCCESS);

        EXPECT_EQ(flashRead(&_flashfile, addr, block_length, buffer), CH_SUCCESS);
        EXPECT_EQ(flashSync(&_flashfile), CH_SUCCESS);

        for (size_t i = 0; i < block_length; ++i)
        {
            if (buffer[i] != (uint8_t)(pattern ^ i))
            {
                EXPECT_EQ(true, false);
            }
        }

        /* go to next block */
        addr += block_length;
        if (addr >= flashfilecfg.sector_size * flashfilecfg.sector_num)
        {
            break;
        }
    }
}

