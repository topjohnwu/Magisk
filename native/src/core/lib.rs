pub use base;
use daemon::*;
use logging::*;
use std::ffi::CStr;

mod daemon;
mod logging;

#[cxx::bridge]
pub mod ffi {
    extern "C++" {
        include!("core.hpp");
        unsafe fn get_prop_rs(name: *const c_char, persist: bool) -> String;
    }

    extern "Rust" {
        fn rust_test_entry();
        fn android_logging();
        fn magisk_logging();
        fn zygisk_logging();
    }

    #[namespace = "rust"]
    extern "Rust" {
        fn daemon_entry();
        fn zygisk_entry();

        type MagiskD;
        fn get_magiskd() -> &'static MagiskD;
        fn get_log_pipe(self: &MagiskD) -> i32;
        fn close_log_pipe(self: &MagiskD);
        fn setup_logfile(self: &MagiskD);
    }
}

fn rust_test_entry() {}

pub fn get_prop(name: &CStr, persist: bool) -> String {
    unsafe { ffi::get_prop_rs(name.as_ptr(), persist) }
}
