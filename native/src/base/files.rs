use crate::{
    Directory, FsPathFollow, LibcReturn, LoggedResult, OsError, OsResult, Utf8CStr, Utf8CStrBuf,
    cstr, errno, error,
};
use bytemuck::{Pod, bytes_of, bytes_of_mut};
use libc::{c_uint, makedev, mode_t};
use nix::errno::Errno;
use nix::fcntl::{AT_FDCWD, OFlag};
use nix::sys::stat::{FchmodatFlags, Mode};
use nix::unistd::{AccessFlags, Gid, Uid};
use num_traits::AsPrimitive;
use std::cmp::min;
use std::ffi::CStr;
use std::fmt::Display;
use std::fs::File;
use std::io::{BufRead, BufReader, Read, Seek, SeekFrom, Write};
use std::mem::MaybeUninit;
use std::os::fd::{AsFd, BorrowedFd};
use std::os::unix::ffi::OsStrExt;
use std::os::unix::io::{AsRawFd, OwnedFd, RawFd};
use std::path::Path;
use std::{io, mem, ptr, slice};

pub trait ReadExt {
    fn skip(&mut self, len: usize) -> io::Result<()>;
    fn read_pod<F: Pod>(&mut self, data: &mut F) -> io::Result<()>;
}

impl<T: Read> ReadExt for T {
    fn skip(&mut self, mut len: usize) -> io::Result<()> {
        let mut buf = MaybeUninit::<[u8; 4096]>::uninit();
        let buf = unsafe { buf.assume_init_mut() };
        while len > 0 {
            let l = min(buf.len(), len);
            self.read_exact(&mut buf[..l])?;
            len -= l;
        }
        Ok(())
    }

    fn read_pod<F: Pod>(&mut self, data: &mut F) -> io::Result<()> {
        self.read_exact(bytes_of_mut(data))
    }
}

pub trait ReadSeekExt {
    fn skip(&mut self, len: usize) -> io::Result<()>;
}

impl<T: Read + Seek> ReadSeekExt for T {
    fn skip(&mut self, len: usize) -> io::Result<()> {
        if self.seek(SeekFrom::Current(len as i64)).is_err() {
            // If the file is not actually seekable, fallback to read
            ReadExt::skip(self, len)?;
        }
        Ok(())
    }
}

pub trait BufReadExt {
    fn for_each_line<F: FnMut(&mut String) -> bool>(&mut self, f: F);
    fn for_each_prop<F: FnMut(&str, &str) -> bool>(&mut self, f: F);
}

impl<T: BufRead> BufReadExt for T {
    fn for_each_line<F: FnMut(&mut String) -> bool>(&mut self, mut f: F) {
        let mut buf = String::new();
        loop {
            match self.read_line(&mut buf) {
                Ok(0) => break,
                Ok(_) => {
                    if !f(&mut buf) {
                        break;
                    }
                }
                Err(e) => {
                    error!("{}", e);
                    break;
                }
            };
            buf.clear();
        }
    }

    fn for_each_prop<F: FnMut(&str, &str) -> bool>(&mut self, mut f: F) {
        self.for_each_line(|line| {
            // Reserve an additional byte, because this string will be manually
            // null terminated on the C++ side, and it may need more space.
            line.reserve(1);
            let line = line.trim();
            if line.starts_with('#') {
                return true;
            }
            if let Some((key, value)) = line.split_once('=') {
                return f(key.trim(), value.trim());
            }
            true
        });
    }
}

pub trait WriteExt {
    fn write_zeros(&mut self, len: usize) -> io::Result<()>;
    fn write_pod<F: Pod>(&mut self, data: &F) -> io::Result<()>;
}

impl<T: Write> WriteExt for T {
    fn write_zeros(&mut self, mut len: usize) -> io::Result<()> {
        let buf = [0_u8; 4096];
        while len > 0 {
            let l = min(buf.len(), len);
            self.write_all(&buf[..l])?;
            len -= l;
        }
        Ok(())
    }

    fn write_pod<F: Pod>(&mut self, data: &F) -> io::Result<()> {
        self.write_all(bytes_of(data))
    }
}

