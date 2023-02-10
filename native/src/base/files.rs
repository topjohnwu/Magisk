use std::ffi::CStr;
use std::fs::File;
use std::io;
use std::io::BufRead;
use std::os::unix::io::{AsRawFd, FromRawFd, OwnedFd, RawFd};
use std::path::Path;

use libc::{c_char, c_uint, mode_t, EEXIST, ENOENT, O_CLOEXEC, O_PATH};

use crate::{bfmt_cstr, errno, xopen};

pub mod unsafe_impl {
    use std::ffi::CStr;

    use libc::c_char;

    use crate::slice_from_ptr_mut;

    pub unsafe fn readlink(path: *const c_char, buf: *mut u8, bufsz: usize) -> isize {
        let r = libc::readlink(path, buf.cast(), bufsz - 1);
        if r >= 0 {
            *buf.offset(r) = b'\0';
        }
        return r;
    }

    #[no_mangle]
    unsafe extern "C" fn canonical_path(path: *const c_char, buf: *mut u8, bufsz: usize) -> isize {
        super::realpath(CStr::from_ptr(path), slice_from_ptr_mut(buf, bufsz))
    }
}

pub fn __open_fd_impl(path: &CStr, flags: i32, mode: mode_t) -> Option<OwnedFd> {
    unsafe {
        let fd = libc::open(path.as_ptr(), flags, mode as c_uint);
        if fd >= 0 {
            Some(OwnedFd::from_raw_fd(fd))
        } else {
            None
        }
    }
}

pub fn __xopen_fd_impl(path: &CStr, flags: i32, mode: mode_t) -> Option<OwnedFd> {
    let fd = xopen(path.as_ptr(), flags, mode);
    if fd >= 0 {
        unsafe { Some(OwnedFd::from_raw_fd(fd)) }
    } else {
        None
    }
}

#[macro_export]
macro_rules! open_fd {
    ($path:expr, $flags:expr) => {
        crate::__open_fd_impl($path, $flags, 0)
    };
    ($path:expr, $flags:expr, $mode:expr) => {
        crate::__open_fd_impl($path, $flags, $mode)
    };
}

#[macro_export]
macro_rules! xopen_fd {
    ($path:expr, $flags:expr) => {
        crate::__xopen_fd_impl($path, $flags, 0)
    };
    ($path:expr, $flags:expr, $mode:expr) => {
        crate::__xopen_fd_impl($path, $flags, $mode)
    };
}

pub fn readlink(path: &CStr, data: &mut [u8]) -> isize {
    unsafe { unsafe_impl::readlink(path.as_ptr(), data.as_mut_ptr(), data.len()) }
}

pub fn fd_path(fd: RawFd, buf: &mut [u8]) -> isize {
    let mut fd_buf: [u8; 40] = [0; 40];
    let fd_path = bfmt_cstr!(&mut fd_buf, "/proc/self/fd/{}", fd);
    readlink(fd_path, buf)
}

// Inspired by https://android.googlesource.com/platform/bionic/+/master/libc/bionic/realpath.cpp
pub fn realpath(path: &CStr, buf: &mut [u8]) -> isize {
    if let Some(fd) = open_fd!(path, O_PATH | O_CLOEXEC) {
        let mut st1: libc::stat;
        let mut st2: libc::stat;
        let mut skip_check = false;
        unsafe {
            st1 = std::mem::zeroed();
            if libc::fstat(fd.as_raw_fd(), &mut st1) < 0 {
                // This shall only fail on Linux < 3.6
                skip_check = true;
            }
        }
        let len = fd_path(fd.as_raw_fd(), buf);
        unsafe {
            st2 = std::mem::zeroed();
            if libc::stat(buf.as_ptr().cast(), &mut st2) < 0
                || (!skip_check && (st2.st_dev != st1.st_dev || st2.st_ino != st1.st_ino))
            {
                *errno() = ENOENT;
                return -1;
            }
        }
        return len;
    } else {
        *errno() = ENOENT;
        -1
    }
}

extern "C" {
    fn strscpy(dst: *mut c_char, src: *const c_char, size: usize) -> usize;
}

#[no_mangle]
pub extern "C" fn mkdirs(path: *const c_char, mode: mode_t) -> i32 {
    unsafe {
        let mut buf = [0 as u8; 4096];
        let ptr: *mut c_char = buf.as_mut_ptr().cast();
        let len = strscpy(ptr, path, buf.len());
        let mut curr = &mut buf[1..len];
        while let Some(p) = curr.iter().position(|c| *c == b'/') {
            curr[p] = b'\0';
            if libc::mkdir(ptr, mode) < 0 && *errno() != EEXIST {
                return -1;
            }
            curr[p] = b'/';
            curr = &mut curr[(p + 1)..];
        }
        if libc::mkdir(ptr, mode) < 0 && *errno() != EEXIST {
            return -1;
        }
        0
    }
}

pub fn read_lines<P: AsRef<Path>>(path: P) -> io::Result<io::Lines<io::BufReader<File>>> {
    let file = File::open(path)?;
    Ok(io::BufReader::new(file).lines())
}
