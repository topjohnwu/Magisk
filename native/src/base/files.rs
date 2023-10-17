use mem::MaybeUninit;
use std::cmp::min;
use std::ffi::CStr;
use std::fs::File;
use std::io::{BufRead, Read, Seek, SeekFrom, Write};
use std::ops::Deref;
use std::os::android::fs::MetadataExt;
use std::os::fd::{AsFd, BorrowedFd, IntoRawFd};
use std::os::unix::fs::FileTypeExt;
use std::os::unix::io::{AsRawFd, FromRawFd, OwnedFd, RawFd};
use std::{io, mem, ptr, slice};

use bytemuck::{bytes_of_mut, Pod};
use libc::{
    c_uint, dirent, mode_t, EEXIST, ENOENT, F_OK, O_CLOEXEC, O_CREAT, O_PATH, O_RDONLY, O_RDWR,
    O_TRUNC, O_WRONLY, S_IFDIR, S_IFLNK, S_IFREG,
};
use num_traits::AsPrimitive;

use crate::cxx_extern::readlinkat_for_cxx;
use crate::{
    copy_cstr, cstr, errno, error, FsPath, FsPathBuf, LibcReturn, Utf8CStr, Utf8CStrBuf,
    Utf8CStrBufArr,
};

pub fn __open_fd_impl(path: &Utf8CStr, flags: i32, mode: mode_t) -> io::Result<OwnedFd> {
    unsafe {
        let fd = libc::open(path.as_ptr(), flags, mode as c_uint).check_os_err()?;
        Ok(OwnedFd::from_raw_fd(fd))
    }
}

#[macro_export]
macro_rules! open_fd {
    ($path:expr, $flags:expr) => {
        $crate::__open_fd_impl($path, $flags, 0)
    };
    ($path:expr, $flags:expr, $mode:expr) => {
        $crate::__open_fd_impl($path, $flags, $mode)
    };
}

pub fn fd_path(fd: RawFd, buf: &mut dyn Utf8CStrBuf) -> io::Result<()> {
    let mut arr = Utf8CStrBufArr::<40>::new();
    let path = FsPathBuf::new(&mut arr).join("/proc/self/fd").join_fmt(fd);
    path.read_link(buf)
}

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
    fn foreach_lines<F: FnMut(&mut String) -> bool>(&mut self, f: F);
    fn foreach_props<F: FnMut(&str, &str) -> bool>(&mut self, f: F);
}