pub enum FileOrStd {
    StdIn,
    StdOut,
    StdErr,
    File(File),
}

impl FileOrStd {
    pub fn as_file(&self) -> &File {
        let raw_fd_ref: &'static RawFd = match self {
            FileOrStd::StdIn => &0,
            FileOrStd::StdOut => &1,
            FileOrStd::StdErr => &2,
            FileOrStd::File(file) => return file,
        };
        // SAFETY: File is guaranteed to have the same ABI as RawFd
        unsafe { mem::transmute(raw_fd_ref) }
    }
}

fn open_fd(path: &Utf8CStr, flags: OFlag, mode: mode_t) -> OsResult<'_, OwnedFd> {
    nix::fcntl::open(path, flags, Mode::from_bits_truncate(mode)).into_os_result(
        "open",
        Some(path),
        None,
    )
}

pub fn fd_path(fd: RawFd, buf: &mut dyn Utf8CStrBuf) -> OsResult<'static, ()> {
    let path = cstr::buf::new::<64>()
        .join_path("/proc/self/fd")
        .join_path_fmt(fd);
    path.read_link(buf).map_err(|e| e.set_args(None, None))
}

pub struct FileAttr {
    pub st: libc::stat,
    #[cfg(feature = "selinux")]
    pub con: crate::Utf8CStrBufArr<128>,
}

impl Default for FileAttr {
    fn default() -> Self {
        Self::new()
    }
}

impl FileAttr {
    pub fn new() -> Self {
        FileAttr {
            st: unsafe { mem::zeroed() },
            #[cfg(feature = "selinux")]
            con: crate::Utf8CStrBufArr::new(),
        }
    }

    #[inline(always)]
    #[allow(clippy::unnecessary_cast)]
    fn is(&self, mode: mode_t) -> bool {
        (self.st.st_mode & libc::S_IFMT as c_uint) as mode_t == mode
    }

    pub fn is_dir(&self) -> bool {
        self.is(libc::S_IFDIR)
    }

    pub fn is_file(&self) -> bool {
        self.is(libc::S_IFREG)
    }

    pub fn is_symlink(&self) -> bool {
        self.is(libc::S_IFLNK)
    }

    pub fn is_block_device(&self) -> bool {
        self.is(libc::S_IFBLK)
    }

    pub fn is_char_device(&self) -> bool {
        self.is(libc::S_IFCHR)
    }

    pub fn is_fifo(&self) -> bool {
        self.is(libc::S_IFIFO)
    }

    pub fn is_socket(&self) -> bool {
        self.is(libc::S_IFSOCK)
    }

    pub fn is_whiteout(&self) -> bool {
        self.is_char_device() && self.st.st_rdev == 0
    }
}

const XATTR_NAME_SELINUX: &CStr = c"security.selinux";

// Low-level methods, we should track the caller when error occurs, so return OsResult.
impl Utf8CStr {
    pub fn follow_link(&self) -> &FsPathFollow {
        unsafe { mem::transmute(self) }
    }

