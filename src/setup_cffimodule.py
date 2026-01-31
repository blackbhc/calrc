from cffi import FFI
import os

# file to the binary dynamic library file: use the cffi API mode
# create an FFI object
ffi = FFI()

# inform Python the function names
ffi.cdef("""
double* calAccRs(int np,
    // NOLINTBEGIN
    const double* sourceMasses,
    const double* sourceCoordinates,
    // NOLINTEND
    int           numGridPoint,
    const double* gridCoords,
    int           numThread);
""")
ffi.cdef("void release(void);")

ffi.set_source(
    "cpp_rcm",
    """
    #include "py.hpp"
    extern "C" {
    double* calAccRs(int np,
        // NOLINTBEGIN
        const double* sourceMasses,
        const double* sourceCoordinates,
        // NOLINTEND
        int           numGridPoint,
        const double* gridCoords,
        int           numThread);

    void release(void);
    }
    """,  # inform cpp compiler the function names
    source_extension=".cpp",  # mandatory for cpp codes
    libraries=["pymodule"],  # for Linux, may require ['m']
    library_dirs=[os.path.abspath("./build")],  # for Linux, may require ['m']
    include_dirs=[os.path.abspath("./include")],
    extra_link_args=["-Wl,-rpath," + os.path.abspath("./build")],
)

if __name__ == "__main__":
    ffi.compile(verbose=True, tmpdir=os.path.abspath("./build"))
