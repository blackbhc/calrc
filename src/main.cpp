/**
 * @file
 * @brief The multithreading rotation curve calculator for Gadget3 hdf5
 * snaspshot.
 */

#include "args_parser.hpp"
#include "grid.hpp"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <fmt/base.h>
#include <fmt/format.h>
#include <highfive/H5File.hpp>
#include <highfive/H5Group.hpp>
#include <highfive/H5Attribute.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <stdexcept>
#include <string>
#include <vector>
using h5file = HighFive::File;
using Matrix = std::vector<std::array<double, 3>>;

int main(int argc, char* argv[])
{
    const ArgsParser   parser(argc, argv);
    auto               paras        = parser.get_polar_paras();
    const std::string& snapFileName = parser.infile();
    const std::string& rcFileName = parser.outfile();
    auto               numThread  = parser.threads();

    if (paras.rmax <= paras.rmin)
        throw std::runtime_error("rmax must be larger than rmin");
    if (paras.rmin == 0)
    {
        fmt::print(stderr, "Warning: Get a minimum radius=0, which set grid "
                           "points at the origin point.\n");
        if (paras.type == RbinType::log)
            throw std::runtime_error("logarithmic radial bins is invalid when rmin=0");
    }

    const PolarGrid testPoints(paras);
    h5file snapshot(snapFileName, h5file::ReadOnly);

    // Read NumPart_ThisFile attribute from Header group
    auto headerGrp = snapshot.getGroup("Header");
    auto attr = headerGrp.getAttribute("NumPart_ThisFile");
    std::vector<int> partNums;
    attr.read(partNums);

    h5file logFile(rcFileName, h5file::Truncate);

    for (int i = 0; i < static_cast<int>(partNums.size()); ++i)
    {
        auto n = partNums[i];
        if (n <= 0)
            continue;

        std::vector<double> masses(n);
        Matrix coordinates(n, std::array<double, 3>());

        auto massDs = snapshot.getDataSet(fmt::format("/PartType{}/Masses", i));
        massDs.read(masses);

        auto coordDs = snapshot.getDataSet(fmt::format("/PartType{}/Coordinates", i));
        coordDs.read(coordinates);

        auto accRs = testPoints.cal_accR_from(masses, coordinates, numThread);
        auto accDs = logFile.createDataSet<double>(
            fmt::format("/PartType{}/AccRs", i),
            HighFive::DataSpace::From(accRs)
        );
        accDs.write_raw(accRs.data());
    }

    {
        auto rs = testPoints.rs();
        auto ds = logFile.createDataSet<double>("Rs", HighFive::DataSpace::From(rs));
        ds.write_raw(rs.data());
    }
    {
        auto phis = testPoints.phis();
        auto ds = logFile.createDataSet<double>("Phis", HighFive::DataSpace::From(phis));
        ds.write_raw(phis.data());
    }

    logFile.createAttribute<double>("Rmin",
        HighFive::DataSpace::From(paras.rmin)).write(paras.rmin);
    logFile.createAttribute<double>("Rmax",
        HighFive::DataSpace::From(paras.rmax)).write(paras.rmax);
    logFile.createAttribute<int>("RBinNum",
        HighFive::DataSpace::From(paras.rbin + 1)).write(paras.rbin + 1);
    logFile.createAttribute<int>("PhiBinNum",
        HighFive::DataSpace::From(paras.phibin)).write(paras.phibin);

    {
        auto edges = testPoints.rEdges();
        auto a = logFile.createAttribute<double>("GridRs",
            HighFive::DataSpace::From(edges));
        a.write_raw(edges.data());
    }
    {
        auto edges = testPoints.phiEdges();
        auto a = logFile.createAttribute<double>("GridPhis",
            HighFive::DataSpace::From(edges));
        a.write_raw(edges.data());
    }

    return EXIT_SUCCESS;
}