    pub fn open(&self, flags: OFlag) -> OsResult<'_, File> {
        Ok(File::from(open_fd(self, flags, 0)?))
    }

    pub fn create(&self, flags: OFlag, mode: mode_t) -> OsResult<'_, File> {
        Ok(File::from(open_fd(self, OFlag::O_CREAT | flags, mode)?))
    }

    pub fn exists(&self) -> bool {
        nix::sys::stat::lstat(self).is_ok()
    }

    pub fn rename_to<'a>(&'a self, name: &'a Utf8CStr) -> OsResult<'a, ()> {
        nix::fcntl::renameat(AT_FDCWD, self, AT_FDCWD, name).check_os_err(
            "rename",
            Some(self),
            Some(name),
        )
    }

    pub fn remove(&self) -> OsResult<'_, ()> {
        unsafe { libc::remove(self.as_ptr()).check_os_err("remove", Some(self), None) }
    }

    #[allow(clippy::unnecessary_cast)]
    pub fn read_link(&self, buf: &mut dyn Utf8CStrBuf) -> OsResult<'_, ()> {
        buf.clear();
        unsafe {
            let r = libc::readlink(self.as_ptr(), buf.as_mut_ptr(), buf.capacity() - 1)
                .into_os_result("readlink", Some(self), None)? as isize;
            *(buf.as_mut_ptr().offset(r) as *mut u8) = b'\0';
        }
        buf.rebuild().ok();
        Ok(())
    }

    pub fn mkdir(&self, mode: mode_t) -> OsResult<'_, ()> {
        match nix::unistd::mkdir(self, Mode::from_bits_truncate(mode)) {
            Ok(_) | Err(Errno::EEXIST) => Ok(()),
            Err(e) => Err(OsError::new(e, "mkdir", Some(self), None)),
        }
    }

    // Inspired by https://android.googlesource.com/platform/bionic/+/master/libc/bionic/realpath.cpp
    pub fn realpath(&self, buf: &mut dyn Utf8CStrBuf) -> OsResult<'_, ()> {
        let fd = self.open(OFlag::O_PATH | OFlag::O_CLOEXEC)?;
        let mut skip_check = false;

        let st1 = match nix::sys::stat::fstat(&fd) {
            Ok(st) => st,
            Err(_) => {
                // This will only fail on Linux < 3.6
                skip_check = true;
                unsafe { mem::zeroed() }
            }
        };

        fd_path(fd.as_raw_fd(), buf)?;

        let st2 = nix::sys::stat::stat(buf.as_cstr()).into_os_result("stat", Some(self), None)?;
        if !skip_check && (st2.st_dev != st1.st_dev || st2.st_ino != st1.st_ino) {
            return Err(OsError::new(Errno::ENOENT, "realpath", Some(self), None));
        }
        Ok(())
    }

    pub fn get_attr(&self) -> OsResult<'_, FileAttr> {
        #[allow(unused_mut)]
        let mut attr = FileAttr {
            st: nix::sys::stat::lstat(self).into_os_result("lstat", Some(self), None)?,
            #[cfg(feature = "selinux")]
            con: cstr::buf::new(),
        };
        #[cfg(feature = "selinux")]
        self.get_secontext(&mut attr.con)?;
        Ok(attr)
    }

    pub fn set_attr<'a>(&'a self, attr: &'a FileAttr) -> OsResult<'a, ()> {
        if !attr.is_symlink()
            && let Err(e) = self.follow_link().chmod((attr.st.st_mode & 0o777).as_())
        {
            // Double check if self is symlink before reporting error
            let self_attr = self.get_attr()?;
            if !self_attr.is_symlink() {
                return Err(e);
            }
        }

        unsafe {
            libc::lchown(self.as_ptr(), attr.st.st_uid, attr.st.st_gid).check_os_err(
                "lchown",
                Some(self),
                None,
            )?;
        }

        #[cfg(feature = "selinux")]
        if !attr.con.is_empty() {
            self.set_secontext(&attr.con)?;
        }
        Ok(())
    }

    pub fn get_secontext(&self, con: &mut dyn Utf8CStrBuf) -> OsResult<'_, ()> {
        con.clear();
        let result = unsafe {
            libc::lgetxattr(
                self.as_ptr(),
                XATTR_NAME_SELINUX.as_ptr(),
                con.as_mut_ptr().cast(),
                con.capacity(),
            )
            .check_err()
        };

        match result {
            Ok(_) => {
                con.rebuild().ok();
                Ok(())
            }
            Err(Errno::ENODATA) => Ok(()),
            Err(e) => Err(OsError::new(e, "lgetxattr", Some(self), None)),
        }
    }

    pub fn set_secontext<'a>(&'a self, con: &'a Utf8CStr) -> OsResult<'a, ()> {
        unsafe {
            libc::lsetxattr(
                self.as_ptr(),
                XATTR_NAME_SELINUX.as_ptr(),
                con.as_ptr().cast(),
                con.len() + 1,
                0,
            )
            .check_os_err("lsetxattr", Some(self), Some(con))
        }
    }

    pub fn parent_dir(&self) -> Option<&str> {
        Path::new(self.as_str())
            .parent()
            .map(Path::as_os_str)
            // SAFETY: all substring of self is valid UTF-8
            .map(|s| unsafe { std::str::from_utf8_unchecked(s.as_bytes()) })
    }

    pub fn file_name(&self) -> Option<&str> {
        Path::new(self.as_str())
            .file_name()
            // SAFETY: all substring of self is valid UTF-8
            .map(|s| unsafe { std::str::from_utf8_unchecked(s.as_bytes()) })
    }

    // ln -s target self
    pub fn create_symlink_to<'a>(&'a self, target: &'a Utf8CStr) -> OsResult<'a, ()> {
        nix::unistd::symlinkat(target, AT_FDCWD, self).check_os_err(
            "symlink",
            Some(target),
            Some(self),
        )
    }

    pub fn mkfifo(&self, mode: mode_t) -> OsResult<'_, ()> {
        nix::unistd::mkfifo(self, Mode::from_bits_truncate(mode)).check_os_err(
            "mkfifo",
            Some(self),
            None,
        )
    }
}

