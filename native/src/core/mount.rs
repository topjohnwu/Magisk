use std::{
    cmp::Ordering::{Greater, Less},
    path::{Path, PathBuf},
    ptr,
};

use num_traits::AsPrimitive;

use base::libc::{c_uint, dev_t};
use base::{
    cstr, debug, info, libc, parse_mount_info, raw_cstr, warn, FsPath, FsPathBuf, LibcReturn,
    LoggedResult, MountInfo, ResultExt, Utf8CStr, Utf8CStrBufArr,
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

// when partitions have the same fs type, the order is:
// - preinit: it's selected previously, so it's always the first
// - data: it has sufficient space and can be safely written
// - cache: size is limited, but still can be safely written
// - metadata: size is limited, and it might cause unexpected behavior if written
// - persist: it's the last resort, as it's dangerous to write to it
#[derive(PartialEq, Eq, PartialOrd, Ord)]
enum PartId {
    PreInit,
    Data,
    Cache,
    Metadata,
    Persist,
}

enum EncryptType {
    None,
    Block,
    File,
    Metadata,
}

pub fn find_preinit_device() -> String {
    let encrypt_type = if get_prop(cstr!("ro.crypto.state"), false) != "encrypted" {
        EncryptType::None
    } else if get_prop(cstr!("ro.crypto.type"), false) != "file" {
        EncryptType::Block
    } else if FsPath::from(cstr!("/metadata/vold/metadata_encryption")).exists() {
        EncryptType::Metadata
    } else {
        EncryptType::File
    };
    let mount = unsafe { libc::getuid() } == 0 && std::env::var("MAGISKTMP").is_ok();
    let make_dev = mount && std::env::var_os("MAKEDEV").is_some();

    let mut matched_info = parse_mount_info("self")
        .into_iter()
        .filter_map(|info| {
            // if preinit is already mounted, choose it unconditionally
            if info.target.ends_with(PREINITMIRR) {
                return Some((PartId::PreInit, info));
            }
            if info.root != "/" || !info.source.starts_with('/') || info.source.contains("/dm-") {
                return None;
            }
            match info.fs_type.as_str() {
                "ext4" | "f2fs" => (),
                _ => return None,
            }
            if !info.fs_option.split(',').any(|s| s == "rw") {
                return None;
            }
            if let Some(path) = Path::new(&info.source).parent() {
                if !path.ends_with("by-name") && !path.ends_with("block") {
                    return None;
                }
            } else {
                return None;
            }
            // take data iff it's not encrypted or file-based encrypted without metadata
            // other partitions are always taken
            match info.target.as_str() {
                "/persist" | "/mnt/vendor/persist" => Some((PartId::Persist, info)),
                "/metadata" => Some((PartId::Metadata, info)),
                "/cache" => Some((PartId::Cache, info)),
                "/data" => Some((PartId::Data, info))
                    .take_if(|_| matches!(encrypt_type, EncryptType::None | EncryptType::File)),
                _ => None,
            }
        })
        .collect::<Vec<_>>();

    if matched_info.is_empty() {
        return String::new();
    }

    let (_, preinit_info, _) = matched_info.select_nth_unstable_by(
        0,
        |(ap, MountInfo { fs_type: at, .. }), (bp, MountInfo { fs_type: bt, .. })| match (
            at.as_str() == "ext4",
            bt.as_str() == "ext4",
        ) {
            // take ext4 over others (f2fs) because f2fs has a kernel bug that causes kernel panic
            (true, false) => Less,
            (false, true) => Greater,
            // if both has the same fs type, compare the mount point
            _ => ap.cmp(bp),
        },
    );
    let preinit_source = match preinit_info {
        (PartId::PreInit, info) => &info.source,
        (_, info) => {
            let mut target = info.target.clone();
            let mut preinit_dir = resolve_preinit_dir(Utf8CStr::from_string(&mut target));
            if mount && let Ok(tmp) = std::env::var("MAGISKTMP") {
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
                            info.device as dev_t,
                        )
                        .as_os_err()
                        .log()
                        .ok();
                    }
                }
            }
            &info.source
        }
    };
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
