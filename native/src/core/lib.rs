pub use base;
use daemon::*;
use logging::*;

mod daemon;
mod logging;

#[cxx::bridge]
pub mod ffi {
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
