use crate::cxx_extern::readlinkat;
use crate::{
    FileAttr, FsPath, FsPathBuilder, LibcReturn, OsError, OsResult, OsResultStatic, Utf8CStr,
    Utf8CStrBuf, cstr, errno, fd_path, fd_set_attr,
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

    pub fn path(&self, buf: &mut dyn Utf8CStrBuf) -> OsResult<'static, ()> {
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
        unsafe {
            libc::unlinkat(self.dir.as_raw_fd(), self.d_name.as_ptr(), flag).check_os_err(
                "unlinkat",
                Some(self.name()),
                None,
            )?;
        }
        Ok(())
    }

    pub fn read_link(&self, buf: &mut dyn Utf8CStrBuf) -> OsResult<()> {
        buf.clear();
        unsafe {
            let r = readlinkat(
                self.dir.as_raw_fd(),
                self.d_name.as_ptr(),
                buf.as_mut_ptr().cast(),
                buf.capacity(),
            )
            .as_os_result("readlinkat", Some(self.name()), None)? as usize;
            buf.set_len(r);
        }
        Ok(())
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
        self.dir.openat_as_dir(self.name())
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
        self.dir.openat_as_file(self.name(), flags, 0)
    }

    pub fn get_attr(&self) -> OsResult<FileAttr> {
        self.dir.get_attr_at(self.name())
    }

    pub fn set_attr(&self, attr: &FileAttr) -> OsResult<()> {
        self.dir.set_attr_at(self.name(), attr)
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
        self.path(buf)?;
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

    pub fn openat_as_dir<'a>(&self, name: &'a Utf8CStr) -> OsResult<'a, Directory> {
        let fd = self.openat(name, O_RDONLY, 0)?;
        Directory::try_from(fd).map_err(|e| e.set_args(Some(name), None))
    }

    pub fn openat_as_file<'a>(
        &self,
        name: &'a Utf8CStr,
        flags: i32,
        mode: u32,
    ) -> OsResult<'a, File> {
        let fd = self.openat(name, flags, mode)?;
        Ok(File::from(fd))
    }

    pub fn get_attr_at<'a>(&self, name: &'a Utf8CStr) -> OsResult<'a, FileAttr> {
        let mut path = cstr::buf::default();
        self.path_at(name, &mut path)?;
        path.get_attr().map_err(|e| e.set_args(Some(name), None))
    }

    pub fn set_attr_at<'a>(&self, name: &'a Utf8CStr, attr: &FileAttr) -> OsResult<'a, ()> {
        let mut path = cstr::buf::default();
        self.path_at(name, &mut path)?;
        path.set_attr(attr)
            .map_err(|e| e.set_args(Some(name), None))
    }

    pub fn get_secontext_at<'a>(
        &self,
        name: &'a Utf8CStr,
        con: &mut dyn Utf8CStrBuf,
    ) -> OsResult<'a, ()> {
        let mut path = cstr::buf::default();
        self.path_at(name, &mut path)?;
        path.get_secontext(con)
            .map_err(|e| e.set_args(Some(name), None))
    }

    pub fn set_secontext_at<'a>(&self, name: &'a Utf8CStr, con: &'a Utf8CStr) -> OsResult<'a, ()> {
        let mut path = cstr::buf::default();
        self.path_at(name, &mut path)?;
        path.set_secontext(con)
            .map_err(|e| e.set_args(Some(name), Some(con)))
    }

    pub fn mkdirat<'a>(&self, name: &'a Utf8CStr, mode: mode_t) -> OsResult<'a, ()> {
        unsafe {
            if libc::mkdirat(self.as_raw_fd(), name.as_ptr(), mode as mode_t) < 0
                && *errno() != EEXIST
            {
                return Err(OsError::last_os_error("mkdirat", Some(name), None));
            }
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

    pub fn path(&self, buf: &mut dyn Utf8CStrBuf) -> OsResult<'static, ()> {
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
        while let Some(ref e) = self.read()? {
            let attr = e.get_attr()?;
            if e.is_dir() {
                dir.mkdirat(e.name(), 0o777)?;
                let mut src = e.open_as_dir()?;
                let dest = dir.openat_as_dir(e.name())?;
                src.copy_into(&dest)?;
                fd_set_attr(dest.as_raw_fd(), &attr)?;
            } else if e.is_file() {
                let mut src = e.open_as_file(O_RDONLY)?;
                let mut dest = dir.openat_as_file(e.name(), O_WRONLY | O_CREAT | O_TRUNC, 0o777)?;
                std::io::copy(&mut src, &mut dest)?;
                fd_set_attr(dest.as_raw_fd(), &attr)?;
            } else if e.is_symlink() {
                let mut target = cstr::buf::default();
                e.read_link(&mut target)?;
                unsafe {
                    libc::symlinkat(target.as_ptr(), dir.as_raw_fd(), e.d_name.as_ptr())
                        .check_os_err("symlinkat", Some(&target), Some(e.name()))?;
                }
                dir.set_attr_at(e.name(), &attr)?;
            }
        }
        Ok(())
    }

    pub fn move_into(&mut self, dir: &Directory) -> OsResultStatic<()> {
        let dir_fd = self.as_raw_fd();
        while let Some(ref e) = self.read()? {
            if e.is_dir() && dir.contains_path(e.name()) {
                // Destination folder exists, needs recursive move
                let mut src = e.open_as_dir()?;
                let dest = dir.openat_as_dir(e.name())?;
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
        let dir_fd = self.as_raw_fd();
        while let Some(ref e) = self.read()? {
            if e.is_dir() {
                dir.mkdirat(e.name(), 0o777)?;
                let attr = e.get_attr()?;
                let mut src = e.open_as_dir()?;
                let dest = dir.openat_as_dir(e.name())?;
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
                    .check_os_err("linkat", Some(e.name()), None)?;
                }
            }
        }
        Ok(())
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
