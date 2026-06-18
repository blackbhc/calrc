/// Polar grid of field points, used to compute radial gravitational
/// accelerations from a particle set via Newton summation.
///
/// The call order is:
/// 1. [`PolarGrid::new`] to create the grid from parameters
/// 2. [`PolarGrid::cal_acc_r_from`] to compute accelerations in parallel
/// 3. [`PolarGrid::rs`] / [`PolarGrid::phis`] to export flattened coordinates

use rayon::prelude::*;

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

/// Numerical gravitational constant in Gadget-4 internal units
/// (kpc, km/s, 1e10 M☉).
const G: f64 = 43007.1;

/// π to full precision.
const PI: f64 = 3.14159265358979323846264338327950288;

// ---------------------------------------------------------------------------
// Radial bin type
// ---------------------------------------------------------------------------

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum RbinType {
    Log,
    Linear,
}

// ---------------------------------------------------------------------------
// Grid parameters (aggregate input)
// ---------------------------------------------------------------------------

#[derive(Debug, Clone, Copy)]
pub struct PolarGridPara {
    pub rmin: f64,
    pub rmax: f64,
    pub rbin: i32,
    pub phibin: i32,
    pub rtype: RbinType,
}

// ---------------------------------------------------------------------------
// Single grid point
// ---------------------------------------------------------------------------

/// A single field point where the radial acceleration is evaluated.
#[derive(Debug, Clone, Copy)]
pub struct GridPoint {
    x: f64,
    y: f64,
    z: f64,
}

impl GridPoint {
    #[inline(always)]
    pub const fn new(x: f64, y: f64, z: f64) -> Self {
        Self { x, y, z }
    }

    /// Cylindrical radius √(x² + y²).
    #[inline(always)]
    pub fn cyl_radius(&self) -> f64 {
        (self.x * self.x + self.y * self.y).sqrt()
    }

    /// Radial component (cylindrical) of a force vector acting on this point.
    ///
    /// When the point lies on the z-axis (`R == 0`) the result is the
    /// magnitude of the force projected onto the xy-plane.
    #[inline(always)]
    fn cyl_radial_comp(&self, force: &[f64; 3]) -> f64 {
        let r = self.cyl_radius();
        if r == 0.0 {
            // On-axis: return norm of xy-plane force
            (force[0] * force[0] + force[1] * force[1]).sqrt()
        } else {
            // Off-axis: dot(force, unit radial vector)
            (force[0] * self.x + force[1] * self.y) / r
        }
    }

    /// Compute the cylindrical radial acceleration at this point produced by
    /// an ensemble of particles.
    ///
    /// # Safety
    ///
    /// - `masses` and `coordinates` must have the same length.
    /// - `coordinates` is a flat Vec<f64> with stride 3 per particle.
    ///
    /// The caller guarantees both invariants, so we deliberately skip all
    /// bounds checking inside the critical loop.
    #[inline(always)]
    unsafe fn acc_r_from_flat_unchecked(
        &self,
        masses: &[f64],
        coord_flat: &[f64],
    ) -> f64 {
        // Stack-allocated accumulator
        let mut sx = 0.0_f64;
        let mut sy = 0.0_f64;
        let mut sz = 0.0_f64;

        let n = masses.len();
        let mp = masses.as_ptr();
        let cp = coord_flat.as_ptr();

        for i in 0..n {
            // SAFETY: caller guarantees length match
            let m = *mp.add(i);
            let cx = *cp.add(3 * i);
            let cy = *cp.add(3 * i + 1);
            let cz = *cp.add(3 * i + 2);

            let dx = cx - self.x;
            let dy = cy - self.y;
            let dz = cz - self.z;

            let r2 = dx * dx + dy * dy + dz * dz;
            let inv_r3 = 1.0 / (r2 * r2.sqrt()); // 1 / r³

            let scalar = m * inv_r3; // m / r³
            sx += scalar * dx;
            sy += scalar * dy;
            sz += scalar * dz;
        }

        // Multiply by G once, outside the loop.
        let acc = [sx * G, sy * G, sz * G];
        self.cyl_radial_comp(&acc)
    }
}

// ---------------------------------------------------------------------------
// Polar grid
// ---------------------------------------------------------------------------

#[derive(Debug, Clone)]
pub struct PolarGrid {
    points: Vec<GridPoint>,
    rbin_edges: Vec<f64>,
    phibin_edges: Vec<f64>,
    rbin: i32,
    phibin: i32,
}

/// Create a linearly-spaced vector of bin edges.
fn linspace(min: f64, max: f64, binnum: i32, with_right_bound: bool) -> Vec<f64> {
    let delta = (max - min) / f64::from(binnum);
    let n = if with_right_bound { binnum + 1 } else { binnum } as usize;
    let mut out = Vec::with_capacity(n);
    for i in 0..n {
        out.push(min + delta * i as f64);
    }
    out
}

