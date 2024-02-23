use std::fs::File;
use std::io::BufReader;
use std::sync::{Mutex, OnceLock};
use std::{io, mem};

use base::libc::{O_CLOEXEC, O_RDONLY};
use base::{
    cstr, libc, open_fd, BufReadExt, Directory, FsPathBuf, ResultExt, Utf8CStr, Utf8CStrBuf,
    Utf8CStrBufArr, Utf8CStrBufRef, WalkResult,
};

use crate::ffi::{get_magisk_tmp, CxxMagiskD, RequestCode};
use crate::logging::magisk_logging;
use crate::{get_prop, MAIN_CONFIG};

// Global magiskd singleton
pub static MAGISKD: OnceLock<MagiskD> = OnceLock::new();

#[repr(u32)]
enum BootState {
    PostFsDataDone = (1 << 0),
    LateStartDone = (1 << 1),
    BootComplete = (1 << 2),
    SafeMode = (1 << 3),
}

#[derive(Default)]
#[repr(transparent)]
struct BootStateFlags(u32);

impl BootStateFlags {
    fn contains(&self, stage: BootState) -> bool {
        (self.0 & stage as u32) != 0
    }

    fn set(&mut self, stage: BootState) {
        self.0 |= stage as u32;
    }
}

#[derive(Default)]
pub struct MagiskD {
    pub logd: Mutex<Option<File>>,
    boot_stage_lock: Mutex<BootStateFlags>,
    is_emulator: bool,
    is_recovery: bool,
}

impl MagiskD {
    pub fn is_emulator(&self) -> bool {
        self.is_emulator
    }

    pub fn is_recovery(&self) -> bool {
        self.is_recovery
    }

    pub fn boot_stage_handler(&self, client: i32, code: i32) {
        // Make sure boot stage execution is always serialized
        let mut state = self.boot_stage_lock.lock().unwrap();

        let code = RequestCode { repr: code };
        match code {
            RequestCode::POST_FS_DATA => {
                if check_data() && !state.contains(BootState::PostFsDataDone) {
                    if self.as_cxx().post_fs_data() {
                        state.set(BootState::SafeMode);
                    }
                    state.set(BootState::PostFsDataDone);
                }
                unsafe { libc::close(client) };
            }
            RequestCode::LATE_START => {
                unsafe { libc::close(client) };
                if state.contains(BootState::PostFsDataDone) && !state.contains(BootState::SafeMode)
                {
                    self.as_cxx().late_start();
                    state.set(BootState::LateStartDone);
                }
            }
            RequestCode::BOOT_COMPLETE => {
                unsafe { libc::close(client) };
                if !state.contains(BootState::SafeMode) {
                    state.set(BootState::BootComplete);
                    self.as_cxx().boot_complete()
                }
            }
            _ => {
                unsafe { libc::close(client) };
            }
        }
    }

    #[inline(always)]
    fn as_cxx(&self) -> &CxxMagiskD {
        unsafe { mem::transmute(self) }
    }
}

pub fn daemon_entry() {
    let mut qemu = get_prop(cstr!("ro.kernel.qemu"), false);
    if qemu.is_empty() {
        qemu = get_prop(cstr!("ro.boot.qemu"), false);
    }
    let is_emulator = qemu == "1";

    // Load config status
    let mut buf = Utf8CStrBufArr::<64>::new();
    let path = FsPathBuf::new(&mut buf)
        .join(get_magisk_tmp())
        .join(MAIN_CONFIG!());
    let mut is_recovery = false;
    if let Ok(file) = path.open(O_RDONLY | O_CLOEXEC) {
        let mut file = BufReader::new(file);
        file.foreach_props(|key, val| {
            if key == "RECOVERYMODE" {
                is_recovery = val == "true";
                return false;
            }
            true
        });
    }

    let magiskd = MagiskD {
        is_emulator,
        is_recovery,
        ..Default::default()
    };
    magiskd.start_log_daemon();
    MAGISKD.set(magiskd).ok();
    magisk_logging();
}

fn check_data() -> bool {
    if let Ok(fd) = open_fd!(cstr!("/proc/mounts"), O_RDONLY | O_CLOEXEC) {
        let file = File::from(fd);
        let mut mnt = false;
        BufReader::new(file).foreach_lines(|line| {
            if line.contains(" /data ") && !line.contains("tmpfs") {
                mnt = true;
                return false;
            }
            true
        });
        if !mnt {
            return false;
        }
        let crypto = get_prop(cstr!("ro.crypto.state"), false);
        return if !crypto.is_empty() {
            if crypto != "encrypted" {
                // Unencrypted, we can directly access data
                true
            } else {
                // Encrypted, check whether vold is started
                !get_prop(cstr!("init.svc.vold"), false).is_empty()
            }
        } else {
            // ro.crypto.state is not set, assume it's unencrypted
            true
        };
    }
    false
}

pub fn get_magiskd() -> &'static MagiskD {
    unsafe { MAGISKD.get().unwrap_unchecked() }
}

pub fn find_apk_path(pkg: &Utf8CStr, data: &mut [u8]) -> usize {
    use WalkResult::*;
    fn inner(pkg: &Utf8CStr, buf: &mut dyn Utf8CStrBuf) -> io::Result<usize> {
        Directory::open(cstr!("/data/app"))?.pre_order_walk(|e| {
            if !e.is_dir() {
                return Ok(Skip);
            }
            let d_name = e.d_name().to_bytes();
            if d_name.starts_with(pkg.as_bytes()) && d_name[pkg.len()] == b'-' {
                // Found the APK path, we can abort now
                e.path(buf)?;
                return Ok(Abort);
            }
            if d_name.starts_with(b"~~") {
                return Ok(Continue);
            }
            Ok(Skip)
        })?;
        if !buf.is_empty() {
            buf.push_str("/base.apk");
        }
        Ok(buf.len())
    }
    inner(pkg, &mut Utf8CStrBufRef::from(data))
        .log()
        .unwrap_or(0)
}
