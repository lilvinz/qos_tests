#include "gtest/gtest.h"

extern "C"
{
#include "semaphore.h"
}

TEST(Semaphore, Nesting)
{
    // create semaphore
    semaphore_s *sema = Semaphore_Create();
    EXPECT_NE(sema, (void*)NULL);

    // this is a binary semaphore which does not support nesting
    EXPECT_EQ(Semaphore_Take(sema, 0), true);
    EXPECT_EQ(Semaphore_Take(sema, 0), false);
    EXPECT_EQ(Semaphore_Give(sema), true);
    EXPECT_EQ(Semaphore_Give(sema), false);
}


TEST(Semaphore, NestingFromISR)
{
    // create semaphore
    semaphore_s *sema = Semaphore_Create();
    EXPECT_NE(sema, (void*)NULL);

    // this is a binary semaphore which does not support nesting
    EXPECT_EQ(Semaphore_Take_FromISR(sema, NULL), true);
    EXPECT_EQ(Semaphore_Take_FromISR(sema, NULL), false);
    EXPECT_EQ(Semaphore_Give_FromISR(sema, NULL), true);
    EXPECT_EQ(Semaphore_Give_FromISR(sema, NULL), false);
}


TEST(Semaphore, Timeout)
{
    // create semaphore
    semaphore_s *sema = Semaphore_Create();
    EXPECT_NE(sema, (void*)NULL);

    // this is a binary semaphore which does not support nesting
    EXPECT_EQ(Semaphore_Take(sema, 0), true);
    EXPECT_EQ(Semaphore_Take(sema, 1000), false);
    EXPECT_EQ(Semaphore_Give(sema), true);
    EXPECT_EQ(Semaphore_Give(sema), false);
}


