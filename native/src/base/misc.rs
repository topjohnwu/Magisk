use std::cmp::min;
use std::ffi::{CStr, FromBytesWithNulError, OsStr};
use std::fmt::{Arguments, Debug, Display, Formatter};
use std::ops::Deref;
use std::path::Path;
use std::process::exit;
use std::str::Utf8Error;
use std::{fmt, io, mem, slice, str};

use argh::EarlyExit;
use libc::c_char;
use thiserror::Error;

use crate::ffi;

pub fn copy_str<T: AsRef<[u8]>>(dest: &mut [u8], src: T) -> usize {
    let src = src.as_ref();
    let len = min(src.len(), dest.len() - 1);
    dest[..len].copy_from_slice(&src[..len]);
    dest[len] = b'\0';
    len
}

pub fn copy_cstr<T: AsRef<CStr> + ?Sized>(dest: &mut [u8], src: &T) -> usize {
    let src = src.as_ref().to_bytes_with_nul();
    let len = min(src.len(), dest.len());
    dest[..len].copy_from_slice(&src[..len]);
    len - 1
}

pub struct BufFormatter<'a> {
    buf: &'a mut [u8],
    pub used: usize,
}

impl<'a> BufFormatter<'a> {
    pub fn new(buf: &'a mut [u8]) -> Self {
        BufFormatter { buf, used: 0 }
    }
}

impl<'a> fmt::Write for BufFormatter<'a> {
    // The buffer should always be null terminated
    fn write_str(&mut self, s: &str) -> fmt::Result {
        if self.used >= self.buf.len() - 1 {
            // Silent truncate
            return Ok(());
        }
        self.used += copy_str(&mut self.buf[self.used..], s);
        // Silent truncate
        Ok(())
    }
}

pub fn fmt_to_buf(buf: &mut [u8], args: Arguments) -> usize {
    let mut w = BufFormatter::new(buf);
    if let Ok(()) = fmt::write(&mut w, args) {
        w.used
    } else {
        0
    }
}

#[macro_export]
macro_rules! bfmt {
    ($buf:expr, $($args:tt)*) => {
        $crate::fmt_to_buf($buf, format_args!($($args)*));
    };
}

#[macro_export]
macro_rules! bfmt_cstr {
    ($buf:expr, $($args:tt)*) => {{
        let len = $crate::fmt_to_buf($buf, format_args!($($args)*));
        #[allow(unused_unsafe, clippy::unnecessary_mut_passed)]
        unsafe {
            $crate::Utf8CStr::from_bytes_unchecked($buf.get_unchecked(..(len + 1)))
        }
    }};
}

// The cstr! macro is copied from https://github.com/bytecodealliance/rustix/blob/main/src/cstr.rs

#[macro_export]
macro_rules! cstr {
    ($str:literal) => {{
        assert!(
            !$str.bytes().any(|b| b == b'\0'),
            "cstr argument contains embedded NUL bytes",
        );
        #[allow(unused_unsafe)]
        unsafe {
            $crate::Utf8CStr::from_bytes_unchecked(concat!($str, "\0").as_bytes())
        }
    }};
}

#[macro_export]
macro_rules! raw_cstr {
    ($s:literal) => {{
        cstr!($s).as_ptr()
    }};
}

