use base::{
    cstr, debug,
    libc::{chdir, chroot, makedev, mount, MS_MOVE},
    raw_cstr, BufReadExt, Directory, LibcReturn, LoggedResult, StringExt, Utf8CStr,
};
use cxx::CxxString;
use std::{
    collections::BTreeSet,
    fs::File,
    io::BufReader,
    ops::Bound::{Excluded, Unbounded},
    pin::Pin,
    ptr::null as nullptr,
};

#[allow(dead_code)]
struct MountInfo {
    id: u32,
    parent: u32,
    device: u64,
    root: String,
    target: String,
    vfs_option: String,
    shared: u32,
    master: u32,
    propagation_from: u32,
    unbindable: bool,
    fs_type: String,
    source: String,
    fs_option: String,
}

fn parse_mount_info_line(line: &str) -> Option<MountInfo> {
    let mut iter = line.split_whitespace();
    let id = iter.next()?.parse().ok()?;
    let parent = iter.next()?.parse().ok()?;
    let (maj, min) = iter.next()?.split_once(":")?;
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

fn parse_mount_info(pid: &str) -> Vec<MountInfo> {
    let mut res = vec![];

    if let Ok(file) = File::open(format!("/proc/{}/mountinfo", pid)) {
        BufReader::new(file).foreach_lines(|line| {
            parse_mount_info_line(line)
                .map(|info| res.push(info))
                .is_some()
        });
    }
    res
}

pub fn switch_root(path: &Utf8CStr) {
    fn inner(path: &Utf8CStr) -> LoggedResult<()> {
        debug!("Switch root to {}", path);
        let mut mounts = BTreeSet::new();
        let mut rootfs = Directory::open(cstr!("/"))?;
        for info in parse_mount_info("self") {
            if info.target == "/" || info.target.as_str() == path.as_str() {
                continue;
            }
            if let Some(last_mount) = mounts
                .range::<String, _>((Unbounded, Excluded(&info.target)))
                .last()
            {
                if info.target.starts_with(&format!("{}/", *last_mount)) {
                    continue;
                }
            }
            let mut new_path = format!("{}/{}", path.as_str(), &info.target);
            std::fs::create_dir(&new_path).ok();

            unsafe {
                let mut target = info.target.clone();
                mount(
                    target.nul_terminate().as_ptr().cast(),
                    new_path.nul_terminate().as_ptr().cast(),
                    nullptr(),
                    MS_MOVE,
                    nullptr(),
                )
                .as_os_err()?;
            }

            mounts.insert(info.target);
        }
        unsafe {
            chdir(path.as_ptr()).as_os_err()?;
            mount(path.as_ptr(), raw_cstr!("/"), nullptr(), MS_MOVE, nullptr()).as_os_err()?;
            chroot(raw_cstr!("."));
        }

        debug!("Cleaning rootfs");
        rootfs.remove_all()?;
        Ok(())
    }
    inner(path).ok();
}

pub fn is_device_mounted(dev: u64, target: Pin<&mut CxxString>) -> bool {
    for mount in parse_mount_info("self") {
        if mount.root == "/" && mount.device == dev {
            target.push_str(&mount.target);
            return true;
        }
    }
    false
}
