use crate::cxx_extern::readlinkat;
use crate::{
    FsPathBuilder, LibcReturn, OsError, OsResult, OsResultStatic, Utf8CStr, Utf8CStrBuf, cstr,
    errno, fd_path, fd_set_attr,
};
use libc::{EEXIST, O_CLOEXEC, O_CREAT, O_RDONLY, O_TRUNC, O_WRONLY, dirent, mode_t};
use std::fs::File;
use std::marker::PhantomData;
use std::ops::{Deref, DerefMut};
use std::os::fd::{AsFd, AsRawFd, BorrowedFd, FromRawFd, IntoRawFd, OwnedFd, RawFd};
use std::ptr::NonNull;
use std::{mem, slice};

pub struct DirEntry<'a> {
    dir: BorrowedDirectory<'a>,
    entry: NonNull<dirent>,
    d_name_len: usize,
}

impl DirEntry<'_> {
    pub fn as_ptr(&self) -> *mut dirent {
        self.entry.as_ptr()
    }

    pub fn name(&self) -> &Utf8CStr {
        unsafe {
            Utf8CStr::from_bytes_unchecked(slice::from_raw_parts(
                self.d_name.as_ptr().cast(),
                self.d_name_len,
            ))
        }
    }

    pub fn resolve_path(&self, buf: &mut dyn Utf8CStrBuf) -> OsResult<'static, ()> {
        self.dir.path_at(self.name(), buf)
    }

    pub fn is_dir(&self) -> bool {
        self.d_type == libc::DT_DIR
    }

    pub fn is_file(&self) -> bool {
        self.d_type == libc::DT_REG
    }

    pub fn is_symlink(&self) -> bool {
        self.d_type == libc::DT_LNK
    }

    pub fn is_block_device(&self) -> bool {
        self.d_type == libc::DT_BLK
    }

    pub fn is_char_device(&self) -> bool {
        self.d_type == libc::DT_CHR
    }

    pub fn is_fifo(&self) -> bool {
        self.d_type == libc::DT_FIFO
    }

    pub fn is_socket(&self) -> bool {
        self.d_type == libc::DT_SOCK
    }

    pub fn unlink(&self) -> OsResult<()> {
        let flag = if self.is_dir() { libc::AT_REMOVEDIR } else { 0 };
        self.dir.unlink_at(self.name(), flag)
    }

    pub fn read_link(&self, buf: &mut dyn Utf8CStrBuf) -> OsResult<()> {
        self.dir.read_link_at(self.name(), buf)
    }

    pub fn open_as_dir(&self) -> OsResult<Directory> {
        if !self.is_dir() {
            return Err(OsError::with_os_error(
                libc::ENOTDIR,
                "fdopendir",
                Some(self.name()),
                None,
            ));
        }
        self.dir.open_as_dir_at(self.name())
    }

    pub fn open_as_file(&self, flags: i32) -> OsResult<File> {
        if self.is_dir() {
            return Err(OsError::with_os_error(
                libc::EISDIR,
                "open_as_file",
                Some(self.name()),
                None,
            ));
        }
        self.dir.open_as_file_at(self.name(), flags, 0)
    }
}

impl Deref for DirEntry<'_> {
    type Target = dirent;

    fn deref(&self) -> &dirent {
        unsafe { self.entry.as_ref() }
    }
}

#[repr(transparent)]
pub struct Directory {
    inner: NonNull<libc::DIR>,
}

#[repr(transparent)]
pub struct BorrowedDirectory<'a> {
    inner: NonNull<libc::DIR>,
    phantom: PhantomData<&'a Directory>,
}

impl Deref for BorrowedDirectory<'_> {
    type Target = Directory;

    fn deref(&self) -> &Directory {
        // SAFETY: layout of NonNull<libc::DIR> is the same as Directory
        // SAFETY: the lifetime of the raw pointer is tracked in the PhantomData
        unsafe { mem::transmute(&self.inner) }
    }
}

impl DerefMut for BorrowedDirectory<'_> {
    fn deref_mut(&mut self) -> &mut Directory {
        // SAFETY: layout of NonNull<libc::DIR> is the same as Directory
        // SAFETY: the lifetime of the raw pointer is tracked in the PhantomData
        unsafe { mem::transmute(&mut self.inner) }
    }
}

pub enum WalkResult {
    Continue,
    Abort,
    Skip,
}

impl Directory {
    fn borrow(&self) -> BorrowedDirectory {
        BorrowedDirectory {
            inner: self.inner,
            phantom: PhantomData,
        }
    }

