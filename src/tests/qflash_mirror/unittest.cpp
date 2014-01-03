#include "gtest/gtest.h"

extern "C"
{
#include "ch.h"
#include "qhal.h"
}

/* Flash file emulation. */
FlashFileDriver ffile_extfram;

/* Flash mirror. */
FlashMirrorDriver fmirror_test;

static const FlashFileConfig ffilecfg_extfram =
{
    // file_name
    "extfram.bin",
    // sector_size
    512,
    // sector_num
    64,
};

static const FlashMirrorConfig fmirrorcfg_test =
{
    // flashp
    (BaseFlashDevice*)&ffile_extfram,
    // sector_header_num
    1,
};

class FlashMirror: public ::testing::Test
{
protected:

    virtual void SetUp()
    {
        ffileObjectInit(&ffile_extfram);
        ffileStart(&ffile_extfram, &ffilecfg_extfram);

        fmirrorObjectInit(&fmirror_test);
        fmirrorStart(&fmirror_test, &fmirrorcfg_test);
    }

    virtual void TearDown()
    {
        fmirrorSync(&fmirror_test);
        fmirrorStop(&fmirror_test);

        ffileSync(&ffile_extfram);
        ffileStop(&ffile_extfram);
    }
};

TEST_F(FlashMirror, fmirrorGetInfo)
{
    FlashDeviceInfo info;
    EXPECT_EQ(fmirrorGetInfo(&fmirror_test, &info), CH_SUCCESS);

    EXPECT_EQ(info.sector_size, ffile_extfram.config->sector_size);

    EXPECT_EQ(info.sector_num, (ffile_extfram.config->sector_num - fmirror_test.config->sector_header_num) / 2);
}

TEST_F(FlashMirror, fmirrorWrite)
{
    uint8_t buffer[ffilecfg_extfram.sector_size];

    EXPECT_EQ(fmirrorWrite(&fmirror_test, 0, sizeof(buffer), buffer), CH_SUCCESS);
}

TEST_F(FlashMirror, fmirrorRead)
{
    uint8_t buffer[ffilecfg_extfram.sector_size];

    EXPECT_EQ(fmirrorRead(&fmirror_test, 0, sizeof(buffer), buffer), CH_SUCCESS);
}

TEST_F(FlashMirror, fmirrorErase)
{
    EXPECT_EQ(fmirrorErase(&fmirror_test, 0, ffilecfg_extfram.sector_size), CH_SUCCESS);
}
/*
TEST_F(FlashMirror, EraseWriteReadVerify)
{
    uint8_t buffer[ffilecfg_extfram.sector_size];

    uint32_t addr = 0;

    while (true)
    {
        uint32_t block_length = ffilecfg_extfram.sector_size;
        if (addr + block_length > ffilecfg_extfram.sector_num * ffilecfg_extfram.sector_size)
            block_length = ffilecfg_extfram.sector_num * ffilecfg_extfram.sector_size - addr;

        uint8_t pattern = 0xa5 ^ (addr / block_length);
        pattern = 0;

        for (size_t i = 0; i < block_length; ++i)
            buffer[i] = pattern ^ i;

        EXPECT_EQ(flashErase(&ffile_extfram, addr, block_length), CH_SUCCESS);
        EXPECT_EQ(flashSync(&ffile_extfram), CH_SUCCESS);

        EXPECT_EQ(flashWrite(&ffile_extfram, addr, block_length, buffer), CH_SUCCESS);
        EXPECT_EQ(flashSync(&ffile_extfram), CH_SUCCESS);

        EXPECT_EQ(flashRead(&ffile_extfram, addr, block_length, buffer), CH_SUCCESS);
        EXPECT_EQ(flashSync(&ffile_extfram), CH_SUCCESS);

        for (size_t i = 0; i < block_length; ++i)
        {
            if (buffer[i] != (uint8_t)(pattern ^ i))
            {
                EXPECT_EQ(true, false);
            }
        }

        // go to next block
        addr += block_length;
        if (addr >= ffilecfg_extfram.sector_size * ffilecfg_extfram.sector_num)
        {
            break;
        }
    }
}
*/

