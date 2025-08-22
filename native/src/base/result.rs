use crate::logging::Formatter;
use crate::{LogLevel, errno, log_with_args, log_with_formatter};
use std::fmt::Display;
use std::panic::Location;
use std::ptr::NonNull;
use std::{fmt, io};
use thiserror::Error;

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

// Internal C++ bridging logging routines
pub(crate) trait CxxResultExt<T> {
    fn log_cxx(self) -> LoggedResult<T>;
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

impl<T, E: Loggable> CxxResultExt<T> for Result<T, E> {
    fn log_cxx(self) -> LoggedResult<T> {
        self.map_err(|e| e.do_log(LogLevel::ErrorCxx, None))
    }
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
        do_log_msg(LogLevel::Error, None, f);
        self
    }

    #[track_caller]
    #[cfg(debug_assertions)]
    fn log_with_msg<F: FnOnce(Formatter) -> fmt::Result>(self, f: F) -> LoggedResult<T> {
        let caller = Some(Location::caller());
        do_log_msg(LogLevel::Error, caller, f);
        self
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
    Self: Copy,
{
    type Value;

    fn is_error(&self) -> bool;
    fn map_val(self) -> Self::Value;

    fn as_os_result<'a>(
        self,
        name: &'static str,
        arg1: Option<&'a str>,
        arg2: Option<&'a str>,
    ) -> OsResult<'a, Self::Value> {
        if self.is_error() {
            Err(OsError::last_os_error(name, arg1, arg2))
        } else {
            Ok(self.map_val())
        }
    }

    fn check_os_err<'a>(
        self,
        name: &'static str,
        arg1: Option<&'a str>,
        arg2: Option<&'a str>,
    ) -> OsResult<'a, ()> {
        if self.is_error() {
            Err(OsError::last_os_error(name, arg1, arg2))
        } else {
            Ok(())
        }
    }

    fn check_io_err(self) -> io::Result<()> {
        if self.is_error() {
            Err(io::Error::last_os_error())
        } else {
            Ok(())
        }
    }
}

macro_rules! impl_libc_return {
    ($($t:ty)*) => ($(
        impl LibcReturn for $t {
            type Value = Self;

            #[inline(always)]
            fn is_error(&self) -> bool {
                *self < 0
            }

            #[inline(always)]
            fn map_val(self) -> Self::Value {
                self
            }
        }
    )*)
}

impl_libc_return! { i8 i16 i32 i64 isize }

impl<T> LibcReturn for *mut T {
    type Value = NonNull<T>;

    #[inline(always)]
    fn is_error(&self) -> bool {
        self.is_null()
    }

    #[inline(always)]
    fn map_val(self) -> NonNull<T> {
        // SAFETY: pointer is null checked by is_error
        unsafe { NonNull::new_unchecked(self.cast()) }
    }
}

#[derive(Debug)]
enum OwnableStr<'a> {
    None,
    Borrowed(&'a str),
    Owned(Box<str>),
}

impl OwnableStr<'_> {
    fn into_owned(self) -> OwnableStr<'static> {
        match self {
            OwnableStr::None => OwnableStr::None,
            OwnableStr::Borrowed(s) => OwnableStr::Owned(Box::from(s)),
            OwnableStr::Owned(s) => OwnableStr::Owned(s),
        }
    }

    fn ok(&self) -> Option<&str> {
        match self {
            OwnableStr::None => None,
            OwnableStr::Borrowed(s) => Some(*s),
            OwnableStr::Owned(s) => Some(s),
        }
    }
}

impl<'a> From<Option<&'a str>> for OwnableStr<'a> {
    fn from(value: Option<&'a str>) -> Self {
        value.map(OwnableStr::Borrowed).unwrap_or(OwnableStr::None)
    }
}

#[derive(Debug)]
pub struct OsError<'a> {
    code: i32,
    name: &'static str,
    arg1: OwnableStr<'a>,
    arg2: OwnableStr<'a>,
}

impl OsError<'_> {
    pub fn with_os_error<'a>(
        code: i32,
        name: &'static str,
        arg1: Option<&'a str>,
        arg2: Option<&'a str>,
    ) -> OsError<'a> {
        OsError {
            code,
            name,
            arg1: OwnableStr::from(arg1),
            arg2: OwnableStr::from(arg2),
        }
    }

    pub fn last_os_error<'a>(
        name: &'static str,
        arg1: Option<&'a str>,
        arg2: Option<&'a str>,
    ) -> OsError<'a> {
        Self::with_os_error(*errno(), name, arg1, arg2)
    }

    pub fn set_args<'a>(self, arg1: Option<&'a str>, arg2: Option<&'a str>) -> OsError<'a> {
        Self::with_os_error(self.code, self.name, arg1, arg2)
    }

    pub fn into_owned(self) -> OsError<'static> {
        OsError {
            code: *errno(),
            name: self.name,
            arg1: self.arg1.into_owned(),
            arg2: self.arg2.into_owned(),
        }
    }

    fn as_io_error(&self) -> io::Error {
        io::Error::from_raw_os_error(self.code)
    }
}

impl Display for OsError<'_> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let error = self.as_io_error();
        if self.name.is_empty() {
            write!(f, "{error:#}")
        } else {
            match (self.arg1.ok(), self.arg2.ok()) {
                (Some(arg1), Some(arg2)) => {
                    write!(f, "{} '{}' '{}': {:#}", self.name, arg1, arg2, error)
                }
                (Some(arg1), None) => {
                    write!(f, "{} '{}': {:#}", self.name, arg1, error)
                }
                _ => {
                    write!(f, "{}: {:#}", self.name, error)
                }
            }
        }
    }
}

impl std::error::Error for OsError<'_> {}

pub type OsResult<'a, T> = Result<T, OsError<'a>>;

#[derive(Debug, Error)]
pub enum OsErrorStatic {
    #[error(transparent)]
    Os(OsError<'static>),
    #[error(transparent)]
    Io(#[from] io::Error),
}

// Convert non-static OsError to static
impl<'a> From<OsError<'a>> for OsErrorStatic {
    fn from(value: OsError<'a>) -> Self {
        OsErrorStatic::Os(value.into_owned())
    }
}

pub type OsResultStatic<T> = Result<T, OsErrorStatic>;
