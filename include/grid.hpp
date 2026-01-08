/**
 * @file
 * @brief The polar grid class (target positions) and point class (single target
 * position).
 */

#pragma once
#include <array>
#include <cmath>
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

    [[nodiscard]] auto  // get the cylindrical radius of the point
    cyl_radius() const -> double
    {
        return std::sqrt(m_pos[0] * m_pos[0] + m_pos[1] * m_pos[1]);
    }

    [[nodiscard]] auto
    // get the distance to a position restored in a std::array
    distance_from(std::array<double, 3> coordinate) const -> double;

    [[nodiscard]] auto  // inner product of the GridPoint and a std::array
    dot_with(std::array<double, 3> coordinate) const -> double
    {
        return std::inner_product(m_pos.begin(), m_pos.end(),
                                  coordinate.begin(), 0.0);
    }

    [[nodiscard]] auto  // access the data
    pos() const -> std::array<double, 3>
    {
        return m_pos;
    }

    /**
     * @brief calculate the radial force from external mass
     *
     * @param masses an std::vector to the masses
     * @param coordinates an nx3 vector<array> to the coordinates
     * @param G the corresponding numerical gravitational constant, default
     * 43007.1 for the default unit of Gadget4 snapshot
     * @return the radial force (or acceleration, as the test particle is unit
     * mass)
     */
    [[nodiscard]]  // radial force from external material
    auto accR_from(const std::vector<double>&                masses,
                   const std::vector<std::array<double, 3>>& coordinates,
                   double G = 43007.1) const -> double;

private:
    std::array<double, 3> m_pos{0, 0, 0};
#ifdef DEBUG
public:
#endif
    [[nodiscard]] auto  // get the radial component of a force at the point
    cyl_radial_comp(std::array<double, 3> force) const -> double;
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

    [[nodiscard]] auto  // get the radial bin edges
    rEdges() const -> std::vector<double>
    {
        return m_rbinEdges;
    }

    [[nodiscard]] auto  // get the azimuthal bin edges
    phiEdges() const -> std::vector<double>
    {
        return m_phibinEdges;
    }

    [[nodiscard]] auto  // calculate the radial force from external material
    cal_accR_from(const std::vector<double>&                masses,
                  const std::vector<std::array<double, 3>>& coordinates,
                  int numThread) const -> std::vector<double>;

    [[nodiscard]] auto  // get the radius of the used grid points
    rs() const -> std::vector<double>;
    [[nodiscard]] auto  // get the phis of the used grid points
    phis() const -> std::vector<double>;

private:
    std::vector<GridPoint> m_points;
    std::vector<double>    m_rbinEdges;
    std::vector<double>    m_phibinEdges;
};
