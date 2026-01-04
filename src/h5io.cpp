#include "h5io.hpp"
#include <cstdio>
#include <exception>
#include <fmt/base.h>
#include <iostream>

namespace {

enum class availabelMode : std::uint8_t
{
    read,
    write,
    unknown
};

auto mode2int(char mode_c) -> availabelMode
{
    if (mode_c == 'w')
    {
        return availabelMode::write;
    }
    if (mode_c == 'r')
    {
        return availabelMode::read;
    }

    return availabelMode::unknown;
}
}  // namespace

H5IO::H5IO(std::string_view filename, char mode_c)
{
    auto mode = mode2int(mode_c);
    if (mode == availabelMode::unknown)
    {
        fmt::println(stderr, "Unknown mode: {}", mode_c);
        fmt::println(stderr,
                     "Only support mode 'w' for write or mode 'r' for read.");
        throw std::invalid_argument("Unknown hdf5 IO mode");
    };

    ( void )filename;
    // open the hdf5 file
}
