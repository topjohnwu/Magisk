use cxx::CxxString;
use std::{
    collections::BTreeSet,
    ops::Bound::{Excluded, Unbounded},
    pin::Pin,
    ptr::null as nullptr,
};

use crate::ffi::MagiskInit;
use base::{
    cstr, debug, libc,
    libc::{chdir, chroot, execve, exit, mount, umount2, MNT_DETACH, MS_MOVE},
    parse_mount_info, raw_cstr, Directory, FsPath, LibcReturn, LoggedResult, ResultExt, StringExt,
    Utf8CStr,
};

extern "C" {
    static environ: *const *mut libc::c_char;
}

pub fn switch_root(path: &Utf8CStr) {
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
    };
    res.ok();
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

impl MagiskInit {
    pub(crate) fn prepare_data(&self) {
        debug!("Setup data tmp");
        fn inner() -> LoggedResult<()> {
            FsPath::from(cstr!("/data")).mkdir(0o755)?;
            unsafe {
                mount(
                    raw_cstr!("magisk"),
                    raw_cstr!("/data"),
                    raw_cstr!("tmpfs"),
                    0,
                    raw_cstr!("mode=755").cast(),
                )
            }
            .as_os_err()?;

            FsPath::from(cstr!("/init")).copy_to(FsPath::from(cstr!("/data/magiskinit")))?;
            FsPath::from(cstr!("/.backup")).copy_to(FsPath::from(cstr!("/data/.backup")))?;
            FsPath::from(cstr!("/overlay.d")).copy_to(FsPath::from(cstr!("/data/overlay.d")))?;

            Ok(())
        }
        inner().ok();
    }

    pub(crate) fn exec_init(&self) {
        unsafe {
            for p in self.mount_list.iter().rev() {
                if umount2(p.as_ptr().cast(), MNT_DETACH)
                    .as_os_err()
                    .log()
                    .is_ok()
                {
                    debug!("Unmount [{}]", p);
                }
            }
            execve(raw_cstr!("/init"), self.argv.cast(), environ.cast())
                .as_os_err()
                .log()
                .ok();
            exit(1);
        }
    }
}
