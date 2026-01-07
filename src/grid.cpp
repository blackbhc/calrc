#include "grid.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>  // for std::inner_product
#include <omp.h>
#include <vector>

// NOLINTBEGIN(*internal-linkage)
#ifdef NDEBUG
namespace {  // anonymous namespace in Release mode
#endif

constexpr auto PI = 3.14159265358979323846264338327950288;

/**
 * @brief mock numpy.linspace(0)
 *
 * @param min minimal value of the data
 * @param max maximal value of the data
 * @param binnum number of intervals (not the number of data point)
 * @param withRightBound whether include the righter bound
 * @return an std::vector<double> for the bin edges
 */
auto linspace(double min,
              double max,
              int    binnum,
              bool   withRightBound = true) noexcept
{
    auto delta = (max - min) / binnum;  // get the bin size

    auto effBinNum{binnum + 1};  // default: include the right most bin edges
    if (not withRightBound)
    {
        --effBinNum;
    }

    std::vector<double> bin_edges(effBinNum, 0);
    for (int i = 0; i < effBinNum; ++i)
    {
        // get the bin edged with linear increment
        bin_edges[i] = min + (delta * i);
    }
    return bin_edges;
};
#ifdef NDEBUG
}  // namespace
#endif
// NOLINTEND(*internal-linkage)

// pass by value, since array<double, 3> is cheap to copy
double GridPoint::distance_from(std::array<double, 3> coordinate) const
{
    for (int i = 0; i < 3; ++i)  // calculate \Delta\vec{r}
    {
        coordinate[i] -= m_pos[i];  // NOLINT(*array-index)
    }

    // get the norm through inner product
    auto r2 = std::inner_product(coordinate.begin(), coordinate.end(),
                                 coordinate.begin(), 0.0);
    return std::sqrt(r2);
}

double GridPoint::cyl_radial_comp(std::array<double, 3> force) const
{
    const double R = cyl_radius();  // cylindrical radial of the GridPoint

    if (R == 0)  // case where the point is located at the z axis
    {
        // in this case, the result is phyically unclear (or undefiend)
        // return the norm of the force vector projected on the xy plane
        return std::sqrt(force[0] * force[0] + force[1] * force[1]);
    }

    // scalar product between the force and the unit radial vector when r!=0
    return (force[0] * m_pos[0] + force[1] * m_pos[1]) / R;
}

auto GridPoint::accR_from(const std::vector<double>&                masses,
                          const std::vector<std::array<double, 3>>& coordinates,
                          double G) const -> double
{
    // to restore the Ftot= G \sum{ m delta(r) / r^3 }
    std::array<double, 3> accSum{0, 0, 0};

    // NOLINTBEGIN(*array-index)
    // iteration to get the sum without times G
    for (int i = 0; i < static_cast<int>(masses.size()); ++i)
    {
        static double d{};       // for r^3
        static double scalar{};  // for m/r^3
        d      = distance_from(coordinates[i]);
        scalar = masses[i] / (d * d * d);
        // acceleration (/G)= \sum m\Delta(r)/|\Delta(r)|^3
        accSum[0] += scalar * (coordinates[i][0] - m_pos[0]);
        accSum[1] += scalar * (coordinates[i][1] - m_pos[1]);
        accSum[2] += scalar * (coordinates[i][2] - m_pos[2]);
    }
    // times G afterward, to avoid redundant multiplication
    accSum[0] *= G;
    accSum[1] *= G;
    accSum[2] *= G;
    // NOLINTEND(*array-index)

    return cyl_radial_comp(accSum);
}

PolarGrid::PolarGrid(const PolarGridPara& para)
{
    const auto numOfPoint = (para.rbin + 1) * para.phibin;
    m_points.reserve(numOfPoint);  // enlarge the capacity of the list

    // get the r&phi bin edges
    // phi bin edges: between 0 and 2pi(without, since as 2pi~0)
    m_phibinEdges = linspace(0.0, 2 * PI, para.phibin, false);
    // radial bin edges
    if (para.type == RbinType::linear)  // linear bins
    {
        m_rbinEdges = linspace(para.rmin, para.rmax, para.rbin, true);
    }
    else  // logarithmic bins: logarithmically linear
    {
        m_rbinEdges = linspace(std::log10(para.rmin), std::log10(para.rmax),
                               para.rbin, true);
        std::transform(m_rbinEdges.begin(), m_rbinEdges.end(),
                       m_rbinEdges.begin(),
                       [](double r) { return std::pow(10, r); });
    }

    // create the target filed point
    for (auto r : m_rbinEdges)
    {
        for (auto phi : m_phibinEdges)
        {
            m_points.emplace_back(r * std::cos(phi), r * std::sin(phi), 0);
        }
    }
};

auto PolarGrid::cal_accR_from(
    const std::vector<double>&                masses,
    const std::vector<std::array<double, 3>>& coordinates,
    int numThread) const -> std::vector<double>
{
    ( void )numThread;
    std::vector<double> accR(m_points.size(), 0);
    // clang-format off
// accelerating the calculation with OpneMP
#pragma omp parallel for num_threads(numThread) default(none) shared(accR, masses, coordinates)
    for (int i = 0; i < static_cast<int>(m_points.size()); ++i) // i must be int for openmp to work
    {
        // get the accR at each grid point
        accR[i] = m_points[i].accR_from(masses, coordinates);  // with default G
    }
    // clang-format on
    return accR;
}

// return the radius of the used grid points
auto PolarGrid::rs() const -> std::vector<double>
{
    std::vector<double> rs;
    rs.reserve(m_points.size());
    // create the target filed point
    for (auto r : m_rbinEdges)
    {
        for ([[maybe_unused]] auto phi : m_phibinEdges)
        {
            rs.emplace_back(r);
        }
    }
    return rs;
}

// return the azimuthal angles of the used grid points
auto PolarGrid::phis() const -> std::vector<double>
{
    std::vector<double> phis;
    phis.reserve(m_points.size());
    // create the target filed point
    for ([[maybe_unused]] auto r : m_rbinEdges)
    {
        for (auto phi : m_phibinEdges)
        {
            phis.emplace_back(phi);
        }
    }
    return phis;
}
