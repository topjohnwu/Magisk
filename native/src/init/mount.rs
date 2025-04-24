use crate::ffi::MagiskInit;
use base::libc::{TMPFS_MAGIC, statfs};
use base::{
    Directory, FsPathBuilder, LibcReturn, LoggedResult, ResultExt, Utf8CStr, cstr, debug, libc,
    libc::{chdir, chroot, execve, exit, mount},
    parse_mount_info, raw_cstr,
};
use cxx::CxxString;
use std::ffi::c_long;
use std::{
    collections::BTreeSet,
    ops::Bound::{Excluded, Unbounded},
    pin::Pin,
};

unsafe extern "C" {
    static environ: *const *mut libc::c_char;
}

pub(crate) fn switch_root(path: &Utf8CStr) {
    let res: LoggedResult<()> = try {
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

            let mut target = info.target.clone();
            let target = Utf8CStr::from_string(&mut target);
            let new_path = cstr::buf::default()
                .join_path(path)
                .join_path(info.target.trim_start_matches('/'));
            new_path.mkdirs(0o755).ok();
            target.move_mount_to(&new_path)?;
            mounts.insert(info.target);
        }
        unsafe {
            chdir(path.as_ptr()).check_io_err()?;
            path.move_mount_to(cstr!("/"))?;
            chroot(raw_cstr!("."));
        }

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

const RAMFS_MAGIC: u64 = 0x858458f6;

pub(crate) fn is_rootfs() -> bool {
    unsafe {
        let mut sfs: statfs = std::mem::zeroed();
        statfs(raw_cstr!("/"), &mut sfs);
        sfs.f_type as u64 == RAMFS_MAGIC || sfs.f_type as c_long == TMPFS_MAGIC
    }
}

impl MagiskInit {
    pub(crate) fn prepare_data(&self) {
        debug!("Setup data tmp");
        cstr!("/data").mkdir(0o755).log_ok();
        unsafe {
            mount(
                raw_cstr!("magisk"),
                raw_cstr!("/data"),
                raw_cstr!("tmpfs"),
                0,
                raw_cstr!("mode=755").cast(),
            )
        }
        .check_io_err()
        .log_ok();

        cstr!("/init").copy_to(cstr!("/data/magiskinit")).log_ok();
        cstr!("/.backup").copy_to(cstr!("/data/.backup")).log_ok();
        cstr!("/overlay.d")
            .copy_to(cstr!("/data/overlay.d"))
            .log_ok();
    }

    pub(crate) fn exec_init(&mut self) {
        for path in self.mount_list.iter_mut().rev() {
            let path = Utf8CStr::from_string(path);
            if path.unmount().log().is_ok() {
                debug!("Unmount [{}]", path);
            }
        }
        unsafe {
            execve(raw_cstr!("/init"), self.argv.cast(), environ.cast())
                .check_io_err()
                .log_ok();
            exit(1);
        }
    }
}
