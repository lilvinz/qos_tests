#include "gtest/gtest.h"

// To use a test fixture, derive a class from testing::Test.
class ExampleTest : public testing::Test
{
protected:
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }
};

TEST_F(ExampleTest, Test1)
{
    EXPECT_EQ(0, 0);
}
