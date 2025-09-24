// Functions in this file are only for exporting to C++, DO NOT USE IN RUST

use crate::cxx_extern::readlinkat;
use crate::{Directory, LibcReturn, ResultExt, Utf8CStr, cstr, slice_from_ptr, slice_from_ptr_mut};
use libc::{c_char, c_uint, c_ulong, c_void, dev_t, mode_t, off_t};
use std::ffi::CStr;
use std::fs::File;
use std::io::{Read, Write};
use std::mem::ManuallyDrop;
use std::os::fd::FromRawFd;
use std::os::unix::io::RawFd;
use std::ptr;
use std::ptr::NonNull;

fn ptr_to_str<'a>(ptr: *const c_char) -> Option<&'a str> {
    if ptr.is_null() {
        None
    } else {
        unsafe { CStr::from_ptr(ptr) }.to_str().ok()
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xrealpath(path: *const c_char, buf: *mut u8, bufsz: usize) -> isize {
    unsafe {
        match Utf8CStr::from_ptr(path) {
            Ok(path) => {
                let mut buf = cstr::buf::wrap_ptr(buf, bufsz);
                path.realpath(&mut buf)
                    .log()
                    .map_or(-1, |_| buf.len() as isize)
            }
            Err(_) => -1,
        }
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xreadlink(path: *const c_char, buf: *mut u8, bufsz: usize) -> isize {
    unsafe {
        match Utf8CStr::from_ptr(path) {
            Ok(path) => {
                let mut buf = cstr::buf::wrap_ptr(buf, bufsz);
                path.read_link(&mut buf)
                    .log()
                    .map_or(-1, |_| buf.len() as isize)
            }
            Err(_) => -1,
        }
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xreadlinkat(
    dirfd: RawFd,
    path: *const c_char,
    buf: *mut u8,
    bufsz: usize,
) -> isize {
    unsafe {
        readlinkat(dirfd, path, buf, bufsz)
            .into_os_result("readlinkat", ptr_to_str(path), None)
            .log()
            .unwrap_or(-1)
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xfopen(path: *const c_char, mode: *const c_char) -> *mut libc::FILE {
    unsafe {
        libc::fopen(path, mode)
            .into_os_result("fopen", ptr_to_str(path), None)
            .log()
            .map_or(ptr::null_mut(), NonNull::as_ptr)
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xfdopen(fd: RawFd, mode: *const c_char) -> *mut libc::FILE {
    unsafe {
        libc::fdopen(fd, mode)
            .into_os_result("fdopen", None, None)
            .log()
            .map_or(ptr::null_mut(), NonNull::as_ptr)
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xopen(path: *const c_char, flags: i32, mode: mode_t) -> RawFd {
    unsafe {
        libc::open(path, flags, mode as c_uint)
            .into_os_result("open", ptr_to_str(path), None)
            .log()
            .unwrap_or(-1)
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xopenat(dirfd: RawFd, path: *const c_char, flags: i32, mode: mode_t) -> RawFd {
    unsafe {
        libc::openat(dirfd, path, flags, mode as c_uint)
            .into_os_result("openat", ptr_to_str(path), None)
            .log()
            .unwrap_or(-1)
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xwrite(fd: RawFd, buf: *const u8, bufsz: usize) -> isize {
    let mut file = unsafe { ManuallyDrop::new(File::from_raw_fd(fd)) };
    let data = unsafe { slice_from_ptr(buf, bufsz) };
    file.write_all(data)
        .log()
        .map_or(-1, |_| data.len() as isize)
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xread(fd: RawFd, buf: *mut c_void, bufsz: usize) -> isize {
    unsafe {
        libc::read(fd, buf, bufsz)
            .into_os_result("read", None, None)
            .log()
            .unwrap_or(-1)
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xxread(fd: RawFd, buf: *mut u8, bufsz: usize) -> isize {
    let mut file = unsafe { ManuallyDrop::new(File::from_raw_fd(fd)) };
    let data = unsafe { slice_from_ptr_mut(buf, bufsz) };
    file.read_exact(data)
        .log()
        .map_or(-1, |_| data.len() as isize)
}

pub(crate) fn xpipe2(fds: &mut [i32; 2], flags: i32) -> i32 {
    unsafe {
        libc::pipe2(fds.as_mut_ptr(), flags)
            .into_os_result("pipe2", None, None)
            .log()
            .unwrap_or(-1)
    }
}

#[unsafe(no_mangle)]
extern "C" fn xsetns(fd: RawFd, nstype: i32) -> i32 {
    unsafe {
        libc::setns(fd, nstype)
            .into_os_result("setns", None, None)
            .log()
            .unwrap_or(-1)
    }
}

#[unsafe(no_mangle)]
extern "C" fn xunshare(flags: i32) -> i32 {
    unsafe {
        libc::unshare(flags)
            .into_os_result("unshare", None, None)
            .log()
            .unwrap_or(-1)
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xopendir(path: *const c_char) -> *mut libc::DIR {
    unsafe {
        libc::opendir(path)
            .into_os_result("opendir", ptr_to_str(path), None)
            .log()
            .map_or(ptr::null_mut(), NonNull::as_ptr)
    }
}

#[unsafe(no_mangle)]
extern "C" fn xfdopendir(fd: RawFd) -> *mut libc::DIR {
    unsafe {
        libc::fdopendir(fd)
            .into_os_result("fdopendir", None, None)
            .log()
            .map_or(ptr::null_mut(), NonNull::as_ptr)
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xreaddir(mut dir: ManuallyDrop<Directory>) -> *mut libc::dirent {
    dir.read()
        .log()
        .ok()
        .flatten()
        .map_or(ptr::null_mut(), |entry| entry.as_ptr())
}

#[unsafe(no_mangle)]
extern "C" fn xsetsid() -> i32 {
    unsafe {
        libc::setsid()
            .into_os_result("setsid", None, None)
            .log()
            .unwrap_or(-1)
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xstat(path: *const c_char, buf: *mut libc::stat) -> i32 {
    unsafe {
        libc::stat(path, buf)
            .into_os_result("stat", ptr_to_str(path), None)
            .log()
            .unwrap_or(-1)
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xfstat(fd: RawFd, buf: *mut libc::stat) -> i32 {
    unsafe {
        libc::fstat(fd, buf)
            .into_os_result("fstat", None, None)
            .log()
            .unwrap_or(-1)
    }
}

#[unsafe(no_mangle)]
extern "C" fn xdup2(oldfd: RawFd, newfd: RawFd) -> RawFd {
    unsafe {
        libc::dup2(oldfd, newfd)
            .into_os_result("dup2", None, None)
            .log()
            .unwrap_or(-1)
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xsymlink(target: *const c_char, linkpath: *const c_char) -> i32 {
    unsafe {
        libc::symlink(target, linkpath)
            .into_os_result("symlink", ptr_to_str(target), ptr_to_str(linkpath))
            .log()
            .unwrap_or(-1)
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xmount(
    src: *const c_char,
    target: *const c_char,
    fstype: *const c_char,
    flags: c_ulong,
    data: *const c_void,
) -> i32 {
    unsafe {
        libc::mount(src, target, fstype, flags, data)
            .into_os_result("mount", ptr_to_str(src), ptr_to_str(target))
            .log()
            .unwrap_or(-1)
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xumount2(target: *const c_char, flags: i32) -> i32 {
    unsafe {
        libc::umount2(target, flags)
            .into_os_result("umount2", ptr_to_str(target), None)
            .log()
            .unwrap_or(-1)
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xrename(oldname: *const c_char, newname: *const c_char) -> i32 {
    unsafe {
        libc::rename(oldname, newname)
            .into_os_result("rename", ptr_to_str(oldname), ptr_to_str(newname))
            .log()
            .unwrap_or(-1)
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xmkdir(path: *const c_char, mode: mode_t) -> i32 {
    unsafe {
        match Utf8CStr::from_ptr(path) {
            Ok(path) => path.mkdir(mode).log().map_or(-1, |_| 0),
            Err(_) => -1,
        }
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xmkdirs(path: *const c_char, mode: mode_t) -> i32 {
    unsafe {
        match Utf8CStr::from_ptr(path) {
            Ok(path) => path.mkdirs(mode).log().map_or(-1, |_| 0),
            Err(_) => -1,
        }
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xsendfile(
    out_fd: RawFd,
    in_fd: RawFd,
    offset: *mut off_t,
    count: usize,
) -> isize {
    unsafe {
        libc::sendfile(out_fd, in_fd, offset, count)
            .into_os_result("sendfile", None, None)
            .log()
            .unwrap_or(-1)
    }
}

#[unsafe(no_mangle)]
extern "C" fn xfork() -> i32 {
    unsafe {
        libc::fork()
            .into_os_result("fork", None, None)
            .log()
            .unwrap_or(-1)
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xmknod(pathname: *const c_char, mode: mode_t, dev: dev_t) -> i32 {
    unsafe {
        libc::mknod(pathname, mode, dev)
            .into_os_result("mknod", ptr_to_str(pathname), None)
            .log()
            .unwrap_or(-1)
    }
}