impl<T: BufRead> BufReadExt for T {
    fn foreach_lines<F: FnMut(&mut String) -> bool>(&mut self, mut f: F) {
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

    fn foreach_props<F: FnMut(&str, &str) -> bool>(&mut self, mut f: F) {
        self.foreach_lines(|line| {
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
}

pub struct FileAttr {
    pub st: libc::stat,
    #[cfg(feature = "selinux")]
    pub con: Utf8CStrBufArr<128>,
}

impl FileAttr {
    fn new() -> Self {
        FileAttr {
            st: unsafe { mem::zeroed() },
            #[cfg(feature = "selinux")]
            con: Utf8CStrBufArr::new(),
        }
    }
}

#[cfg(feature = "selinux")]
const XATTR_NAME_SELINUX: &[u8] = b"security.selinux\0";

pub struct DirEntry<'a> {
    dir: &'a Directory,
    entry: &'a dirent,
    d_name_len: usize,
}

impl DirEntry<'_> {
    pub fn d_name(&self) -> &CStr {
        unsafe {
            CStr::from_bytes_with_nul_unchecked(slice::from_raw_parts(
                self.d_name.as_ptr().cast(),
                self.d_name_len,
            ))
        }
    }

    pub fn path(&self, buf: &mut dyn Utf8CStrBuf) -> io::Result<()> {
        self.dir.path(buf)?;
        buf.push_str("/");
        buf.push_lossy(self.d_name().to_bytes());
        Ok(())
    }

    pub fn is_dir(&self) -> bool {
        self.d_type == libc::DT_DIR
    }

    pub fn is_file(&self) -> bool {
        self.d_type == libc::DT_REG
    }

    pub fn is_lnk(&self) -> bool {
        self.d_type == libc::DT_LNK
    }

    pub fn unlink(&self) -> io::Result<()> {
        let flag = if self.is_dir() { libc::AT_REMOVEDIR } else { 0 };
        unsafe {
            libc::unlinkat(self.dir.as_raw_fd(), self.d_name.as_ptr(), flag).check_os_err()?;
        }
        Ok(())
    }

    pub fn read_link(&self, buf: &mut dyn Utf8CStrBuf) -> io::Result<()> {
        buf.clear();
        unsafe {
            let r = readlinkat_for_cxx(
                self.dir.as_raw_fd(),
                self.d_name.as_ptr(),
                buf.as_mut_ptr().cast(),
                buf.capacity(),
            )
            .check_os_err()? as usize;
            buf.set_len(r);
        }
        Ok(())
    }

    unsafe fn open_fd(&self, flags: i32) -> io::Result<RawFd> {
        self.dir.open_fd(self.d_name(), flags, 0)
    }

    pub fn open_as_dir(&self) -> io::Result<Directory> {
        if !self.is_dir() {
            return Err(io::Error::from(io::ErrorKind::NotADirectory));
        }
        unsafe { Directory::try_from(OwnedFd::from_raw_fd(self.open_fd(O_RDONLY)?)) }
    }

    pub fn open_as_file(&self, flags: i32) -> io::Result<File> {
        if self.is_dir() {
            return Err(io::Error::from(io::ErrorKind::IsADirectory));
        }
        unsafe { Ok(File::from_raw_fd(self.open_fd(flags)?)) }
    }

    pub fn get_attr(&self) -> io::Result<FileAttr> {
        let mut path = Utf8CStrBufArr::default();
        self.path(&mut path)?;
        FsPath::from(&path).get_attr()
    }

    pub fn set_attr(&self, attr: &FileAttr) -> io::Result<()> {
        let mut path = Utf8CStrBufArr::default();
        self.path(&mut path)?;
        FsPath::from(&path).set_attr(attr)
    }
}

impl Deref for DirEntry<'_> {
    type Target = dirent;

    fn deref(&self) -> &dirent {
        self.entry
    }
}

pub struct Directory {
    dirp: *mut libc::DIR,
}

pub enum WalkResult {
    Continue,
    Abort,
    Skip,
}

impl Directory {
    pub fn open(path: &Utf8CStr) -> io::Result<Directory> {
        let dirp = unsafe { libc::opendir(path.as_ptr()) }.check_os_err()?;
        Ok(Directory { dirp })
    }

    pub fn read(&mut self) -> io::Result<Option<DirEntry<'_>>> {
        *errno() = 0;
        let e = unsafe { libc::readdir(self.dirp) };
        if e.is_null() {
            return if *errno() != 0 {
                Err(io::Error::last_os_error())
            } else {
                Ok(None)
            };
        }
        // Skip both "." and ".."
        unsafe {
            let entry = &*e;
            let d_name = CStr::from_ptr(entry.d_name.as_ptr());
            return if d_name == cstr!(".") || d_name == cstr!("..") {
                self.read()
            } else {
                let e = DirEntry {
                    dir: self,
                    entry,
                    d_name_len: d_name.to_bytes_with_nul().len(),
                };
                Ok(Some(e))
            };
        }
    }

    pub fn rewind(&mut self) {
        unsafe { libc::rewinddir(self.dirp) }
    }

    unsafe fn open_fd(&self, name: &CStr, flags: i32, mode: i32) -> io::Result<RawFd> {
        libc::openat(self.as_raw_fd(), name.as_ptr(), flags | O_CLOEXEC, mode).check_os_err()
    }

    pub fn contains_path(&self, path: &CStr) -> bool {
        // WARNING: Using faccessat is incorrect, because the raw linux kernel syscall
        // does not support the flag AT_SYMLINK_NOFOLLOW until 5.8 with faccessat2.
        // Use fstatat to check the existence of a file instead.
        unsafe {
            let mut st: libc::stat = mem::zeroed();
            libc::fstatat(
                self.as_raw_fd(),
                path.as_ptr(),
                &mut st,
                libc::AT_SYMLINK_NOFOLLOW,
            ) == 0
        }
    }

    pub fn path(&self, buf: &mut dyn Utf8CStrBuf) -> io::Result<()> {
        fd_path(self.as_raw_fd(), buf)
    }