    fn openat<'a>(&self, name: &'a Utf8CStr, flags: i32, mode: u32) -> OsResult<'a, OwnedFd> {
        unsafe {
            libc::openat(self.as_raw_fd(), name.as_ptr(), flags | O_CLOEXEC, mode)
                .as_os_result("openat", Some(name), None)
                .map(|fd| OwnedFd::from_raw_fd(fd))
        }
    }

    fn path_at(&self, name: &Utf8CStr, buf: &mut dyn Utf8CStrBuf) -> OsResult<'static, ()> {
        self.resolve_path(buf)?;
        buf.append_path(name);
        Ok(())
    }
}

impl Directory {
    pub fn open(path: &Utf8CStr) -> OsResult<Directory> {
        let dirp = unsafe { libc::opendir(path.as_ptr()) };
        let dirp = dirp.as_os_result("opendir", Some(path), None)?;
        Ok(Directory { inner: dirp })
    }

    pub fn read(&mut self) -> OsResult<'static, Option<DirEntry>> {
        *errno() = 0;
        let e = unsafe { libc::readdir(self.inner.as_ptr()) };
        if e.is_null() {
            return if *errno() != 0 {
                Err(OsError::last_os_error("readdir", None, None))
            } else {
                Ok(None)
            };
        }
        // Skip non UTF-8 entries, ".", and ".."
        unsafe {
            let entry = &*e;

            let Ok(name) = Utf8CStr::from_ptr(entry.d_name.as_ptr()) else {
                return self.read();
            };

            if name == "." || name == ".." {
                self.read()
            } else {
                let e = DirEntry {
                    dir: self.borrow(),
                    entry: NonNull::from(entry),
                    d_name_len: name.as_bytes_with_nul().len(),
                };
                Ok(Some(e))
            }
        }
    }

    pub fn rewind(&mut self) {
        unsafe { libc::rewinddir(self.inner.as_ptr()) };
    }

    pub fn open_as_dir_at<'a>(&self, name: &'a Utf8CStr) -> OsResult<'a, Directory> {
        let fd = self.openat(name, O_RDONLY, 0)?;
        Directory::try_from(fd).map_err(|e| e.set_args(Some(name), None))
    }

    pub fn open_as_file_at<'a>(
        &self,
        name: &'a Utf8CStr,
        flags: i32,
        mode: u32,
    ) -> OsResult<'a, File> {
        let fd = self.openat(name, flags, mode)?;
        Ok(File::from(fd))
    }

    pub fn read_link_at<'a>(
        &self,
        name: &'a Utf8CStr,
        buf: &mut dyn Utf8CStrBuf,
    ) -> OsResult<'a, ()> {
        buf.clear();
        unsafe {
            let r = readlinkat(
                self.as_raw_fd(),
                name.as_ptr(),
                buf.as_mut_ptr().cast(),
                buf.capacity(),
            )
            .as_os_result("readlinkat", Some(name), None)? as usize;
            buf.set_len(r);
        }
        Ok(())
    }

    pub fn mkdir_at<'a>(&self, name: &'a Utf8CStr, mode: mode_t) -> OsResult<'a, ()> {
        unsafe {
            if libc::mkdirat(self.as_raw_fd(), name.as_ptr(), mode as mode_t) < 0
                && *errno() != EEXIST
            {
                return Err(OsError::last_os_error("mkdirat", Some(name), None));
            }
        }
        Ok(())
    }

    // ln -s target self/name
    pub fn create_symlink_at<'a>(
        &self,
        name: &'a Utf8CStr,
        target: &'a Utf8CStr,
    ) -> OsResult<'a, ()> {
        unsafe {
            libc::symlinkat(target.as_ptr(), self.as_raw_fd(), name.as_ptr()).check_os_err(
                "symlinkat",
                Some(target),
                Some(name),
            )
        }
    }

    pub fn unlink_at<'a>(&self, name: &'a Utf8CStr, flag: i32) -> OsResult<'a, ()> {
        unsafe {
            libc::unlinkat(self.as_raw_fd(), name.as_ptr(), flag).check_os_err(
                "unlinkat",
                Some(name),
                None,
            )?;
        }
        Ok(())
    }

    pub fn contains_path(&self, path: &Utf8CStr) -> bool {
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

    pub fn resolve_path(&self, buf: &mut dyn Utf8CStrBuf) -> OsResult<'static, ()> {
        fd_path(self.as_raw_fd(), buf)
    }

    pub fn post_order_walk<F: FnMut(&DirEntry) -> OsResultStatic<WalkResult>>(
        &mut self,
        mut f: F,
    ) -> OsResultStatic<WalkResult> {
        self.post_order_walk_impl(&mut f)
    }

    pub fn pre_order_walk<F: FnMut(&DirEntry) -> OsResultStatic<WalkResult>>(
        &mut self,
        mut f: F,
    ) -> OsResultStatic<WalkResult> {
        self.pre_order_walk_impl(&mut f)
    }

    pub fn remove_all(&mut self) -> OsResultStatic<()> {
        self.post_order_walk(|e| {
            e.unlink()?;
            Ok(WalkResult::Continue)
        })?;
        Ok(())
    }

    pub fn copy_into(&mut self, dir: &Directory) -> OsResultStatic<()> {
        let mut buf = cstr::buf::default();
        self.copy_into_impl(dir, &mut buf)
    }

    pub fn move_into(&mut self, dir: &Directory) -> OsResultStatic<()> {
        let dir_fd = self.as_raw_fd();
        while let Some(ref e) = self.read()? {
            if e.is_dir() && dir.contains_path(e.name()) {
                // Destination folder exists, needs recursive move
                let mut src = e.open_as_dir()?;
                let dest = dir.open_as_dir_at(e.name())?;
                src.move_into(&dest)?;
                return Ok(e.unlink()?);
            }

            unsafe {
                libc::renameat(
                    dir_fd,
                    e.d_name.as_ptr(),
                    dir.as_raw_fd(),
                    e.d_name.as_ptr(),
                )
                .check_os_err("renameat", Some(e.name()), None)?;
            }
        }
        Ok(())
    }

    pub fn link_into(&mut self, dir: &Directory) -> OsResultStatic<()> {
        let mut buf = cstr::buf::default();
        self.link_into_impl(dir, &mut buf)
    }
}

