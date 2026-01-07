# Overview

A brutal but straightforward rotation curve calculator using the Newtonian summation.

Read a standard GADGET4 snapshot file (in HDF5 format) and get the corresponding
radial force at specified polar grid. It require the galactic center at the origin
and the disk located in the X-Y plane.

---

## Dependencies

- A C++ compiler that supports at least C++17.

- [`HDF5`](https://www.hdfgroup.org/solutions/hdf5/) for file IO.

- [`CMake`](https://cmake.org/) >= 3.13

- [`fmt`](https://github.com/fmtlib/fmt.git) for format output, included as a git
  sub module.

- [`highfive`](https://github.com/highfive-devs/highfive.git) for HDF5 file IO,
  included as a git sub module.

- (Already integrated) [`cmdline`](https://github.com/tanakh/cmdline) for command
  line parser.

- (optional) [`GoogleTest`](https://github.com/google/googletest) for unit test.

---

## Installation

The installation steps are straightforward if you have installed all the dependent
software and libraries. Just follow the below steps:

1. Clone the repository: `git clone https://github.com/blackbhc/calrc.git`
2. Get into the directory: `cd calrc`
3. Initialize the sub modules: `git submodule update --init --recursive --depth=1`
4. Configuration of the building:

   ```shell
   cmake -S . -DCMAKE_BUILD_TYPE=Release -B build
   ```

5. Compile: `cmake --build build`, if there is nothing wrong, then everything is
   done.

6. The executable is in `build/calrc`, copy/move it to anywhere you want to work
   with it.

---

## Usage

Use `calrc --help` to see the help message:

```shell
usage: ./calrc --if=string --rmax=double --rbin=int [options] ...
options:
  -i, --if        input file (usually, a snapshot) (string)
  -o, --of        output file of the radial velocities (string [=rc.hdf5])
  -r, --rmin      minimum radius in kpc (double [=0])
  -R, --rmax      maximum radius in kpc (double)
  -m, --rbin      radial bin number (int)
  -n, --phibin    azimuthal bin number (int [=16])
  -t, --type      form of the radial bins (string [=log])
  -c, --thread    Thread count (int [=4])
  -?, --help      print this message

```

Parameters with default values indicated in the trailing `[ ]` can be ignored,
and other parameters must be given.

An example (assume `calrc` is located at the current directory):

```shell
./calrc --if=snapshot_000.hdf5 --of=acc.hdf5 --rmin=0.01 --rmax=40 --rbin=40 --phibin=16 --thread=12
```

The above command with calculate the radial force due to the material in the
snapshot file specified by `--if`, then save the results as a file specified by
`--of`. The target positions of the measurement is a polar grid in the X-Y
plane, with range and bin counts specified by other parameters.

---

## Output file format

The output file is a HDF5 file with the following structure:

/ (Root Group)
│
├── . (Attributes: Rmin, Rmax, RBinNum, PhiBinNum used to run `calrc`, and the radius and azimuthal angles of the grid points.)
│
├── PartTypeX (Group)
│ ├── Dataset `AccRs` for the radial acceleration due to particles with type ID=PartTypeX.
│
├── Other PartTypes, if exist.
│ ├── ...