// High-level helper methods, composed of multiple operations.
// We should treat these as application logic and log ASAP, so return LoggedResult.
impl Utf8CStr {
    pub fn remove_all(&self) -> LoggedResult<()> {
        let attr = match self.get_attr() {
            Ok(attr) => attr,
            Err(e) => {
                return match e.errno {
                    // Allow calling remove_all on non-existence file
                    Errno::ENOENT => Ok(()),
                    _ => Err(e)?,
                };
            }
        };
        if attr.is_dir() {
            let dir = Directory::open(self)?;
            dir.remove_all()?;
        }
        Ok(self.remove()?)
    }

    pub fn mkdirs(&self, mode: mode_t) -> LoggedResult<()> {
        if self.is_empty() {
            return Ok(());
        }

        let mut path = cstr::buf::default();
        let mut components = self.split('/').filter(|s| !s.is_empty());

        if self.starts_with('/') {
            path.append_path("/");
        }

        loop {
            let Some(s) = components.next() else {
                break;
            };
            path.append_path(s);
            path.mkdir(mode)?;
        }

        *errno() = 0;
        Ok(())
    }

    pub fn copy_to(&self, path: &Utf8CStr) -> LoggedResult<()> {
        let attr = self.get_attr()?;
        if attr.is_dir() {
            path.mkdir(0o777)?;
            let mut src = Directory::open(self)?;
            let dest = Directory::open(path)?;
            src.copy_into(&dest)?;
        } else {
            // It's OK if remove failed
            path.remove().ok();
            if attr.is_file() {
                let mut src = self.open(OFlag::O_RDONLY | OFlag::O_CLOEXEC)?;
                let mut dest = path.create(
                    OFlag::O_WRONLY | OFlag::O_CREAT | OFlag::O_TRUNC | OFlag::O_CLOEXEC,
                    0o777,
                )?;
                std::io::copy(&mut src, &mut dest)?;
            } else if attr.is_symlink() {
                let mut buf = cstr::buf::default();
                self.read_link(&mut buf)?;
                unsafe {
                    libc::symlink(buf.as_ptr(), path.as_ptr()).check_os_err(
                        "symlink",
                        Some(&buf),
                        Some(path),
                    )?;
                }
            }
        }
        path.set_attr(&attr)?;
        Ok(())
    }

    pub fn move_to(&self, path: &Utf8CStr) -> LoggedResult<()> {
        if path.exists() {
            let attr = path.get_attr()?;
            if attr.is_dir() {
                let mut src = Directory::open(self)?;
                let dest = Directory::open(path)?;
                return src.move_into(&dest);
            } else {
                path.remove()?;
            }
        }
        self.rename_to(path)?;
        Ok(())
    }