#[derive(Debug, Error)]
pub enum StrErr {
    #[error(transparent)]
    Utf8Error(#[from] Utf8Error),
    #[error(transparent)]
    CStrError(#[from] FromBytesWithNulError),
    #[error("argument is null")]
    NullPointerError,
}

// The better CStr: UTF-8 validated + null terminated buffer
pub struct Utf8CStr {
    inner: [u8],
}

impl Utf8CStr {
    pub fn from_cstr(cstr: &CStr) -> Result<&Utf8CStr, StrErr> {
        // Validate the buffer during construction
        str::from_utf8(cstr.to_bytes())?;
        Ok(unsafe { Self::from_bytes_unchecked(cstr.to_bytes_with_nul()) })
    }

    pub fn from_bytes(buf: &[u8]) -> Result<&Utf8CStr, StrErr> {
        Self::from_cstr(CStr::from_bytes_with_nul(buf)?)
    }

    pub fn from_string(s: &mut String) -> &Utf8CStr {
        if s.capacity() == s.len() {
            s.reserve(1);
        }
        // SAFETY: the string is reserved to have enough capacity to fit in the null byte
        // SAFETY: the null byte is explicitly added outside of the string's length
        unsafe {
            let buf = slice::from_raw_parts_mut(s.as_mut_ptr(), s.len() + 1);
            *buf.get_unchecked_mut(s.len()) = b'\0';
            Self::from_bytes_unchecked(buf)
        }
    }

    #[inline]
    pub unsafe fn from_bytes_unchecked(buf: &[u8]) -> &Utf8CStr {
        mem::transmute(buf)
    }

    pub unsafe fn from_ptr<'a>(ptr: *const c_char) -> Result<&'a Utf8CStr, StrErr> {
        if ptr.is_null() {
            return Err(StrErr::NullPointerError);
        }
        Self::from_cstr(unsafe { CStr::from_ptr(ptr) })
    }

    #[inline]
    pub fn as_bytes(&self) -> &[u8] {
        // The length of the slice is at least 1 due to null termination check
        unsafe { self.inner.get_unchecked(..self.inner.len() - 1) }
    }

    #[inline]
    pub fn as_bytes_with_nul(&self) -> &[u8] {
        &self.inner
    }

    #[inline]
    pub fn as_ptr(&self) -> *const c_char {
        self.inner.as_ptr().cast()
    }

    #[inline]
    pub fn as_cstr(&self) -> &CStr {
        // SAFETY: Already validated as null terminated during construction
        unsafe { CStr::from_bytes_with_nul_unchecked(&self.inner) }
    }
}

impl Deref for Utf8CStr {
    type Target = str;

    #[inline]
    fn deref(&self) -> &str {
        // SAFETY: Already UTF-8 validated during construction
        unsafe { str::from_utf8_unchecked(self.as_bytes()) }
    }
}

impl Display for Utf8CStr {
    #[inline]
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        Display::fmt(self.deref(), f)
    }
}

impl Debug for Utf8CStr {
    #[inline]
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        Debug::fmt(self.deref(), f)
    }
}

impl AsRef<CStr> for Utf8CStr {
    #[inline]
    fn as_ref(&self) -> &CStr {
        self.as_cstr()
    }
}

impl AsRef<str> for Utf8CStr {
    #[inline]
    fn as_ref(&self) -> &str {
        self.deref()
    }
}

impl AsRef<OsStr> for Utf8CStr {
    #[inline]
    fn as_ref(&self) -> &OsStr {
        OsStr::new(self.deref())
    }
}

impl AsRef<Path> for Utf8CStr {
    #[inline]
    fn as_ref(&self) -> &Path {
        Path::new(self.deref())
    }
}

impl PartialEq<CStr> for Utf8CStr {
    #[inline]
    fn eq(&self, other: &CStr) -> bool {
        self.as_cstr() == other
    }
}

impl PartialEq<str> for Utf8CStr {
    #[inline]
    fn eq(&self, other: &str) -> bool {
        self.deref() == other
    }
}

impl PartialEq<Utf8CStr> for CStr {
    #[inline]
    fn eq(&self, other: &Utf8CStr) -> bool {
        self == other.as_cstr()
    }
}

impl PartialEq<Utf8CStr> for str {
    #[inline]
    fn eq(&self, other: &Utf8CStr) -> bool {
        self == other.deref()
    }
}

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

pub trait FlatData
where
    Self: Sized,
{
    fn as_raw_bytes(&self) -> &[u8] {
        unsafe {
            let self_ptr = self as *const Self as *const u8;
            slice::from_raw_parts(self_ptr, mem::size_of::<Self>())
        }
    }
    fn as_raw_bytes_mut(&mut self) -> &mut [u8] {
        unsafe {
            let self_ptr = self as *mut Self as *mut u8;
            slice::from_raw_parts_mut(self_ptr, mem::size_of::<Self>())
        }
    }

    fn bytes_size(&self) -> usize {
        mem::size_of::<Self>()
    }
}

macro_rules! impl_flat_data {
    ($($t:ty)*) => ($(impl FlatData for $t {})*)
}

impl_flat_data!(usize u8 u16 u32 u64 isize i8 i16 i32 i64);

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
        .map(|s| unsafe { Utf8CStr::from_ptr(*s) }.map(|s| s.deref()))
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
