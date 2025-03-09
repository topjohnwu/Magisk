// Functions in this file are only for exporting to C++, DO NOT USE IN RUST

use std::ffi::CStr;
use std::os::unix::io::RawFd;

use libc::{
    c_char, c_uint, c_ulong, c_void, dev_t, mode_t, nfds_t, off_t, pollfd, sockaddr, socklen_t,
    ssize_t,
};

use crate::cxx_extern::readlinkat_for_cxx;
use crate::{CxxResultExt, FsPath, Utf8CStr, Utf8CStrBufRef, errno, raw_cstr};

fn ptr_to_str<'a, T>(ptr: *const T) -> &'a str {
    if ptr.is_null() {
        "(null)"
    } else {
        unsafe { CStr::from_ptr(ptr.cast()) }.to_str().unwrap_or("")
    }
}

fn error_str() -> &'static str {
    unsafe { ptr_to_str(libc::strerror(*errno())) }
}

macro_rules! error_cxx {
    ($($args:tt)+) => {
        ($crate::log_with_args($crate::LogLevel::ErrorCxx, format_args_nl!($($args)+)))
    }
}

macro_rules! perror {
    ($fmt:expr) => {
        $crate::log_with_formatter($crate::LogLevel::ErrorCxx, |w| {
            w.write_str($fmt)?;
            w.write_fmt(format_args_nl!(" failed with {}: {}", $crate::errno(), error_str()))
        })
    };
    ($fmt:expr, $($args:tt)*) => {
        $crate::log_with_formatter($crate::LogLevel::ErrorCxx, |w| {
            w.write_fmt(format_args!($fmt, $($args)*))?;
            w.write_fmt(format_args_nl!(" failed with {}: {}", $crate::errno(), error_str()))
        })
    };
}

mod c_export {
    use std::os::unix::io::RawFd;

    use crate::{slice_from_ptr, slice_from_ptr_mut};

    #[unsafe(no_mangle)]
    unsafe extern "C" fn xwrite(fd: RawFd, buf: *const u8, bufsz: usize) -> isize {
        unsafe { super::xwrite(fd, slice_from_ptr(buf, bufsz)) }
    }

