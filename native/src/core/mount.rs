use std::path::{Path, PathBuf};
use std::ptr;

use num_traits::AsPrimitive;

use base::libc::{c_uint, dev_t};
use base::{
    cstr, debug, info, libc, parse_mount_info, raw_cstr, warn, FsPath, FsPathBuf, LibcReturn,
    LoggedResult, ResultExt, Utf8CStr, Utf8CStrBufArr,
};

use crate::consts::{MODULEMNT, MODULEROOT, PREINITDEV, PREINITMIRR, WORKERDIR};
use crate::ffi::{get_magisk_tmp, resolve_preinit_dir, switch_mnt_ns};
use crate::get_prop;

pub fn setup_mounts() {
    info!("* Setup internal mounts");

    let magisk_tmp = get_magisk_tmp();
    let mut buf = Utf8CStrBufArr::default();

    // Mount preinit directory
    let mut dev_buf = Utf8CStrBufArr::<64>::new();
    let dev_path = FsPathBuf::new(&mut dev_buf)
        .join(magisk_tmp)
        .join(PREINITDEV);
    let mut mounted = false;
    if let Ok(attr) = dev_path.get_attr() {
        if attr.st.st_mode & libc::S_IFMT as c_uint == libc::S_IFBLK.as_() {
            // DO NOT mount the block device directly, as we do not know the flags and configs
            // to properly mount the partition; mounting block devices directly as rw could cause
            // crashes if the filesystem driver is crap (e.g. some broken F2FS drivers).
            // What we do instead is to scan through the current mountinfo and find a pre-existing
            // mount point mounting our desired partition, and then bind mount the target folder.
            let preinit_dev = attr.st.st_rdev;
            let mnt_path = FsPathBuf::new(&mut buf).join(magisk_tmp).join(PREINITMIRR);
            for info in parse_mount_info("self") {
                if info.root == "/" && info.device == preinit_dev {
                    if !info.fs_option.split(',').any(|s| s == "rw") {
                        // Only care about rw mounts
                        continue;
                    }
                    let mut target = info.target;
                    let target = Utf8CStr::from_string(&mut target);
                    let mut preinit_dir = resolve_preinit_dir(target);
                    let preinit_dir = Utf8CStr::from_string(&mut preinit_dir);
                    let r: LoggedResult<()> = try {
                        FsPath::from(preinit_dir).mkdir(0o700)?;
                        mnt_path.mkdirs(0o755)?;
                        unsafe {
                            libc::mount(
                                preinit_dir.as_ptr(),
                                mnt_path.as_ptr(),
                                ptr::null(),
                                libc::MS_BIND,
                                ptr::null(),
                            )
                            .as_os_err()?
                        }
                    };
                    if r.is_ok() {
                        mounted = true;
                        break;
                    }
                }
            }
        }
    }
    if !mounted {
        warn!("mount: preinit mirror not mounted");
        dev_path.remove().ok();
    } else {
        debug!("mount: preinit mirror mounted");
    }

    // Bind remount module root to clear nosuid
    let module_mnt = FsPathBuf::new(&mut buf).join(magisk_tmp).join(MODULEMNT);
    let _: LoggedResult<()> = try {
        module_mnt.mkdir(0o755)?;
        unsafe {
            libc::mount(
                raw_cstr!(MODULEROOT),
                module_mnt.as_ptr(),
                ptr::null(),
                libc::MS_BIND,
                ptr::null(),
            )
            .as_os_err()?;
            libc::mount(
                ptr::null(),
                module_mnt.as_ptr(),
                ptr::null(),
                libc::MS_REMOUNT | libc::MS_BIND | libc::MS_RDONLY,
                ptr::null(),
            )
            .as_os_err()?;
            libc::mount(
                ptr::null(),
                module_mnt.as_ptr(),
                ptr::null(),
                libc::MS_PRIVATE,
                ptr::null(),
            )
            .as_os_err()?;
        }
    };

    // Prepare worker
    let worker_dir = FsPathBuf::new(&mut buf).join(magisk_tmp).join(WORKERDIR);
    let _: LoggedResult<()> = try {
        worker_dir.mkdir(0)?;
        unsafe {
            libc::mount(
                worker_dir.as_ptr(),
                worker_dir.as_ptr(),
                ptr::null(),
                libc::MS_BIND,
                ptr::null(),
            )
            .as_os_err()?;
            libc::mount(
                ptr::null(),
                worker_dir.as_ptr(),
                ptr::null(),
                libc::MS_PRIVATE,
                ptr::null(),
            )
            .as_os_err()?;
        }
    };
}

#[derive(Ord, PartialOrd, Eq, PartialEq)]
enum PartId {
    Unknown,
    Persist,
    Metadata,
    Cache,
    Data,
}