    pub fn post_order_walk<F: FnMut(&DirEntry) -> io::Result<WalkResult>>(
        &mut self,
        mut f: F,
    ) -> io::Result<WalkResult> {
        self.post_order_walk_impl(&mut f)
    }

    pub fn pre_order_walk<F: FnMut(&DirEntry) -> io::Result<WalkResult>>(
        &mut self,
        mut f: F,
    ) -> io::Result<WalkResult> {
        self.pre_order_walk_impl(&mut f)
    }

    pub fn remove_all(&mut self) -> io::Result<()> {
        self.post_order_walk(|e| {
            e.unlink()?;
            Ok(WalkResult::Continue)
        })?;
        Ok(())
    }

    pub fn copy_into(&mut self, dir: &Directory) -> io::Result<()> {
        while let Some(ref e) = self.read()? {
            let attr = e.get_attr()?;
            let new_entry = DirEntry {
                dir,
                entry: e.entry,
                d_name_len: e.d_name_len,
            };
            if e.is_dir() {
                unsafe {
                    libc::mkdirat(dir.as_raw_fd(), e.d_name.as_ptr(), 0o777).as_os_err()?;
                }
                let mut src = e.open_as_dir()?;
                let dest = new_entry.open_as_dir()?;
                src.copy_into(&dest)?;
                fd_set_attr(dest.as_raw_fd(), &attr)?;
            } else if e.is_file() {
                let mut src = e.open_as_file(O_RDONLY)?;
                let mut dest = unsafe {
                    File::from_raw_fd(dir.open_fd(
                        e.d_name(),
                        O_WRONLY | O_CREAT | O_TRUNC,
                        0o777,
                    )?)
                };
                std::io::copy(&mut src, &mut dest)?;
                fd_set_attr(dest.as_raw_fd(), &attr)?;
            } else if e.is_lnk() {
                let mut path = Utf8CStrBufArr::default();
                e.read_link(&mut path)?;
                unsafe {
                    libc::symlinkat(path.as_ptr(), dir.as_raw_fd(), e.d_name.as_ptr())
                        .as_os_err()?;
                }
                new_entry.set_attr(&attr)?;
            }
        }
        Ok(())
    }

    pub fn move_into(&mut self, dir: &Directory) -> io::Result<()> {
        let dir_fd = self.as_raw_fd();
        while let Some(ref e) = self.read()? {
            if e.is_dir() && dir.contains_path(e.d_name()) {
                // Destination folder exists, needs recursive move
                let mut src = e.open_as_dir()?;
                let new_entry = DirEntry {
                    dir,
                    entry: e.entry,
                    d_name_len: e.d_name_len,
                };
                let dest = new_entry.open_as_dir()?;
                src.move_into(&dest)?;
                return e.unlink();
            }

            unsafe {
                libc::renameat(
                    dir_fd,
                    e.d_name.as_ptr(),
                    dir.as_raw_fd(),
                    e.d_name.as_ptr(),
                )
                .as_os_err()?;
            }
        }
        Ok(())
    }

    pub fn link_into(&mut self, dir: &Directory) -> io::Result<()> {
        let dir_fd = self.as_raw_fd();
        while let Some(ref e) = self.read()? {
            if e.is_dir() {
                unsafe {
                    libc::mkdirat(dir.as_raw_fd(), e.d_name.as_ptr(), 0o777).as_os_err()?;
                }
                let attr = e.get_attr()?;
                let new_entry = DirEntry {
                    dir,
                    entry: e.entry,
                    d_name_len: e.d_name_len,
                };
                let mut src = e.open_as_dir()?;
                let dest = new_entry.open_as_dir()?;
                src.link_into(&dest)?;
                fd_set_attr(dest.as_raw_fd(), &attr)?;
            } else {
                unsafe {
                    libc::linkat(
                        dir_fd,
                        e.d_name.as_ptr(),
                        dir.as_raw_fd(),
                        e.d_name.as_ptr(),
                        0,
                    )
                    .as_os_err()?;
                }
            }
        }
        Ok(())
    }
}

