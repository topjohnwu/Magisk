#![feature(vec_into_raw_parts)]
#![allow(clippy::missing_safety_doc)]

pub use {const_format, libc, nix};

pub use cstr::{
    FsPathFollow, StrErr, Utf8CStr, Utf8CStrBuf, Utf8CStrBufArr, Utf8CStrBufRef, Utf8CString,
};
use cxx_extern::*;
pub use derive;
pub use dir::*;
pub use ffi::{Utf8CStrRef, fork_dont_care, set_nice_name};
pub use files::*;
pub use logging::*;
pub use misc::*;
pub use result::*;

pub mod argh;
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
mod ffi {
    #[derive(Copy, Clone)]
    #[repr(i32)]
    #[cxx_name = "LogLevel"]
    pub(crate) enum LogLevelCxx {
        Error,
        Warn,
        Info,
        Debug,
    }

    unsafe extern "C++" {
        include!("base.hpp");

        #[cxx_name = "Utf8CStr"]
        type Utf8CStrRef<'a> = &'a crate::cstr::Utf8CStr;

        fn mut_u8_patch(buf: &mut [u8], from: &[u8], to: &[u8]) -> Vec<usize>;
        fn fork_dont_care() -> i32;
        fn set_nice_name(name: Utf8CStrRef);

        type FnBoolStrStr;
        fn call(self: &FnBoolStrStr, key: &str, value: &str) -> bool;

        type FnBoolStr;
        fn call(self: &FnBoolStr, key: Utf8CStrRef) -> bool;
    }

    extern "Rust" {
        #[cxx_name = "log_with_rs"]
        fn log_from_cxx(level: LogLevelCxx, msg: Utf8CStrRef);
        fn cmdline_logging();
        fn parse_prop_file_rs(name: Utf8CStrRef, f: &FnBoolStrStr);
        #[cxx_name = "file_readline"]
        fn file_readline_for_cxx(fd: i32, f: &FnBoolStr);
        fn xpipe2(fds: &mut [i32; 2], flags: i32) -> i32;
    }

    #[namespace = "rust"]
    extern "Rust" {
        #[cxx_name = "map_file"]
        fn map_file_for_cxx(path: Utf8CStrRef, rw: bool) -> &'static mut [u8];
        #[cxx_name = "map_file_at"]
        fn map_file_at_for_cxx(fd: i32, path: Utf8CStrRef, rw: bool) -> &'static mut [u8];
        #[cxx_name = "map_fd"]
        fn map_fd_for_cxx(fd: i32, sz: usize, rw: bool) -> &'static mut [u8];
    }
}

// In Rust, we do not want to deal with raw pointers, so we change the
// signature of all *mut c_void to usize for new_daemon_thread.
pub type ThreadEntry = extern "C" fn(usize) -> usize;
unsafe extern "C" {
    pub fn new_daemon_thread(entry: ThreadEntry, arg: usize);
}