    #[unsafe(no_mangle)]
    unsafe extern "C" fn xxread(fd: RawFd, buf: *mut u8, bufsz: usize) -> isize {
        unsafe { super::xxread(fd, slice_from_ptr_mut(buf, bufsz)) }
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xrealpath(path: *const c_char, buf: *mut u8, bufsz: usize) -> isize {
    unsafe {
        match Utf8CStr::from_ptr(path) {
            Ok(p) => {
                let mut buf = Utf8CStrBufRef::from_ptr(buf, bufsz);
                FsPath::from(p)
                    .realpath(&mut buf)
                    .log_cxx_with_msg(|w| w.write_fmt(format_args!("realpath {} failed", p)))
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
            Ok(p) => {
                let mut buf = Utf8CStrBufRef::from_ptr(buf, bufsz);
                FsPath::from(p)
                    .read_link(&mut buf)
                    .log_cxx_with_msg(|w| w.write_fmt(format_args!("readlink {} failed", p)))
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
        let r = readlinkat_for_cxx(dirfd, path, buf, bufsz);
        if r < 0 {
            perror!("readlinkat {}", ptr_to_str(path))
        }
        r
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xfopen(path: *const c_char, mode: *const c_char) -> *mut libc::FILE {
    unsafe {
        let fp = libc::fopen(path, mode);
        if fp.is_null() {
            perror!("fopen {}", ptr_to_str(path));
        }
        fp
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xfdopen(fd: RawFd, mode: *const c_char) -> *mut libc::FILE {
    unsafe {
        let fp = libc::fdopen(fd, mode);
        if fp.is_null() {
            perror!("fdopen");
        }
        fp
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xopen(path: *const c_char, flags: i32, mode: mode_t) -> RawFd {
    unsafe {
        let r = libc::open(path, flags, mode as c_uint);
        if r < 0 {
            perror!("open {}", ptr_to_str(path));
        }
        r
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xopenat(dirfd: RawFd, path: *const c_char, flags: i32, mode: mode_t) -> RawFd {
    unsafe {
        let r = libc::openat(dirfd, path, flags, mode as c_uint);
        if r < 0 {
            perror!("openat {}", ptr_to_str(path));
        }
        r
    }
}

// Fully write data slice
fn xwrite(fd: RawFd, data: &[u8]) -> isize {
    unsafe {
        let mut write_sz: usize = 0;
        let mut r: ssize_t;
        let mut remain: &[u8] = data;
        loop {
            r = libc::write(fd, remain.as_ptr().cast(), remain.len());
            if r < 0 {
                if *errno() == libc::EINTR {
                    continue;
                }
                perror!("write");
                return r;
            }
            let r = r as usize;
            write_sz += r;
            remain = &remain[r..];
            if r == 0 || remain.is_empty() {
                break;
            }
        }
        if !remain.is_empty() {
            error_cxx!("write ({} != {})", write_sz, data.len())
        }
        write_sz as isize
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xread(fd: RawFd, buf: *mut c_void, bufsz: usize) -> isize {
    unsafe {
        let r = libc::read(fd, buf, bufsz);
        if r < 0 {
            perror!("read");
        }
        r
    }
}

// Fully read size of data slice
fn xxread(fd: RawFd, data: &mut [u8]) -> isize {
    unsafe {
        let mut read_sz: usize = 0;
        let mut r: ssize_t;
        let mut remain: &mut [u8] = data;
        loop {
            r = libc::read(fd, remain.as_mut_ptr().cast(), remain.len());
            if r < 0 {
                if *errno() == libc::EINTR {
                    continue;
                }
                perror!("read");
                return r;
            }
            let r = r as usize;
            read_sz += r;
            remain = &mut remain[r..];
            if r == 0 || remain.is_empty() {
                break;
            }
        }
        if !remain.is_empty() {
            error_cxx!("read ({} != {})", read_sz, data.len())
        }
        read_sz as isize
    }
}

pub(crate) fn xpipe2(fds: &mut [i32; 2], flags: i32) -> i32 {
    unsafe {
        let r = libc::pipe2(fds.as_mut_ptr(), flags);
        if r < 0 {
            perror!("pipe2");
        }
        r
    }
}

#[unsafe(no_mangle)]
extern "C" fn xsetns(fd: RawFd, nstype: i32) -> i32 {
    unsafe {
        let r = libc::setns(fd, nstype);
        if r < 0 {
            perror!("setns");
        }
        r
    }
}

#[unsafe(no_mangle)]
extern "C" fn xunshare(flags: i32) -> i32 {
    unsafe {
        let r = libc::unshare(flags);
        if r < 0 {
            perror!("unshare");
        }
        r
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xopendir(path: *const c_char) -> *mut libc::DIR {
    unsafe {
        let dp = libc::opendir(path);
        if dp.is_null() {
            perror!("opendir {}", ptr_to_str(path));
        }
        dp
    }
}

#[unsafe(no_mangle)]
extern "C" fn xfdopendir(fd: RawFd) -> *mut libc::DIR {
    unsafe {
        let dp = libc::fdopendir(fd);
        if dp.is_null() {
            perror!("fdopendir");
        }
        dp
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xreaddir(dirp: *mut libc::DIR) -> *mut libc::dirent {
    unsafe {
        *errno() = 0;
        loop {
            let e = libc::readdir(dirp);
            if e.is_null() {
                if *errno() != 0 {
                    perror!("readdir")
                }
            } else {
                // Filter out . and ..
                let s = (*e).d_name.as_ptr();
                if libc::strcmp(s, raw_cstr!(".")) == 0 || libc::strcmp(s, raw_cstr!("..")) == 0 {
                    continue;
                }
            };
            return e;
        }
    }
}

#[unsafe(no_mangle)]
extern "C" fn xsetsid() -> i32 {
    unsafe {
        let r = libc::setsid();
        if r < 0 {
            perror!("setsid");
        }
        r
    }
}

#[unsafe(no_mangle)]
extern "C" fn xsocket(domain: i32, ty: i32, protocol: i32) -> RawFd {
    unsafe {
        let fd = libc::socket(domain, ty, protocol);
        if fd < 0 {
            perror!("socket");
        }
        fd
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xbind(socket: i32, address: *const sockaddr, len: socklen_t) -> i32 {
    unsafe {
        let r = libc::bind(socket, address, len);
        if r < 0 {
            perror!("bind");
        }
        r
    }
}

#[unsafe(no_mangle)]
extern "C" fn xlisten(socket: i32, backlog: i32) -> i32 {
    unsafe {
        let r = libc::listen(socket, backlog);
        if r < 0 {
            perror!("listen");
        }
        r
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xaccept4(
    sockfd: RawFd,
    addr: *mut sockaddr,
    len: *mut socklen_t,
    flg: i32,
) -> RawFd {
    unsafe {
        let fd = libc::accept4(sockfd, addr, len, flg);
        if fd < 0 {
            perror!("accept4");
        }
        fd
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xaccess(path: *const c_char, mode: i32) -> i32 {
    unsafe {
        let r = libc::access(path, mode);
        if r < 0 {
            perror!("access {}", ptr_to_str(path));
        }
        r
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xstat(path: *const c_char, buf: *mut libc::stat) -> i32 {
    unsafe {
        let r = libc::stat(path, buf);
        if r < 0 {
            perror!("stat {}", ptr_to_str(path));
        }
        r
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xfstat(fd: RawFd, buf: *mut libc::stat) -> i32 {
    unsafe {
        let r = libc::fstat(fd, buf);
        if r < 0 {
            perror!("fstat");
        }
        r
    }
}

#[unsafe(no_mangle)]
extern "C" fn xdup(oldfd: RawFd) -> RawFd {
    unsafe {
        let fd = libc::dup(oldfd);
        if fd < 0 {
            perror!("dup");
        }
        fd
    }
}

#[unsafe(no_mangle)]
extern "C" fn xdup2(oldfd: RawFd, newfd: RawFd) -> RawFd {
    unsafe {
        let fd = libc::dup2(oldfd, newfd);
        if fd < 0 {
            perror!("dup2");
        }
        fd
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xsymlink(target: *const c_char, linkpath: *const c_char) -> i32 {
    unsafe {
        let r = libc::symlink(target, linkpath);
        if r < 0 {
            perror!("symlink {} -> {}", ptr_to_str(target), ptr_to_str(linkpath));
        }
        r
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
        let r = libc::mount(src, target, fstype, flags, data);
        if r < 0 {
            perror!("mount {} -> {}", ptr_to_str(src), ptr_to_str(target));
        }
        r
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xumount2(target: *const c_char, flags: i32) -> i32 {
    unsafe {
        let r = libc::umount2(target, flags);
        if r < 0 {
            perror!("umount2 {}", ptr_to_str(target));
        }
        r
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xrename(oldname: *const c_char, newname: *const c_char) -> i32 {
    unsafe {
        let r = libc::rename(oldname, newname);
        if r < 0 {
            perror!("rename {} -> {}", ptr_to_str(oldname), ptr_to_str(newname));
        }
        r
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xmkdir(path: *const c_char, mode: mode_t) -> i32 {
    unsafe {
        let r = libc::mkdir(path, mode);
        if r < 0 && *errno() != libc::EEXIST {
            perror!("mkdir {}", ptr_to_str(path));
        }
        r
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xmkdirs(path: *const c_char, mode: mode_t) -> i32 {
    unsafe {
        match Utf8CStr::from_ptr(path) {
            Ok(p) => FsPath::from(p)
                .mkdirs(mode)
                .log_cxx_with_msg(|w| w.write_fmt(format_args!("mkdirs {} failed", p)))
                .map_or(-1, |_| 0),
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
        let r = libc::sendfile(out_fd, in_fd, offset, count);
        if r < 0 {
            perror!("sendfile");
        }
        r
    }
}

#[unsafe(no_mangle)]
extern "C" fn xfork() -> i32 {
    unsafe {
        let r = libc::fork();
        if r < 0 {
            perror!("fork");
        }
        r
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xpoll(fds: *mut pollfd, nfds: nfds_t, timeout: i32) -> i32 {
    unsafe {
        let r = libc::poll(fds, nfds, timeout);
        if r < 0 {
            perror!("poll");
        }
        r
    }
}

#[unsafe(no_mangle)]
unsafe extern "C" fn xmknod(pathname: *const c_char, mode: mode_t, dev: dev_t) -> i32 {
    unsafe {
        let r = libc::mknod(pathname, mode, dev);
        if r < 0 {
            perror!("mknod {}", ptr_to_str(pathname));
        }
        r
    }
}
