use crate::{
    cstr_buf, errno, error, Directory, FsPath, FsPathBuf, FsPathFollow, LibcReturn, Utf8CStr,
    Utf8CStrBuf,
};
use bytemuck::{bytes_of, bytes_of_mut, Pod};
use libc::{
    c_uint, makedev, mode_t, stat, EEXIST, ENOENT, F_OK, O_CLOEXEC, O_CREAT, O_PATH, O_RDONLY,
    O_RDWR, O_TRUNC, O_WRONLY,
};
use mem::MaybeUninit;
use num_traits::AsPrimitive;
use std::cmp::min;
use std::ffi::CStr;
use std::fs::File;
use std::io::{BufRead, BufReader, Read, Seek, SeekFrom, Write};
use std::os::fd::{AsFd, BorrowedFd};
use std::os::unix::ffi::OsStrExt;
use std::os::unix::io::{AsRawFd, FromRawFd, OwnedFd, RawFd};
use std::path::Path;
use std::{io, mem, ptr, slice};

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
    let path = FsPathBuf::default().join("/proc/self/fd").join_fmt(fd);
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

pub struct FileAttr {
    pub st: libc::stat,
    #[cfg(feature = "selinux")]
    pub con: crate::Utf8CStrBufArr<128>,
}

impl FileAttr {
    fn new() -> Self {
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
}

const XATTR_NAME_SELINUX: &CStr = c"security.selinux";

impl FsPath {
    pub fn follow_link(&self) -> &FsPathFollow {
        unsafe { mem::transmute(self) }
    }

    pub fn open(&self, flags: i32) -> io::Result<File> {
        Ok(File::from(open_fd!(self, flags)?))
    }

    pub fn create(&self, flags: i32, mode: mode_t) -> io::Result<File> {
        Ok(File::from(open_fd!(self, flags, mode)?))
    }

    pub fn exists(&self) -> bool {
        unsafe {
            let mut st: stat = mem::zeroed();
            libc::lstat(self.as_ptr(), &mut st) == 0
        }
    }

    pub fn rename_to<T: AsRef<Utf8CStr>>(&self, name: T) -> io::Result<()> {
        unsafe { libc::rename(self.as_ptr(), name.as_ref().as_ptr()).as_os_err() }
    }

    pub fn remove(&self) -> io::Result<()> {
        unsafe { libc::remove(self.as_ptr()).as_os_err() }
    }

    pub fn remove_all(&self) -> io::Result<()> {
        let attr = self.get_attr()?;
        if attr.is_dir() {
            let mut dir = Directory::try_from(open_fd!(self, O_RDONLY | O_CLOEXEC)?)?;
            dir.remove_all()?;
        }
        self.remove()
    }

