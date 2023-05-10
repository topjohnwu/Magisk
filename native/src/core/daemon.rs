use crate::logging::{magisk_logging, zygisk_logging};
use std::cell::RefCell;
use std::fs::File;
use std::sync::{Mutex, OnceLock};

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