    // ln self path
    pub fn link_to(&self, path: &Utf8CStr) -> LoggedResult<()> {
        let attr = self.get_attr()?;
        if attr.is_dir() {
            path.mkdir(0o777)?;
            path.set_attr(&attr)?;
            let mut src = Directory::open(self)?;
            let dest = Directory::open(path)?;
            Ok(src.link_into(&dest)?)
        } else {
            unsafe {
                libc::link(self.as_ptr(), path.as_ptr()).check_os_err(
                    "link",
                    Some(self),
                    Some(path),
                )?;
            }
            Ok(())
        }
    }
}

impl FsPathFollow {
    pub fn exists(&self) -> bool {
        nix::unistd::access(self.as_utf8_cstr(), AccessFlags::F_OK).is_ok()
    }

    pub fn chmod(&self, mode: mode_t) -> OsResult<'_, ()> {
        nix::sys::stat::fchmodat(
            AT_FDCWD,
            self.as_utf8_cstr(),
            Mode::from_bits_truncate(mode),
            FchmodatFlags::FollowSymlink,
        )
        .check_os_err("chmod", Some(self), None)
    }

    pub fn get_attr(&self) -> OsResult<'_, FileAttr> {
        #[allow(unused_mut)]
        let mut attr = FileAttr {
            st: nix::sys::stat::stat(self.as_utf8_cstr()).into_os_result(
                "lstat",
                Some(self),
                None,
            )?,
            #[cfg(feature = "selinux")]
            con: cstr::buf::new(),
        };
        #[cfg(feature = "selinux")]
        self.get_secontext(&mut attr.con)?;
        Ok(attr)
    }

    pub fn set_attr<'a>(&'a self, attr: &'a FileAttr) -> OsResult<'a, ()> {
        self.chmod((attr.st.st_mode & 0o777).as_())?;

        nix::unistd::chown(
            self.as_utf8_cstr(),
            Some(Uid::from(attr.st.st_uid)),
            Some(Gid::from(attr.st.st_gid)),
        )
        .check_os_err("chown", Some(self), None)?;

        #[cfg(feature = "selinux")]
        if !attr.con.is_empty() {
            self.set_secontext(&attr.con)?;
        }
        Ok(())
    }

    pub fn get_secontext(&self, con: &mut dyn Utf8CStrBuf) -> OsResult<'_, ()> {
        con.clear();
        let result = unsafe {
            libc::getxattr(
                self.as_ptr(),
                XATTR_NAME_SELINUX.as_ptr(),
                con.as_mut_ptr().cast(),
                con.capacity(),
            )
            .check_err()
        };

        match result {
            Ok(_) => {
                con.rebuild().ok();
                Ok(())
            }
            Err(Errno::ENODATA) => Ok(()),
            Err(e) => Err(OsError::new(e, "getxattr", Some(self), None)),
        }
    }

    pub fn set_secontext<'a>(&'a self, con: &'a Utf8CStr) -> OsResult<'a, ()> {
        unsafe {
            libc::setxattr(
                self.as_ptr(),
                XATTR_NAME_SELINUX.as_ptr(),
                con.as_ptr().cast(),
                con.len() + 1,
                0,
            )
            .check_os_err("setxattr", Some(self), Some(con))
        }
    }
}

pub trait FsPathBuilder {
    fn join_path<T: AsRef<str>>(mut self, path: T) -> Self
    where
        Self: Sized,
    {
        self.append_path(path);
        self
    }
    fn join_path_fmt<T: Display>(mut self, name: T) -> Self
    where
        Self: Sized,
    {
        self.append_path_fmt(name);
        self
    }
    fn append_path<T: AsRef<str>>(&mut self, path: T) -> &mut Self;
    fn append_path_fmt<T: Display>(&mut self, name: T) -> &mut Self;
}

fn append_path_impl(buf: &mut dyn Utf8CStrBuf, path: &str) {
    if path.starts_with('/') {
        buf.clear();
    }
    if !buf.is_empty() && !buf.ends_with('/') {
        buf.push_str("/");
    }
    buf.push_str(path);
}

impl<S: Utf8CStrBuf + Sized> FsPathBuilder for S {
    fn append_path<T: AsRef<str>>(&mut self, path: T) -> &mut Self {
        append_path_impl(self, path.as_ref());
        self
    }

