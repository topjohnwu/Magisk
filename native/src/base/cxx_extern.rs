// Functions in this file are only for exporting to C++, DO NOT USE IN RUST

use std::io;
use std::os::fd::{BorrowedFd, OwnedFd, RawFd};

use cfg_if::cfg_if;
use cxx::private::c_char;
use libc::mode_t;

use crate::logging::CxxResultExt;
pub(crate) use crate::xwrap::*;
use crate::{
    clone_attr, fclone_attr, fd_path, map_fd, map_file, Directory, FsPath, Utf8CStr, Utf8CStrBufRef,
};

pub(crate) fn fd_path_for_cxx(fd: RawFd, buf: &mut [u8]) -> isize {
    let mut buf = Utf8CStrBufRef::from(buf);
    fd_path(fd, &mut buf)
        .log_cxx_with_msg(|w| w.write_str("fd_path failed"))
        .map_or(-1_isize, |_| buf.len() as isize)
}

#[no_mangle]
unsafe extern "C" fn canonical_path(path: *const c_char, buf: *mut u8, bufsz: usize) -> isize {
    match Utf8CStr::from_ptr(path) {
        Ok(p) => {
            let mut buf = Utf8CStrBufRef::from_ptr(buf, bufsz);
            FsPath::from(p)
                .realpath(&mut buf)
                .map_or(-1, |_| buf.len() as isize)
        }
        Err(_) => -1,
    }
}

#[export_name = "mkdirs"]
unsafe extern "C" fn mkdirs_for_cxx(path: *const c_char, mode: mode_t) -> i32 {
    match Utf8CStr::from_ptr(path) {
        Ok(p) => FsPath::from(p).mkdirs(mode).map_or(-1, |_| 0),
        Err(_) => -1,
    }
}

#[export_name = "rm_rf"]
unsafe extern "C" fn rm_rf_for_cxx(path: *const c_char) -> bool {
    match Utf8CStr::from_ptr(path) {
        Ok(p) => FsPath::from(p).remove_all().is_ok(),
        Err(_) => false,
    }
}

#[no_mangle]
unsafe extern "C" fn frm_rf(fd: OwnedFd) -> bool {
    fn inner(fd: OwnedFd) -> io::Result<()> {
        Directory::try_from(fd)?.remove_all()
    }
    inner(fd).is_ok()
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

pub(crate) unsafe fn readlinkat_for_cxx(
    dirfd: RawFd,
    path: *const c_char,
    buf: *mut u8,
    bufsz: usize,
) -> isize {
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

#[export_name = "cp_afc"]
unsafe extern "C" fn cp_afc_for_cxx(src: *const c_char, dest: *const c_char) -> bool {
    if let Ok(src) = Utf8CStr::from_ptr(src) {
        if let Ok(dest) = Utf8CStr::from_ptr(dest) {
            let src = FsPath::from(src);
            let dest = FsPath::from(dest);
            return src
                .copy_to(dest)
                .log_cxx_with_msg(|w| {
                    w.write_fmt(format_args!("cp_afc {} -> {} failed", src, dest))
                })
                .is_ok();
        }
    }
    false
}

#[export_name = "mv_path"]
unsafe extern "C" fn mv_path_for_cxx(src: *const c_char, dest: *const c_char) -> bool {
    if let Ok(src) = Utf8CStr::from_ptr(src) {
        if let Ok(dest) = Utf8CStr::from_ptr(dest) {
            let src = FsPath::from(src);
            let dest = FsPath::from(dest);
            return src
                .move_to(dest)
                .log_cxx_with_msg(|w| {
                    w.write_fmt(format_args!("mv_path {} -> {} failed", src, dest))
                })
                .is_ok();
        }
    }
    false
}

#[export_name = "link_path"]
unsafe extern "C" fn link_path_for_cxx(src: *const c_char, dest: *const c_char) -> bool {
    if let Ok(src) = Utf8CStr::from_ptr(src) {
        if let Ok(dest) = Utf8CStr::from_ptr(dest) {
            let src = FsPath::from(src);
            let dest = FsPath::from(dest);
            return src
                .link_to(dest)
                .log_cxx_with_msg(|w| {
                    w.write_fmt(format_args!("link_path {} -> {} failed", src, dest))
                })
                .is_ok();
        }
    }
    false
}

#[export_name = "clone_attr"]
unsafe extern "C" fn clone_attr_for_cxx(src: *const c_char, dest: *const c_char) -> bool {
    if let Ok(src) = Utf8CStr::from_ptr(src) {
        if let Ok(dest) = Utf8CStr::from_ptr(dest) {
            let src = FsPath::from(src);
            let dest = FsPath::from(dest);
            return clone_attr(src, dest)
                .log_cxx_with_msg(|w| {
                    w.write_fmt(format_args!("clone_attr {} -> {} failed", src, dest))
                })
                .is_ok();
        }
    }
    false
}

#[export_name = "fclone_attr"]
unsafe extern "C" fn fclone_attr_for_cxx(a: RawFd, b: RawFd) -> bool {
    fclone_attr(a, b)
        .log_cxx_with_msg(|w| w.write_str("fclone_attr failed"))
        .is_ok()
}
