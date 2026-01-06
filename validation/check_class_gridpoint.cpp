#include "grid.hpp"
#include <array>
#include <cmath>
#include <gtest/gtest.h>
#include <iostream>

#define isSmall(T) (sizeof(T) <= 2 * sizeof(void*))

// test the case when the grid point is at the origin point
TEST(Linspace, OriginPoint)
{
    GridPoint             point(0, 0, 0);
    auto                  x = 4 / std::sqrt(3);
    std::array<double, 3> mockF{x, x, x};
    std::array<double, 3> mockCoord{3 * x, 4 * x, 5 * x};

    auto r  = point.radius();
    auto fr = point.radial_comp(mockF);
    auto d  = point.distance_from(mockCoord);

    EXPECT_DOUBLE_EQ(r, 0);
    EXPECT_DOUBLE_EQ(fr, 4);
    EXPECT_DOUBLE_EQ(d, x * std::sqrt(3 * 3 + 4 * 4 + 5 * 5));

    std::cout << std::boolalpha
              << "GridPoint is cheap to copy: " << isSmall(GridPoint) << "\n";
    std::cout << "It's size: " << sizeof(GridPoint) << "\n";
}

// test the case when the grid point is at x axis
TEST(Linspace, Xaxis)
{
    GridPoint             point(-2.5, 0, 0);
    std::array<double, 3> mockF{3, -4, 5};
    std::array<double, 3> mockCoord{9, -8, -7};

    auto r  = point.radius();
    auto fr = point.radial_comp(mockF);
    auto d  = point.distance_from(mockCoord);

    EXPECT_DOUBLE_EQ(r, 2.5);
    EXPECT_DOUBLE_EQ(fr, -3);
    EXPECT_DOUBLE_EQ(d, std::sqrt(11.5 * 11.5 + 8 * 8 + 7 * 7));
}

// test the case when the grid point is at y-z plane
TEST(Linspace, YZplane)
{
    GridPoint             point(0, -2, 3.14);
    std::array<double, 3> mockF{-10, 3, -4};
    std::array<double, 3> mockCoord{-9, -8, -7};

    auto r  = point.radius();
    auto fr = point.radial_comp(mockF);
    auto d  = point.distance_from(mockCoord);

    EXPECT_DOUBLE_EQ(r, std::sqrt(4.0 + 3.14 * 3.14));
    EXPECT_DOUBLE_EQ(fr, (-6.0 - 3.14 * 4.0) / std::sqrt(4.0 + 3.14 * 3.14));
    EXPECT_DOUBLE_EQ(d, std::sqrt(81 + 36 + 10.14 * 10.14));
}

// // test the case when the grid point is at a random point
// TEST(Linspace, RandomLoc)
// {
//     GridPoint             point(-2.5, 0, 0);
//     std::array<double, 3> mockF{3, -4, 5};
//     std::array<double, 3> mockCoord{9, -8, -7};
//
//     auto r  = point.radius();
//     auto fr = point.radial_comp(mockF);
//     auto d  = point.distance_from(mockCoord);
//
//     EXPECT_DOUBLE_EQ(r, 2.5);
//     EXPECT_DOUBLE_EQ(fr, -3);
//     EXPECT_DOUBLE_EQ(d, std::sqrt(11.5 * 11.5 + 8 * 8 + 7 * 7));
// }
