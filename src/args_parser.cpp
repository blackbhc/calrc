#include "args_parser.hpp"

ArgsParser::ArgsParser(int argc, char* argv[])
{
    ( void )argc;
    ( void )argv;
}

auto ArgsParser::get_polar_paras() const -> PolarGridPara
{
    return {rmin(), rmax(), r_binnum(), phi_binnum()};
}
