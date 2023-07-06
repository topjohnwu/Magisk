// Functions in this file are only for exporting to C++, DO NOT USE IN RUST

use std::fmt::Write;
use std::io;
use std::os::fd::{BorrowedFd, OwnedFd, RawFd};

use cxx::private::c_char;
use libc::mode_t;

pub(crate) use crate::xwrap::*;
use crate::{
    fd_path, map_fd, map_file, mkdirs, realpath, rm_rf, slice_from_ptr_mut, Directory, ResultExt,
    Utf8CStr,
};

pub(crate) fn fd_path_for_cxx(fd: RawFd, buf: &mut [u8]) -> isize {
    fd_path(fd, buf)
        .log_cxx_with_msg(|w| w.write_str("fd_path failed"))
        .map_or(-1_isize, |v| v as isize)
}

#[no_mangle]
unsafe extern "C" fn canonical_path(path: *const c_char, buf: *mut u8, bufsz: usize) -> isize {
    match Utf8CStr::from_ptr(path) {
        Ok(p) => realpath(p, slice_from_ptr_mut(buf, bufsz)).map_or(-1, |v| v as isize),
        Err(_) => -1,
    }
}

#[export_name = "mkdirs"]
unsafe extern "C" fn mkdirs_for_cxx(path: *const c_char, mode: mode_t) -> i32 {
    match Utf8CStr::from_ptr(path) {
        Ok(p) => mkdirs(p, mode).map_or(-1, |_| 0),
        Err(_) => -1,
    }
}

#[export_name = "rm_rf"]
unsafe extern "C" fn rm_rf_for_cxx(path: *const c_char) -> bool {
    match Utf8CStr::from_ptr(path) {
        Ok(p) => rm_rf(p).map_or(false, |_| true),
        Err(_) => false,
    }
}

#[no_mangle]
unsafe extern "C" fn frm_rf(fd: OwnedFd) -> bool {
    fn inner(fd: OwnedFd) -> io::Result<()> {
        Directory::try_from(fd)?.remove_all()
    }
    inner(fd).map_or(false, |_| true)
}

pub(crate) fn map_file_for_cxx(path: &[u8], rw: bool) -> &'static mut [u8] {
    unsafe {
        map_file(Utf8CStr::from_bytes_unchecked(path), rw)
            .log_cxx()
            .unwrap_or(&mut [])
    }
}

pub(crate) fn map_fd_for_cxx(fd: RawFd, sz: usize, rw: bool) -> &'static mut [u8] {
    unsafe {
        map_fd(BorrowedFd::borrow_raw(fd), sz, rw)
            .log_cxx()
            .unwrap_or(&mut [])
    }
}
