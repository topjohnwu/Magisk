use std::fmt::Arguments;
use std::io::{stderr, stdout, Write};
use std::process::exit;

use crate::ffi::LogLevel;

// Ugly hack to avoid using enum
#[allow(non_snake_case, non_upper_case_globals)]
mod LogFlag {
    pub const DisableError: u32 = 1 << 0;
    pub const DisableWarn: u32 = 1 << 1;
    pub const DisableInfo: u32 = 1 << 2;
    pub const DisableDebug: u32 = 1 << 3;
    pub const ExitOnError: u32 = 1 << 4;
}

// We don't need to care about thread safety, because all
// logger changes will only happen on the main thread.
pub static mut LOGGER: Logger = Logger {
    fmt: |_, _| {},
    write: |_, _| {},
    flags: 0,
};

#[derive(Copy, Clone)]
pub struct Logger {
    pub fmt: fn(level: LogLevel, args: Arguments),
    pub write: fn(level: LogLevel, msg: &[u8]),
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
    fn to_disable_flag(&self) -> u32 {
        match *self {
            LogLevel::Error => LogFlag::DisableError,
            LogLevel::Warn => LogFlag::DisableWarn,
            LogLevel::Info => LogFlag::DisableInfo,
            LogLevel::Debug => LogFlag::DisableDebug,
            _ => 0,
        }
    }
}

pub fn set_log_level_state(level: LogLevel, enabled: bool) {
    let flag = level.to_disable_flag();
    unsafe {
        if enabled {
            LOGGER.flags &= !flag
        } else {
            LOGGER.flags |= flag
        }
    }
}

pub fn log_with_rs(level: LogLevel, msg: &str) {
    let logger = unsafe { LOGGER };
    if (logger.flags & level.to_disable_flag()) != 0 {
        return;
    }
    (logger.write)(level, msg.as_bytes());
    if level == LogLevel::Error && (logger.flags & LogFlag::ExitOnError) != 0 {
        exit(1);
    }
}

pub fn log_impl(level: LogLevel, args: Arguments) {
    let logger = unsafe { LOGGER };
    if (logger.flags & level.to_disable_flag()) != 0 {
        return;
    }
    (logger.fmt)(level, args);
    if level == LogLevel::Error && (logger.flags & LogFlag::ExitOnError) != 0 {
        exit(1);
    }
}

pub fn cmdline_logging() {
    fn cmdline_print(level: LogLevel, args: Arguments) {
        if level == LogLevel::Info {
            print!("{}", args);
        } else {
            eprint!("{}", args);
        }
    }
    fn cmdline_write(level: LogLevel, msg: &[u8]) {
        if level == LogLevel::Info {
            stdout().write_all(msg).ok();
        } else {
            stderr().write_all(msg).ok();
        }
    }

    let logger = Logger {
        fmt: cmdline_print,
        write: cmdline_write,
        flags: LogFlag::ExitOnError,
    };
    unsafe {
        LOGGER = logger;
    }
}

#[macro_export]
macro_rules! perror {
    ($fmt:expr) => {
        $crate::log_impl($crate::ffi::LogLevel::Error, format_args_nl!(
            concat!($fmt, " failed with {}: {}"),
            crate::errno(),
            crate::error_str()
        ))
    };
    ($fmt:expr, $($args:tt)*) => {
        $crate::log_impl($crate::ffi::LogLevel::Error, format_args_nl!(
            concat!($fmt, " failed with {}: {}"),
            $($args)*,
            crate::errno(),
            crate::error_str()
        ))
    };
}

#[macro_export]
macro_rules! error {
    ($($args:tt)+) => ($crate::log_impl($crate::ffi::LogLevel::Error, format_args_nl!($($args)+)))
}

#[macro_export]
macro_rules! warn {
    ($($args:tt)+) => ($crate::log_impl($crate::ffi::LogLevel::Warn, format_args_nl!($($args)+)))
}

#[macro_export]
macro_rules! info {
    ($($args:tt)+) => ($crate::log_impl($crate::ffi::LogLevel::Info, format_args_nl!($($args)+)))
}

#[cfg(debug_assertions)]
#[macro_export]
macro_rules! debug {
    ($($args:tt)+) => ($crate::log_impl($crate::ffi::LogLevel::Debug, format_args_nl!($($args)+)))
}

#[cfg(not(debug_assertions))]
#[macro_export]
macro_rules! debug {
    ($($args:tt)+) => {};
}
