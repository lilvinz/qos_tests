#include "gtest/gtest.h"

extern "C"
{
#include "qchprintf.h"
}

TEST(format, float)
{
    char test[100];

    EXPECT_GE(qchsnprintf(test, sizeof(test), "%f", 0.999f), 0);
    EXPECT_STREQ("0.999000", test);

    EXPECT_GE(qchsnprintf(test, sizeof(test), "%.4f", 0.999f), 0);
    EXPECT_STREQ("0.9990", test);

    EXPECT_GE(qchsnprintf(test, sizeof(test), "%.3f", 0.999f), 0);
    EXPECT_STREQ("0.999", test);

    // This is a rounding test.
    EXPECT_GE(qchsnprintf(test, sizeof(test), "%.2f", 0.999f), 0);
    EXPECT_STREQ("1.00", test);
}

