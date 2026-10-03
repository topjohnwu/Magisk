use crate::{BufReadExt, LibcReturn, OsResult, Utf8CStr};
use libc::makedev;
use nix::fcntl::OFlag;
use nix::mount::{MntFlags, MsFlags, mount, umount2};
use std::io::BufReader;

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
