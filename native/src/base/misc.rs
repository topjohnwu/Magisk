use std::process::exit;
use std::{io, slice, str};

use argh::EarlyExit;
use libc::c_char;

use crate::{ffi, StrErr, Utf8CStr};

pub fn errno() -> &'static mut i32 {
    unsafe { &mut *libc::__errno() }
}

// When len is 0, don't care whether buf is null or not
#[inline]
pub unsafe fn slice_from_ptr<'a, T>(buf: *const T, len: usize) -> &'a [T] {
    if len == 0 {
        &[]
    } else {
        slice::from_raw_parts(buf, len)
    }
}

// When len is 0, don't care whether buf is null or not
#[inline]
pub unsafe fn slice_from_ptr_mut<'a, T>(buf: *mut T, len: usize) -> &'a mut [T] {
    if len == 0 {
        &mut []
    } else {
        slice::from_raw_parts_mut(buf, len)
    }
}

// Check libc return value and map to Result
pub trait LibcReturn
where
    Self: Copy,
{
    fn is_error(&self) -> bool;
    fn check_os_err(self) -> io::Result<Self> {
        if self.is_error() {
            Err(io::Error::last_os_error())
        } else {
            Ok(self)
        }
    }
    fn as_os_err(self) -> io::Result<()> {
        self.check_os_err()?;
        Ok(())
    }
}

macro_rules! impl_libc_return {
    ($($t:ty)*) => ($(
        impl LibcReturn for $t {
            #[inline]
            fn is_error(&self) -> bool {
                *self < 0
            }
        }
    )*)
}

impl_libc_return! { i8 i16 i32 i64 isize }

impl<T> LibcReturn for *const T {
    #[inline]
    fn is_error(&self) -> bool {
        self.is_null()
    }
}

impl<T> LibcReturn for *mut T {
    #[inline]
    fn is_error(&self) -> bool {
        self.is_null()
    }
}

pub trait MutBytesExt {
    fn patch(&mut self, from: &[u8], to: &[u8]) -> Vec<usize>;
}

impl<T: AsMut<[u8]>> MutBytesExt for T {
    fn patch(&mut self, from: &[u8], to: &[u8]) -> Vec<usize> {
        ffi::mut_u8_patch(self.as_mut(), from, to)
    }
}

// SAFETY: libc guarantees argc and argv are properly setup and are static
#[allow(clippy::not_unsafe_ptr_arg_deref)]
pub fn map_args(argc: i32, argv: *const *const c_char) -> Result<Vec<&'static str>, StrErr> {
    unsafe { slice::from_raw_parts(argv, argc as usize) }
        .iter()
        .map(|s| unsafe { Utf8CStr::from_ptr(*s) }.map(|s| s.as_str()))
        .collect()
}

pub trait EarlyExitExt<T> {
    fn on_early_exit<F: FnOnce()>(self, print_help_msg: F) -> T;
}

impl<T> EarlyExitExt<T> for Result<T, EarlyExit> {
    fn on_early_exit<F: FnOnce()>(self, print_help_msg: F) -> T {
        match self {
            Ok(t) => t,
            Err(EarlyExit { output, status }) => match status {
                Ok(_) => {
                    print_help_msg();
                    exit(0)
                }
                Err(_) => {
                    eprintln!("{}", output);
                    print_help_msg();
                    exit(1)
                }
            },
        }
    }
}