    fn append_path_fmt<T: Display>(&mut self, name: T) -> &mut Self {
        self.write_fmt(format_args!("/{name}")).ok();
        self
    }
}

impl FsPathBuilder for dyn Utf8CStrBuf + '_ {
    fn append_path<T: AsRef<str>>(&mut self, path: T) -> &mut Self {
        append_path_impl(self, path.as_ref());
        self
    }

    fn append_path_fmt<T: Display>(&mut self, name: T) -> &mut Self {
        self.write_fmt(format_args!("/{name}")).ok();
        self
    }
}

pub fn fd_get_attr(fd: RawFd) -> OsResult<'static, FileAttr> {
    let mut attr = FileAttr::new();
    unsafe {
        libc::fstat(fd, &mut attr.st).check_os_err("fstat", None, None)?;

        #[cfg(feature = "selinux")]
        fd_get_secontext(fd, &mut attr.con)?;
    }
    Ok(attr)
}

pub fn fd_set_attr(fd: RawFd, attr: &FileAttr) -> OsResult<'_, ()> {
    unsafe {
        libc::fchmod(fd, (attr.st.st_mode & 0o777).as_()).check_os_err("fchmod", None, None)?;
        libc::fchown(fd, attr.st.st_uid, attr.st.st_gid).check_os_err("fchown", None, None)?;

        #[cfg(feature = "selinux")]
        if !attr.con.is_empty() {
            fd_set_secontext(fd, &attr.con)?;
        }
    }
    Ok(())
}

pub fn fd_get_secontext(fd: RawFd, con: &mut dyn Utf8CStrBuf) -> OsResult<'static, ()> {
    con.clear();
    let result = unsafe {
        libc::fgetxattr(
            fd,
            XATTR_NAME_SELINUX.as_ptr(),
            con.as_mut_ptr().cast(),
            con.capacity(),
        )
        .check_err()
    };

    match result {
        Ok(_) => {
            con.rebuild().ok();
            Ok(())
        }
        Err(Errno::ENODATA) => Ok(()),
        Err(e) => Err(OsError::new(e, "fgetxattr", None, None)),
    }
}

pub fn fd_set_secontext(fd: RawFd, con: &Utf8CStr) -> OsResult<'_, ()> {
    unsafe {
        libc::fsetxattr(
            fd,
            XATTR_NAME_SELINUX.as_ptr(),
            con.as_ptr().cast(),
            con.len() + 1,
            0,
        )
        .check_os_err("fsetxattr", Some(con), None)
    }
}

pub fn clone_attr<'a>(a: &'a Utf8CStr, b: &'a Utf8CStr) -> OsResult<'a, ()> {
    let attr = a.get_attr().map_err(|e| e.set_args(Some(a), None))?;
    b.set_attr(&attr).map_err(|e| e.set_args(Some(b), None))
}

pub fn fclone_attr(a: RawFd, b: RawFd) -> OsResult<'static, ()> {
    let attr = fd_get_attr(a)?;
    fd_set_attr(b, &attr).map_err(|e| e.set_args(None, None))
}

pub struct MappedFile(&'static mut [u8]);

impl MappedFile {
    pub fn open(path: &Utf8CStr) -> OsResult<'_, MappedFile> {
        Ok(MappedFile(map_file(path, false)?))
    }

    pub fn open_rw(path: &Utf8CStr) -> OsResult<'_, MappedFile> {
        Ok(MappedFile(map_file(path, true)?))
    }

    pub fn openat<'a, T: AsFd>(dir: &T, path: &'a Utf8CStr) -> OsResult<'a, MappedFile> {
        Ok(MappedFile(map_file_at(dir.as_fd(), path, false)?))
    }

    pub fn openat_rw<'a, T: AsFd>(dir: &T, path: &'a Utf8CStr) -> OsResult<'a, MappedFile> {
        Ok(MappedFile(map_file_at(dir.as_fd(), path, true)?))
    }

    pub fn create(fd: BorrowedFd, sz: usize, rw: bool) -> OsResult<MappedFile> {
        Ok(MappedFile(map_fd(fd, sz, rw)?))
    }
}

