"""
Python bindings for calrc Rust library via ctypes.

Provides:
- PolarGrid: polar grid utility
- Calculator: computes radial accelerations from GADGET-4 snapshots or arrays

Usage:
    from rcpy import PolarGrid, Calculator
    grid = PolarGrid(rmin=0.1, rmax=40, rbinnum=40, phibinnum=16, type="log")
    calc = Calculator(thread=8)
    accs = calc.accs_from_snapshot("snapshot.hdf5", grid.coords())
"""

import ctypes
import platform
import numpy as np
from pathlib import Path


# ---------------------------------------------------------------------------
# Load the shared library
# ---------------------------------------------------------------------------

_lib_path = Path(__file__).parent / "target/release"
_system = platform.system()

if _system == "Linux":
    _lib_file = _lib_path / "libcalrc.so"
elif _system == "Darwin":
    _lib_file = _lib_path / "libcalrc.dylib"
else:
    raise OSError(f"Unsupported platform: {_system}")

if not _lib_file.exists():
    raise FileNotFoundError(
        f"calrc shared library not found at {_lib_file}. "
        f"Run: cd {_lib_path.parent} && cargo build --release"
    )

_lib = ctypes.cdll.LoadLibrary(str(_lib_file))

# Define function signatures
_lib.cal_acc_rs.argtypes = [
    ctypes.c_int32,       # num_grid
    ctypes.POINTER(ctypes.c_double),  # grid_xy
    ctypes.c_int32,       # num_part
    ctypes.POINTER(ctypes.c_double),  # masses
    ctypes.POINTER(ctypes.c_double),  # coords
    ctypes.c_int32,       # num_thread
]
_lib.cal_acc_rs.restype = ctypes.POINTER(ctypes.c_double)

_lib.free_acc_rs.argtypes = []
_lib.free_acc_rs.restype = None


# ---------------------------------------------------------------------------
# Polar grid utility
# ---------------------------------------------------------------------------

class PolarGrid:
    """
    Organize polar grid parameters and generate grid coordinates.

    Parameters
    ----------
    rmin : float
        Minimum radius (kpc).
    rmax : float
        Maximum radius (kpc).
    rbinnum : int
        Number of radial bins.
    phibinnum : int
        Number of azimuthal bins.
    type : str
        "log" or "linear".
    """

    def __init__(self, rmin, rmax, rbinnum, phibinnum, type="log"):
        self._rmin = rmin
        self._rmax = rmax
        self._rbinnum = rbinnum
        self._phibinnum = phibinnum
        self._type = type

    def _get_grid(self, rmin, rmax, rbinnum, phibin, type="log"):
        """Generate radius and phi 2D grids."""
        phis = np.linspace(0, 2 * np.pi, phibin, endpoint=False)
        if type.lower() == "log":
            rs = 10 ** np.linspace(np.log10(rmin), np.log10(rmax), rbinnum + 1)
        elif type.lower() == "linear":
            rs = np.linspace(rmin, rmax, rbinnum + 1)
        else:
            raise ValueError(f"Unknown grid type: {type}")
        r_grid, phi_grid = np.meshgrid(rs, phis)
        return r_grid, phi_grid

    def coords(self):
        """Return field point coordinates as (N, 3) ndarray."""
        r_grid, phi_grid = self._get_grid(
            self._rmin, self._rmax, self._rbinnum, self._phibinnum, self._type
        )
        coords = np.zeros((r_grid.size, 3), dtype=np.float64)
        coords[:, 0] = (r_grid * np.cos(phi_grid)).ravel()
        coords[:, 1] = (r_grid * np.sin(phi_grid)).ravel()
        coords[:, 2] = 0.0
        return coords

    def rs_phis(self):
        """Return flattened (rs, phis) arrays."""
        r_grid, phi_grid = self._get_grid(
            self._rmin, self._rmax, self._rbinnum, self._phibinnum, self._type
        )
        return r_grid.ravel(), phi_grid.ravel()


# ---------------------------------------------------------------------------
# Calculator
# ---------------------------------------------------------------------------

class Calculator:
    """
    Compute radial accelerations using the Rust calrc engine.

    Parameters
    ----------
    thread : int
        Number of parallel worker threads.
    """

    def __init__(self, thread=1):
        self.thread = thread

    def _acc_single(self, grid_xy, masses, coords):
        """Call C FFI for a single particle type."""
        ng = grid_xy.size // 2
        np = masses.size

        # Ensure C-contiguous float64 arrays
        grid_xy = np.ascontiguousarray(grid_xy, dtype=np.float64)
        masses = np.ascontiguousarray(masses, dtype=np.float64)
        coords = np.ascontiguousarray(coords, dtype=np.float64)

        ptr = _lib.cal_acc_rs(
            ctypes.c_int32(ng),
            grid_xy.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
            ctypes.c_int32(np),
            masses.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
            coords.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
            ctypes.c_int32(self.thread),
        )

        if not ptr:
            raise RuntimeError("cal_acc_rs returned null pointer")

        # Copy data from Rust-allocated buffer
        buf = (ctypes.c_double * ng).from_address(ctypes.addressof(ptr.contents))
        result = np.frombuffer(buf, dtype=np.float64).copy()

        _lib.free_acc_rs()
        return result

    def accs_from_snapshot(self, filename, test_pos):
        """
        Compute radial accelerations from a GADGET-4 HDF5 snapshot.

        Parameters
        ----------
        filename : str
            Path to the HDF5 snapshot file.
        test_pos : ndarray
            (N, 3) array of field point coordinates.

        Returns
        -------
        dict[str, ndarray]
            Mapping from particle type name to accelerations.
        """
        import h5py

        file = h5py.File(filename, "r")
        part_nums = file["Header"].attrs["NumPart_ThisFile"]
        acc_rs = {}

        for i, num in enumerate(part_nums):
            if num <= 0:
                continue
            typename = f"PartType{i}"
            coords = file[typename]["Coordinates"][...].astype(np.float64)
            masses = file[typename]["Masses"][...].astype(np.float64)
            grid_xy = np.ascontiguousarray(test_pos[:, :2])  # (x, y) only
            acc_rs[typename] = self._acc_single(grid_xy, masses, coords)

        file.close()
        return acc_rs

    def accs_from_array(self, field_coords, field_masses, test_pos):
        """
        Compute radial accelerations from in-memory arrays.

        Parameters
        ----------
        field_coords : ndarray
            (N, 3) array of particle coordinates.
        field_masses : ndarray
            (N,) array of particle masses.
        test_pos : ndarray
            (M, 3) array of field point coordinates.

        Returns
        -------
        ndarray
            (M,) array of radial accelerations.
        """
        grid_xy = np.ascontiguousarray(test_pos[:, :2])
        masses = np.ascontiguousarray(field_masses, dtype=np.float64)
        coords = np.ascontiguousarray(field_coords, dtype=np.float64)
        return self._acc_single(grid_xy, masses, coords)