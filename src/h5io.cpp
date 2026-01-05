#include "h5io.hpp"
#include "H5Ppublic.h"
#include <utility>

// Open the specified file based on given file mode.
H5IO::H5IO(std::string_view filename, filemode mode)
    : m_filename{filename}, m_filemode{mode}, m_file_handle(H5I_INVALID_HID)
{
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

// close the file if necessary
H5IO::~H5IO()
{
    if (m_file_handle != H5I_INVALID_HID)  // set as H5I_INVALID_HID if moved
    {
        H5Fclose(m_file_handle);  // close the hdf5 file
        m_file_handle = H5I_INVALID_HID;
    }
    // set the file handle as the sentinel value
}

// move assignment operator
H5IO& H5IO::operator=(H5IO&& another) noexcept
{
    m_filename            = another.m_filename;
    m_filemode            = another.m_filemode;
    m_file_handle         = another.m_file_handle;
    another.m_file_handle = H5I_INVALID_HID;
    return *this;
}

// move constructor
H5IO::H5IO(H5IO&& another) noexcept
    : m_filename{std::move(another.m_filename)}, m_filemode{another.m_filemode},
      m_file_handle{another.m_file_handle}
{
    another.m_file_handle = H5I_INVALID_HID;
}
