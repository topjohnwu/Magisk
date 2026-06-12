#![allow(dead_code)]
use base::const_format::concatcp;

#[path = "../../out/generated/flags.rs"]
mod flags;

pub const POST_FS_DATA_WAIT_TIME: i32 = 40;
pub const APPLET_NAMES: &[&str] = &["su", "resetprop"];

// versions
pub use flags::*;
pub const MAGISK_FULL_VER: &str = concatcp!(MAGISK_VERSION, "(", MAGISK_VER_CODE, ")");

pub const APP_PACKAGE_NAME: &str = "com.topjohnwu.magisk";

pub const LOGFILE: &str = "/cache/magisk.log";

// data paths
pub const SECURE_DIR: &str = "/data/adb";
pub const MODULEROOT: &str = concatcp!(SECURE_DIR, "/modules");
pub const MODULEUPGRADE: &str = concatcp!(SECURE_DIR, "/modules_update");
pub const DATABIN: &str = concatcp!(SECURE_DIR, "/magisk");
pub const MAGISKDB: &str = concatcp!(SECURE_DIR, "/magisk.db");

// tmpfs paths
pub const INTERNAL_DIR: &str = ".magisk";
pub const MAIN_CONFIG: &str = concatcp!(INTERNAL_DIR, "/config");
pub const PREINITMIRR: &str = concatcp!(INTERNAL_DIR, "/preinit");
pub const MODULEMNT: &str = concatcp!(INTERNAL_DIR, "/modules");
pub const WORKERDIR: &str = concatcp!(INTERNAL_DIR, "/worker");
pub const BBPATH: &str = concatcp!(INTERNAL_DIR, "/busybox");
pub const DEVICEDIR: &str = concatcp!(INTERNAL_DIR, "/device");
pub const MAIN_SOCKET: &str = concatcp!(DEVICEDIR, "/socket");
pub const PREINITDEV: &str = concatcp!(DEVICEDIR, "/preinit");
pub const LOG_PIPE: &str = concatcp!(DEVICEDIR, "/log");
pub const ROOTOVL: &str = concatcp!(INTERNAL_DIR, "/rootdir");
pub const ROOTMNT: &str = concatcp!(ROOTOVL, "/.mount_list");
pub const SELINUXMOCK: &str = concatcp!(INTERNAL_DIR, "/selinux");

// Unconstrained domain the daemon and root processes run in
pub const SEPOL_PROC_DOMAIN: &str = "magisk";
pub const MAGISK_PROC_CON: &str = concatcp!("u:r:", SEPOL_PROC_DOMAIN, ":s0");
// Unconstrained file type that anyone can access
pub const SEPOL_FILE_TYPE: &str = "magisk_file";
pub const MAGISK_FILE_CON: &str = concatcp!("u:object_r:", SEPOL_FILE_TYPE, ":s0");
// Log pipe that only root and zygote can open
pub const SEPOL_LOG_TYPE: &str = "magisk_log_file";
pub const MAGISK_LOG_CON: &str = concatcp!("u:object_r:", SEPOL_LOG_TYPE, ":s0");
