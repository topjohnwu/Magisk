use base::const_format::concatcp;

pub const LOGFILE: &str = "/cache/magisk.log";

// data paths
const SECURE_DIR: &str = "/data/adb";
pub const MODULEROOT: &str = concatcp!(SECURE_DIR, "/modules");

// tmpfs paths
const INTERNAL_DIR: &str = ".magisk";
pub const LOG_PIPE: &str = concatcp!(INTERNAL_DIR, "/device/log");
pub const MAIN_CONFIG: &str = concatcp!(INTERNAL_DIR, "/config");
pub const PREINITMIRR: &str = concatcp!(INTERNAL_DIR, "/preinit");
pub const MODULEMNT: &str = concatcp!(INTERNAL_DIR, "/modules");
pub const WORKERDIR: &str = concatcp!(INTERNAL_DIR, "/worker");
pub const DEVICEDIR: &str = concatcp!(INTERNAL_DIR, "/device");
pub const PREINITDEV: &str = concatcp!(DEVICEDIR, "/preinit");
