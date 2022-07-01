use std::fmt::Arguments;
use std::process::exit;

use crate::ffi::LogLevel;

// We don't need to care about thread safety, because all
// logger changes will only happen on the main thread.
pub static mut LOGGER: Logger = Logger {
    d: nop_log,
    i: nop_log,
    w: nop_log,
    e: nop_log,
};
static mut EXIT_ON_ERROR: bool = false;

#[derive(Copy, Clone)]
pub struct Logger {
    pub d: fn(args: Arguments),
    pub i: fn(args: Arguments),
    pub w: fn(args: Arguments),
    pub e: fn(args: Arguments),
}

pub fn nop_log(_: Arguments) {}

fn println(args: Arguments) { println!("{}", args); }

fn eprintln(args: Arguments) { eprintln!("{}", args); }

pub fn log_with_rs(level: LogLevel, msg: &str) {
    log_impl(level, format_args!("{}", msg));
}

pub fn exit_on_error(b: bool) {
    unsafe { EXIT_ON_ERROR = b; }
}

pub fn cmdline_logging() {
    let logger = Logger {
        d: eprintln,
        i: println,
        w: eprintln,
        e: eprintln,
    };
    unsafe {
        LOGGER = logger;
        EXIT_ON_ERROR = true;
    }
}

pub fn log_impl(level: LogLevel, args: Arguments) {
    let logger = unsafe { LOGGER };
    let aoe = unsafe { EXIT_ON_ERROR };
    match level {
        LogLevel::Error => {
            (logger.e)(args);
            if aoe { exit(1); }
        }
        LogLevel::Warn => (logger.w)(args),
        LogLevel::Info => (logger.i)(args),
        LogLevel::Debug => (logger.d)(args),
        _ => ()
    }
}

#[macro_export]
macro_rules! error {
    ($($arg:tt)+) => ($crate::log_impl($crate::ffi::LogLevel::Error, format_args!($($arg)+)))
}

#[macro_export]
macro_rules! warn {
    ($($arg:tt)+) => ($crate::log_impl($crate::ffi::LogLevel::Warn, format_args!($($arg)+)))
}

#[macro_export]
macro_rules! info {
    ($($arg:tt)+) => ($crate::log_impl($crate::ffi::LogLevel::Info, format_args!($($arg)+)))
}

#[cfg(debug_assertions)]
#[macro_export]
macro_rules! debug {
    ($($arg:tt)+) => ($crate::log_impl($crate::ffi::LogLevel::Debug, format_args!($($arg)+)))
}

#[cfg(not(debug_assertions))]
#[macro_export]
macro_rules! debug {
    ($($arg:tt)+) => ()
}
