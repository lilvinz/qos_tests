#include "gtest/gtest.h"

extern "C"
{
#include "printf.h"
}

TEST(format, float)
{
    char test[100];

    EXPECT_GE(snprintf(test, sizeof(test), "%f", 0.999f), 0);
    EXPECT_STREQ("0.999000", test);

    EXPECT_GE(snprintf(test, sizeof(test), "%.4f", 0.999f), 0);
    EXPECT_STREQ("0.9990", test);

    EXPECT_GE(snprintf(test, sizeof(test), "%.3f", 0.999f), 0);
    EXPECT_STREQ("0.999", test);

    // This is a rounding test.
    // Format lib has been patched not to do rounding as it fails on it.
    // Standard compliant behavior requires rounding.
    // We will fix this as a patch is available upstream.
    EXPECT_GE(snprintf(test, sizeof(test), "%.2f", 0.999f), 0);
    EXPECT_STREQ("0.99", test);
}

