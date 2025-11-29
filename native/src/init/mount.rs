use crate::ffi::MagiskInit;
use base::WalkResult::{Continue, Skip};
use base::nix::mount::{MntFlags, mount, umount2};
use base::{
    Directory, FsPathBuilder, LibcReturn, LoggedResult, ResultExt, Utf8CStr, cstr, debug, libc,
    nix, parse_mount_info, raw_cstr,
};
use cxx::CxxString;
use nix::mount::MsFlags;
use nix::sys::statfs::{FsType, TMPFS_MAGIC, statfs};
use nix::unistd::{chdir, chroot};
use num_traits::AsPrimitive;
use std::collections::BTreeSet;
use std::ops::Bound::{Excluded, Unbounded};
use std::pin::Pin;

unsafe extern "C" {
    static environ: *const *mut libc::c_char;
}

pub(crate) fn switch_root(path: &Utf8CStr) {
    let res: LoggedResult<()> = try {
        debug!("Switch root to {}", path);
        let mut mounts = BTreeSet::new();
        let rootfs = Directory::open(cstr!("/"))?;
        for info in parse_mount_info("self") {
            if info.target == "/" || info.target.as_str() == path.as_str() {
                continue;
            }
            if let Some(last_mount) = mounts
                .range::<String, _>((Unbounded, Excluded(&info.target)))
                .last()
                && info.target.starts_with(&format!("{}/", *last_mount))
            {
                continue;
            }

            let mut target = info.target.clone();
            let target = Utf8CStr::from_string(&mut target);
            let new_path = cstr::buf::default()
                .join_path(path)
                .join_path(info.target.trim_start_matches('/'));
            new_path.mkdirs(0o755).ok();
            target.move_mount_to(&new_path)?;
            mounts.insert(info.target);
        }
        chdir(path)?;
        path.move_mount_to(cstr!("/"))?;
        chroot(cstr!("."))?;

        debug!("Cleaning rootfs");
        rootfs.remove_all()?;
    };
    res.ok();
}

pub(crate) fn is_device_mounted(dev: u64, target: Pin<&mut CxxString>) -> bool {
    for mount in parse_mount_info("self") {
        if mount.root == "/" && mount.device == dev {
            target.push_str(&mount.target);
            return true;
        }
    }
    false
}

pub(crate) fn occupy(path: &Utf8CStr) {
    Directory::open(path)
        .map(|mut dir| {
            dir.pre_order_walk(|entry| {
                let mut path = cstr::buf::default();
                entry.resolve_path(&mut path)?;
                let path = path.as_utf8_cstr();
                mount(
                    Some(path),
                    path,
                    None::<&Utf8CStr>,
                    MsFlags::MS_BIND | MsFlags::MS_RDONLY,
                    None::<&Utf8CStr>,
                )
                .check_os_err("occupy", Some(path), None)?;
                Ok(Continue)
            })
            .log_ok();
        })
        .log_ok();
}

pub(crate) fn unoccupy(path: &Utf8CStr) -> bool {
    let mut ok = false;
    Directory::open(path)
        .map(|mut dir| {
            ok = dir
                .pre_order_walk(|entry| {
                    let mut path = cstr::buf::default();
                    entry.resolve_path(&mut path)?;
                    let path = path.as_utf8_cstr();
                    umount2(path, MntFlags::MNT_DETACH).check_os_err(
                        "unoccupy",
                        Some(path),
                        None,
                    )?;
                    if entry.is_dir() {
                        Ok(Skip)
                    } else {
                        Ok(Continue)
                    }
                })
                .is_ok();
        })
        .log_ok();
    ok
}

const RAMFS_MAGIC: u32 = 0x858458f6;

pub(crate) fn is_rootfs() -> bool {
    if let Ok(s) = statfs(cstr!("/")) {
        s.filesystem_type() == FsType(RAMFS_MAGIC.as_()) || s.filesystem_type() == TMPFS_MAGIC
    } else {
        false
    }
}

impl MagiskInit {
    pub(crate) fn prepare_data(&self, use_rootfs: bool) -> bool {
        debug!("Setup data tmp");
        cstr!("/data").mkdir(0o755).log_ok();

        let mut rootfs_magisktmp = false;

        if use_rootfs {
            cstr!("/magisk").mkdir(0o755).log_ok();
            rootfs_magisktmp = cstr!("/magisk")
                .bind_mount_to(cstr!("/data"), false)
                .is_ok();
        }

        if rootfs_magisktmp {
            cstr!("/init")
                .rename_to(cstr!("/magisk/magiskinit"))
                .log_ok();
            cstr!("/.backup").copy_to(cstr!("/magisk/.backup")).ok();
            cstr!("/overlay.d")
                .rename_to(cstr!("/magisk/overlay.d"))
                .ok();
        } else {
            nix::mount::mount(
                Some(cstr!("magisk")),
                cstr!("/data"),
                Some(cstr!("tmpfs")),
                MsFlags::empty(),
                Some(cstr!("mode=755")),
            )
            .check_os_err("mount", Some("/data"), Some("tmpfs"))
            .log_ok();

            cstr!("/init").copy_to(cstr!("/data/magiskinit")).ok();
            cstr!("/.backup").copy_to(cstr!("/data/.backup")).ok();
            cstr!("/overlay.d").copy_to(cstr!("/data/overlay.d")).ok();
        }

        rootfs_magisktmp
    }

    pub(crate) fn exec_init(&mut self) {
        for path in self.mount_list.iter_mut().rev() {
            let path = Utf8CStr::from_string(path);
            if path.unmount().log().is_ok() {
                debug!("Unmount [{}]", path);
            }
        }
        unsafe {
            libc::execve(raw_cstr!("/init"), self.argv.cast(), environ.cast())
                .check_err()
                .log_ok();
        }
        std::process::exit(1);
    }
}
