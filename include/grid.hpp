/**
 * @file
 * @brief The polar grid class (target positions) and point class (single target
 * position).
 */

#ifndef GRID_HPP
#define GRID_HPP
#include <array>
#include <cstdint>
#include <numeric>  // for std::inner_product
#include <vector>

// used to specify whether the radial grid is logarithmic or linear
enum class RbinType : std::uint8_t
{
    log,
    linear,
};

// an aggregate for the polar grid parameters
struct PolarGridPara
{
    double   rmin{};
    double   rmax{};
    int      rbin{};
    int      phibin{};
    RbinType type{};
};

// convenient interface with a single field point
class Point
{
public:
    // to get inner product with the local unit radial vector

    // get local radius
    [[nodiscard]] auto r() const;

private:
    std::array<double, 3> m_pos{0, 0, 0};
};

// the base grid class, currect only a container of the points, designed for the
// future extension beyond polar grid
class Grid
{
private:
    std::vector<Point> m_points;
};

// the polar grid of the target field points to calculate the radial force
class PolarGrid : public Grid
{
public:
    explicit PolarGrid(const PolarGridPara& para);
};
#endif
