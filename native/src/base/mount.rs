use crate::{FsPath, LibcReturn, Utf8CStr};
use libc::c_ulong;
use std::ptr;

impl FsPath {
    pub fn bind_mount_to(&self, path: &FsPath) -> std::io::Result<()> {
        unsafe {
            libc::mount(
                self.as_ptr(),
                path.as_ptr(),
                ptr::null(),
                libc::MS_BIND,
                ptr::null(),
            )
            .as_os_err()
        }
    }

    pub fn remount_with_flags(&self, flags: c_ulong) -> std::io::Result<()> {
        unsafe {
            libc::mount(
                ptr::null(),
                self.as_ptr(),
                ptr::null(),
                libc::MS_BIND | libc::MS_REMOUNT | flags,
                ptr::null(),
            )
            .as_os_err()
        }
    }

    pub fn remount_with_data(&self, data: &Utf8CStr) -> std::io::Result<()> {
        unsafe {
            libc::mount(
                ptr::null(),
                self.as_ptr(),
                ptr::null(),
                libc::MS_REMOUNT,
                data.as_ptr().cast(),
            )
            .as_os_err()
        }
    }

    pub fn move_mount_to(&self, path: &FsPath) -> std::io::Result<()> {
        unsafe {
            libc::mount(
                self.as_ptr(),
                path.as_ptr(),
                ptr::null(),
                libc::MS_MOVE,
                ptr::null(),
            )
            .as_os_err()
        }
    }

    pub fn unmount(&self) -> std::io::Result<()> {
        unsafe { libc::umount2(self.as_ptr(), libc::MNT_DETACH).as_os_err() }
    }

    pub fn set_mount_private(&self, recursive: bool) -> std::io::Result<()> {
        let flag = if recursive { libc::MS_REC } else { 0 };
        unsafe {
            libc::mount(
                ptr::null(),
                self.as_ptr(),
                ptr::null(),
                libc::MS_PRIVATE | flag,
                ptr::null(),
            )
            .as_os_err()
        }
    }
}
