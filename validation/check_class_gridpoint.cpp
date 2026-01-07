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

    auto R  = point.cyl_radius();
    auto aR = point.cyl_radial_comp(mockF);
    auto d  = point.distance_from(mockCoord);

    EXPECT_DOUBLE_EQ(R, 0);
    EXPECT_DOUBLE_EQ(aR, x * std::sqrt(2));
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

    auto R  = point.cyl_radius();
    auto aR = point.cyl_radial_comp(mockF);
    auto d  = point.distance_from(mockCoord);

    EXPECT_DOUBLE_EQ(R, 2.5);
    EXPECT_DOUBLE_EQ(aR, -3);
    EXPECT_DOUBLE_EQ(d, std::sqrt(11.5 * 11.5 + 8 * 8 + 7 * 7));
}

// test the case when the grid point is at y-z plane
TEST(Linspace, YZplane)
{
    GridPoint             point(0, -2, 3.14);
    std::array<double, 3> mockF{-10, 3, -4};
    std::array<double, 3> mockCoord{-9, -8, -7};

    auto R  = point.cyl_radius();
    auto aR = point.cyl_radial_comp(mockF);
    auto d  = point.distance_from(mockCoord);

    EXPECT_DOUBLE_EQ(R, 2);
    EXPECT_DOUBLE_EQ(aR, -3);
    EXPECT_DOUBLE_EQ(d, std::sqrt(81 + 36 + 10.14 * 10.14));
}

// test the case when the grid point is at a random point
TEST(Linspace, RandomLoc)
{
    GridPoint             point(-2.5, 4.17, -8.12);
    std::array<double, 3> mockF{-6, 7, 19};
    std::array<double, 3> mockCoord{-1.0, -8.12, -7};

    auto R  = point.cyl_radius();
    auto aR = point.cyl_radial_comp(mockF);
    auto d  = point.distance_from(mockCoord);

    EXPECT_DOUBLE_EQ(R, std::sqrt(2.5 * 2.5 + 4.17 * 4.17));
    EXPECT_DOUBLE_EQ(aR, (15 + 7 * 4.17) / std::sqrt(2.5 * 2.5 + 4.17 * 4.17));
    EXPECT_DOUBLE_EQ(d, 12.431753697688833);
}

// test the force from a single point
TEST(Linspace, SinglePoint)
{
    GridPoint                          point(0, 0, 0);
    std::vector<double>                masses{1};
    std::vector<std::array<double, 3>> coordinates{{1, 0, 0}};

    auto aR = point.accR_from(masses, coordinates, 1.0);
    EXPECT_DOUBLE_EQ(aR, 1.0);

    masses[0]      = 5.0;
    coordinates[0] = {1, 2, 3};
    aR             = point.accR_from(masses, coordinates, 1.0);
    EXPECT_DOUBLE_EQ(aR, std::pow(5.0 / (1.0 + 4.0 + 9.0), 1.5));
}

// test the force from two points
TEST(Linspace, TwoPoints)
{
    GridPoint           point(1, 0, 0);
    std::vector<double> masses{1, 1};
    // clang-format off
    std::vector<std::array<double, 3>> coordinates{
        {1+1.0, 1.0, 0},
        {1-1.0, -1.0, 0},
    };
    // clang-format on

    auto aR = point.accR_from(masses, coordinates, 1.0);
    EXPECT_DOUBLE_EQ(aR, 0.0);

    masses[0] *= 4.0;
    coordinates[1] = {1 - 0.5, -0.5, 0};
    aR             = point.accR_from(masses, coordinates, 1.0);
    EXPECT_DOUBLE_EQ(aR, 0.0);

    masses[0]      = 1.0;
    masses[1]      = 1.0;
    coordinates[0] = {1 + 1.0, 1.0, 1};
    coordinates[1] = {1 - 1.0, -1.0, 1};
    aR             = point.accR_from(masses, coordinates, 1.0);
    EXPECT_DOUBLE_EQ(aR, 0);

    masses[0]      = 1.0;
    masses[1]      = 1.0;
    coordinates[0] = {1 + 1.0, 1.0, 1};
    coordinates[1] = {1 - 1.0, -1.0, -1};
    aR             = point.accR_from(masses, coordinates, 1.0);
    EXPECT_DOUBLE_EQ(aR, 0.0);

    point          = {0, -1, 0};
    masses[0]      = 1.0;
    masses[1]      = 3.0;
    coordinates[0] = {1 + 1.0, 2.0, 1};
    coordinates[1] = {1 - 1.0, -1.0, -1};
    aR             = point.accR_from(masses, coordinates, 1.0);
    EXPECT_DOUBLE_EQ(aR, -3.0 / std::pow(14.0, 1.5));
}
