/**
 * @file
 * @brief The multithreading rotation curve calculator for Gadget3 hdf5
 * snaspshot.
 */

#include "args_parser.hpp"
#include "grid.hpp"
#include <cstdlib>
#include <fmt/base.h>  // for format print
#include <highfive/H5Easy.hpp>
#include <string_view>
using namespace std::string_view_literals;
using h5file = H5Easy::File;
using Matrix = std::vector<std::array<double, 3>>;

int main(int argc, char* argv[])
{
    // cmd line parameter parser
    const ArgsParser parser(argc, argv);
    auto             paras        = parser.get_polar_paras();
    auto             snapFileName = parser.infile();
    auto             rcFileName   = parser.outfile();
    auto             numThread    = parser.threads();
    // TODO: check the parameters are logically correct (e.g., rmin<rmax,
    // rbinnum>1 ...)

#ifdef DEBUG
    fmt::println("The input snapshot file name: {}", snapFileName);
    fmt::println("The output radial force file name: {}", rcFileName);
    fmt::println("The miminal radius: {}", paras.rmin);
    fmt::println("The maximal radius: {}", paras.rmax);
    fmt::println("The radial binnum: {}", paras.rbin);
    auto type2str = [](RbinType type) -> std::string {
        if (type == RbinType::linear)
        {
            return "linear";
        }
        return "log";
    };
    fmt::println("The radial bin type: {}", type2str(paras.type));
    fmt::println("The azimuthal binnum: {}", paras.phibin);
#endif

    // create the polar grid
    PolarGrid testPoints(paras);

    // read the hdf5 snapshot file
    h5file snapshot(snapFileName, h5file::ReadOnly);

    // get the particle number of each type
    auto partNums = H5Easy::loadAttribute<std::vector<int>>(snapshot, "Header",
                                                            "NumPart_ThisFile");

    // open the log file
    h5file logFile(rcFileName, h5file::Truncate);

    for (int i = 0; i < static_cast<int>(partNums.size()); ++i)
    {
        auto n = partNums[i];

        if (n <= 0)  // ignore 0 partile types
        {
            continue;
        }

        // read the coordinates and mass of each type
        std::vector<double> masses(n);                   // container
        Matrix coordinates(n, std::array<double, 3>());  // container
        masses = H5Easy::load<std::vector<double>>(
            snapshot, fmt::format("/PartType{}/Masses", i));
        coordinates = H5Easy::load<Matrix>(
            snapshot, fmt::format("/PartType{}/Coordinates", i));

        // calculate the radial force at target positions
        auto accRs = testPoints.cal_accR_from(masses, coordinates, numThread);

        H5Easy::dump(logFile, fmt::format("/PartType{}/AccRs", i), accRs);
    }

    // write the used rs and phis of the test points
    H5Easy::dump(logFile, "Rs", testPoints.rs());
    H5Easy::dump(logFile, "Phis", testPoints.phis());

    // write the basic parameters
    H5Easy::dumpAttribute(logFile, "/", "Rmin", paras.rmin);
    H5Easy::dumpAttribute(logFile, "/", "Rmax", paras.rmax);
    H5Easy::dumpAttribute(logFile, "/", "RbinNum", paras.rbin + 1);
    H5Easy::dumpAttribute(logFile, "/", "PhiBinNum", paras.phibin);

    // write the bin edges
    H5Easy::dumpAttribute(logFile, "/", "GridRs", testPoints.rEdges());
    H5Easy::dumpAttribute(logFile, "/", "GridPhis", testPoints.phiEdges());

    return EXIT_SUCCESS;
}
