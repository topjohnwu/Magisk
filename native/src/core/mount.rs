use std::{
    cmp::Ordering::{Greater, Less},
    path::{Path, PathBuf},
};

use num_traits::AsPrimitive;

use base::libc::{c_uint, dev_t};
use base::{
    FsPathBuilder, LibcReturn, LoggedResult, MountInfo, ResultExt, Utf8CStr, Utf8CStrBuf, cstr,
    debug, info, libc, parse_mount_info, warn,
};

use crate::consts::{MODULEMNT, MODULEROOT, PREINITDEV, PREINITMIRR, WORKERDIR};
use crate::ffi::{get_magisk_tmp, resolve_preinit_dir, switch_mnt_ns};
use crate::get_prop;

pub fn setup_preinit_dir() {
    let magisk_tmp = get_magisk_tmp();

    // Mount preinit directory
    let dev_path = cstr::buf::new::<64>()
        .join_path(magisk_tmp)
        .join_path(PREINITDEV);
    if let Ok(attr) = dev_path.get_attr()
        && attr.st.st_mode & libc::S_IFMT as c_uint == libc::S_IFBLK.as_()
    {
        // DO NOT mount the block device directly, as we do not know the flags and configs
        // to properly mount the partition; mounting block devices directly as rw could cause
        // crashes if the filesystem driver is crap (e.g. some broken F2FS drivers).
        // What we do instead is to scan through the current mountinfo and find a pre-existing
        // mount point mounting our desired partition, and then bind mount the target folder.
        let preinit_dev = attr.st.st_rdev;
        let mnt_path = cstr::buf::default()
            .join_path(magisk_tmp)
            .join_path(PREINITMIRR);
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
                    preinit_dir.mkdir(0o700)?;
                    mnt_path.mkdirs(0o755)?;
                    mnt_path.remove().ok();
                    mnt_path.create_symlink_to(preinit_dir)?;
                };
                if r.is_ok() {
                    info!("* Found preinit dir: {}", preinit_dir);
                    return;
                }
            }
        }
    }

    warn!("mount: preinit dir not found");
}

pub fn setup_module_mount() {
    // Bind remount module root to clear nosuid
    let module_mnt = cstr::buf::default()
        .join_path(get_magisk_tmp())
        .join_path(MODULEMNT);
    let _: LoggedResult<()> = try {
        module_mnt.mkdir(0o755)?;
        cstr!(MODULEROOT).bind_mount_to(&module_mnt, false)?;
        module_mnt.remount_mount_point_flags(libc::MS_RDONLY)?;
    };
}

pub fn clean_mounts() {
    let magisk_tmp = get_magisk_tmp();

    let mut buf = cstr::buf::default();

    let module_mnt = buf.append_path(magisk_tmp).append_path(MODULEMNT);
    module_mnt.unmount().log_ok();
    buf.clear();

    let worker_dir = buf.append_path(magisk_tmp).append_path(WORKERDIR);
    let _: LoggedResult<()> = try {
        worker_dir.set_mount_private(true)?;
        worker_dir.unmount()?;
    };
}

// when partitions have the same fs type, the order is:
// - data: it has sufficient space and can be safely written
// - cache: size is limited, but still can be safely written
// - metadata: size is limited, and it might cause unexpected behavior if written
// - persist: it's the last resort, as it's dangerous to write to it
#[derive(PartialEq, Eq, PartialOrd, Ord)]
enum PartId {
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
    } else if get_prop(cstr!("ro.crypto.type"), false) == "block" {
        EncryptType::Block
    } else if get_prop(cstr!("ro.crypto.metadata.enabled"), false) == "true" {
        EncryptType::Metadata
    } else {
        EncryptType::File
    };

    let mut matched_info = parse_mount_info("self")
        .into_iter()
        .filter_map(|info| {
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
            ap,
            bp,
            at.as_str() == "ext4",
            bt.as_str() == "ext4",
        ) {
            // metadata is not affected by f2fs kernel bug
            (PartId::Metadata, _, _, true) | (_, PartId::Metadata, true, _) => ap.cmp(bp),
            // otherwise, take ext4 f2fs because f2fs has a kernel bug that causes kernel panic
            (_, _, true, false) => Less,
            (_, _, false, true) => Greater,
            // if both has the same fs type, compare the mount point
            _ => ap.cmp(bp),
        },
    );
    let info = &preinit_info.1;
    let mut target = info.target.clone();
    let mut preinit_dir = resolve_preinit_dir(Utf8CStr::from_string(&mut target));
    if unsafe { libc::getuid() } == 0
        && let Ok(tmp) = std::env::var("MAGISKTMP")
        && !tmp.is_empty()
    {
        let mut buf = cstr::buf::default();
        let mirror_dir = buf.append_path(&tmp).append_path(PREINITMIRR);
        let preinit_dir = Utf8CStr::from_string(&mut preinit_dir);
        let _: LoggedResult<()> = try {
            preinit_dir.mkdirs(0o700)?;
            mirror_dir.mkdirs(0o755)?;
            mirror_dir.unmount().ok();
            mirror_dir.remove().ok();
            mirror_dir.create_symlink_to(preinit_dir)?;
        };
        if std::env::var_os("MAKEDEV").is_some() {
            buf.clear();
            let dev_path = buf.append_path(&tmp).append_path(PREINITDEV);
            unsafe {
                libc::mknod(
                    dev_path.as_ptr(),
                    libc::S_IFBLK | 0o600,
                    info.device as dev_t,
                )
                .check_io_err()
                .log()
                .ok();
            }
        }
    }
    Path::new(&info.source)
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
        if let Some(prev) = &prev
            && Path::new(target).starts_with(prev)
        {
            return false;
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