impl PolarGrid {
    /// Build the polar grid from the given parameters.
    pub fn new(para: &PolarGridPara) -> Self {
        let rbin = para.rbin;
        let phibin = para.phibin;

        let phibin_edges = linspace(0.0, 2.0 * PI, phibin, false);

        let rbin_edges = match para.rtype {
            RbinType::Linear => linspace(para.rmin, para.rmax, rbin, true),
            RbinType::Log => {
                let raw = linspace(para.rmin.log10(), para.rmax.log10(), rbin, true);
                raw.into_iter().map(|v| 10.0_f64.powf(v)).collect()
            }
        };

        let npoints = (rbin + 1) as usize * phibin as usize;
        let mut points = Vec::with_capacity(npoints);
        for &r in &rbin_edges {
            for &phi in &phibin_edges {
                points.push(GridPoint::new(r * phi.cos(), r * phi.sin(), 0.0));
            }
        }

        Self {
            points,
            rbin_edges,
            phibin_edges,
            rbin,
            phibin,
        }
    }

    /// Compute the cylindrical radial acceleration at every grid point from
    /// the given particle set.
    ///
    /// Accepts flat coordinates for zero-copy interop with HDF5 output.
    /// Each grid point is processed independently in parallel using Rayon.
    pub fn cal_acc_r_from_flat(
        &self,
        masses: &[f64],
        coord_flat: &[f64],
        pool: &rayon::ThreadPool,
    ) -> Vec<f64> {
        let points = &self.points;
        let mut acc_r = vec![0.0_f64; points.len()];

        pool.install(|| {
            acc_r.par_iter_mut().enumerate().for_each(|(i, a)| {
                // SAFETY: masses.len() * 3 == coord_flat.len(), caller guarantees
                *a = unsafe { points[i].acc_r_from_flat_unchecked(masses, coord_flat) };
            });
        });

        acc_r
    }

    /// Create a Rayon thread pool with the specified number of threads.
    pub fn create_pool(num_thread: usize) -> rayon::ThreadPool {
        rayon::ThreadPoolBuilder::new()
            .num_threads(num_thread)
            .build()
            .expect("Failed to build Rayon thread pool")
    }

    /// Flattened radial coordinates of all grid points.
    pub fn rs(&self) -> Vec<f64> {
        let mut out = Vec::with_capacity(self.points.len());
        for &r in &self.rbin_edges {
            for _ in 0..self.phibin as usize {
                out.push(r);
            }
        }
        out
    }

    /// Flattened azimuthal angles of all grid points.
    pub fn phis(&self) -> Vec<f64> {
        let mut out = Vec::with_capacity(self.points.len());
        for _ in &self.rbin_edges {
            for &phi in &self.phibin_edges {
                out.push(phi);
            }
        }
        out
    }

    pub fn rbin_edges(&self) -> &[f64] {
        &self.rbin_edges
    }

    pub fn phibin_edges(&self) -> &[f64] {
        &self.phibin_edges
    }
}

// ---------------------------------------------------------------------------
// Unit tests
// ---------------------------------------------------------------------------

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_linspace_inclusive() {
        let v = linspace(0.0, 1.0, 4, true);
        assert_eq!(v.len(), 5);
        let expected = [0.0, 0.25, 0.5, 0.75, 1.0];
        for (a, b) in v.iter().zip(expected.iter()) {
            assert!((a - b).abs() < 1e-15);
        }
    }

    #[test]
    fn test_linspace_exclusive() {
        let v = linspace(0.0, 2.0 * PI, 16, false);
        assert_eq!(v.len(), 16);
        for (i, &val) in v.iter().enumerate() {
            let expected = (2.0 * PI / 16.0) * i as f64;
            assert!((val - expected).abs() < 1e-15);
        }
    }

    #[test]
    fn test_gridpoint_cyl_radius() {
        let gp = GridPoint::new(3.0, 4.0, 0.0);
        assert!((gp.cyl_radius() - 5.0).abs() < 1e-15);
    }

    #[test]
    fn test_single_particle_acc_r_flat() {
        let gp = GridPoint::new(1.0, 0.0, 0.0);
        let masses = vec![1.0];
        let coords = vec![1.0, 0.0, 0.0];
        let _acc = unsafe { gp.acc_r_from_flat_unchecked(&masses, &coords) };
    }

    #[test]
    fn test_two_particles_symmetric_flat() {
        let gp = GridPoint::new(0.0, 0.0, 0.0);
        let masses = vec![1.0, 1.0];
        let coords = vec![-1.0, 0.0, 0.0, 1.0, 0.0, 0.0];
        let acc = unsafe { gp.acc_r_from_flat_unchecked(&masses, &coords) };
        assert!(acc.abs() < 1e-10);
    }

    #[test]
    fn test_polar_grid_rs_phis_length() {
        let para = PolarGridPara {
            rmin: 0.1, rmax: 10.0, rbin: 5, phibin: 4, rtype: RbinType::Log,
        };
        let grid = PolarGrid::new(&para);
        assert_eq!(grid.rs().len(), (5 + 1) * 4);
        assert_eq!(grid.phis().len(), (5 + 1) * 4);
    }

    #[test]
    fn test_polar_grid_cal_acc_r_flat() {
        let para = PolarGridPara {
            rmin: 1.0, rmax: 2.0, rbin: 2, phibin: 2, rtype: RbinType::Linear,
        };
        let grid = PolarGrid::new(&para);
        let masses = vec![1e10];
        let coords = vec![0.0, 0.0, 0.0];
        let pool = PolarGrid::create_pool(2);
        let acc = grid.cal_acc_r_from_flat(&masses, &coords, &pool);
        assert_eq!(acc.len(), (2 + 1) * 2);
    }
}