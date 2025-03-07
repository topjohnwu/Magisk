use crate::{ffi, StrErr, Utf8CStr};
use argh::EarlyExit;
use libc::c_char;
use std::fmt::Arguments;
use std::io::Write;
use std::mem::ManuallyDrop;
use std::process::exit;
use std::sync::atomic::{AtomicPtr, Ordering};
use std::sync::Arc;
use std::{fmt, io, slice, str};

pub fn errno() -> &'static mut i32 {
    unsafe { &mut *libc::__errno() }
}

// When len is 0, don't care whether buf is null or not
#[inline]
pub unsafe fn slice_from_ptr<'a, T>(buf: *const T, len: usize) -> &'a [T] {
    unsafe {
        if len == 0 {
            &[]
        } else {
            slice::from_raw_parts(buf, len)
        }
    }
}

// When len is 0, don't care whether buf is null or not
#[inline]
pub unsafe fn slice_from_ptr_mut<'a, T>(buf: *mut T, len: usize) -> &'a mut [T] {
    unsafe {
        if len == 0 {
            &mut []
        } else {
            slice::from_raw_parts_mut(buf, len)
        }
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

pub trait BytesExt {
    fn find(&self, needle: &[u8]) -> Option<usize>;
    fn contains(&self, needle: &[u8]) -> bool {
        self.find(needle).is_some()
    }
}

impl<T: AsRef<[u8]> + ?Sized> BytesExt for T {
    fn find(&self, needle: &[u8]) -> Option<usize> {
        fn inner(haystack: &[u8], needle: &[u8]) -> Option<usize> {
            unsafe {
                let ptr: *const u8 = libc::memmem(
                    haystack.as_ptr().cast(),
                    haystack.len(),
                    needle.as_ptr().cast(),
                    needle.len(),
                )
                .cast();
                if ptr.is_null() {
                    None
                } else {
                    Some(ptr.offset_from(haystack.as_ptr()) as usize)
                }
            }
        }
        inner(self.as_ref(), needle)
    }
}

pub trait MutBytesExt {
    fn patch(&mut self, from: &[u8], to: &[u8]) -> Vec<usize>;
}

impl<T: AsMut<[u8]> + ?Sized> MutBytesExt for T {
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

pub struct FmtAdaptor<'a, T>(pub &'a mut T)
where
    T: Write;

impl<T: Write> fmt::Write for FmtAdaptor<'_, T> {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        self.0.write_all(s.as_bytes()).map_err(|_| fmt::Error)
    }
    fn write_fmt(&mut self, args: Arguments<'_>) -> fmt::Result {
        self.0.write_fmt(args).map_err(|_| fmt::Error)
    }
}

pub struct AtomicArc<T> {
    ptr: AtomicPtr<T>,
}

impl<T> AtomicArc<T> {
    pub fn new(arc: Arc<T>) -> AtomicArc<T> {
        let raw = Arc::into_raw(arc);
        Self {
            ptr: AtomicPtr::new(raw as *mut _),
        }
    }

    pub fn load(&self) -> Arc<T> {
        let raw = self.ptr.load(Ordering::Acquire);
        // SAFETY: the raw pointer is always created from Arc::into_raw
        let arc = ManuallyDrop::new(unsafe { Arc::from_raw(raw) });
        ManuallyDrop::into_inner(arc.clone())
    }

    fn swap_ptr(&self, raw: *const T) -> Arc<T> {
        let prev = self.ptr.swap(raw as *mut _, Ordering::AcqRel);
        // SAFETY: the raw pointer is always created from Arc::into_raw
        unsafe { Arc::from_raw(prev) }
    }

    pub fn swap(&self, arc: Arc<T>) -> Arc<T> {
        let raw = Arc::into_raw(arc);
        self.swap_ptr(raw)
    }

    pub fn store(&self, arc: Arc<T>) {
        // Drop the previous value
        let _ = self.swap(arc);
    }
}

impl<T> Drop for AtomicArc<T> {
    fn drop(&mut self) {
        // Drop the internal value
        let _ = self.swap_ptr(std::ptr::null());
    }
}

impl<T: Default> Default for AtomicArc<T> {
    fn default() -> Self {
        Self::new(Default::default())
    }
}
