// Functions listed here are just to export to C++

use crate::{fd_path, mkdirs, realpath, rm_rf, slice_from_ptr_mut, Directory, ResultExt};
use anyhow::Context;
use cxx::private::c_char;
use libc::mode_t;
use std::ffi::CStr;
use std::io;
use std::os::fd::{OwnedFd, RawFd};

pub fn fd_path_for_cxx(fd: RawFd, buf: &mut [u8]) -> isize {
    fd_path(fd, buf)
        .context("fd_path failed")
        .log()
        .map_or(-1_isize, |v| v as isize)
}

#[no_mangle]
unsafe extern "C" fn canonical_path(path: *const c_char, buf: *mut u8, bufsz: usize) -> isize {
    realpath(CStr::from_ptr(path), slice_from_ptr_mut(buf, bufsz)).map_or(-1, |v| v as isize)
}

#[export_name = "mkdirs"]
unsafe extern "C" fn mkdirs_for_cxx(path: *const c_char, mode: mode_t) -> i32 {
    mkdirs(CStr::from_ptr(path), mode).map_or(-1, |_| 0)
}

#[export_name = "rm_rf"]
unsafe extern "C" fn rm_rf_for_cxx(path: *const c_char) -> bool {
    rm_rf(CStr::from_ptr(path)).map_or(false, |_| true)
}

#[no_mangle]
unsafe extern "C" fn frm_rf(fd: OwnedFd) -> bool {
    fn inner(fd: OwnedFd) -> io::Result<()> {
        Directory::try_from(fd)?.remove_all()
    }
    inner(fd).map_or(false, |_| true)
}
