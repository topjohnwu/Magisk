use std::fmt;
use std::fmt::Display;
use std::panic::Location;

use crate::logging::Formatter;
use crate::{log_with_args, log_with_formatter, LogLevel};

// Error handling throughout the Rust codebase in Magisk:
//
// All errors should be logged and consumed as soon as possible and converted into LoggedError.
// For `Result` with errors that implement the `Display` trait, use the `?` operator to
// log and convert to LoggedResult.
//
// To log an error with more information, use `ResultExt::log_with_msg()`.
//
// The "cxx" method variants in `CxxResultExt` are only used for C++ interop and
// should not be used directly in any Rust code.

#[derive(Default)]
pub struct LoggedError {}
pub type LoggedResult<T> = Result<T, LoggedError>;

#[macro_export]
macro_rules! log_err {
    ($($args:tt)+) => {{
        $crate::log_with_args($crate::LogLevel::Error, format_args_nl!($($args)+));
        $crate::LoggedError::default()
    }};
}

// Any result or option can be silenced
pub trait SilentResultExt<T> {
    fn silent(self) -> LoggedResult<T>;
}

impl<T, E> SilentResultExt<T> for Result<T, E> {
    fn silent(self) -> LoggedResult<T> {
        match self {
            Ok(v) => Ok(v),
            Err(_) => Err(LoggedError::default()),
        }
    }
}

impl<T> SilentResultExt<T> for Option<T> {
    fn silent(self) -> LoggedResult<T> {
        match self {
            Some(v) => Ok(v),
            None => Err(LoggedError::default()),
        }
    }
}

// Public API for logging results
pub trait ResultExt<T> {
    fn log(self) -> LoggedResult<T>;
    fn log_with_msg<F: FnOnce(Formatter) -> fmt::Result>(self, f: F) -> LoggedResult<T>;
    fn log_ok(self);
}

// Internal C++ bridging logging routines
pub(crate) trait CxxResultExt<T> {
    fn log_cxx(self) -> LoggedResult<T>;
    fn log_cxx_with_msg<F: FnOnce(Formatter) -> fmt::Result>(self, f: F) -> LoggedResult<T>;
}

trait Loggable<T> {
    fn do_log(self, level: LogLevel, caller: Option<&'static Location>) -> LoggedResult<T>;
    fn do_log_msg<F: FnOnce(Formatter) -> fmt::Result>(
        self,
        level: LogLevel,
        caller: Option<&'static Location>,
        f: F,
    ) -> LoggedResult<T>;
}

impl<T, R: Loggable<T>> CxxResultExt<T> for R {
    fn log_cxx(self) -> LoggedResult<T> {
        self.do_log(LogLevel::ErrorCxx, None)
    }

    fn log_cxx_with_msg<F: FnOnce(Formatter) -> fmt::Result>(self, f: F) -> LoggedResult<T> {
        self.do_log_msg(LogLevel::ErrorCxx, None, f)
    }
}

impl<T, R: Loggable<T>> ResultExt<T> for R {
    #[cfg(not(debug_assertions))]
    fn log(self) -> LoggedResult<T> {
        self.do_log(LogLevel::Error, None)
    }

    #[track_caller]
    #[cfg(debug_assertions)]
    fn log(self) -> LoggedResult<T> {
        self.do_log(LogLevel::Error, Some(Location::caller()))
    }

    #[cfg(not(debug_assertions))]
    fn log_with_msg<F: FnOnce(Formatter) -> fmt::Result>(self, f: F) -> LoggedResult<T> {
        self.do_log_msg(LogLevel::Error, None, f)
    }

    #[track_caller]
    #[cfg(debug_assertions)]
    fn log_with_msg<F: FnOnce(Formatter) -> fmt::Result>(self, f: F) -> LoggedResult<T> {
        self.do_log_msg(LogLevel::Error, Some(Location::caller()), f)
    }

    #[cfg(not(debug_assertions))]
    fn log_ok(self) {
        self.log().ok();
    }

    #[track_caller]
    #[cfg(debug_assertions)]
    fn log_ok(self) {
        self.do_log(LogLevel::Error, Some(Location::caller())).ok();
    }
}

impl<T> Loggable<T> for LoggedResult<T> {
    fn do_log(self, _: LogLevel, _: Option<&'static Location>) -> LoggedResult<T> {
        self
    }

    fn do_log_msg<F: FnOnce(Formatter) -> fmt::Result>(
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

impl<T, E: Display> Loggable<T> for Result<T, E> {
    fn do_log(self, level: LogLevel, caller: Option<&'static Location>) -> LoggedResult<T> {
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

    fn do_log_msg<F: FnOnce(Formatter) -> fmt::Result>(
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

// Automatically convert all printable errors to LoggedError to support `?` operator
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
