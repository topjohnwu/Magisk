use std::fmt;
use std::io::{Write, stderr, stdout};
use std::process::exit;

use num_derive::{FromPrimitive, ToPrimitive};
use num_traits::FromPrimitive;

use crate::ffi::LogLevelCxx;
use crate::{Utf8CStr, cstr};

// Ugly hack to avoid using enum
#[allow(non_snake_case, non_upper_case_globals)]
mod LogFlag {
    pub const DisableError: u32 = 1 << 0;
    pub const DisableWarn: u32 = 1 << 1;
    pub const DisableInfo: u32 = 1 << 2;
    pub const DisableDebug: u32 = 1 << 3;
    pub const ExitOnError: u32 = 1 << 4;
}

#[derive(Copy, Clone, FromPrimitive, ToPrimitive)]
#[repr(i32)]
pub enum LogLevel {
    ErrorCxx = LogLevelCxx::ErrorCxx.repr,
    Error = LogLevelCxx::Error.repr,
    Warn = LogLevelCxx::Warn.repr,
    Info = LogLevelCxx::Info.repr,
    Debug = LogLevelCxx::Debug.repr,
}

// We don't need to care about thread safety, because all
// logger changes will only happen on the main thread.
pub static mut LOGGER: Logger = Logger {
    write: |_, _| {},
    flags: 0,
};

type LogWriter = fn(level: LogLevel, msg: &Utf8CStr);
pub(crate) type Formatter<'a> = &'a mut dyn fmt::Write;

#[derive(Copy, Clone)]
pub struct Logger {
    pub write: LogWriter,
    pub flags: u32,
}

pub fn exit_on_error(b: bool) {
    unsafe {
        if b {
            LOGGER.flags |= LogFlag::ExitOnError;
        } else {
            LOGGER.flags &= !LogFlag::ExitOnError;
        }
    }
}

impl LogLevel {
    fn as_disable_flag(&self) -> u32 {
        match *self {
            LogLevel::Error | LogLevel::ErrorCxx => LogFlag::DisableError,
            LogLevel::Warn => LogFlag::DisableWarn,
            LogLevel::Info => LogFlag::DisableInfo,
            LogLevel::Debug => LogFlag::DisableDebug,
        }
    }
}

pub fn set_log_level_state(level: LogLevel, enabled: bool) {
    let flag = level.as_disable_flag();
    unsafe {
        if enabled {
            LOGGER.flags &= !flag
        } else {
            LOGGER.flags |= flag
        }
    }
}

fn log_with_writer<F: FnOnce(LogWriter)>(level: LogLevel, f: F) {
    let logger = unsafe { LOGGER };
    if (logger.flags & level.as_disable_flag()) != 0 {
        return;
    }
    f(logger.write);
    if matches!(level, LogLevel::ErrorCxx) && (logger.flags & LogFlag::ExitOnError) != 0 {
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

    let logger = Logger {
        write: cmdline_write,
        flags: LogFlag::ExitOnError,
    };
    unsafe {
        LOGGER = logger;
    }
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
