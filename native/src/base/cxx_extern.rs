// Functions in this file are only for exporting to C++, DO NOT USE IN RUST

use std::os::fd::{BorrowedFd, OwnedFd, RawFd};

use cfg_if::cfg_if;
use libc::{c_char, mode_t};

use crate::files::map_file_at;
pub(crate) use crate::xwrap::*;
use crate::{
    CxxResultExt, Directory, OsResultStatic, Utf8CStr, clone_attr, cstr, fclone_attr, fd_path,
    map_fd, map_file, slice_from_ptr,
};

pub(crate) fn fd_path_for_cxx(fd: RawFd, buf: &mut [u8]) -> isize {
    let mut buf = cstr::buf::wrap(buf);
    fd_path(fd, &mut buf)
        .log_cxx()
        .map_or(-1_isize, |_| buf.len() as isize)
}

#[unsafe(no_mangle)]
unsafe extern "C" fn canonical_path(path: *const c_char, buf: *mut u8, bufsz: usize) -> isize {
    unsafe {
        match Utf8CStr::from_ptr(path) {
            Ok(path) => {
                let mut buf = cstr::buf::wrap_ptr(buf, bufsz);
                path.realpath(&mut buf)
                    .log_cxx()
                    .map_or(-1_isize, |_| buf.len() as isize)
            }
            Err(_) => -1,
        }
    }
}

#[unsafe(export_name = "mkdirs")]
unsafe extern "C" fn mkdirs_for_cxx(path: *const c_char, mode: mode_t) -> i32 {
    unsafe {
        match Utf8CStr::from_ptr(path) {
            Ok(path) => path.mkdirs(mode).map_or(-1, |_| 0),
            Err(_) => -1,
        }
    }
}

#[unsafe(export_name = "rm_rf")]
unsafe extern "C" fn rm_rf_for_cxx(path: *const c_char) -> bool {
    unsafe {
        match Utf8CStr::from_ptr(path) {
            Ok(path) => path.remove_all().is_ok(),
            Err(_) => false,
        }
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn frm_rf(fd: OwnedFd) -> bool {
    fn inner(fd: OwnedFd) -> OsResultStatic<()> {
        Directory::try_from(fd)?.remove_all()
    }
    inner(fd).is_ok()
}

pub(crate) fn map_file_for_cxx(path: &Utf8CStr, rw: bool) -> &'static mut [u8] {
    map_file(path, rw).log_cxx().unwrap_or(&mut [])
}

pub(crate) fn map_file_at_for_cxx(fd: RawFd, path: &Utf8CStr, rw: bool) -> &'static mut [u8] {
    unsafe {
        map_file_at(BorrowedFd::borrow_raw(fd), path, rw)
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

pub(crate) unsafe fn readlinkat(
    dirfd: RawFd,
    path: *const c_char,
    buf: *mut u8,
    bufsz: usize,
) -> isize {
    unsafe {
        // readlinkat() may fail on x86 platform, returning random value
        // instead of number of bytes placed in buf (length of link)
        cfg_if! {
            if #[cfg(any(target_arch = "x86", target_arch = "x86_64"))] {
                libc::memset(buf.cast(), 0, bufsz);
                let mut r = libc::readlinkat(dirfd, path, buf.cast(), bufsz - 1);
                if r > 0 {
                    r = libc::strlen(buf.cast()) as isize;
                }
            } else {
                let r = libc::readlinkat(dirfd, path, buf.cast(), bufsz - 1);
                if r >= 0 {
                    *buf.offset(r) = b'\0';
                }
            }
        }
        r
    }
}

#[unsafe(export_name = "cp_afc")]
unsafe extern "C" fn cp_afc_for_cxx(src: *const c_char, dest: *const c_char) -> bool {
    unsafe {
        if let Ok(src) = Utf8CStr::from_ptr(src) {
            if let Ok(dest) = Utf8CStr::from_ptr(dest) {
                return src.copy_to(dest).log_cxx().is_ok();
            }
        }
        false
    }
}

#[unsafe(export_name = "mv_path")]
unsafe extern "C" fn mv_path_for_cxx(src: *const c_char, dest: *const c_char) -> bool {
    unsafe {
        if let Ok(src) = Utf8CStr::from_ptr(src) {
            if let Ok(dest) = Utf8CStr::from_ptr(dest) {
                return src.move_to(dest).log_cxx().is_ok();
            }
        }
        false
    }
}

#[unsafe(export_name = "link_path")]
unsafe extern "C" fn link_path_for_cxx(src: *const c_char, dest: *const c_char) -> bool {
    unsafe {
        if let Ok(src) = Utf8CStr::from_ptr(src) {
            if let Ok(dest) = Utf8CStr::from_ptr(dest) {
                return src.link_to(dest).log_cxx().is_ok();
            }
        }
        false
    }
}

#[unsafe(export_name = "clone_attr")]
unsafe extern "C" fn clone_attr_for_cxx(src: *const c_char, dest: *const c_char) -> bool {
    unsafe {
        if let Ok(src) = Utf8CStr::from_ptr(src) {
            if let Ok(dest) = Utf8CStr::from_ptr(dest) {
                return clone_attr(src, dest).log_cxx().is_ok();
            }
        }
        false
    }
}

#[unsafe(export_name = "fclone_attr")]
unsafe extern "C" fn fclone_attr_for_cxx(a: RawFd, b: RawFd) -> bool {
    fclone_attr(a, b).log_cxx().is_ok()
}

#[unsafe(export_name = "cxx$utf8str$new")]
unsafe extern "C" fn str_new(this: &mut &Utf8CStr, s: *const u8, len: usize) {
    unsafe {
        *this = Utf8CStr::from_bytes(slice_from_ptr(s, len)).unwrap_or(cstr!(""));
    }
}

#[unsafe(export_name = "cxx$utf8str$ptr")]
unsafe extern "C" fn str_ptr(this: &&Utf8CStr) -> *const u8 {
    this.as_ptr().cast()
}

#[unsafe(export_name = "cxx$utf8str$len")]
unsafe extern "C" fn str_len(this: &&Utf8CStr) -> usize {
    this.len()
}