impl Directory {
    fn post_order_walk_impl<F: FnMut(&DirEntry) -> io::Result<WalkResult>>(
        &mut self,
        f: &mut F,
    ) -> io::Result<WalkResult> {
        use WalkResult::*;
        loop {
            match self.read()? {
                None => return Ok(Continue),
                Some(ref e) => {
                    if e.is_dir() {
                        let mut dir = e.open_as_dir()?;
                        if let Abort = dir.post_order_walk_impl(f)? {
                            return Ok(Abort);
                        }
                    }
                    match f(e)? {
                        Abort => return Ok(Abort),
                        Skip => return Ok(Continue),
                        Continue => {}
                    }
                }
            }
        }
    }

    fn pre_order_walk_impl<F: FnMut(&DirEntry) -> io::Result<WalkResult>>(
        &mut self,
        f: &mut F,
    ) -> io::Result<WalkResult> {
        use WalkResult::*;
        loop {
            match self.read()? {
                None => return Ok(Continue),
                Some(ref e) => match f(e)? {
                    Abort => return Ok(Abort),
                    Skip => continue,
                    Continue => {
                        if e.is_dir() {
                            let mut dir = e.open_as_dir()?;
                            if let Abort = dir.pre_order_walk_impl(f)? {
                                return Ok(Abort);
                            }
                        }
                    }
                },
            }
        }
    }
}

impl TryFrom<OwnedFd> for Directory {
    type Error = io::Error;

    fn try_from(fd: OwnedFd) -> io::Result<Self> {
        let dirp = unsafe { libc::fdopendir(fd.into_raw_fd()) }.check_os_err()?;
        Ok(Directory { dirp })
    }
}

impl AsRawFd for Directory {
    fn as_raw_fd(&self) -> RawFd {
        unsafe { libc::dirfd(self.dirp) }
    }
}

impl AsFd for Directory {
    fn as_fd(&self) -> BorrowedFd {
        unsafe { BorrowedFd::borrow_raw(self.as_raw_fd()) }
    }
}

impl Drop for Directory {
    fn drop(&mut self) {
        unsafe {
            libc::closedir(self.dirp);
        }
    }
}

impl FsPath {
    pub fn open(&self, flags: i32) -> io::Result<File> {
        Ok(File::from(open_fd!(self, flags)?))
    }

    pub fn create(&self, flags: i32, mode: mode_t) -> io::Result<File> {
        Ok(File::from(open_fd!(self, flags, mode)?))
    }

    pub fn exists(&self) -> bool {
        unsafe { libc::access(self.as_ptr(), F_OK) == 0 }
    }

    pub fn rename_to<T: AsRef<Utf8CStr>>(&self, name: T) -> io::Result<()> {
        unsafe { libc::rename(self.as_ptr(), name.as_ref().as_ptr()).as_os_err() }
    }

    pub fn remove(&self) -> io::Result<()> {
        unsafe { libc::remove(self.as_ptr()).as_os_err() }
    }

    pub fn remove_all(&self) -> io::Result<()> {
        let f = self.open(O_RDONLY | O_CLOEXEC)?;
        let st = f.metadata()?;
        if st.is_dir() {
            let mut dir = Directory::try_from(OwnedFd::from(f))?;
            dir.remove_all()?;
        }
        self.remove()
    }

    pub fn read_link(&self, buf: &mut dyn Utf8CStrBuf) -> io::Result<()> {
        buf.clear();
        unsafe {
            let r = libc::readlink(self.as_ptr(), buf.as_mut_ptr().cast(), buf.capacity() - 1)
                .check_os_err()? as usize;
            *buf.mut_buf().get_unchecked_mut(r) = b'\0';
            buf.set_len(r);
        }
        Ok(())
    }

    pub fn mkdir(&self, mode: mode_t) -> io::Result<()> {
        unsafe {
            if libc::mkdir(self.as_ptr(), mode) < 0 {
                if *errno() == EEXIST {
                    libc::chmod(self.as_ptr(), mode).as_os_err()?;
                } else {
                    return Err(io::Error::last_os_error());
                }
            }
        }
        Ok(())
    }