impl AsRef<[u8]> for MappedFile {
    fn as_ref(&self) -> &[u8] {
        self.0
    }
}

impl AsMut<[u8]> for MappedFile {
    fn as_mut(&mut self) -> &mut [u8] {
        self.0
    }
}

impl Drop for MappedFile {
    fn drop(&mut self) {
        unsafe {
            libc::munmap(self.0.as_mut_ptr().cast(), self.0.len());
        }
    }
}

unsafe extern "C" {
    // Don't use the declaration from the libc crate as request should be u32 not i32
    fn ioctl(fd: RawFd, request: u32, ...) -> i32;
}

// We mark the returned slice static because it is valid until explicitly unmapped
pub(crate) fn map_file(path: &Utf8CStr, rw: bool) -> OsResult<'_, &'static mut [u8]> {
    map_file_at(AT_FDCWD, path, rw)
}

pub(crate) fn map_file_at<'a>(
    dirfd: BorrowedFd,
    path: &'a Utf8CStr,
    rw: bool,
) -> OsResult<'a, &'static mut [u8]> {
    #[cfg(target_pointer_width = "64")]
    const BLKGETSIZE64: u32 = 0x80081272;

    #[cfg(target_pointer_width = "32")]
    const BLKGETSIZE64: u32 = 0x80041272;

    let flag = if rw { OFlag::O_RDWR } else { OFlag::O_RDONLY };
    let fd = nix::fcntl::openat(dirfd, path, flag | OFlag::O_CLOEXEC, Mode::empty())
        .into_os_result("openat", Some(path), None)?;
    let attr = fd_get_attr(fd.as_raw_fd())?;
    let sz = if attr.is_block_device() {
        let mut sz = 0_u64;
        unsafe {
            ioctl(fd.as_raw_fd(), BLKGETSIZE64, &mut sz).check_os_err("ioctl", Some(path), None)?;
        }
        sz
    } else {
        attr.st.st_size as u64
    };

    map_fd(fd.as_fd(), sz as usize, rw).map_err(|e| e.set_args(Some(path), None))
}

pub(crate) fn map_fd(fd: BorrowedFd, sz: usize, rw: bool) -> OsResult<'static, &'static mut [u8]> {
    let flag = if rw {
        libc::MAP_SHARED
    } else {
        libc::MAP_PRIVATE
    };
    unsafe {
        let ptr = libc::mmap(
            ptr::null_mut(),
            sz,
            libc::PROT_READ | libc::PROT_WRITE,
            flag,
            fd.as_raw_fd(),
            0,
        );
        if ptr == libc::MAP_FAILED {
            return Err(OsError::last_os_error("mmap", None, None));
        }
        Ok(slice::from_raw_parts_mut(ptr.cast(), sz))
    }
}

#[allow(dead_code)]
pub struct MountInfo {
    pub id: u32,
    pub parent: u32,
    pub device: u64,
    pub root: String,
    pub target: String,
    pub vfs_option: String,
    pub shared: u32,
    pub master: u32,
    pub propagation_from: u32,
    pub unbindable: bool,
    pub fs_type: String,
    pub source: String,
    pub fs_option: String,
}

#[allow(clippy::useless_conversion)]
fn parse_mount_info_line(line: &str) -> Option<MountInfo> {
    let mut iter = line.split_whitespace();
    let id = iter.next()?.parse().ok()?;
    let parent = iter.next()?.parse().ok()?;
    let (maj, min) = iter.next()?.split_once(':')?;
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
    Some(MountInfo {
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

pub fn parse_mount_info(pid: &str) -> Vec<MountInfo> {
    let mut res = vec![];
    let mut path = format!("/proc/{pid}/mountinfo");
    if let Ok(file) = Utf8CStr::from_string(&mut path).open(OFlag::O_RDONLY | OFlag::O_CLOEXEC) {
        BufReader::new(file).for_each_line(|line| {
            parse_mount_info_line(line)
                .map(|info| res.push(info))
                .is_some()
        });
    }
    res
}
