/**
 * @file
 * @brief The multithreading rotation curve calculator for Gadget3 hdf5
 * snaspshot.
 */

#include "args_parser.hpp"
#include "grid.hpp"
#include "h5io.hpp"
#include <cstdlib>
#include <fmt/base.h>  // for format print
#include <string_view>
using namespace std::string_view_literals;

int main(int argc, char* argv[])
{
    // cmd line parameter parser
    const ArgsParser parser(argc, argv);
    auto             paras = parser.get_polar_paras();

    // create the polar grid
    PolarGrid targetLocs(paras);

    // read the hdf5 snapshot file
    H5IO snapFile("snapshot_000.hdf5"sv, H5IO::filemode::read);

    // get the snapshot data
    // std::vector<> datasets{read_coordiantes_and_masses()};

    // calculate the radial force at target positions
    // auto resutls = calculate_radial_force(datasets, target_locs);

    // write the results to the log file
    H5IO rcFile("output.hdf5"sv, H5IO::filemode::write);
    // h5io.write_dataset(results);

    fmt::println("Hello world from {{fmt}}.");

    return EXIT_SUCCESS;
}
