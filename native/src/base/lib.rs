#![allow(clippy::missing_safety_doc)]

pub use const_format;
pub use libc;
use num_traits::FromPrimitive;

pub use cstr::{
    FsPathFollow, StrErr, Utf8CStr, Utf8CStrBuf, Utf8CStrBufArr, Utf8CStrBufRef, Utf8CString,
};
use cxx_extern::*;
pub use dir::*;
pub use ffi::fork_dont_care;
pub use files::*;
pub use logging::*;
pub use misc::*;
pub use result::*;

pub mod cstr;
mod cxx_extern;
mod dir;
mod files;
mod logging;
mod misc;
mod mount;
mod result;
mod xwrap;

#[cxx::bridge]
pub mod ffi {
    #[derive(Copy, Clone)]
    #[repr(i32)]
    #[cxx_name = "LogLevel"]
    pub(crate) enum LogLevelCxx {
        ErrorCxx,
        Error,
        Warn,
        Info,
        Debug,
    }

    unsafe extern "C++" {
        include!("misc.hpp");

        #[namespace = "rust"]
        #[cxx_name = "Utf8CStr"]
        type Utf8CStrRef<'a> = &'a crate::cstr::Utf8CStr;

        fn mut_u8_patch(buf: &mut [u8], from: &[u8], to: &[u8]) -> Vec<usize>;
        fn fork_dont_care() -> i32;
    }

    extern "Rust" {
        #[cxx_name = "log_with_rs"]
        fn log_from_cxx(level: LogLevelCxx, msg: Utf8CStrRef);
        #[cxx_name = "set_log_level_state"]
        fn set_log_level_state_cxx(level: LogLevelCxx, enabled: bool);
        fn exit_on_error(b: bool);
        fn cmdline_logging();
    }

    #[namespace = "rust"]
    extern "Rust" {
        fn xpipe2(fds: &mut [i32; 2], flags: i32) -> i32;
        #[cxx_name = "fd_path"]
        fn fd_path_for_cxx(fd: i32, buf: &mut [u8]) -> isize;
        #[cxx_name = "map_file"]
        fn map_file_for_cxx(path: Utf8CStrRef, rw: bool) -> &'static mut [u8];
        #[cxx_name = "map_file_at"]
        fn map_file_at_for_cxx(fd: i32, path: Utf8CStrRef, rw: bool) -> &'static mut [u8];
        #[cxx_name = "map_fd"]
        fn map_fd_for_cxx(fd: i32, sz: usize, rw: bool) -> &'static mut [u8];
    }
}

fn set_log_level_state_cxx(level: ffi::LogLevelCxx, enabled: bool) {
    if let Some(level) = LogLevel::from_i32(level.repr) {
        set_log_level_state(level, enabled)
    }
}
