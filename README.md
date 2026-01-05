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

To be complemented.

---

## Usage

To be complemented.
