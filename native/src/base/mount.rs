use crate::{LibcReturn, OsResult, Utf8CStr};
use libc::c_ulong;
use std::ptr;

impl Utf8CStr {
    pub fn bind_mount_to<'a>(&'a self, path: &'a Utf8CStr, rec: bool) -> OsResult<'a, ()> {
        let flag = if rec { libc::MS_REC } else { 0 };
        unsafe {
            libc::mount(
                self.as_ptr(),
                path.as_ptr(),
                ptr::null(),
                libc::MS_BIND | flag,
                ptr::null(),
            )
            .check_os_err("bind_mount", Some(self), Some(path))
        }
    }

    pub fn remount_mount_point_flags(&self, flags: c_ulong) -> OsResult<()> {
        unsafe {
            libc::mount(
                ptr::null(),
                self.as_ptr(),
                ptr::null(),
                libc::MS_BIND | libc::MS_REMOUNT | flags,
                ptr::null(),
            )
            .check_os_err("remount", Some(self), None)
        }
    }

    pub fn remount_mount_flags(&self, flags: c_ulong) -> OsResult<()> {
        unsafe {
            libc::mount(
                ptr::null(),
                self.as_ptr(),
                ptr::null(),
                libc::MS_REMOUNT | flags,
                ptr::null(),
            )
            .check_os_err("remount", Some(self), None)
        }
    }

    pub fn remount_with_data(&self, data: &Utf8CStr) -> OsResult<()> {
        unsafe {
            libc::mount(
                ptr::null(),
                self.as_ptr(),
                ptr::null(),
                libc::MS_REMOUNT,
                data.as_ptr().cast(),
            )
            .check_os_err("remount", Some(self), None)
        }
    }

    pub fn move_mount_to<'a>(&'a self, path: &'a Utf8CStr) -> OsResult<'a, ()> {
        unsafe {
            libc::mount(
                self.as_ptr(),
                path.as_ptr(),
                ptr::null(),
                libc::MS_MOVE,
                ptr::null(),
            )
            .check_os_err("move_mount", Some(self), Some(path))
        }
    }

    pub fn unmount(&self) -> OsResult<()> {
        unsafe {
            libc::umount2(self.as_ptr(), libc::MNT_DETACH).check_os_err("unmount", Some(self), None)
        }
    }

    pub fn set_mount_private(&self, recursive: bool) -> OsResult<()> {
        let flag = if recursive { libc::MS_REC } else { 0 };
        unsafe {
            libc::mount(
                ptr::null(),
                self.as_ptr(),
                ptr::null(),
                libc::MS_PRIVATE | flag,
                ptr::null(),
            )
            .check_os_err("set_mount_private", Some(self), None)
        }
    }
}
