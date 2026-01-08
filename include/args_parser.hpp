/**
 * @file
 * @brief A simple argument parser.
 */

#pragma once
#include "grid.hpp"
#include <string>

// enable automatic argument parsing
class ArgsParser
{
public:
    // use char*, compatible with the main args convention
    ArgsParser(int argc, char* argv[]);  // NOLINT(*c-arrays)
    // get a polar parameter structure after parsing
    [[nodiscard]] auto get_polar_paras() const -> PolarGridPara;
    [[nodiscard]] auto  // accesser to the in file name
    infile() const -> const std::string&
    {
        return m_snapshot_file;
    }
    [[nodiscard]] auto  // accesser to the out file name
    outfile() const -> const std::string&
    {
        return m_acc_file;
    }
    [[nodiscard]] auto threads() const { return m_threads; }

private:
    double      m_rmin{0};  // in kpc
    double      m_rmax{0};
    int         m_rbin{0};  // bin counts
    int         m_phibin{0};
    int         m_threads{0};  // thread count
    std::string m_snapshot_file;
    std::string m_acc_file;
    RbinType    m_type;

    // basic accessers: for internal use
    [[nodiscard]] auto rmin() const { return m_rmin; }
    [[nodiscard]] auto rmax() const { return m_rmax; }
    [[nodiscard]] auto rbin() const { return m_rbin; }
    [[nodiscard]] auto phibin() const { return m_phibin; }
    [[nodiscard]] auto rbintype() const { return m_type; }
};
