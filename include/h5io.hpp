/**
 * @file
 * @brief The organizer class of the hdf5 file IO.
 */

#ifndef H5IO_HPP
#define H5IO_HPP
#include <string>

// to enable user-friendly h5 IO as experience with h5py
class H5IO
{
public:
    H5IO(std::string_view filename, char mode);

private:
    std::string m_filename;
    void        open_readFile();
    void        open_writeFile();
};

#endif
