from cffi import FFI
import os

# file to the binary dynamic library file: use the cffi API mode
# create an FFI object
ffi = FFI()

base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
include_dir = os.path.join(base_dir, "include")
lib_dir = os.path.join(base_dir, "build")

# inform Python the function names
ffi.cdef("""
    double* calAccRs(int np,
        const double* sourceMasses,
        const double* sourceCoordinates,
        int           numGridPoint,
        const double* gridCoords,
        int           numThread);
    void release(void);
""")

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
    # path to source files
    sources=[os.path.join(base_dir, "src", "py.cpp")],
    source_extension=".cpp",
    libraries=["pymodule"],
    library_dirs=[lib_dir],
    include_dirs=[include_dir],
    # path to run time lib
    extra_link_args=["-Wl,-rpath," + lib_dir],
)

if __name__ == "__main__":
    ffi.compile(verbose=True, tmpdir=os.path.abspath("./build"))
