use crate::logging::Formatter;
use crate::{LogLevel, log_with_args, log_with_formatter};
use nix::errno::Errno;
use std::fmt;
use std::fmt::Display;
use std::panic::Location;
use std::ptr::NonNull;

// Error handling throughout the Rust codebase in Magisk:
//
// All errors should be logged and consumed as soon as possible and converted into LoggedError.
// For `Result` with errors that implement the `Display` trait, use the `?` operator to
// log and convert to LoggedResult.
//
// To log an error with more information, use `ResultExt::log_with_msg()`.

#[derive(Default)]
pub struct LoggedError {}
pub type LoggedResult<T> = Result<T, LoggedError>;

#[macro_export]
macro_rules! log_err {
    () => {{
        Err($crate::LoggedError::default())
    }};
    ($($args:tt)+) => {{
        $crate::error!($($args)+);
        Err($crate::LoggedError::default())
    }};
}

// Any result or option can be silenced
pub trait SilentLogExt<T> {
    fn silent(self) -> LoggedResult<T>;
}

impl<T, E> SilentLogExt<T> for Result<T, E> {
    fn silent(self) -> LoggedResult<T> {
        self.map_err(|_| LoggedError::default())
    }
}

impl<T> SilentLogExt<T> for Option<T> {
    fn silent(self) -> LoggedResult<T> {
        self.ok_or_else(LoggedError::default)
    }
}

// Public API for logging results
pub trait ResultExt<T> {
    fn log(self) -> LoggedResult<T>;
    fn log_with_msg<F: FnOnce(Formatter) -> fmt::Result>(self, f: F) -> LoggedResult<T>;
    fn log_ok(self);
}

// Public API for converting Option to LoggedResult
pub trait OptionExt<T> {
    fn ok_or_log(self) -> LoggedResult<T>;
    fn ok_or_log_msg<F: FnOnce(Formatter) -> fmt::Result>(self, f: F) -> LoggedResult<T>;
}

impl<T> OptionExt<T> for Option<T> {
    #[inline(always)]
    fn ok_or_log(self) -> LoggedResult<T> {
        self.ok_or_else(LoggedError::default)
    }

    #[cfg(not(debug_assertions))]
    fn ok_or_log_msg<F: FnOnce(Formatter) -> fmt::Result>(self, f: F) -> LoggedResult<T> {
        self.ok_or_else(|| {
            do_log_msg(LogLevel::Error, None, f);
            LoggedError::default()
        })
    }

    #[track_caller]
    #[cfg(debug_assertions)]
    fn ok_or_log_msg<F: FnOnce(Formatter) -> fmt::Result>(self, f: F) -> LoggedResult<T> {
        let caller = Some(Location::caller());
        self.ok_or_else(|| {
            do_log_msg(LogLevel::Error, caller, f);
            LoggedError::default()
        })
    }
}

trait Loggable {
    fn do_log(self, level: LogLevel, caller: Option<&'static Location>) -> LoggedError;
    fn do_log_msg<F: FnOnce(Formatter) -> fmt::Result>(
        self,
        level: LogLevel,
        caller: Option<&'static Location>,
        f: F,
    ) -> LoggedError;
}

impl<T, E: Loggable> ResultExt<T> for Result<T, E> {
    #[cfg(not(debug_assertions))]
    fn log(self) -> LoggedResult<T> {
        self.map_err(|e| e.do_log(LogLevel::Error, None))
    }

    #[track_caller]
    #[cfg(debug_assertions)]
    fn log(self) -> LoggedResult<T> {
        let caller = Some(Location::caller());
        self.map_err(|e| e.do_log(LogLevel::Error, caller))
    }

    #[cfg(not(debug_assertions))]
    fn log_with_msg<F: FnOnce(Formatter) -> fmt::Result>(self, f: F) -> LoggedResult<T> {
        self.map_err(|e| e.do_log_msg(LogLevel::Error, None, f))
    }

    #[track_caller]
    #[cfg(debug_assertions)]
    fn log_with_msg<F: FnOnce(Formatter) -> fmt::Result>(self, f: F) -> LoggedResult<T> {
        let caller = Some(Location::caller());
        self.map_err(|e| e.do_log_msg(LogLevel::Error, caller, f))
    }

    #[cfg(not(debug_assertions))]
    fn log_ok(self) {
        self.map_err(|e| e.do_log(LogLevel::Error, None)).ok();
    }

    #[track_caller]
    #[cfg(debug_assertions)]
    fn log_ok(self) {
        let caller = Some(Location::caller());
        self.map_err(|e| e.do_log(LogLevel::Error, caller)).ok();
    }
}

impl<T> ResultExt<T> for LoggedResult<T> {
    fn log(self) -> LoggedResult<T> {
        self
    }

    #[cfg(not(debug_assertions))]
    fn log_with_msg<F: FnOnce(Formatter) -> fmt::Result>(self, f: F) -> LoggedResult<T> {
        self.inspect_err(|_| do_log_msg(LogLevel::Error, None, f))
    }

