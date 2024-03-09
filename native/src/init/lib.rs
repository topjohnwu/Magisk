#![feature(format_args_nl)]

use logging::setup_klog;
use mount::{is_device_mounted, switch_root};
use rootdir::inject_magisk_rc;
// Has to be pub so all symbols in that crate is included
pub use magiskpolicy;

mod logging;
mod mount;
mod rootdir;

#[cxx::bridge]
pub mod ffi {
    #[namespace = "rust"]
    extern "Rust" {
        fn setup_klog();
        fn inject_magisk_rc(fd: i32, tmp_dir: Utf8CStrRef);
        fn switch_root(path: Utf8CStrRef);
        fn is_device_mounted(dev: u64, target: Pin<&mut CxxString>) -> bool;
    }

    unsafe extern "C++" {
        include!("../base/include/base.hpp");

        #[namespace = "rust"]
        #[cxx_name = "Utf8CStr"]
        type Utf8CStrRef<'a> = base::ffi::Utf8CStrRef<'a>;
    }
}
