#include "grid.hpp"
#include <array>
#include <cmath>
#include <numeric>  // for std::inner_product

#ifdef NDEBUG
namespace {
#endif
/**
 * @brief mock numpy.linspace(0)
 *
 * @param min minimal value of the data
 * @param max maximal value of the data
 * @param binnum number of intervals (not the number of data point)
 * @param withRightBound whether include the righter bound
 * @return an std::vector<double> for the bin edges
 */
[[nodiscard]] auto linspace(double min,
                            double max,
                            int    binnum,
                            bool   withRightBound = true) noexcept
{
    auto delta = (max - min) / binnum;

    auto effBinNum{binnum + 1};
    if (not withRightBound)
    {
        --effBinNum;
    }

    std::vector<double> bin_edges(effBinNum, 0);
    for (int i = 0; i < effBinNum; ++i)
    {
        bin_edges[i] = min + (delta * i);
    }
    return bin_edges;
};
#ifdef NDEBUG
}  // namespace
#endif

auto GridPoint::r() const
{
    // get the radius of the grid point
    auto r2 =
        std::inner_product(m_pos.begin(), m_pos.end(), m_pos.begin(), 0.0);
    return std::sqrt(r2);
}

// pass by value, since array<double, 3> is cheap to copy
auto GridPoint::distance_from(std::array<double, 3> coordinate) const
{
    for (int i = 0; i < 3; ++i)  // calculate \Delta\vec{r}
    {
        // NOLINTBEGIN(*array-index)
        coordinate[i] -= m_pos[i];
        // NOLINTEND(*array-index)
    }

    // get the norm through inner product
    auto r2 = std::inner_product(coordinate.begin(), coordinate.end(),
                                 coordinate.begin(), 0.0);
    return std::sqrt(r2);
}


PolarGrid::PolarGrid(const PolarGridPara& para)
{
    auto numOfPoint = (para.rbin + 1) * para.phibin;
    m_points.reserve(numOfPoint);  // enlarge the capacity of the list

    // get the r&phi bin edges
    m_phibinEdges = linspace(0.0, 2 * M_PI, para.phibin, false);  // 0 to 2pi
    if (para.type == RbinType::linear)                            // linear bins
    {
        m_rbinEdges = linspace(para.rmin, para.rmax, para.rbin, true);
    }
    else  // logarithmic bins
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
