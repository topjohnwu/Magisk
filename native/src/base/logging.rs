use crate::ffi::LogLevelCxx;
use crate::{Utf8CStr, cstr};
use bitflags::bitflags;
use num_derive::{FromPrimitive, ToPrimitive};
use num_traits::FromPrimitive;
use std::fmt;
use std::io::{Write, stderr, stdout};
use std::process::exit;

bitflags! {
    #[derive(Copy, Clone)]
    struct LogFlag : u32 {
        const DISABLE_ERROR = 1 << 0;
        const DISABLE_WARN = 1 << 1;
        const DISABLE_INFO = 1 << 2;
        const DISABLE_DEBUG = 1 << 3;
        const EXIT_ON_ERROR = 1 << 4;
    }
}

#[derive(Copy, Clone, FromPrimitive, ToPrimitive)]
#[repr(i32)]
pub enum LogLevel {
    Error = LogLevelCxx::Error.repr,
    Warn = LogLevelCxx::Warn.repr,
    Info = LogLevelCxx::Info.repr,
    Debug = LogLevelCxx::Debug.repr,
}

// We don't need to care about thread safety, because all
// logger changes will only happen on the main thread.
pub static mut LOGGER: Logger = Logger {
    write: |_, _| {},
    flags: LogFlag::empty(),
};

type LogWriter = fn(level: LogLevel, msg: &Utf8CStr);
pub(crate) type Formatter<'a> = &'a mut dyn fmt::Write;

#[derive(Copy, Clone)]
pub struct Logger {
    pub write: LogWriter,
    flags: LogFlag,
}

pub fn update_logger(f: impl FnOnce(&mut Logger)) {
    let mut logger = unsafe { LOGGER };
    f(&mut logger);
    unsafe {
        LOGGER = logger;
    }
}

pub fn exit_on_error(b: bool) {
    update_logger(|logger| logger.flags.set(LogFlag::EXIT_ON_ERROR, b));
}

impl LogLevel {
    fn as_disable_flag(&self) -> LogFlag {
        match *self {
            LogLevel::Error => LogFlag::DISABLE_ERROR,
            LogLevel::Warn => LogFlag::DISABLE_WARN,
            LogLevel::Info => LogFlag::DISABLE_INFO,
            LogLevel::Debug => LogFlag::DISABLE_DEBUG,
        }
    }
}

pub fn set_log_level_state(level: LogLevel, enabled: bool) {
    update_logger(|logger| logger.flags.set(level.as_disable_flag(), enabled));
}

fn log_with_writer<F: FnOnce(LogWriter)>(level: LogLevel, f: F) {
    let logger = unsafe { LOGGER };
    if logger.flags.contains(level.as_disable_flag()) {
        return;
    }
    f(logger.write);
    if matches!(level, LogLevel::Error) && logger.flags.contains(LogFlag::EXIT_ON_ERROR) {
        exit(-1);
    }
}

pub fn log_from_cxx(level: LogLevelCxx, msg: &Utf8CStr) {
    if let Some(level) = LogLevel::from_i32(level.repr) {
        log_with_writer(level, |write| write(level, msg));
    }
}

pub fn log_with_formatter<F: FnOnce(Formatter) -> fmt::Result>(level: LogLevel, f: F) {
    log_with_writer(level, |write| {
        let mut buf = cstr::buf::default();
        f(&mut buf).ok();
        write(level, &buf);
    });
}

pub fn cmdline_logging() {
    fn cmdline_write(level: LogLevel, msg: &Utf8CStr) {
        if matches!(level, LogLevel::Info) {
            stdout().write_all(msg.as_bytes()).ok();
        } else {
            stderr().write_all(msg.as_bytes()).ok();
        }
    }
    update_logger(|logger| logger.write = cmdline_write);
}

#[macro_export]
macro_rules! log_with_args {
    ($level:expr, $($args:tt)+) => {
        $crate::log_with_formatter($level, |w| writeln!(w, $($args)+))
    }
}

#[macro_export]
macro_rules! error {
    ($($args:tt)+) => {
        $crate::log_with_formatter($crate::LogLevel::Error, |w| writeln!(w, $($args)+))
    }
}

#[macro_export]
macro_rules! warn {
    ($($args:tt)+) => {
        $crate::log_with_formatter($crate::LogLevel::Warn, |w| writeln!(w, $($args)+))
    }
}

#[macro_export]
macro_rules! info {
    ($($args:tt)+) => {
        $crate::log_with_formatter($crate::LogLevel::Info, |w| writeln!(w, $($args)+))
    }
}

#[cfg(debug_assertions)]
#[macro_export]
macro_rules! debug {
    ($($args:tt)+) => {
        $crate::log_with_formatter($crate::LogLevel::Debug, |w| writeln!(w, $($args)+))
    }
}

#[cfg(not(debug_assertions))]
#[macro_export]
macro_rules! debug {
    ($($args:tt)+) => {};
}
