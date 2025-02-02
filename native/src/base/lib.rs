#![allow(clippy::missing_safety_doc)]
#![feature(format_args_nl)]
#![feature(io_error_more)]

use std::ffi::CStr;
use std::ops::Deref;
pub use const_format;
pub use libc;
use num_traits::FromPrimitive;

pub use cstr::*;
use cxx_extern::*;
pub use ffi::fork_dont_care;
pub use files::*;
pub use logging::*;
pub use misc::*;
pub use result::*;
pub use crate::ffi::Utf8CStrRef;

mod cstr;
mod cxx_extern;
mod files;
mod logging;
mod misc;
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

    // struct Utf8CStrRef {}
    #[namespace = "rust"]
    #[cxx_name = "Utf8CStr"]
    struct Utf8CStrRef<'a> {
        #[cxx_name = "data_"]
        data: &'a [u8],
    }

    unsafe extern "C++" {
        include!("misc.hpp");

        #[namespace = "std"]
        #[cxx_name = "string_view"]
        type StringView;

        fn mut_u8_patch(buf: &mut [u8], from: &[u8], to: &[u8]) -> Vec<usize>;
        fn fork_dont_care() -> i32;
    }
    
    #[namespace = "rust"]
    unsafe extern "C++" {
        #[Self = "Utf8CStrRef"]
        unsafe fn from_string_view<'a>(s: &StringView) -> Self;

        #[Self = "Utf8CStrRef"]
        unsafe fn from_string<'a>(s: &CxxString) -> Self;
    }

    extern "Rust" {
        #[cxx_name = "log_with_rs"]
        fn log_from_cxx(level: LogLevelCxx, msg: Utf8CStrRef);
        #[cxx_name = "set_log_level_state"]
        fn set_log_level_state_cxx(level: LogLevelCxx, enabled: bool);
        fn exit_on_error(b: bool);
        fn cmdline_logging();
        fn resize_vec(vec: &mut Vec<u8>, size: usize);
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
        #[Self = "Utf8CStrRef"]
        unsafe fn from_slice_unchecked<'a>(ptr: *const c_char, len: usize) -> Self;

        #[Self = "Utf8CStrRef"]
        unsafe fn from_ptr_unchecked<'a>(ptr: *const c_char) -> Self;

        unsafe fn data(self: &Utf8CStrRef) -> *const c_char;
        unsafe fn c_str(self: &Utf8CStrRef) -> *const c_char;

        fn size(self: &Utf8CStrRef) -> usize;
        fn empty(self: &Utf8CStrRef) -> bool;
    } // namespace rust
}

fn set_log_level_state_cxx(level: ffi::LogLevelCxx, enabled: bool) {
    if let Some(level) = LogLevel::from_i32(level.repr) {
        set_log_level_state(level, enabled)
    }
}

fn resize_vec(vec: &mut Vec<u8>, size: usize) {
    if size > vec.len() {
        vec.reserve(size - vec.len());
    }
    unsafe {
        vec.set_len(size);
    }
}

impl <'a> Utf8CStrRef<'a> {
    unsafe fn data(&self) -> *const std::ffi::c_char {
        self.data.as_ptr().cast()
    }
    
    unsafe fn c_str(&self) -> *const std::ffi::c_char {
        self.data.as_ptr().cast()
    }
    #[inline(always)]
    pub unsafe fn from_bytes_unchecked(buf: &'a [u8]) -> Self {
        Self {
            data: buf,
        }
    }
    #[inline(always)]
    pub unsafe fn from_bytes_unchecked_mut(buf: &'a mut [u8]) -> Self {
        Self::from_bytes_unchecked(buf)
    }

    pub unsafe fn from_ptr_unchecked(ptr: *const std::ffi::c_char) -> Self {
        let cstr = CStr::from_ptr(ptr);
        Self::from_bytes_unchecked(cstr.to_bytes_with_nul())
    }

    pub unsafe fn from_slice_unchecked(ptr: *const std::ffi::c_char, _len: usize) -> Self {
        Self::from_bytes_unchecked(CStr::from_ptr(ptr).to_bytes_with_nul())
    }
    
    pub fn size(&self) -> usize {
        self.data.len()
    }
    
    pub fn empty(&self) -> bool {
        self.data.is_empty()
    }
}

impl <'a> Deref for Utf8CStrRef<'a> {
    type Target = Utf8CStr;

    fn deref(&self) -> &Self::Target {
        unsafe { Utf8CStr::from_bytes_unchecked(self.data) }
    }
}

impl <'a> From<&'a Utf8CStr> for Utf8CStrRef<'a> {
    fn from(s: &'a Utf8CStr) -> Self {
        unsafe { Utf8CStrRef::from_bytes_unchecked(s.as_bytes()) }
    }
}

impl <'a> From<&'a mut Utf8CStr> for Utf8CStrRef<'a> {
    fn from(s: &'a mut Utf8CStr) -> Self {
        unsafe { Utf8CStrRef::from_bytes_unchecked(s.as_bytes()) }
    }
}
