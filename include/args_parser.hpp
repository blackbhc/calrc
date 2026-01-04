/**
 * @file
 * @brief A simple argument parser.
 */

#ifndef ARGS_PARSER_HPP
#define ARGS_PARSER_HPP
#include <string>
// enable automatic argument parsing
class ArgsParser
{
private:
    double      m_rmin{};  // in kpc
    double      m_rmax{};
    int         m_r_binnum{0};
    double      m_phi_binnum{0};
    std::string m_snapshot_file;
    std::string m_rotation_curve_file;
};
#endif
