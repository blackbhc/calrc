/**
 * @file
 * @brief The organizer class of the hdf5 file IO.
 */

#ifndef H5IO_HPP
#define H5IO_HPP
#include <hdf5.h>
#include <string>

// to enable user-friendly h5 IO as experience with h5py
class H5IO
{
public:
    // used to specify the hdf5 file IO mode
    enum class filemode : std::uint8_t
    {
        read,
        write,
    };

    H5IO(std::string_view filename, filemode mode);
    ~H5IO();

private:
    std::string m_filename;
    filemode    m_mode;
    hid_t       m_file_handle{};
    auto        get_mode() { return m_mode; }
    void        open_readFile(std::string_view filename);
    void        open_writeFile(std::string_view filename);
};

#endif
