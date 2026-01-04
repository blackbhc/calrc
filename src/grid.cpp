#include "grid.hpp"

auto Point::r() const
{
    auto product =
        std::inner_product(m_pos.begin(), m_pos.end(), m_pos.begin(), 0.0);
    return std::sqrt(product);
}


PolarGrid::PolarGrid(const PolarGridPara& para) { ( void )para; };
