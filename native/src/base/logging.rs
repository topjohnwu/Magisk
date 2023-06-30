use std::fmt;
use std::fmt::{Arguments, Display, Write as fWrite};
use std::io::{stderr, stdout, Write};
use std::panic::Location;
use std::process::exit;

use crate::ffi::LogLevel;
use crate::BufFormatter;

// Error handling and logging throughout the Rust codebase in Magisk:
//
// All errors should be logged and consumed as soon as possible and converted into LoggedError.
// Implement `From<ErrorType> for LoggedError` for non-standard error types so that we can
// directly use the `?` operator to propagate LoggedResult.
//
// To log an error with more information, use `ResultExt::log_with_msg()`.
//
// The "cxx" method variants in `ResultExt` are only used for C++ interop and
// should not be used directly in any Rust code.
//
// For general logging, use the <level>!(...) macros.

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
    write: |_, _| {},
    flags: 0,
};

#[derive(Copy, Clone)]
pub struct Logger {
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
    fn as_disable_flag(&self) -> u32 {
        match *self {
            LogLevel::Error | LogLevel::ErrorCxx => LogFlag::DisableError,
            LogLevel::Warn => LogFlag::DisableWarn,
            LogLevel::Info => LogFlag::DisableInfo,
            LogLevel::Debug => LogFlag::DisableDebug,
            _ => 0,
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

fn do_log<F: FnOnce(fn(level: LogLevel, msg: &[u8]))>(level: LogLevel, f: F) {
    let logger = unsafe { LOGGER };
    if (logger.flags & level.as_disable_flag()) != 0 {
        return;
    }
    f(logger.write);
    if level == LogLevel::ErrorCxx && (logger.flags & LogFlag::ExitOnError) != 0 {
        exit(1);
    }
}

pub fn log_from_cxx(level: LogLevel, msg: &[u8]) {
    do_log(level, |write| write(level, msg));
}

pub fn log_with_formatter<F: FnOnce(&mut BufFormatter) -> fmt::Result>(level: LogLevel, f: F) {
    do_log(level, |write| {
        let mut buf = [0_u8; 4096];
        let mut w = BufFormatter::new(&mut buf);
        let len = if f(&mut w).is_ok() { w.used } else { 0 };
        write(level, &buf[..len]);
    });
}

pub fn log_with_args(level: LogLevel, args: Arguments) {
    log_with_formatter(level, |w| w.write_fmt(args));
}

pub fn cmdline_logging() {
    fn cmdline_write(level: LogLevel, msg: &[u8]) {
        if level == LogLevel::Info {
            stdout().write_all(msg).ok();
        } else {
            stderr().write_all(msg).ok();
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
macro_rules! error {
    ($($args:tt)+) => {
        $crate::log_with_args($crate::ffi::LogLevel::Error, format_args_nl!($($args)+))
    }
}

#[macro_export]
macro_rules! warn {
    ($($args:tt)+) => {
        $crate::log_with_args($crate::ffi::LogLevel::Warn, format_args_nl!($($args)+))
    }
}

#[macro_export]
macro_rules! info {
    ($($args:tt)+) => {
        $crate::log_with_args($crate::ffi::LogLevel::Info, format_args_nl!($($args)+))
    }
}

#[cfg(debug_assertions)]
#[macro_export]
macro_rules! debug {
    ($($args:tt)+) => {
        $crate::log_with_args($crate::ffi::LogLevel::Debug, format_args_nl!($($args)+))
    }
}

#[cfg(not(debug_assertions))]
#[macro_export]
macro_rules! debug {
    ($($args:tt)+) => {};
}

#[derive(Default)]
pub struct LoggedError {}

// Automatically handle all printable errors
impl<T: Display> From<T> for LoggedError {
    #[cfg(not(debug_assertions))]
    fn from(e: T) -> Self {
        log_with_args(LogLevel::Error, format_args_nl!("{:#}", e));
        LoggedError::default()
    }

    #[track_caller]
    #[cfg(debug_assertions)]
    fn from(e: T) -> Self {
        let caller = Location::caller();
        log_with_args(
            LogLevel::Error,
            format_args_nl!("[{}:{}] {:#}", caller.file(), caller.line(), e),
        );
        LoggedError::default()
    }
}

pub type LoggedResult<T> = Result<T, LoggedError>;

#[macro_export]
macro_rules! log_err {
    ($msg:literal $(,)?) => {{
        $crate::log_with_args($crate::ffi::LogLevel::Error, format_args_nl!($msg));
        $crate::LoggedError::default()
    }};
    ($err:expr $(,)?) => {{
        $crate::log_with_args($crate::ffi::LogLevel::Error, format_args_nl!("{}", $err));
        $crate::LoggedError::default()
    }};
    ($($args:tt)+) => {{
        $crate::log_with_args($crate::ffi::LogLevel::Error, format_args_nl!($($args)+));
        $crate::LoggedError::default()
    }};
}

pub trait ResultExt<T>
where
    Self: Sized,
{
    #[cfg(not(debug_assertions))]
    fn log(self) -> LoggedResult<T> {
        self.log_impl(LogLevel::Error, None)
    }

    #[track_caller]
    #[cfg(debug_assertions)]
    fn log(self) -> LoggedResult<T> {
        self.log_impl(LogLevel::Error, Some(Location::caller()))
    }

    #[cfg(not(debug_assertions))]
    fn log_with_msg<F: FnOnce(&mut BufFormatter) -> fmt::Result>(self, f: F) -> LoggedResult<T> {
        self.log_with_msg_impl(LogLevel::Error, None, f)
    }

    #[track_caller]
    #[cfg(debug_assertions)]
    fn log_with_msg<F: FnOnce(&mut BufFormatter) -> fmt::Result>(self, f: F) -> LoggedResult<T> {
        self.log_with_msg_impl(LogLevel::Error, Some(Location::caller()), f)
    }

    fn log_cxx(self) -> LoggedResult<T> {
        self.log_impl(LogLevel::ErrorCxx, None)
    }

    fn log_cxx_with_msg<F: FnOnce(&mut BufFormatter) -> fmt::Result>(
        self,
        f: F,
    ) -> LoggedResult<T> {
        self.log_with_msg_impl(LogLevel::ErrorCxx, None, f)
    }

    fn log_impl(self, level: LogLevel, caller: Option<&'static Location>) -> LoggedResult<T>;
    fn log_with_msg_impl<F: FnOnce(&mut BufFormatter) -> fmt::Result>(
        self,
        level: LogLevel,
        caller: Option<&'static Location>,
        f: F,
    ) -> LoggedResult<T>;
}

impl<T> ResultExt<T> for LoggedResult<T> {
    fn log_impl(self, _: LogLevel, _: Option<&'static Location>) -> LoggedResult<T> {
        self
    }

    fn log_with_msg_impl<F: FnOnce(&mut BufFormatter) -> fmt::Result>(
        self,
        level: LogLevel,
        caller: Option<&'static Location>,
        f: F,
    ) -> LoggedResult<T> {
        match self {
            Ok(v) => Ok(v),
            Err(_) => {
                log_with_formatter(level, |w| {
                    if let Some(caller) = caller {
                        write!(w, "[{}:{}] ", caller.file(), caller.line())?;
                    }
                    f(w)?;
                    w.write_char('\n')
                });
                Err(LoggedError::default())
            }
        }
    }
}

impl<T, E: Display> ResultExt<T> for Result<T, E> {
    fn log_impl(self, level: LogLevel, caller: Option<&'static Location>) -> LoggedResult<T> {
        match self {
            Ok(v) => Ok(v),
            Err(e) => {
                if let Some(caller) = caller {
                    log_with_args(
                        level,
                        format_args_nl!("[{}:{}] {:#}", caller.file(), caller.line(), e),
                    );
                } else {
                    log_with_args(level, format_args_nl!("{:#}", e));
                }
                Err(LoggedError::default())
            }
        }
    }

    fn log_with_msg_impl<F: FnOnce(&mut BufFormatter) -> fmt::Result>(
        self,
        level: LogLevel,
        caller: Option<&'static Location>,
        f: F,
    ) -> LoggedResult<T> {
        match self {
            Ok(v) => Ok(v),
            Err(e) => {
                log_with_formatter(level, |w| {
                    if let Some(caller) = caller {
                        write!(w, "[{}:{}] ", caller.file(), caller.line())?;
                    }
                    f(w)?;
                    writeln!(w, ": {:#}", e)
                });
                Err(LoggedError::default())
            }
        }
    }
}
