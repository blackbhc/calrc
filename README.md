# calrc

A robust and straightforward rotation curve calculator utilizing **Newtonian summation**.

`calrc` reads standard **GADGET-4** snapshot files (in HDF5 format) to calculate the radial
gravitational force on a specified polar grid. It assumes the galactic center is
at the origin and the galactic disk is aligned with the **X-Y plane**.

---

## Dependencies

- **C++** Compiler: Supports at least C++17 with [OpenMP](https://www.openmp.org/) integrated.
- [HDF5](https://www.hdfgroup.org/solutions/hdf5/): For high-performance file I/O.
- [CMake](https://cmake.org/): Version 3.13 or higher.
- [fmt](https://github.com/fmtlib/fmt.git): For formatted output (included as a git submodule).
- [HighFive](https://github.com/highfive-devs/highfive.git): A C++ header-only interface for HDF5 (included as a git submodule).
- [cmdline](https://github.com/tanakh/cmdline): A command-line argument parser (already integrated).
- [GoogleTest](https://github.com/google/googletest) (Optional): For unit testing when build in DEBUG mode.

Note: the compiler is configured at the 6th line of ./CMakeLists.txt, default
to the system compiler. Make sure it supports C++17 and OpenMP before continue.

---

## Installation

The installation process is standard for CMake projects. Ensure all dependencies
are installed, then run the following:

```shell
# 1. Clone the repository
git clone https://github.com/blackbhc/calrc.git --depth=1
cd calrc

# 2. Initialize submodules
git submodule update --init --recursive --depth=1

# 3. Configure the build
cmake -S . -DCMAKE_BUILD_TYPE=Release -B build

# 4. Compile
cmake --build build
The executable will be located in build/calrc. You can move it to any directory in your $PATH.
```

---

## Usage

Run `./calrc --help` to view the available options:

```shell
usage: ./calrc --if=string --rmax=double --rbin=int [options] ...
options:
-i, --if Input file (usually a GADGET-4 snapshot) (string)
-o, --of Output file for radial velocities (string [=rc.hdf5])
-r, --rmin Minimum radius in kpc (double [=0])
-R, --rmax Maximum radius in kpc (double)
-m, --rbin Number of radial bins (int)
-n, --phibin Number of azimuthal bins (int [=16])
-t, --type Spacing of radial bins (linear/log) (string [=log])
-c, --thread Thread count for parallel processing (int [=4])
-h, --help Print this message
```

## Example

```shell
./calrc --if=snapshot_000.hdf5 --of=acc.hdf5 --rmin=0.01 --rmax=40 --rbin=40 --phibin=16 --thread=12
```

This command calculates the radial force exerted by the material in
snapshot_000.hdf5 and saves the results to acc.hdf5. The measurement is performed
on a polar grid in the X-Y plane.

## Output File Format

This HDF5 file stores radial accelerations, together with the grid parameters.

### Root Group `/`

#### Global Attributes

The root group defines the polar grid configuration used at runtime.

| Attribute Name | Type               | Description                            |
| -------------- | ------------------ | -------------------------------------- |
| `Rmin`         | `float`            | Minimum radius of the polar grid.      |
| `Rmax`         | `float`            | Maximum radius of the polar grid.      |
| `RBinNum`      | `int`              | Number of radial bins.                 |
| `PhiBinNum`    | `int`              | Number of azimuthal bins.              |
| `GridRs`       | `float[RBinNum]`   | Radial coordinates of the grid points. |
| `GridPhis`     | `float[PhiBinNum]` | Azimuthal angles of the grid points.   |

---

#### Datasets

| Dataset Name | Shape                   | Type    | Description                                               |
| ------------ | ----------------------- | ------- | --------------------------------------------------------- |
| `Rs`         | `(RBinNum × PhiBinNum)` | `float` | Flattened array of radial coordinates for all grid cells. |
| `Phis`       | `(RBinNum × PhiBinNum)` | `float` | Flattened array of azimuthal angles for all grid cells.   |

---

#### Particle Type Groups

For each particle type (e.g., `PartType0`, `PartType1`, ...), a corresponding group exists under the root group.

##### Group `/PartTypeX`

| Dataset Name | Shape                   | Type    | Description                                                                          |
| ------------ | ----------------------- | ------- | ------------------------------------------------------------------------------------ |
| `AccRs`      | `(RBinNum × PhiBinNum)` | `float` | Radial acceleration contribution from particle type `X` evaluated on the polar grid. |

Following the usual physical convention, the outward direction is defined as
positive, and the acceleration is therefore frequently negative.
