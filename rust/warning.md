warning[E0133]: dereference of raw pointer is unsafe and requires unsafe block
--> src/grid.rs:112:21
|
112 | let m = \*mp.add(i);
| ^^^^^^^^^^ dereference of raw pointer
|
= note: raw pointers may be null, dangling or unaligned; they can violate aliasing rules and cause data races: all of these are undefined behavior
note: an unsafe function restricts its caller, but its body is safe by default
--> src/grid.rs:96:5
|
96 | / pub unsafe fn acc_r_from_flat_unchecked(
97 | | &self,
98 | | masses: &[f64],
99 | | coord_flat: &[f64],
100 | | ) -> f64 {
| |****\_\_\_\_****^
= note: for more information, see <https://doc.rust-lang.org/edition-guide/rust-2024/unsafe-op-in-unsafe-fn.html>
= note: `#[warn(unsafe_op_in_unsafe_fn)]` (part of `#[warn(rust_2024_compatibility)]`) on by default

warning[E0133]: call to unsafe function `std::ptr::const_ptr::<impl *const T>::add` is unsafe and requires unsafe block
--> src/grid.rs:112:22
|
112 | let m = \*mp.add(i);
| ^^^^^^^^^ call to unsafe function
|
= note: consult the function's documentation for information on how to avoid undefined behavior
= note: for more information, see <https://doc.rust-lang.org/edition-guide/rust-2024/unsafe-op-in-unsafe-fn.html>

warning[E0133]: dereference of raw pointer is unsafe and requires unsafe block
--> src/grid.rs:113:22
|
113 | let cx = _cp.add(3 _ i);
| ^^^^^^^^^^^^^^ dereference of raw pointer
|
= note: raw pointers may be null, dangling or unaligned; they can violate aliasing rules and cause data races: all of these are undefined behavior
= note: for more information, see <https://doc.rust-lang.org/edition-guide/rust-2024/unsafe-op-in-unsafe-fn.html>

warning[E0133]: call to unsafe function `std::ptr::const_ptr::<impl *const T>::add` is unsafe and requires unsafe block
--> src/grid.rs:113:23
|
113 | let cx = _cp.add(3 _ i);
| ^^^^^^^^^^^^^ call to unsafe function
|
= note: consult the function's documentation for information on how to avoid undefined behavior
= note: for more information, see <https://doc.rust-lang.org/edition-guide/rust-2024/unsafe-op-in-unsafe-fn.html>

warning[E0133]: dereference of raw pointer is unsafe and requires unsafe block
--> src/grid.rs:114:22
|
114 | let cy = _cp.add(3 _ i + 1);
| ^^^^^^^^^^^^^^^^^^ dereference of raw pointer
|
= note: raw pointers may be null, dangling or unaligned; they can violate aliasing rules and cause data races: all of these are undefined behavior
= note: for more information, see <https://doc.rust-lang.org/edition-guide/rust-2024/unsafe-op-in-unsafe-fn.html>

warning[E0133]: call to unsafe function `std::ptr::const_ptr::<impl *const T>::add` is unsafe and requires unsafe block
--> src/grid.rs:114:23
|
114 | let cy = _cp.add(3 _ i + 1);
| ^^^^^^^^^^^^^^^^^ call to unsafe function
|
= note: consult the function's documentation for information on how to avoid undefined behavior
= note: for more information, see <https://doc.rust-lang.org/edition-guide/rust-2024/unsafe-op-in-unsafe-fn.html>

warning[E0133]: dereference of raw pointer is unsafe and requires unsafe block
--> src/grid.rs:115:22
|
115 | let cz = _cp.add(3 _ i + 2);
| ^^^^^^^^^^^^^^^^^^ dereference of raw pointer
|
= note: raw pointers may be null, dangling or unaligned; they can violate aliasing rules and cause data races: all of these are undefined behavior
= note: for more information, see <https://doc.rust-lang.org/edition-guide/rust-2024/unsafe-op-in-unsafe-fn.html>

warning[E0133]: call to unsafe function `std::ptr::const_ptr::<impl *const T>::add` is unsafe and requires unsafe block
--> src/grid.rs:115:23
|
115 | let cz = _cp.add(3 _ i + 2);
| ^^^^^^^^^^^^^^^^^ call to unsafe function
|
= note: consult the function's documentation for information on how to avoid undefined behavior
= note: for more information, see <https://doc.rust-lang.org/edition-guide/rust-2024/unsafe-op-in-unsafe-fn.html>

warning: field `rbin` is never read
--> src/grid.rs:145:5
|
141 | pub struct PolarGrid {
| --------- field in this struct
...
145 | rbin: i32,
| ^^^^
|
= note: `PolarGrid` has derived impls for the traits `Clone` and `Debug`, but these are intentionally ignored during dead code analysis
= note: `#[warn(dead_code)]` (part of `#[warn(unused)]`) on by default

For more information about this error, try `rustc --explain E0133`.
warning: `calrc` (lib) generated 9 warnings (run `cargo fix --lib -p calrc` to apply 1 suggestion)
Finished `release` profile [optimized] target(s) in 18.51s