    pub fn mkdirs(&self, mode: mode_t) -> io::Result<()> {
        let mut buf = [0_u8; 4096];
        let len = copy_cstr(&mut buf, self);
        let buf = &mut buf[..len];
        let mut off = 1;
        unsafe {
            while let Some(p) = buf[off..].iter().position(|c| *c == b'/') {
                buf[off + p] = b'\0';
                if libc::mkdir(buf.as_ptr().cast(), mode) < 0 && *errno() != EEXIST {
                    return Err(io::Error::last_os_error());
                }
                buf[off + p] = b'/';
                off += p + 1;
            }
            if libc::mkdir(buf.as_ptr().cast(), mode) < 0 && *errno() != EEXIST {
                return Err(io::Error::last_os_error());
            }
        }
        *errno() = 0;
        Ok(())
    }

    // Inspired by https://android.googlesource.com/platform/bionic/+/master/libc/bionic/realpath.cpp
    pub fn realpath(&self, buf: &mut dyn Utf8CStrBuf) -> io::Result<()> {
        let fd = open_fd!(self, O_PATH | O_CLOEXEC)?;
        let mut st1: libc::stat;
        let mut st2: libc::stat;
        let mut skip_check = false;
        unsafe {
            st1 = mem::zeroed();
            if libc::fstat(fd.as_raw_fd(), &mut st1) < 0 {
                // This will only fail on Linux < 3.6
                skip_check = true;
            }
        }
        fd_path(fd.as_raw_fd(), buf)?;
        unsafe {
            st2 = mem::zeroed();
            libc::stat(buf.as_ptr(), &mut st2).as_os_err()?;
            if !skip_check && (st2.st_dev != st1.st_dev || st2.st_ino != st1.st_ino) {
                *errno() = ENOENT;
                return Err(io::Error::last_os_error());
            }
        }
        Ok(())
    }

    pub fn get_attr(&self) -> io::Result<FileAttr> {
        let mut attr = FileAttr::new();
        unsafe {
            libc::lstat(self.as_ptr(), &mut attr.st).as_os_err()?;

            #[cfg(feature = "selinux")]
            {
                let sz = libc::lgetxattr(
                    self.as_ptr(),
                    XATTR_NAME_SELINUX.as_ptr().cast(),
                    attr.con.as_mut_ptr().cast(),
                    attr.con.capacity(),
                )
                .check_os_err()?;
                attr.con.set_len((sz - 1) as usize);
            }
        }
        Ok(attr)
    }

    pub fn set_attr(&self, attr: &FileAttr) -> io::Result<()> {
        unsafe {
            if (attr.st.st_mode & libc::S_IFMT as c_uint) != S_IFLNK.as_() {
                libc::chmod(self.as_ptr(), (attr.st.st_mode & 0o777).as_()).as_os_err()?;
            }
            libc::lchown(self.as_ptr(), attr.st.st_uid, attr.st.st_gid).as_os_err()?;

            #[cfg(feature = "selinux")]
            if !attr.con.is_empty() {
                libc::lsetxattr(
                    self.as_ptr(),
                    XATTR_NAME_SELINUX.as_ptr().cast(),
                    attr.con.as_ptr().cast(),
                    attr.con.len() + 1,
                    0,
                )
                .as_os_err()?;
            }
        }
        Ok(())
    }

    pub fn copy_to(&self, path: &FsPath) -> io::Result<()> {
        let attr = self.get_attr()?;
        if (attr.st.st_mode & libc::S_IFMT as c_uint) == S_IFDIR.as_() {
            path.mkdir(0o777)?;
            let mut src = Directory::open(self)?;
            let dest = Directory::open(path)?;
            src.copy_into(&dest)?;
        } else {
            // It's OK if remove failed
            path.remove().ok();
            if (attr.st.st_mode & libc::S_IFMT as c_uint) == S_IFREG.as_() {
                let mut src = self.open(O_RDONLY | O_CLOEXEC)?;
                let mut dest = path.create(O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0o777)?;
                std::io::copy(&mut src, &mut dest)?;
            } else if (attr.st.st_mode & libc::S_IFMT as c_uint) == S_IFLNK.as_() {
                let mut buf = Utf8CStrBufArr::default();
                self.read_link(&mut buf)?;
                unsafe {
                    libc::symlink(buf.as_ptr(), path.as_ptr()).as_os_err()?;
                }
            }
        }
        path.set_attr(&attr)?;
        Ok(())
    }

