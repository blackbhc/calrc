/// Binary entry point — rotation curve calculator CLI.

use anyhow::{bail, Context, Result};
use clap::Parser;
use hdf5::File;

mod args;
use args::Args;

// Import the library crate's grid module
use calrc::{PolarGrid, RbinType};

fn main() -> Result<()> {
    let args = Args::parse();
    let paras = args.polar_paras();

    if paras.rmax <= paras.rmin {
        bail!("rmax ({}) must be larger than rmin ({})", paras.rmax, paras.rmin);
    }
    if paras.rmin == 0.0 {
        eprintln!("Warning: minimum radius = 0, grid points at the origin.");
        if paras.rtype == RbinType::Log {
            bail!("Logarithmic radial bins are invalid when rmin = 0");
        }
    }

    let grid = PolarGrid::new(&paras);

    let snapshot = File::open(&args.infile)
        .with_context(|| format!("Cannot open snapshot '{}'", args.infile))?;

    let header = snapshot.group("Header")?;
    let num_part_this_file: Vec<i32> = header
        .attr("NumPart_ThisFile")?
        .read_raw()
        .context("Failed to read NumPart_ThisFile attribute")?;

    let outfile = File::create(&args.outfile)
        .with_context(|| format!("Cannot create output '{}'", args.outfile))?;

    let pool = PolarGrid::create_pool(args.thread);

    for (i, &n) in num_part_this_file.iter().enumerate() {
        if n <= 0 {
            continue;
        }

        let type_path = format!("/PartType{}", i);

        let masses: Vec<f64> = snapshot
            .dataset(&format!("{}/Masses", type_path))?
            .read_raw()
            .with_context(|| format!("Failed to read {}/Masses", type_path))?;

        let flat_coords: Vec<f64> = snapshot
            .dataset(&format!("{}/Coordinates", type_path))?
            .read_raw()
            .with_context(|| format!("Failed to read {}/Coordinates", type_path))?;

        let acc_rs = grid.cal_acc_r_from_flat(&masses, &flat_coords, &pool);

        let ds_name = format!("{}/AccRs", type_path);
        let ds = outfile
            .new_dataset::<f64>()
            .shape(acc_rs.len())
            .create(ds_name.as_str())
            .with_context(|| format!("Failed to create dataset {}/AccRs", type_path))?;
        ds.write_raw(&acc_rs)
            .with_context(|| format!("Failed to write {}/AccRs", type_path))?;
    }

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

    calrc::write_attr_f64(&outfile, "Rmin", paras.rmin)?;
    calrc::write_attr_f64(&outfile, "Rmax", paras.rmax)?;
    calrc::write_attr_i32(&outfile, "RBinNum", paras.rbin + 1)?;
    calrc::write_attr_i32(&outfile, "PhiBinNum", paras.phibin)?;
    calrc::write_attr_f64_vec(&outfile, "GridRs", grid.rbin_edges())?;
    calrc::write_attr_f64_vec(&outfile, "GridPhis", grid.phibin_edges())?;

    Ok(())
}