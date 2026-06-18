/// Rotation-curve calculator for GADGET-4 HDF5 snapshots.

use anyhow::{bail, Context, Result};
use clap::Parser;
use hdf5::File;

mod args;
mod grid;

use args::Args;
use grid::{PolarGrid, RbinType};

fn main() -> Result<()> {
    // ---- 1. Parse CLI arguments
    let args = Args::parse();
    let paras = args.polar_paras();

    // ---- 2. Input validation
    if paras.rmax <= paras.rmin {
        bail!("rmax ({}) must be larger than rmin ({})", paras.rmax, paras.rmin);
    }
    if paras.rmin == 0.0 {
        eprintln!("Warning: minimum radius = 0, grid points at the origin.");
        if paras.rtype == RbinType::Log {
            bail!("Logarithmic radial bins are invalid when rmin = 0");
        }
    }

    // ---- 3. Build polar grid
    let grid = PolarGrid::new(&paras);

    // ---- 4. Open input snapshot
    let snapshot = File::open(&args.infile)
        .with_context(|| format!("Cannot open snapshot '{}'", args.infile))?;

    // ---- 5. Read particle counts per type
    let header = snapshot.group("Header")?;
    let num_part_this_file: Vec<i32> = header
        .attr("NumPart_ThisFile")?
        .read_raw()
        .context("Failed to read NumPart_ThisFile attribute")?;

    // ---- 6. Create output file
    let outfile = File::create(&args.outfile)
        .with_context(|| format!("Cannot create output '{}'", args.outfile))?;

    // ---- 6b. Create thread pool once, reuse for all types
    let pool = PolarGrid::create_pool(args.thread);

    // ---- 7. Process each particle type
    for (i, &n) in num_part_this_file.iter().enumerate() {
        if n <= 0 {
            continue;
        }

        let type_path = format!("/PartType{}", i);

        // -- 7a. Read masses
        let masses: Vec<f64> = snapshot
            .dataset(&format!("{}/Masses", type_path))?
            .read_raw()
            .with_context(|| format!("Failed to read {}/Masses", type_path))?;

        // -- 7b. Read coordinates flat (no chunk_exact conversion)
        let flat_coords: Vec<f64> = snapshot
            .dataset(&format!("{}/Coordinates", type_path))?
            .read_raw()
            .with_context(|| format!("Failed to read {}/Coordinates", type_path))?;

        // -- 7c. Compute radial accelerations (flat API, no Vec<[f64;3]> alloc)
        let acc_rs = grid.cal_acc_r_from_flat(&masses, &flat_coords, &pool);

        // -- 7d. Write output dataset
        let ds_name = format!("{}/AccRs", type_path);
        let ds = outfile
            .new_dataset::<f64>()
            .shape(acc_rs.len())
            .create(ds_name.as_str())
            .with_context(|| format!("Failed to create dataset {}/AccRs", type_path))?;
        ds.write_raw(&acc_rs)
            .with_context(|| format!("Failed to write {}/AccRs", type_path))?;
    }

    // ---- 8. Write Rs and Phis datasets
    let rs = grid.rs();
    let ds_rs = outfile
        .new_dataset::<f64>()
        .shape(rs.len())
        .create("Rs")?;
    ds_rs.write_raw(&rs)?;

    let phis = grid.phis();
    let ds_phis = outfile
        .new_dataset::<f64>()
        .shape(phis.len())
        .create("Phis")?;
    ds_phis.write_raw(&phis)?;

    // ---- 9. Write global attributes
    write_attr_f64(&outfile, "Rmin", paras.rmin)?;
    write_attr_f64(&outfile, "Rmax", paras.rmax)?;
    write_attr_i32(&outfile, "RBinNum", paras.rbin + 1)?;
    write_attr_i32(&outfile, "PhiBinNum", paras.phibin)?;
    write_attr_f64_vec(&outfile, "GridRs", grid.rbin_edges())?;
    write_attr_f64_vec(&outfile, "GridPhis", grid.phibin_edges())?;

    Ok(())
}

fn write_attr_f64(file: &File, name: &str, value: f64) -> Result<()> {
    let attr = file
        .new_attr::<f64>()
        .shape::<[usize; 0]>([])
        .create(name)
        .with_context(|| format!("Failed to create attribute '{}'", name))?;
    attr.write_scalar(&value)?;
    Ok(())
}

fn write_attr_i32(file: &File, name: &str, value: i32) -> Result<()> {
    let attr = file
        .new_attr::<i32>()
        .shape::<[usize; 0]>([])
        .create(name)
        .with_context(|| format!("Failed to create attribute '{}'", name))?;
    attr.write_scalar(&value)?;
    Ok(())
}

fn write_attr_f64_vec(file: &File, name: &str, data: &[f64]) -> Result<()> {
    let attr = file
        .new_attr::<f64>()
        .shape([data.len()])
        .create(name)
        .with_context(|| format!("Failed to create attribute '{}'", name))?;
    attr.write_raw(data)?;
    Ok(())
}