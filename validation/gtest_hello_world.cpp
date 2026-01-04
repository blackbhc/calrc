#include <gtest/gtest.h>

int square(int x) { return x * x; }

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions)
{
    // Expect two strings not to be equal.
    EXPECT_STRNE("hello", "world");
    // Expect equality.
    EXPECT_EQ(7 * 6, 42);
}

TEST(HelloTest, TestSquare)
{
    // Expect equality.
    EXPECT_EQ(square(6), 36);
}

// add a main if not link to libgtest_main
int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
