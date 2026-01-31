import h5py
import numpy as np
from cpp_rcm import ffi, lib


class PolarGrid(object):
    """
    Organize the polar grid parameters.
    """

    def __init__(self, rmin, rmax, rbinnum, phibinnum, type="log"):
        self.__rmin = rmin
        self.__rmax = rmax
        self.__rbinnum = rbinnum
        self.__phibinnum = phibinnum
        self.__type = type

    def get_grid(self, rmin, rmax, rbinnum, phibin, type="log"):
        phis = np.linspace(0, 2 * np.pi, phibin, endpoint=False)
        match type.lower():
            case "log":
                rs = 10 ** np.linspace(np.log10(rmin), np.log10(rmax), rbinnum + 1)
            case "linear":
                rs = np.linspace(rmin, rmax, rbinnum + 1)
            case _:
                raise ValueError(f"Get an unkown grid type: {type}")

        r_grid, phi_grid = np.meshgrid(rs, phis)
        return r_grid, phi_grid

    def coords(self):
        r_grid, phi_grid = self.rs_phis()
        grid_coords = np.zeros((r_grid.size, 3))
        grid_coords[:, 0] = (r_grid * np.cos(phi_grid)).flatten()
        grid_coords[:, 1] = (r_grid * np.sin(phi_grid)).flatten()
        grid_coords[:, 2] = np.zeros_like(r_grid.flatten())
        return grid_coords

    def rs_phis(self):
        return self.get_grid(
            self.__rmin, self.__rmax, self.__rbinnum, self.__phibinnum, self.__type
        )


class Calculator(object):
    def __init__(self, thread=1):
        self.thread = thread

    def __acc_single_parttype(self, gridCoords, masses, coordinates, thread):
        partNum = len(coordinates)
        gridNum = len(gridCoords)
        return lib.calAccRs(
            partNum,
            ffi.from_buffer("double *", masses),
            ffi.from_buffer("double *", coordinates),
            gridNum,
            ffi.from_buffer("double *", gridCoords),
            thread,
        )

    def get_accs(self, filename: str, testPos: np.ndarray):
        file = h5py.File(filename, "r")
        part_nums = file["Header"].attrs["NumPart_Total"]
        accs = {}
        for i, num in enumerate(part_nums):
            if num <= 0:  # bouncer for 0 particle types
                continue
            typename = f"PartType{i}"
            coordinates = file[typename]["Coordinates"][...].astype(np.float64)
            masses = file[typename]["Masses"][...].astype(np.float64)
            ptr = self.__acc_single_parttype(
                np.ascontiguousarray(testPos),
                masses,
                coordinates,
                thread=self.thread,
            )
            buf = ffi.buffer(ptr, len(testPos) * ffi.sizeof("double"))
            accs[typename] = np.frombuffer(
                buf, dtype=np.float64
            ).copy()  # copy() to avoid dangling numpy.ndarray
            lib.release()  # release the allocated memory
        file.close()
        return accs
