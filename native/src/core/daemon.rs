use std::fs::File;
use std::io;
use std::io::BufReader;
use std::sync::{Mutex, OnceLock};

use base::libc::{O_CLOEXEC, O_RDONLY};
use base::{
    cstr, BufReadExt, Directory, FsPathBuf, ResultExt, Utf8CStr, Utf8CStrBuf, Utf8CStrBufArr,
    Utf8CStrBufRef, WalkResult,
};

use crate::logging::magisk_logging;
use crate::{get_prop, MAIN_CONFIG};

// Global magiskd singleton
pub static MAGISKD: OnceLock<MagiskD> = OnceLock::new();

#[derive(Default)]
pub struct MagiskD {
    pub logd: Mutex<Option<File>>,
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
}

mod cxx_extern {
    use base::libc::c_char;

    extern "C" {
        pub fn get_magisk_tmp() -> *const c_char;
    }
}

pub fn get_magisk_tmp() -> &'static Utf8CStr {
    unsafe { Utf8CStr::from_ptr(cxx_extern::get_magisk_tmp()).unwrap_unchecked() }
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
        logd: Default::default(),
        is_emulator,
        is_recovery,
    };
    magiskd.start_log_daemon();
    MAGISKD.set(magiskd).ok();
    magisk_logging();
}

pub fn get_magiskd() -> &'static MagiskD {
    unsafe { MAGISKD.get().unwrap_unchecked() }
}

pub fn find_apk_path(pkg: &[u8], data: &mut [u8]) -> usize {
    use WalkResult::*;
    fn inner(pkg: &[u8], buf: &mut dyn Utf8CStrBuf) -> io::Result<usize> {
        let pkg = match Utf8CStr::from_bytes(pkg) {
            Ok(pkg) => pkg,
            Err(e) => return Err(io::Error::new(io::ErrorKind::Other, e)),
        };
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
