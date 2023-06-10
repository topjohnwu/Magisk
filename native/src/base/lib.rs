#![allow(clippy::missing_safety_doc)]
#![feature(format_args_nl)]
#![feature(io_error_more)]

pub use libc;

use cxx_extern::*;
pub use files::*;
pub use logging::*;
pub use misc::*;
pub use xwrap::*;

mod cxx_extern;
mod files;
mod logging;
mod misc;
mod xwrap;

#[cxx::bridge]
pub mod ffi {
    #[derive(Copy, Clone)]
    pub enum LogLevel {
        Error,
        Warn,
        Info,
        Debug,
    }

    extern "Rust" {
        fn log_with_rs(level: LogLevel, msg: &[u8]);
        fn exit_on_error(b: bool);
        fn set_log_level_state(level: LogLevel, enabled: bool);
        fn cmdline_logging();
    }

    #[namespace = "rust"]
    extern "Rust" {
        fn xpipe2(fds: &mut [i32; 2], flags: i32) -> i32;
        #[rust_name = "fd_path_for_cxx"]
        fn fd_path(fd: i32, buf: &mut [u8]) -> isize;
    }
}
