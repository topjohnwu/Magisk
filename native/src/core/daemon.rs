use std::cell::RefCell;
use std::fs::File;
use std::io;
use std::sync::{Mutex, OnceLock};

use base::{cstr, Directory, ResultExt, Utf8CStr, Utf8CStrBuf, Utf8CStrBufRef, WalkResult};

use crate::logging::{magisk_logging, zygisk_logging};

// Global magiskd singleton
pub static MAGISKD: OnceLock<MagiskD> = OnceLock::new();

#[derive(Default)]
pub struct MagiskD {
    pub logd: Mutex<RefCell<Option<File>>>,
}

pub fn daemon_entry() {
    let magiskd = MagiskD::default();
    magiskd.start_log_daemon();
    MAGISKD.set(magiskd).ok();
    magisk_logging();
}

pub fn zygisk_entry() {
    let magiskd = MagiskD::default();
    MAGISKD.set(magiskd).ok();
    zygisk_logging();
}

pub fn get_magiskd() -> &'static MagiskD {
    MAGISKD.get().unwrap()
}

impl MagiskD {}

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