    #[allow(clippy::unnecessary_cast)]
    pub fn read_link(&self, buf: &mut dyn Utf8CStrBuf) -> io::Result<()> {
        buf.clear();
        unsafe {
            let r = libc::readlink(self.as_ptr(), buf.as_mut_ptr(), buf.capacity() - 1)
                .check_os_err()? as isize;
            *(buf.as_mut_ptr().offset(r) as *mut u8) = b'\0';
            buf.set_len(r as usize);
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
        if self.is_empty() {
            return Ok(());
        }
        let mut arr = cstr_buf::default();
        arr.push_str(self);
        let mut off = 1;
        unsafe {
            let buf = arr.as_bytes_mut();
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
            self.get_secontext(&mut attr.con)?;
        }
        Ok(attr)
    }

    pub fn set_attr(&self, attr: &FileAttr) -> io::Result<()> {
        unsafe {
            if !attr.is_symlink() {
                libc::chmod(self.as_ptr(), (attr.st.st_mode & 0o777).as_()).as_os_err()?;
            }
            libc::lchown(self.as_ptr(), attr.st.st_uid, attr.st.st_gid).as_os_err()?;

            #[cfg(feature = "selinux")]
            if !attr.con.is_empty() {
                self.set_secontext(&attr.con)?;
            }
        }
        Ok(())
    }

    pub fn get_secontext(&self, con: &mut dyn Utf8CStrBuf) -> io::Result<()> {
        unsafe {
            let sz = libc::lgetxattr(
                self.as_ptr(),
                XATTR_NAME_SELINUX.as_ptr(),
                con.as_mut_ptr().cast(),
                con.capacity(),
            );
            if sz < 1 {
                con.clear();
                if *errno() != libc::ENODATA {
                    return Err(io::Error::last_os_error());
                }
            } else {
                con.set_len((sz - 1) as usize);
            }
        }
        Ok(())
    }

    pub fn set_secontext(&self, con: &Utf8CStr) -> io::Result<()> {
        unsafe {
            libc::lsetxattr(
                self.as_ptr(),
                XATTR_NAME_SELINUX.as_ptr(),
                con.as_ptr().cast(),
                con.len() + 1,
                0,
            )
            .as_os_err()
        }
    }

    pub fn copy_to(&self, path: &FsPath) -> io::Result<()> {
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
                let mut src = self.open(O_RDONLY | O_CLOEXEC)?;
                let mut dest = path.create(O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0o777)?;
                std::io::copy(&mut src, &mut dest)?;
            } else if attr.is_symlink() {
                let mut buf = cstr_buf::default();
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
            if attr.is_dir() {
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
        if attr.is_dir() {
            path.mkdir(0o777)?;
            path.set_attr(&attr)?;
            let mut src = Directory::open(self)?;
            let dest = Directory::open(path)?;
            src.link_into(&dest)
        } else {
            unsafe { libc::link(self.as_ptr(), path.as_ptr()).as_os_err() }
        }
    }

    pub fn symlink_to(&self, path: &FsPath) -> io::Result<()> {
        unsafe { libc::symlink(self.as_ptr(), path.as_ptr()).as_os_err() }
    }

    pub fn parent(&self, buf: &mut dyn Utf8CStrBuf) -> bool {
        buf.clear();
        if let Some(parent) = Path::new(self.as_str()).parent() {
            let bytes = parent.as_os_str().as_bytes();
            // SAFETY: all substring of self is valid UTF-8
            let parent = unsafe { std::str::from_utf8_unchecked(bytes) };
            buf.push_str(parent);
            true
        } else {
            false
        }
    }
}

impl FsPathFollow {
    pub fn exists(&self) -> bool {
        unsafe { libc::access(self.as_ptr(), F_OK) == 0 }
    }

    pub fn get_attr(&self) -> io::Result<FileAttr> {
        let mut attr = FileAttr::new();
        unsafe {
            libc::stat(self.as_ptr(), &mut attr.st).as_os_err()?;

            #[cfg(feature = "selinux")]
            self.get_secontext(&mut attr.con)?;
        }
        Ok(attr)
    }

    pub fn set_attr(&self, attr: &FileAttr) -> io::Result<()> {
        unsafe {
            libc::chmod(self.as_ptr(), (attr.st.st_mode & 0o777).as_()).as_os_err()?;
            libc::chown(self.as_ptr(), attr.st.st_uid, attr.st.st_gid).as_os_err()?;

            #[cfg(feature = "selinux")]
            if !attr.con.is_empty() {
                self.set_secontext(&attr.con)?;
            }
        }
        Ok(())
    }

    pub fn get_secontext(&self, con: &mut dyn Utf8CStrBuf) -> io::Result<()> {
        unsafe {
            let sz = libc::getxattr(
                self.as_ptr(),
                XATTR_NAME_SELINUX.as_ptr(),
                con.as_mut_ptr().cast(),
                con.capacity(),
            );
            if sz < 1 {
                con.clear();
                if *errno() != libc::ENODATA {
                    return Err(io::Error::last_os_error());
                }
            } else {
                con.set_len((sz - 1) as usize);
            }
        }
        Ok(())
    }

    pub fn set_secontext(&self, con: &Utf8CStr) -> io::Result<()> {
        unsafe {
            libc::setxattr(
                self.as_ptr(),
                XATTR_NAME_SELINUX.as_ptr(),
                con.as_ptr().cast(),
                con.len() + 1,
                0,
            )
            .as_os_err()
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
                XATTR_NAME_SELINUX.as_ptr(),
                attr.con.as_mut_ptr().cast(),
                attr.con.capacity(),
            );
            if sz < 1 {
                if *errno() != libc::ENODATA {
                    return Err(io::Error::last_os_error());
                }
            } else {
                attr.con.set_len((sz - 1) as usize);
            }
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
                XATTR_NAME_SELINUX.as_ptr(),
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

    pub fn openat<T: AsFd>(dir: &T, path: &Utf8CStr) -> io::Result<MappedFile> {
        Ok(MappedFile(map_file_at(dir.as_fd(), path, false)?))
    }

    pub fn openat_rw<T: AsFd>(dir: &T, path: &Utf8CStr) -> io::Result<MappedFile> {
        Ok(MappedFile(map_file_at(dir.as_fd(), path, true)?))
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

unsafe extern "C" {
    // Don't use the declaration from the libc crate as request should be u32 not i32
    fn ioctl(fd: RawFd, request: u32, ...) -> i32;
}

// We mark the returned slice static because it is valid until explicitly unmapped
pub(crate) fn map_file(path: &Utf8CStr, rw: bool) -> io::Result<&'static mut [u8]> {
    unsafe { map_file_at(BorrowedFd::borrow_raw(libc::AT_FDCWD), path, rw) }
}

pub(crate) fn map_file_at(
    dirfd: BorrowedFd,
    path: &Utf8CStr,
    rw: bool,
) -> io::Result<&'static mut [u8]> {
    #[cfg(target_pointer_width = "64")]
    const BLKGETSIZE64: u32 = 0x80081272;

    #[cfg(target_pointer_width = "32")]
    const BLKGETSIZE64: u32 = 0x80041272;

    let flag = if rw { O_RDWR } else { O_RDONLY };
    let fd = unsafe {
        OwnedFd::from_raw_fd(
            libc::openat(dirfd.as_raw_fd(), path.as_ptr(), flag | O_CLOEXEC).check_os_err()?,
        )
    };

    let attr = fd_get_attr(fd.as_raw_fd())?;
    let sz = if attr.is_block_device() {
        let mut sz = 0_u64;
        unsafe { ioctl(fd.as_raw_fd(), BLKGETSIZE64, &mut sz) }.as_os_err()?;
        sz
    } else {
        attr.st.st_size as u64
    };

    map_fd(fd.as_fd(), sz as usize, rw)
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
    let mut path = format!("/proc/{}/mountinfo", pid);
    if let Ok(fd) = open_fd!(Utf8CStr::from_string(&mut path), O_RDONLY | O_CLOEXEC) {
        let file = File::from(fd);
        BufReader::new(file).foreach_lines(|line| {
            parse_mount_info_line(line)
                .map(|info| res.push(info))
                .is_some()
        });
    }
    res
}
