use std::collections::BTreeSet;
use std::ops::Bound::{Excluded, Unbounded};
use std::path::{Path, PathBuf};
use std::{fs, ptr};

use procfs::process::Process;

use base::{
    cstr, debug, libc, raw_cstr, Directory, LibcReturn, LoggedError, LoggedResult, StringExt,
    Utf8CStr,
};

pub fn switch_root(path: &Utf8CStr) {
    fn inner(path: &Utf8CStr) -> LoggedResult<()> {
        debug!("Switching root to {}", path);
        let mut rootfs = Directory::open(cstr!("/"))?;

        let procfs = Process::myself()?;
        let mut mounts: BTreeSet<PathBuf> = BTreeSet::new();
        for info in procfs.mountinfo()?.0.into_iter() {
            let mut target = info.mount_point;
            if target == Path::new("/") || target == Path::new(path) {
                continue;
            }
            let iter = mounts.range::<Path, _>((Unbounded, Excluded(target.as_path())));
            if let Some(last_mount) = iter.last() {
                if Path::new(path).starts_with(last_mount) {
                    continue;
                }
            }
            let mut new_path = PathBuf::from(path);
            new_path.push(target.strip_prefix("/").unwrap());
            fs::create_dir(&new_path).ok(); /* Error is OK */
            unsafe {
                libc::mount(
                    target.nul_terminate().as_ptr().cast(),
                    new_path.nul_terminate().as_ptr().cast(),
                    ptr::null(),
                    libc::MS_MOVE,
                    ptr::null(),
                )
                .as_os_err()?;
            }

            // Record all moved paths
            mounts.insert(target);
        }

        unsafe {
            libc::chdir(path.as_ptr()).as_os_err()?;
            libc::mount(
                path.as_ptr(),
                raw_cstr!("/"),
                ptr::null(),
                libc::MS_MOVE,
                ptr::null(),
            )
            .as_os_err()?;
            libc::chroot(raw_cstr!(".")).as_os_err()?;
        }

        debug!("Cleaning rootfs");
        rootfs.remove_all()?;
        Ok(())
    }
    inner(path).ok();
}

pub fn is_device_mounted(dev: u64, mnt_point: &mut Vec<u8>) -> bool {
    fn inner(dev: u64, mount_point: &mut Vec<u8>) -> LoggedResult<()> {
        let procfs = Process::myself()?;
        for mut info in procfs.mountinfo()?.0 {
            if info.root != "/" {
                continue;
            }
            let mut iter = info.majmin.split(':').map(|s| s.parse::<u32>());
            let maj = match iter.next() {
                Some(Ok(s)) => s,
                _ => continue,
            };
            let min = match iter.next() {
                Some(Ok(s)) => s,
                _ => continue,
            };
            if dev == libc::makedev(maj, min).into() {
                *mount_point = info.mount_point.nul_terminate().to_vec();
                return Ok(());
            }
        }
        Err(LoggedError::default())
    }
    inner(dev, mnt_point).is_ok()
}
