# calrc — Rotation Curve Calculator (Rust)

A Rust re-implementation of the GADGET-4 HDF5 rotation curve calculator, originally written in C++17 with OpenMP. This version prioritises **running efficiency over safety** and uses **Rayon data parallelism** for full concurrency across grid points.

## Overview

`calrc` reads a GADGET-4 HDF5 snapshot, builds a polar grid of field points in the xy-plane, and computes the cylindrical radial gravitational acceleration at each grid point via direct Newton summation:

$$a_R(R, \phi) = G \sum_{i} \frac{m_i}{|\vec{r}_i - \vec{R}|^3} \big[(\vec{r}_i - \vec{R}) \cdot \hat{R}\big]$$

The result is written back to an HDF5 file with the same structure as the original C++ version.

## Prerequisites

- **Rust** edition 2024 (≥ 1.96)
- **HDF5** C library (development headers)

### macOS (Homebrew)

```bash
brew install hdf5
```

### Linux (Debian/Ubuntu)

```bash
sudo apt install libhdf5-dev pkg-config
```

### Linux (RHEL/Fedora)

```bash
sudo dnf install hdf5-devel pkgconfig
```

## Build

Set `HDF5_DIR` to your HDF5 installation prefix, then build:

```bash
# macOS Homebrew
export HDF5_DIR="/opt/homebrew"
# Linux apt – find your path with: pkg-config --variable=prefix hdf5
export HDF5_DIR="/usr/lib/x86_64-linux-gnu/hdf5/serial"
# Linux conda
export HDF5_DIR="$CONDA_PREFIX"

# Debug build
cargo build

# Optimised release build (recommended for production use)
RUSTFLAGS="-C target-cpu=native" cargo build --release
```

> **Tip:** The `.cargo/config.toml` file contains commented-out examples for common platforms. You can also set `HDF5_DIR` once in your shell profile (`~/.bashrc` / `~/.zshrc`).

The `RUSTFLAGS` flag enables the CPU's full instruction set (AVX2, FMA, etc.)
| `--thread` | `-c` | Number of parallel threads | `1` | No |

### Output HDF5 Structure

```
/
├── Rs                    [dataset, f64]  — flattened radial coordinates of all grid points
├── Phis                  [dataset, f64]  — flattened azimuthal angles
├── Rmin                  [attr, f64]     — minimum radius
├── Rmax                  [attr, f64]     — maximum radius
├── RBinNum               [attr, i32]     — number of radial bins (+1 for the outer edge)
├── PhiBinNum             [attr, i32]     — number of azimuthal bins
├── GridRs                [attr, f64[n]]  — radial bin edges
├── GridPhis              [attr, f64[n]]  — azimuthal bin edges
├── /PartType0/AccRs      [dataset, f64]  — radial accelerations from particle type 0
├── /PartType1/AccRs      [dataset, f64]  — (and so on for each type with N > 0)
└── ...
```

## Run Tests

```bash
cargo test
```

9 unit tests covering linspace generation, cylindrical geometry (radius, distance, radial projection — both on-axis and off-axis), N-body summation, and Rayon parallel grid computation.

## Architecture

```
src/
├── main.rs     — Pipeline orchestration: CLI → grid → HDF5 I/O
├── args.rs     — CLI argument parsing via clap derive (matches original cmdline.h)
└── grid.rs     — Core physics: GridPoint, PolarGrid, Newton summation
```

### Data Flow

```
CLI args
  → PolarGridPara
    → PolarGrid::new()           // generate grid points (r × φ)
      → open HDF5 snapshot (read-only)
        → read Header/NumPart_ThisFile
          → for each PartType with N > 0:
              1. read Masses (Vec<f64>)
              2. read Coordinates (Vec<[f64;3]>)
              3. cal_acc_r_from() → Rayon parallel grid acceleration
              4. write PartType{X}/AccRs
        → write Rs, Phis datasets
        → write global attributes (Rmin, Rmax, RBinNum, PhiBinNum, GridRs, GridPhis)
```

## Performance Design

This project follows the principle **running efficiency ≥ safety**. Deliberate `unsafe` usage in hot paths:

| Location | Technique | Benefit |
|----------|-----------|---------|
| `acc_r_from_unchecked` inner loop | Raw pointer indexing (`*mp.add(i)`, `*cp.add(3*i)`) | Zero bounds-check overhead for masses & coordinates |
| `distance_from` / `cyl_radial_comp` | `get_unchecked` on `[f64; 3]` | Skip slice bounds checks (length guaranteed by array type) |
| G constant | Multiplied once outside the particle loop | Avoids N redundant floating-point multiplications |
| Accumulator | Stack-allocated `[f64; 3]` | No heap allocation in hot path |
| Parallelism | Rayon `par_iter_mut()` with dedicated thread pool | Each grid point is independent — perfect parallel scaling |
| Compilation | `target-cpu=native` + LTO fat + codegen-units=1 | Maximum CPU-specific optimisation |

### Parallelisation Strategy

| Level | Method | Rationale |
|-------|--------|-----------|
| **Grid points** | Rayon `par_iter_mut().for_each()` | Each grid point's N-body sum is independent — replace OpenMP `#pragma omp parallel for` |
| **Thread pool** | New pool per `cal_acc_r_from` call | Controlled by `--thread`, avoids global pool interference |
| **Particle loop** | Serial (single-threaded per grid point) | Grid-level parallelism already saturates cores; inner parallelism adds overhead |
| **File I/O** | Serial | HDF5 library is thread-safe, but single-file writes gain nothing from parallelism |

## Comparison with Original C++ Version

| Aspect | Original C++ | This Rust Version |
|--------|-------------|-------------------|
| Language | C++17 | Rust 2024 |
| Parallelism | OpenMP `#pragma omp parallel for` | Rayon work-stealing thread pool |
| Boundary checks | None (native pointers) | `unsafe` raw pointers / `get_unchecked` |
| G multiplication | Outside loop | Outside loop |
| Build system | CMake + git submodules (HighFive, fmt) | Cargo (single `cargo build`) |
| Dependencies | External: HighFive, fmt, cmdline.h | Crates: clap, hdf5-metno, rayon, anyhow |
| Unit tests | GTest (separate build target) | `cargo test` (integrated) |

## Verification

Compare output with the original C++ version:

```bash
h5diff acc_cpp.hdf5 acc_rust.hdf5
```

Differences should be within floating-point rounding error (< 1e-12).

## Out of Scope (Phase 2)

- Python bindings (via PyO3)
- SIMD vectorisation of the inner particle loop
- SoA memory layout for coordinates (x, y, z as separate Vec<f64>)
- Particle chunking for large-N snapshots