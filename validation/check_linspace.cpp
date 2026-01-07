#include "../src/grid.cpp"
#include <gtest/gtest.h>

// test the correctness of the function linspace (mock Python np.linspace)
TEST(Linspace, NoRBound)
{
    int  num = 12;
    auto res = linspace(0, 12, num, false);
    EXPECT_EQ(static_cast<int>(res.size()), num);
    EXPECT_DOUBLE_EQ(res[res.size() - 1], 11);
}

TEST(Linspace, WithRBound)
{
    int  num = 12;
    auto res = linspace(0, 12, num, true);
    EXPECT_EQ(static_cast<int>(res.size()), num + 1);
    EXPECT_DOUBLE_EQ(res[res.size() - 1], 12);
}

TEST(Linspace, MockRadialBins)
{
    int  binnum = 11;
    auto res    = linspace(1, 12, binnum, true);
    EXPECT_DOUBLE_EQ(res[res.size() - 1], 12);
    EXPECT_DOUBLE_EQ(res[0], 1);
    EXPECT_DOUBLE_EQ(res[1] - res[0], 1);

    binnum = 20;
    res    = linspace(0.5, 60.5, binnum, true);
    EXPECT_DOUBLE_EQ(res[res.size() - 1], 60.5);
    EXPECT_DOUBLE_EQ(res[0], 0.5);
    EXPECT_DOUBLE_EQ(res[1] - res[0], 3);
}

TEST(Linspace, MockPhiBins)
{
    int  binnum = 6;
    auto res    = linspace(0, 6, binnum, false);
    EXPECT_DOUBLE_EQ(res[res.size() - 1], 5);
    EXPECT_DOUBLE_EQ(res[0], 0);
    EXPECT_DOUBLE_EQ(res[1] - res[0], 1);

    binnum = 30;
    res    = linspace(0, 6, binnum, false);
    EXPECT_DOUBLE_EQ(res[res.size() - 1], 5.8);
    EXPECT_DOUBLE_EQ(res[0], 0);
    EXPECT_DOUBLE_EQ(res[1] - res[0], 0.2);
}
