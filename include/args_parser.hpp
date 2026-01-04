/**
 * @file
 * @brief A simple argument parser.
 */

#ifndef ARGS_PARSER_HPP
#define ARGS_PARSER_HPP
#include "grid.hpp"
#include <string>
// enable automatic argument parsing
class ArgsParser
{
public:
    // use char*, compatible with the main args convention
    ArgsParser(int argc, char* argv[]);
    // get a polar parameter structure after parsing
    [[nodiscard]] auto get_polar_paras() const -> PolarGridPara;

private:
    double      m_rmin{};  // in kpc
    double      m_rmax{};
    int         m_r_binnum{0};
    double      m_phi_binnum{0};
    std::string m_snapshot_file;
    std::string m_rotation_curve_file;

    // basic accessers: for internal use
    [[nodiscard]] auto rmin() const { return m_rmin; }
    [[nodiscard]] auto rmax() const { return m_rmax; }
    [[nodiscard]] auto r_binnum() const { return m_r_binnum; }
    [[nodiscard]] auto r_phi_binnum() const { return m_phi_binnum; }
};
#endif