impl Directory {
    fn post_order_walk_impl<F: FnMut(&DirEntry) -> OsResultStatic<WalkResult>>(
        &mut self,
        f: &mut F,
    ) -> OsResultStatic<WalkResult> {
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

    fn pre_order_walk_impl<F: FnMut(&DirEntry) -> OsResultStatic<WalkResult>>(
        &mut self,
        f: &mut F,
    ) -> OsResultStatic<WalkResult> {
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

    fn copy_into_impl(
        &mut self,
        dest_dir: &Directory,
        buf: &mut dyn Utf8CStrBuf,
    ) -> OsResultStatic<()> {
        while let Some(ref e) = self.read()? {
            e.resolve_path(buf)?;
            let attr = buf.get_attr()?;
            if e.is_dir() {
                dest_dir.mkdir_at(e.name(), 0o777)?;
                let mut src = e.open_as_dir()?;
                let dest = dest_dir.open_as_dir_at(e.name())?;
                src.copy_into_impl(&dest, buf)?;
                fd_set_attr(dest.as_raw_fd(), &attr)?;
            } else if e.is_file() {
                let mut src = e.open_as_file(O_RDONLY)?;
                let mut dest =
                    dest_dir.open_as_file_at(e.name(), O_WRONLY | O_CREAT | O_TRUNC, 0o777)?;
                std::io::copy(&mut src, &mut dest)?;
                fd_set_attr(dest.as_raw_fd(), &attr)?;
            } else if e.is_symlink() {
                e.read_link(buf)?;
                dest_dir.create_symlink_at(e.name(), buf)?;
                dest_dir.path_at(e.name(), buf)?;
                buf.set_attr(&attr)?;
            }
        }
        Ok(())
    }

    fn link_into_impl(
        &mut self,
        dest_dir: &Directory,
        buf: &mut dyn Utf8CStrBuf,
    ) -> OsResultStatic<()> {
        let dir_fd = self.as_raw_fd();
        while let Some(ref e) = self.read()? {
            if e.is_dir() {
                dest_dir.mkdir_at(e.name(), 0o777)?;
                e.resolve_path(buf)?;
                let attr = buf.get_attr()?;
                let mut src = e.open_as_dir()?;
                let dest = dest_dir.open_as_dir_at(e.name())?;
                src.link_into_impl(&dest, buf)?;
                fd_set_attr(dest.as_raw_fd(), &attr)?;
            } else {
                unsafe {
                    libc::linkat(
                        dir_fd,
                        e.d_name.as_ptr(),
                        dest_dir.as_raw_fd(),
                        e.d_name.as_ptr(),
                        0,
                    )
                    .check_os_err("linkat", Some(e.name()), None)?;
                }
            }
        }
        Ok(())
    }
}

impl TryFrom<OwnedFd> for Directory {
    type Error = OsError<'static>;

    fn try_from(fd: OwnedFd) -> OsResult<'static, Self> {
        let dirp = unsafe { libc::fdopendir(fd.into_raw_fd()) };
        let dirp = dirp.as_os_result("fdopendir", None, None)?;
        Ok(Directory { inner: dirp })
    }
}

impl AsRawFd for Directory {
    fn as_raw_fd(&self) -> RawFd {
        unsafe { libc::dirfd(self.inner.as_ptr()) }
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
            libc::closedir(self.inner.as_ptr());
        }
    }
}
