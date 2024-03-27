use const_format::concatcp;

pub const LOGFILE: &str = "/cache/magisk.log";
pub const INTLROOT: &str = ".magisk";
pub const LOG_PIPE: &str = concatcp!(INTLROOT, "/device/log");
pub const MAIN_CONFIG: &str = concatcp!(INTLROOT, "/config");
