use crate::cxx_extern::readlinkat_for_cxx;
use crate::{
    cstr, cstr_buf, errno, fd_path, fd_set_attr, FileAttr, FsPath, LibcReturn, Utf8CStr,
    Utf8CStrBuf,
};
use libc::{dirent, O_CLOEXEC, O_CREAT, O_RDONLY, O_TRUNC, O_WRONLY};
use std::ffi::CStr;
use std::fs::File;
use std::ops::Deref;
use std::os::fd::{AsFd, AsRawFd, BorrowedFd, FromRawFd, IntoRawFd, OwnedFd, RawFd};
use std::{io, mem, slice};

pub struct DirEntry<'a> {
    dir: &'a Directory,
    entry: &'a dirent,
    d_name_len: usize,
}

impl DirEntry<'_> {
    pub fn name(&self) -> &CStr {
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
        buf.push_lossy(self.name().to_bytes());
        Ok(())
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
        unsafe { self.dir.open_raw_fd(self.name(), flags, 0) }
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
        let mut path = cstr_buf::default();
        self.path(&mut path)?;
        FsPath::from(&path).get_attr()
    }

    pub fn set_attr(&self, attr: &FileAttr) -> io::Result<()> {
        let mut path = cstr_buf::default();
        self.path(&mut path)?;
        FsPath::from(&path).set_attr(attr)
    }

    pub fn get_secontext(&self, con: &mut dyn Utf8CStrBuf) -> io::Result<()> {
        let mut path = cstr_buf::default();
        self.path(&mut path)?;
        FsPath::from(&path).get_secontext(con)
    }

    pub fn set_secontext(&self, con: &Utf8CStr) -> io::Result<()> {
        let mut path = cstr_buf::default();
        self.path(&mut path)?;
        FsPath::from(&path).set_secontext(con)
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
            if d_name == cstr!(".") || d_name == cstr!("..") {
                self.read()
            } else {
                let e = DirEntry {
                    dir: self,
                    entry,
                    d_name_len: d_name.to_bytes_with_nul().len(),
                };
                Ok(Some(e))
            }
        }
    }

    pub fn rewind(&mut self) {
        unsafe { libc::rewinddir(self.dirp) }
    }

    unsafe fn open_raw_fd(&self, name: &CStr, flags: i32, mode: i32) -> io::Result<RawFd> {
        unsafe {
            libc::openat(self.as_raw_fd(), name.as_ptr(), flags | O_CLOEXEC, mode).check_os_err()
        }
    }

    pub fn open_fd(&self, name: &Utf8CStr, flags: i32, mode: i32) -> io::Result<OwnedFd> {
        unsafe {
            self.open_raw_fd(name.as_cstr(), flags, mode)
                .map(|fd| OwnedFd::from_raw_fd(fd))
        }
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
                    File::from_raw_fd(dir.open_raw_fd(
                        e.name(),
                        O_WRONLY | O_CREAT | O_TRUNC,
                        0o777,
                    )?)
                };
                std::io::copy(&mut src, &mut dest)?;
                fd_set_attr(dest.as_raw_fd(), &attr)?;
            } else if e.is_symlink() {
                let mut path = cstr_buf::default();
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
            if e.is_dir() && dir.contains_path(e.name()) {
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
