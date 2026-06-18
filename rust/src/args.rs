/// CLI argument parsing using `clap::Parser`.
///
/// Maps directly to the original C++ `cmdline.h` interface.

use clap::Parser;

use crate::grid::{PolarGridPara, RbinType};

#[derive(Parser, Debug)]
#[command(
    name = "calrc",
    about = "Rotation curve calculator for GADGET-4 HDF5 snapshots",
    version
)]
pub struct Args {
    /// Input snapshot file (GADGET-4 HDF5)
    #[arg(short = 'i', long = "if", required = true)]
    pub infile: String,

    /// Output HDF5 file for radial accelerations
    #[arg(short = 'o', long = "of", default_value = "rc.hdf5")]
    pub outfile: String,

    /// Minimum radius in kpc
    #[arg(short = 'r', long = "rmin", default_value_t = 0.0)]
    pub rmin: f64,

    /// Maximum radius in kpc
    #[arg(short = 'R', long = "rmax", required = true)]
    pub rmax: f64,

    /// Number of radial bins
    #[arg(short = 'm', long = "rbin", required = true)]
    pub rbin: i32,

    /// Number of azimuthal bins
    #[arg(short = 'n', long = "phibin", default_value_t = 16)]
    pub phibin: i32,

    /// Radial bin type: "log" or "linear"
    #[arg(short = 't', long = "type", default_value = "log")]
    pub rtype: String,

    /// Number of parallel threads
    #[arg(short = 'c', long = "thread", default_value_t = 1)]
    pub thread: usize,
}

impl Args {
    /// Convert the CLI arguments into a [`PolarGridPara`] struct.
    pub fn polar_paras(&self) -> PolarGridPara {
        let rtype = match self.rtype.as_str() {
            "linear" => RbinType::Linear,
            "log" => RbinType::Log,
            other => panic!("Invalid radial bin type '{}': must be 'log' or 'linear'", other),
        };
        PolarGridPara {
            rmin: self.rmin,
            rmax: self.rmax,
            rbin: self.rbin,
            phibin: self.phibin,
            rtype,
        }
    }
}