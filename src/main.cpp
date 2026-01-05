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

int main(int argc, char* argv[])
{
    // cmd line parameter parser
    const ArgsParser parser(argc, argv);
    auto             paras = parser.get_polar_paras();

    // create the polar grid
    PolarGrid targetLocs(paras);

    // read the hdf5 snapshot file
    h5file snapshot("snapshot_000.hdf5", h5file::ReadOnly);
    // get the particle number of each type
    auto partNums = H5Easy::loadAttribute<std::vector<int>>(snapshot, "Header",
                                                            "NumPart_ThisFile");
    /* for (auto num : partNums)
    {
        fmt::println("{} ", num);
    } */
    // read the coordinates and mass of each type
    std::vector<double> mass;
    auto                n = partNums[1];
    mass.resize(n);
    using Matrix = std::vector<std::vector<double>>;
    Matrix coordinate(n, std::vector<double>(3));
    mass.resize(partNums[1]);

    mass = H5Easy::load<std::vector<double>>(snapshot, "/PartType1/Masses");
    coordinate = H5Easy::load<Matrix>(snapshot, "/PartType1/Coordinates");

    /* for (int i = 100; i < 110; i++)
    {
        fmt::println("m={}, (x, y, z)=({}, {}, {})", mass[i], coordinate[i][0],
                     coordinate[i][1], coordinate[i][2]);
    } */
    // get the snapshot data
    // std::vector<> datasets{read_coordiantes_and_masses()};

    // calculate the radial force at target positions
    // auto resutls = calculate_radial_force(datasets, target_locs);

    // write the results to the log file
    // H5IO rcFile("output.hdf5"sv, H5IO::filemode::write);
    // h5io.write_dataset(results);

    h5file              file("output.hdf5", h5file::Truncate);
    std::vector<double> mock_rs(10, 2.12);
    std::vector<double> mock_phis(10, 3.14 / 2);
    std::vector<double> mock_frs(10, 101);
    H5Easy::dump(file, "rs", mock_rs);
    H5Easy::dump(file, "phis", mock_phis);
    H5Easy::dump(file, "frs", mock_frs);

    H5Easy::dumpAttribute(file, "/", "Rmin", 0.0);
    H5Easy::dumpAttribute(file, "/", "Rmax", 10.0);
    H5Easy::dumpAttribute(file, "/", "Rbinnum", 20);
    H5Easy::dumpAttribute(file, "/", "PhiBinnum", 16);

    return EXIT_SUCCESS;
}
