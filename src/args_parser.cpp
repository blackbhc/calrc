#include "args_parser.hpp"
#include "cmdline.h"
#include "grid.hpp"

ArgsParser::ArgsParser(int argc, char* argv[])
{
    cmdline::parser cmdParser;
    // add specified type of variable.
    // 1st argument is long name
    // 2nd argument is short name (no short name if '\0' specified)
    // 3rd argument is description
    // 4th argument is mandatory (optional. default is false)
    // 5th argument is default value  (optional. it used when mandatory is
    // false)
    // 6th argument is extra constraint

    cmdParser.add<std::string>("if", 'i', "input file (usually, a snapshot)",
                               true, "");
    cmdParser.add<std::string>(
        "of", 'o', "output file of the radial velocities", false, "rc.hdf5");

    cmdParser.add<double>("rmin", 'r', "minimum radius in kpc", false, 0.0,
                          cmdline::range(0.0, 1000000.0));
    cmdParser.add<double>("rmax", 'R', "maximum radius in kpc", true, 0.0,
                          cmdline::range(0.0, 1000000.0));

    cmdParser.add<int>("rbin", 'm', "radial bin number", true, 10,
                       cmdline::range(0, 10000000));
    cmdParser.add<int>("phibin", 'n', "azimuthal bin number", false, 16,
                       cmdline::range(0, 36000));

    // cmdline::oneof() can restrict options.
    cmdParser.add<std::string>("type", 't', "form of the radial bins", false,
                               "log",
                               cmdline::oneof<std::string>("linear", "log"));

    cmdParser.add<int>("thread", 'c', "Thread count", false, 4,
                       cmdline::range(1, 72 * 4));

    // Run parser.
    // It returns only if command line arguments are valid.
    // If arguments are invalid, a parser output error msgs then exit program.
    // If help flag ('--help' or '-?') is specified, a parser output usage
    // message then exit program.
    cmdParser.parse_check(argc, argv);

    m_snapshot_file       = cmdParser.get<std::string>("if");
    m_rotation_curve_file = cmdParser.get<std::string>("of");
    m_rmin                = cmdParser.get<double>("rmin");
    m_rmax                = cmdParser.get<double>("rmax");
    m_rbin                = cmdParser.get<int>("rbin");
    m_phibin              = cmdParser.get<int>("phibin");
    m_threads             = cmdParser.get<int>("thread");
    auto str2type         = [](const std::string& typeStr) {
        if (typeStr == "linear")
        {
            return RbinType::linear;
        }
        return RbinType::log;
    };
    m_type = str2type(cmdParser.get<std::string>("type"));
}

auto ArgsParser::get_polar_paras() const -> PolarGridPara
{
    return {rmin(), rmax(), rbin(), phibin(), rbintype()};
}
