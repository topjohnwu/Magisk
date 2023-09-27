#![allow(clippy::missing_safety_doc)]
#![feature(format_args_nl)]
#![feature(io_error_more)]
#![feature(utf8_chunks)]

pub use libc;

pub use cstr::*;
use cxx_extern::*;
pub use files::*;
pub use logging::*;
pub use misc::*;

mod cstr;
mod cxx_extern;
mod files;
mod logging;
mod misc;
mod xwrap;

#[cxx::bridge]
pub mod ffi {
    #[derive(Copy, Clone)]
    pub enum LogLevel {
        ErrorCxx,
        Error,
        Warn,
        Info,
        Debug,
    }

    unsafe extern "C++" {
        include!("misc.hpp");
        fn mut_u8_patch(buf: &mut [u8], from: &[u8], to: &[u8]) -> Vec<usize>;
    }

    extern "Rust" {
        #[cxx_name = "log_with_rs"]
        fn log_from_cxx(level: LogLevel, msg: &[u8]);
        fn exit_on_error(b: bool);
        fn set_log_level_state(level: LogLevel, enabled: bool);
        fn cmdline_logging();
    }

    #[namespace = "rust"]
    extern "Rust" {
        fn xpipe2(fds: &mut [i32; 2], flags: i32) -> i32;
        #[cxx_name = "fd_path"]
        fn fd_path_for_cxx(fd: i32, buf: &mut [u8]) -> isize;
        #[cxx_name = "map_file"]
        fn map_file_for_cxx(path: &[u8], rw: bool) -> &'static mut [u8];
        #[cxx_name = "map_fd"]
        fn map_fd_for_cxx(fd: i32, sz: usize, rw: bool) -> &'static mut [u8];
        fn enable_selinux();
    }
}
