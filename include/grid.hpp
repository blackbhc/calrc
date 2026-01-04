/**
 * @file
 * @brief The polar grid class (target positions) and point class (single target position).
 */

#ifndef GRID_HPP
#define GRID_HPP
#include <array>
#include <numeric>  // for std::inner_product
#include <vector>

// convenient interface with a single field point
class Point
{
public:
    // to get inner product with the local unit radial vector

    // get local radius
    [[nodiscard]] auto r() const -> double
    {
        auto product = std::inner_product(m_pos.begin(), m_pos.end(), m_pos.begin(), 0.0);
        return std::sqrt(product);
    }

private:
    std::array<double, 3> m_pos{0, 0, 0};
};

class Grid
{
private:
    std::vector<Point> m_points;
};

// the polar grid of the target field points to calculate the radial force
class PolarGrid : public Grid
{
public:
    explicit PolarGrid(double Rmin = 0.1, double Rmax = 10, int Rbinnum = 10, int PhiBinNum = 16);
};
#endif
