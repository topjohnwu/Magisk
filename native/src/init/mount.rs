use std::{
    collections::BTreeSet,
    ops::Bound::{Excluded, Unbounded},
    pin::Pin,
    ptr::null as nullptr,
};

use cxx::CxxString;

use base::{
    cstr, debug,
    libc::{chdir, chroot, mount, MS_MOVE},
    parse_mount_info, raw_cstr, Directory, LibcReturn, LoggedResult, StringExt, Utf8CStr,
};

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