    pub fn move_to(&self, path: &FsPath) -> io::Result<()> {
        if path.exists() {
            let attr = path.get_attr()?;
            if (attr.st.st_mode & libc::S_IFMT as c_uint) == S_IFDIR.as_() {
                let mut src = Directory::open(self)?;
                let dest = Directory::open(path)?;
                return src.move_into(&dest);
            } else {
                path.remove()?;
            }
        }
        self.rename_to(path)
    }

    pub fn link_to(&self, path: &FsPath) -> io::Result<()> {
        let attr = self.get_attr()?;
        if (attr.st.st_mode & libc::S_IFMT as c_uint) == S_IFDIR.as_() {
            path.mkdir(0o777)?;
            path.set_attr(&attr)?;
            let mut src = Directory::open(self)?;
            let dest = Directory::open(path)?;
            src.link_into(&dest)
        } else {
            unsafe { libc::link(self.as_ptr(), path.as_ptr()).as_os_err() }
        }
    }
}

pub fn fd_get_attr(fd: RawFd) -> io::Result<FileAttr> {
    let mut attr = FileAttr::new();
    unsafe {
        libc::fstat(fd, &mut attr.st).as_os_err()?;

        #[cfg(feature = "selinux")]
        {
            let sz = libc::fgetxattr(
                fd,
                XATTR_NAME_SELINUX.as_ptr().cast(),
                attr.con.as_mut_ptr().cast(),
                attr.con.capacity(),
            )
            .check_os_err()?;
            attr.con.set_len((sz - 1) as usize);
        }
    }
    Ok(attr)
}

pub fn fd_set_attr(fd: RawFd, attr: &FileAttr) -> io::Result<()> {
    unsafe {
        libc::fchmod(fd, (attr.st.st_mode & 0o777).as_()).as_os_err()?;
        libc::fchown(fd, attr.st.st_uid, attr.st.st_gid).as_os_err()?;

        #[cfg(feature = "selinux")]
        if !attr.con.is_empty() {
            libc::fsetxattr(
                fd,
                XATTR_NAME_SELINUX.as_ptr().cast(),
                attr.con.as_ptr().cast(),
                attr.con.len() + 1,
                0,
            )
            .as_os_err()?;
        }
    }
    Ok(())
}

pub fn clone_attr(a: &FsPath, b: &FsPath) -> io::Result<()> {
    let attr = a.get_attr()?;
    b.set_attr(&attr)
}

pub fn fclone_attr(a: RawFd, b: RawFd) -> io::Result<()> {
    let attr = fd_get_attr(a)?;
    fd_set_attr(b, &attr)
}

pub struct MappedFile(&'static mut [u8]);

impl MappedFile {
    pub fn open(path: &Utf8CStr) -> io::Result<MappedFile> {
        Ok(MappedFile(map_file(path, false)?))
    }

    pub fn open_rw(path: &Utf8CStr) -> io::Result<MappedFile> {
        Ok(MappedFile(map_file(path, true)?))
    }

    pub fn create(fd: BorrowedFd, sz: usize, rw: bool) -> io::Result<MappedFile> {
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

extern "C" {
    // Don't use the declaration from the libc crate as request should be u32 not i32
    fn ioctl(fd: RawFd, request: u32, ...) -> i32;
}

// We mark the returned slice static because it is valid until explicitly unmapped
pub(crate) fn map_file(path: &Utf8CStr, rw: bool) -> io::Result<&'static mut [u8]> {
    #[cfg(target_pointer_width = "64")]
    const BLKGETSIZE64: u32 = 0x80081272;

    #[cfg(target_pointer_width = "32")]
    const BLKGETSIZE64: u32 = 0x80041272;

    let flag = if rw { O_RDWR } else { O_RDONLY };
    let f = File::from(open_fd!(path, flag | O_CLOEXEC)?);

    let st = f.metadata()?;
    let sz = if st.file_type().is_block_device() {
        let mut sz = 0_u64;
        unsafe { ioctl(f.as_raw_fd(), BLKGETSIZE64, &mut sz) }.as_os_err()?;
        sz
    } else {
        st.st_size()
    };

    map_fd(f.as_fd(), sz as usize, rw)
}

pub(crate) fn map_fd(fd: BorrowedFd, sz: usize, rw: bool) -> io::Result<&'static mut [u8]> {
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
            return Err(io::Error::last_os_error());
        }
        Ok(slice::from_raw_parts_mut(ptr.cast(), sz))
    }
}
