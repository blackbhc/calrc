import h5py
import numpy as np
from cpp_rcpy import ffi, lib


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

    def accs_from_snapshot(self, filename: str, testPos: np.ndarray):
        """
        Get the radial acceleration from a Gadget4 snapshot file at specified positions.
        --------------------------------------------------------------------------------
        filename: the str of the path to the snapshot file.
        testPos: the locations to measure the radial acceleration.

        Return:
        -------
        An dictionary of the accelerations due to each particle types.
        """
        file = h5py.File(filename, "r")
        part_nums = file["Header"].attrs["NumPart_Total"]
        accRs = {}
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
            accRs[typename] = np.frombuffer(
                buf, dtype=np.float64
            ).copy()  # copy() to avoid dangling numpy.ndarray
            lib.release()  # release the allocated memory
        file.close()
        return accRs

    def accs_from_array(
        self, fieldCoords: np.ndarray, fieldMasses: np.ndarray, testPos: np.ndarray
    ):
        """
        Get the radial accelerations at specified positions from arrays of field mass points.
        -------------------------------------------------------------------------------------
        fieldCoords: coordinates of the mass points.
        fieldMasses: masses of the mass points.

        Return:
        -------
        An array of the accelerations due to the given masses and coordinates of the field mass points.
        """
        coordinates = np.array(fieldCoords, dtype=np.float64)
        masses = np.array(fieldMasses, dtype=np.float64)
        ptr = self.__acc_single_parttype(
            np.ascontiguousarray(testPos),
            np.ascontiguousarray(masses),
            np.ascontiguousarray(coordinates),
            thread=self.thread,
        )
        buf = ffi.buffer(ptr, len(testPos) * ffi.sizeof("double"))
        accRs = np.frombuffer(
            buf, dtype=np.float64
        ).copy()  # copy() to avoid dangling numpy.ndarray
        lib.release()  # release the allocated memory
        return accRs
