use crate::{LibcReturn, OsResult, Utf8CStr};
use nix::mount::{MntFlags, MsFlags, mount, umount2};

impl Utf8CStr {
    pub fn bind_mount_to<'a>(&'a self, path: &'a Utf8CStr, rec: bool) -> OsResult<'a, ()> {
        let flag = if rec {
            MsFlags::MS_REC
        } else {
            MsFlags::empty()
        };
        mount(
            Some(self),
            path,
            None::<&Utf8CStr>,
            flag | MsFlags::MS_BIND,
            None::<&Utf8CStr>,
        )
        .check_os_err("bind_mount", Some(self), Some(path))
    }

    pub fn remount_mount_point_flags(&self, flags: MsFlags) -> OsResult<'_, ()> {
        mount(
            None::<&Utf8CStr>,
            self,
            None::<&Utf8CStr>,
            MsFlags::MS_BIND | MsFlags::MS_REMOUNT | flags,
            None::<&Utf8CStr>,
        )
        .check_os_err("remount", Some(self), None)
    }

    pub fn remount_mount_flags(&self, flags: MsFlags) -> OsResult<'_, ()> {
        mount(
            None::<&Utf8CStr>,
            self,
            None::<&Utf8CStr>,
            MsFlags::MS_REMOUNT | flags,
            None::<&Utf8CStr>,
        )
        .check_os_err("remount", Some(self), None)
    }

    pub fn remount_with_data(&self, data: &Utf8CStr) -> OsResult<'_, ()> {
        mount(
            None::<&Utf8CStr>,
            self,
            None::<&Utf8CStr>,
            MsFlags::MS_REMOUNT,
            Some(data),
        )
        .check_os_err("remount", Some(self), None)
    }

    pub fn move_mount_to<'a>(&'a self, path: &'a Utf8CStr) -> OsResult<'a, ()> {
        mount(
            Some(self),
            path,
            None::<&Utf8CStr>,
            MsFlags::MS_MOVE,
            None::<&Utf8CStr>,
        )
        .check_os_err("move_mount", Some(self), Some(path))
    }

    pub fn unmount(&self) -> OsResult<'_, ()> {
        umount2(self, MntFlags::MNT_DETACH).check_os_err("unmount", Some(self), None)
    }

    pub fn set_mount_private(&self, rec: bool) -> OsResult<'_, ()> {
        let flag = if rec {
            MsFlags::MS_REC
        } else {
            MsFlags::empty()
        };
        mount(
            None::<&Utf8CStr>,
            self,
            None::<&Utf8CStr>,
            flag | MsFlags::MS_PRIVATE,
            None::<&Utf8CStr>,
        )
        .check_os_err("set_mount_private", Some(self), None)
    }
}
