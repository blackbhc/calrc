#include "h5io.hpp"
#include "H5Ppublic.h"

H5IO::H5IO(std::string_view filename, filemode mode)
    : m_filename{filename}, m_mode{mode}, m_file_handle(-1)
{
    // open the hdf5 file
    switch (get_mode())
    {
    case filemode::read:
        m_file_handle =
            H5Fopen(m_filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
        break;
    case filemode::write:
        m_file_handle = H5Fcreate(m_filename.c_str(), H5F_ACC_TRUNC,
                                  H5P_DEFAULT, H5P_DEFAULT);
        break;
    }
}

H5IO::~H5IO()
{
    H5Fclose(m_file_handle);  // close the hdf5 file
    // set the file handle as the sentinel value
    m_file_handle = -1;
}
