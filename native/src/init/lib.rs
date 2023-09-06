#![feature(format_args_nl)]

use logging::setup_klog;
use unxz::unxz;
// Has to be pub so all symbols in that crate is included
pub use magiskpolicy;

mod logging;
mod unxz;

#[cxx::bridge]
pub mod ffi {
    #[namespace = "rust"]
    extern "Rust" {
        fn setup_klog();
        fn unxz(fd: i32, buf: &[u8]) -> bool;
    }
}
