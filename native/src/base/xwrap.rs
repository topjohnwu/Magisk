use libc::{c_char, c_uint, mode_t};

use crate::{perror, ptr_to_str};

#[no_mangle]
pub extern "C" fn xfopen(path: *const c_char, mode: *const c_char) -> *mut libc::FILE {
    unsafe {
        let fp = libc::fopen(path, mode);
        if fp.is_null() {
            perror!("fopen: {}", ptr_to_str(path));
        }
        return fp;
    }
}

#[no_mangle]
pub extern "C" fn xfdopen(fd: i32, mode: *const c_char) -> *mut libc::FILE {
    unsafe {
        let fp = libc::fdopen(fd, mode);
        if fp.is_null() {
            perror!("fdopen");
        }
        return fp;
    }
}

#[no_mangle]
pub extern "C" fn xopen(path: *const c_char, flags: i32, mode: mode_t) -> i32 {
    unsafe {
        let r = libc::open(path, flags, mode as c_uint);
        if r < 0 {
            perror!("open: {}", ptr_to_str(path));
        }
        return r;
    }
}

#[no_mangle]
pub extern "C" fn xopenat(dirfd: i32, path: *const c_char, flags: i32, mode: mode_t) -> i32 {
    unsafe {
        let r = libc::openat(dirfd, path, flags, mode as c_uint);
        if r < 0 {
            perror!("openat: {}", ptr_to_str(path));
        }
        return r;
    }
}

#[macro_export]
macro_rules! xopen {
    ($path:expr, $flags:expr) => {
        xopen($path, $flags, 0)
    };
    ($path:expr, $flags:expr, $mode:expr) => {
        xopen($path, $flags, $mode)
    };
}
