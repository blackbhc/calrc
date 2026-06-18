/// calrc — Rotation Curve Calculator library.
///
/// Provides:
/// - [`GridPoint`] / [`PolarGrid`]: core N-body summation engine
/// - C FFI (`cal_acc_rs`, `free_acc_rs`): callable from Python via ctypes
/// - HDF5 I/O helpers (`write_attr_f64`, etc.)

pub mod grid;

use std::alloc::{alloc, dealloc, Layout};
use std::sync::Mutex;

pub use grid::{GridPoint, PolarGrid, PolarGridPara, RbinType};

// ---------------------------------------------------------------------------
// HDF5 attribute helpers
// ---------------------------------------------------------------------------

use anyhow::{Context, Result};
use hdf5::File;

pub fn write_attr_f64(file: &File, name: &str, value: f64) -> Result<()> {
    let attr = file
        .new_attr::<f64>()
        .shape::<[usize; 0]>([])
        .create(name)
        .with_context(|| format!("Failed to create attribute '{}'", name))?;
    attr.write_scalar(&value)?;
    Ok(())
}

pub fn write_attr_i32(file: &File, name: &str, value: i32) -> Result<()> {
    let attr = file
        .new_attr::<i32>()
        .shape::<[usize; 0]>([])
        .create(name)
        .with_context(|| format!("Failed to create attribute '{}'", name))?;
    attr.write_scalar(&value)?;
    Ok(())
}

pub fn write_attr_f64_vec(file: &File, name: &str, data: &[f64]) -> Result<()> {
    let attr = file
        .new_attr::<f64>()
        .shape([data.len()])
        .create(name)
        .with_context(|| format!("Failed to create attribute '{}'", name))?;
    attr.write_raw(data)?;
    Ok(())
}

// ---------------------------------------------------------------------------
// C FFI — for Python bindings
// ---------------------------------------------------------------------------

/// Wrapper to make `*mut f64` implement `Send` for static storage.
/// This mirrors the original C++ `py.cpp` pattern (global `double* data`)
/// where thread-safety is the caller's responsibility.
struct SendPtr(*mut f64);
unsafe impl Send for SendPtr {}

/// Global state to track the allocated output buffer.
static ALLOC: Mutex<Option<(Layout, SendPtr)>> = Mutex::new(None);

use rayon::prelude::*;

/// Compute cylindrical radial accelerations at grid points.
///
/// The returned pointer must be freed with [`free_acc_rs`].
///
/// # Safety
///
/// All pointer arguments must be valid, aligned, non-overlapping,
/// and point to at least the declared number of elements.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn cal_acc_rs(
    num_grid: i32,
    grid_xy: *const f64,
    num_part: i32,
    masses: *const f64,
    coords: *const f64,
    num_thread: i32,
) -> *mut f64 {
    if num_grid <= 0 || num_part <= 0
        || grid_xy.is_null() || masses.is_null() || coords.is_null()
    {
        return std::ptr::null_mut();
    }

    let ng = num_grid as usize;
    let np = num_part as usize;
    let nt = if num_thread > 0 { num_thread as usize } else { 1 };

    // SAFETY: caller guarantees valid pointers and lengths
    let grid_slice = unsafe { std::slice::from_raw_parts(grid_xy, ng * 2) };
    let masses_slice = unsafe { std::slice::from_raw_parts(masses, np) };
    let coords_slice = unsafe { std::slice::from_raw_parts(coords, np * 3) };

    // Build GridPoint vector from flat (x, y) — z = 0 always
    let points: Vec<GridPoint> = grid_slice
        .chunks_exact(2)
        .map(|xy| GridPoint::new(xy[0], xy[1], 0.0))
        .collect();

    // Allocate output buffer
    let layout = Layout::array::<f64>(ng).expect("Layout error");
    let out_ptr = unsafe { alloc(layout) as *mut f64 };
    if out_ptr.is_null() {
        return std::ptr::null_mut();
    }
    // Zero-initialise
    unsafe { std::ptr::write_bytes(out_ptr, 0, ng); }

    // Create mutable slice before entering the pool (avoids raw ptr in closure)
    let out_slice = unsafe { std::slice::from_raw_parts_mut(out_ptr, ng) };

    // Create thread pool and compute in parallel
    let pool = rayon::ThreadPoolBuilder::new()
        .num_threads(nt)
        .build()
        .expect("Failed to build Rayon thread pool");

    pool.install(|| {
        out_slice.par_iter_mut().enumerate().for_each(|(i, a)| {
            // SAFETY: all slices have correct lengths (caller guarantee)
            *a = unsafe { points[i].acc_r_from_flat_unchecked(masses_slice, coords_slice) };
        });
    });

    // Store layout + pointer for later cleanup
    if let Ok(mut guard) = ALLOC.lock() {
        *guard = Some((layout, SendPtr(out_ptr)));
    }

    out_ptr
}

/// Free memory allocated by [`cal_acc_rs`].
///
/// # Safety
///
/// Must only be called after [`cal_acc_rs`]. Calling twice without an
/// intervening call is undefined behaviour.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn free_acc_rs() {
    if let Ok(mut guard) = ALLOC.lock() {
        if let Some((layout, SendPtr(ptr))) = guard.take() {
            if !ptr.is_null() {
                unsafe { dealloc(ptr as *mut u8, layout); }
            }
        }
    }
}