pub fn find_preinit_device() -> String {
    let encrypted = get_prop(cstr!("ro.crypto.state"), false) == "encrypted";
    let mount = unsafe { libc::getuid() } == 0 && std::env::var("MAGISKTMP").is_ok();
    let make_dev = mount && std::env::var_os("MAKEDEV").is_some();

    let mut ext4_type = PartId::Unknown;
    let mut f2fs_type = PartId::Unknown;

    let mut preinit_source: String = String::new();
    let mut preinit_dir: String = String::new();
    let mut preinit_dev: u64 = 0;

    'info_loop: for info in parse_mount_info("self") {
        if info.target.ends_with(PREINITMIRR) {
            return Path::new(&info.source)
                .file_name()
                .unwrap()
                .to_str()
                .unwrap()
                .to_string();
        }
        if info.root != "/" || !info.source.starts_with('/') || info.source.contains("/dm-") {
            continue;
        }
        if ext4_type != PartId::Unknown && info.fs_type != "ext4" {
            // Skip all non ext4 partitions once we found a matching ext4 partition
            continue;
        }
        if info.fs_type != "ext4" && info.fs_type != "f2fs" {
            // Only care about ext4 and f2fs filesystems
            continue;
        }
        if !info.fs_option.split(',').any(|s| s == "rw") {
            // Only care about rw mounts
            continue;
        }
        if let Some(path) = Path::new(&info.source).parent() {
            if !path.ends_with("by-name") && !path.ends_with("block") {
                continue;
            }
        } else {
            continue;
        }

        let matched_type = if info.fs_type == "f2fs" {
            &mut f2fs_type
        } else {
            &mut ext4_type
        };

        'block: {
            if *matched_type <= PartId::Unknown
                && (info.target == "/persist" || info.target == "/mnt/vendor/persist")
            {
                *matched_type = PartId::Persist;
                break 'block;
            }
            if *matched_type <= PartId::Persist && info.target == "/metadata" {
                *matched_type = PartId::Metadata;
                break 'block;
            }
            if *matched_type <= PartId::Metadata && info.target == "/cache" {
                *matched_type = PartId::Cache;
                break 'block;
            }
            if *matched_type <= PartId::Cache
                && info.target == "/data"
                && (!encrypted || FsPath::from(cstr!("/data/unencrypted")).exists())
            {
                *matched_type = PartId::Data;
            }

            // No matches, continue through the loop
            continue 'info_loop;
        }

        if mount {
            let mut target = info.target;
            preinit_dir = resolve_preinit_dir(Utf8CStr::from_string(&mut target));
            preinit_dev = info.device;
        }
        preinit_source = info.source;

        // Cannot find any better partition, stop finding
        if ext4_type == PartId::Data {
            break;
        }
    }

    if preinit_source.is_empty() {
        return String::new();
    }

    if !preinit_dir.is_empty() {
        if let Ok(tmp) = std::env::var("MAGISKTMP") {
            let mut buf = Utf8CStrBufArr::default();
            let mirror_dir = FsPathBuf::new(&mut buf).join(&tmp).join(PREINITMIRR);
            let preinit_dir = FsPath::from(Utf8CStr::from_string(&mut preinit_dir));
            let _: LoggedResult<()> = try {
                preinit_dir.mkdirs(0o700)?;
                mirror_dir.mkdirs(0o700)?;
                unsafe {
                    libc::mount(
                        preinit_dir.as_ptr(),
                        mirror_dir.as_ptr(),
                        ptr::null(),
                        libc::MS_BIND,
                        ptr::null(),
                    )
                    .as_os_err()?;
                }
            };
            if make_dev {
                let dev_path = FsPathBuf::new(&mut buf).join(&tmp).join(PREINITDEV);
                unsafe {
                    libc::mknod(
                        dev_path.as_ptr(),
                        libc::S_IFBLK | 0o600,
                        preinit_dev as dev_t,
                    )
                    .as_os_err()
                    .log()
                    .ok();
                }
            }
        }
    }

    Path::new(&preinit_source)
        .file_name()
        .unwrap()
        .to_str()
        .unwrap()
        .to_string()
}

pub fn revert_unmount(pid: i32) {
    if pid > 0 {
        if switch_mnt_ns(pid) != 0 {
            return;
        }
        debug!("denylist: handling PID=[{}]", pid);
    }

    let mut targets = Vec::new();

    // Unmount Magisk tmpfs and mounts from module files
    for info in parse_mount_info("self") {
        if info.source == "magisk" || info.root.starts_with("/adb/modules") {
            targets.push(info.target);
        }
    }

    if targets.is_empty() {
        return;
    }

    let mut prev: Option<PathBuf> = None;
    targets.sort();
    targets.retain(|target| {
        if let Some(prev) = &prev {
            if Path::new(target).starts_with(prev) {
                return false;
            }
        }
        prev = Some(PathBuf::from(target.clone()));
        true
    });

    for mut target in targets {
        let target = Utf8CStr::from_string(&mut target);
        unsafe {
            if libc::umount2(target.as_ptr(), libc::MNT_DETACH) == 0 {
                debug!("denylist: Unmounted ({})", target);
            }
        }
    }
}
