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
class GridPoint
{
public:
    GridPoint(double x, double y, double z) : m_pos({x, y, z}) {}

    [[nodiscard]]  // get local radius
    auto radius() const -> double;

    [[nodiscard]]  // get the distance to a position restored in a std::array
    auto distance_from(std::array<double, 3> coordinate) const -> double;


    [[nodiscard]]  // get the radial component of a force at the point
    auto radial_comp(std::array<double, 3> force) const -> double;


    [[nodiscard]]  // inner product of the GridPoint and a std::array
    auto dot_with(std::array<double, 3> coordinate) const -> double
    {
        return std::inner_product(m_pos.begin(), m_pos.end(),
                                  coordinate.begin(), 0.0);
    }

    [[nodiscard]]  // access the data
    auto pos() const -> std::array<double, 3>
    {
        return m_pos;
    }

private:
    std::array<double, 3> m_pos{0, 0, 0};
    // to get inner product with the local unit radial vector
};

/*
// the base grid class, currect only a container of the points, designed for the
// future extension beyond polar grid
class Grid
{
protected:
    std::vector<GridPoint> m_points;
}; */

// the polar grid of the target field points to calculate the radial force
class PolarGrid
{
public:
    explicit PolarGrid(const PolarGridPara& para);

    [[nodiscard]]  // get the radial bin edges
    auto rs() -> std::vector<double>
    {
        return m_rbinEdges;
    }

    [[nodiscard]]  // get the azimuthal bin edges
    auto phis() -> std::vector<double>
    {
        return m_phibinEdges;
    }

private:
    std::vector<GridPoint> m_points;
    std::vector<double>    m_rbinEdges;
    std::vector<double>    m_phibinEdges;
    auto                   get_linear_rbin_nodes() {}
};
#endif