    #[track_caller]
    #[cfg(debug_assertions)]
    fn log_with_msg<F: FnOnce(Formatter) -> fmt::Result>(self, f: F) -> LoggedResult<T> {
        let caller = Some(Location::caller());
        self.inspect_err(|_| do_log_msg(LogLevel::Error, caller, f))
    }

    fn log_ok(self) {}
}

// Allow converting Loggable errors to LoggedError to support `?` operator
impl<T: Loggable> From<T> for LoggedError {
    #[cfg(not(debug_assertions))]
    fn from(e: T) -> Self {
        e.do_log(LogLevel::Error, None)
    }

    #[track_caller]
    #[cfg(debug_assertions)]
    fn from(e: T) -> Self {
        let caller = Some(Location::caller());
        e.do_log(LogLevel::Error, caller)
    }
}

// Actual logging implementation

// Make all printable objects Loggable
impl<T: Display> Loggable for T {
    fn do_log(self, level: LogLevel, caller: Option<&'static Location>) -> LoggedError {
        if let Some(caller) = caller {
            log_with_args!(level, "[{}:{}] {:#}", caller.file(), caller.line(), self);
        } else {
            log_with_args!(level, "{:#}", self);
        }
        LoggedError::default()
    }

    fn do_log_msg<F: FnOnce(Formatter) -> fmt::Result>(
        self,
        level: LogLevel,
        caller: Option<&'static Location>,
        f: F,
    ) -> LoggedError {
        log_with_formatter(level, |w| {
            if let Some(caller) = caller {
                write!(w, "[{}:{}] ", caller.file(), caller.line())?;
            }
            f(w)?;
            writeln!(w, ": {self:#}")
        });
        LoggedError::default()
    }
}

fn do_log_msg<F: FnOnce(Formatter) -> fmt::Result>(
    level: LogLevel,
    caller: Option<&'static Location>,
    f: F,
) {
    log_with_formatter(level, |w| {
        if let Some(caller) = caller {
            write!(w, "[{}:{}] ", caller.file(), caller.line())?;
        }
        f(w)?;
        w.write_char('\n')
    });
}

// Check libc return value and map to Result
pub trait LibcReturn
where
    Self: Sized,
{
    type Value;

    fn check_err(self) -> nix::Result<Self::Value>;

    fn into_os_result<'a>(
        self,
        name: &'static str,
        arg1: Option<&'a str>,
        arg2: Option<&'a str>,
    ) -> OsResult<'a, Self::Value> {
        self.check_err()
            .map_err(|e| OsError::new(e, name, arg1, arg2))
    }

    fn check_os_err<'a>(
        self,
        name: &'static str,
        arg1: Option<&'a str>,
        arg2: Option<&'a str>,
    ) -> OsResult<'a, ()> {
        self.check_err()
            .map(|_| ())
            .map_err(|e| OsError::new(e, name, arg1, arg2))
    }
}

macro_rules! impl_libc_return {
    ($($t:ty)*) => ($(
        impl LibcReturn for $t {
            type Value = Self;

            #[inline(always)]
            fn check_err(self) -> nix::Result<Self::Value> {
                if self < 0 {
                    Err(Errno::last())
                } else {
                    Ok(self)
                }
            }
        }
    )*)
}

impl_libc_return! { i8 i16 i32 i64 isize }

impl<T> LibcReturn for *mut T {
    type Value = NonNull<T>;

    #[inline(always)]
    fn check_err(self) -> nix::Result<Self::Value> {
        NonNull::new(self).ok_or_else(Errno::last)
    }
}

impl<T> LibcReturn for nix::Result<T> {
    type Value = T;

    #[inline(always)]
    fn check_err(self) -> Self {
        self
    }
}

#[derive(Debug)]
pub struct OsError<'a> {
    pub errno: Errno,
    name: &'static str,
    arg1: Option<&'a str>,
    arg2: Option<&'a str>,
}

impl OsError<'_> {
    pub fn new<'a>(
        errno: Errno,
        name: &'static str,
        arg1: Option<&'a str>,
        arg2: Option<&'a str>,
    ) -> OsError<'a> {
        OsError {
            errno,
            name,
            arg1,
            arg2,
        }
    }

    pub fn last_os_error<'a>(
        name: &'static str,
        arg1: Option<&'a str>,
        arg2: Option<&'a str>,
    ) -> OsError<'a> {
        Self::new(Errno::last(), name, arg1, arg2)
    }

    pub fn set_args<'a>(self, arg1: Option<&'a str>, arg2: Option<&'a str>) -> OsError<'a> {
        Self::new(self.errno, self.name, arg1, arg2)
    }
}

impl Display for OsError<'_> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if self.name.is_empty() {
            write!(f, "{}", self.errno)
        } else {
            match (self.arg1, self.arg2) {
                (Some(arg1), Some(arg2)) => {
                    write!(f, "{} '{arg1}' '{arg2}': {}", self.name, self.errno)
                }
                (Some(arg1), None) => {
                    write!(f, "{} '{arg1}': {}", self.name, self.errno)
                }
                _ => {
                    write!(f, "{}: {}", self.name, self.errno)
                }
            }
        }
    }
}

impl std::error::Error for OsError<'_> {}

pub type OsResult<'a, T> = Result<T, OsError<'a>>;
