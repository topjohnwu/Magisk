// Functions in this file are only for exporting to C++, DO NOT USE IN RUST

use std::io;
use std::os::fd::{BorrowedFd, OwnedFd, RawFd};

use crate::{
    cxx_extern::io::BufReader,
    ffi::{MountInfoCxx, Utf8CStrRef},
    files::BufReadExt,
    FsPathBuf, Utf8CStrBufArr,
};
use cfg_if::cfg_if;
use cxx::private::c_char;
use libc::{makedev, mode_t, O_CLOEXEC, O_RDONLY};
use std::{fs::File, os::fd::FromRawFd};

use crate::logging::CxxResultExt;
pub(crate) use crate::xwrap::*;
use crate::{
    clone_attr, cstr, fclone_attr, fd_path, map_fd, map_file, slice_from_ptr, Directory, FsPath,
    Utf8CStr, Utf8CStrBufRef,
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

pub(crate) fn map_file_for_cxx(path: &Utf8CStr, rw: bool) -> &'static mut [u8] {
    map_file(path, rw).log_cxx().unwrap_or(&mut [])
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

fn parse_mount_info_line(line: &str) -> Option<MountInfoCxx> {
    let mut iter = line.split_whitespace();
    let id = iter.next()?.parse().ok()?;
    let parent = iter.next()?.parse().ok()?;
    let (maj, min) = iter.next()?.split_once(":")?;
    let maj = maj.parse().ok()?;
    let min = min.parse().ok()?;
    let device = makedev(maj, min).into();
    let root = iter.next()?.to_string();
    let target = iter.next()?.to_string();
    let vfs_option = iter.next()?.to_string();
    let mut optional = iter.next()?;
    let mut shared = 0;
    let mut master = 0;
    let mut propagation_from = 0;
    let mut unbindable = false;
    while optional != "-" {
        if let Some(peer) = optional.strip_prefix("master:") {
            master = peer.parse().ok()?;
        } else if let Some(peer) = optional.strip_prefix("shared:") {
            shared = peer.parse().ok()?;
        } else if let Some(peer) = optional.strip_prefix("propagate_from:") {
            propagation_from = peer.parse().ok()?;
        } else if optional == "unbindable" {
            unbindable = true;
        }
        optional = iter.next()?;
    }
    let fs_type = iter.next()?.to_string();
    let source = iter.next()?.to_string();
    let fs_option = iter.next()?.to_string();
    Some(MountInfoCxx {
        id,
        parent,
        device,
        root,
        target,
        vfs_option,
        shared,
        master,
        propagation_from,
        unbindable,
        fs_type,
        source,
        fs_option,
    })
}

pub(crate) fn parse_mount_info_for_cxx(path: Utf8CStrRef) -> Vec<MountInfoCxx> {
    let mut res = vec![];
    let mut buf = Utf8CStrBufArr::default();
    let path = FsPathBuf::new(&mut buf)
        .join("/proc/")
        .join(path)
        .join("/mountinfo");

    let fd = unsafe { libc::open(path.as_ptr(), O_RDONLY | O_CLOEXEC) };
    if fd > 0 {
        let file = File::from(unsafe { OwnedFd::from_raw_fd(fd) });
        BufReader::new(file).foreach_lines(|line| {
            parse_mount_info_line(line)
                .map(|info| res.push(info))
                .is_some()
        });
    }
    res
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

#[export_name = "cxx$utf8str$new"]
unsafe extern "C" fn str_new(this: &mut &Utf8CStr, s: *const u8, len: usize) {
    *this = Utf8CStr::from_bytes(slice_from_ptr(s, len)).unwrap_or(cstr!(""));
}

#[export_name = "cxx$utf8str$ptr"]
unsafe extern "C" fn str_ptr(this: &&Utf8CStr) -> *const u8 {
    this.as_ptr().cast()
}

#[export_name = "cxx$utf8str$len"]
unsafe extern "C" fn str_len(this: &&Utf8CStr) -> usize {
    this.len()
}
