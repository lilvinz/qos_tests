#include "gtest/gtest.h"

extern "C"
{
#include "irq.h"
}

TEST(Irq, Nesting)
{
    EXPECT_EQ(IRQ_Disable(), true);
    EXPECT_EQ(IRQ_Disable(), true);
    EXPECT_EQ(IRQ_Disable(), true);
    EXPECT_EQ(IRQ_Disable(), true);
    EXPECT_EQ(IRQ_Disable(), true);

    EXPECT_EQ(IRQ_Enable(), true);
    EXPECT_EQ(IRQ_Enable(), true);
    EXPECT_EQ(IRQ_Enable(), true);
    EXPECT_EQ(IRQ_Enable(), true);
    EXPECT_EQ(IRQ_Enable(), true);

    EXPECT_EQ(IRQ_Enable(), false);
}
