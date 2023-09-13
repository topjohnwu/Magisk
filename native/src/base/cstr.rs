use std::cmp::min;
use std::ffi::{CStr, FromBytesWithNulError, OsStr};
use std::fmt::{Arguments, Debug, Display, Formatter, Write};
use std::ops::{Deref, DerefMut};
use std::path::Path;
use std::str::{Utf8Chunks, Utf8Error};
use std::{fmt, mem, slice, str};

use crate::slice_from_ptr_mut;
use libc::c_char;
use thiserror::Error;

pub fn copy_cstr<T: AsRef<CStr> + ?Sized>(dest: &mut [u8], src: &T) -> usize {
    let src = src.as_ref().to_bytes_with_nul();
    let len = min(src.len(), dest.len());
    dest[..len].copy_from_slice(&src[..len]);
    len - 1
}

fn utf8_cstr_append(buf: &mut dyn Utf8CStrBuf, s: &[u8]) -> usize {
    let mut used = buf.len();
    if used >= buf.capacity() - 1 {
        // Truncate
        return 0;
    }
    let dest = unsafe { &mut buf.mut_buf()[used..] };
    let len = min(s.len(), dest.len() - 1);
    dest[..len].copy_from_slice(&s[..len]);
    dest[len] = b'\0';
    used += len;
    unsafe { buf.set_len(used) };
    len
}

fn utf8_cstr_append_lossy(buf: &mut dyn Utf8CStrBuf, s: &[u8]) -> usize {
    let chunks = Utf8Chunks::new(s);
    let mut len = 0_usize;
    for chunk in chunks {
        len += utf8_cstr_append(buf, chunk.valid().as_bytes());
        if !chunk.invalid().is_empty() {
            len += utf8_cstr_append(
                buf,
                char::REPLACEMENT_CHARACTER
                    .encode_utf8(&mut [0; 4])
                    .as_bytes(),
            );
        }
    }
    len
}

pub trait Utf8CStrBuf:
    Write + AsRef<Utf8CStr> + AsMut<Utf8CStr> + Deref<Target = Utf8CStr> + DerefMut
{
    fn buf(&self) -> &[u8];
    fn len(&self) -> usize;

    // Modifying the underlying buffer or length is unsafe because it can either:
    // 1. Break null termination
    // 2. Break UTF-8 validation
    // 3. Introduce inner null byte in the string
    unsafe fn mut_buf(&mut self) -> &mut [u8];
    unsafe fn set_len(&mut self, len: usize);

    fn append(&mut self, s: &str) -> usize;
    fn append_lossy(&mut self, s: &[u8]) -> usize;
    unsafe fn append_unchecked(&mut self, s: &[u8]) -> usize;

    #[inline(always)]
    fn is_empty(&self) -> bool {
        self.len() == 0
    }

    #[inline(always)]
    fn capacity(&self) -> usize {
        self.buf().len()
    }

    fn clear(&mut self) {
        unsafe {
            self.mut_buf()[0] = b'\0';
            self.set_len(0);
        }
    }
}

trait Utf8CStrBufImpl: Utf8CStrBuf {
    #[inline(always)]
    fn as_utf8_cstr(&self) -> &Utf8CStr {
        // SAFETY: the internal buffer is always UTF-8 checked
        // SAFETY: self.used is guaranteed to always <= SIZE - 1
        unsafe { Utf8CStr::from_bytes_unchecked(self.buf().get_unchecked(..(self.len() + 1))) }
    }

    #[inline(always)]
    fn as_utf8_cstr_mut(&mut self) -> &mut Utf8CStr {
        // SAFETY: the internal buffer is always UTF-8 checked
        // SAFETY: self.used is guaranteed to always <= SIZE - 1
        unsafe {
            let len = self.len() + 1;
            Utf8CStr::from_bytes_unchecked_mut(self.mut_buf().get_unchecked_mut(..len))
        }
    }
}

// UTF-8 validated + null terminated reference to buffer
pub struct Utf8CStrSlice<'a> {
    used: usize,
    buf: &'a mut [u8],
}

impl<'a> Utf8CStrSlice<'a> {
    pub unsafe fn from_ptr(buf: *mut u8, len: usize) -> Utf8CStrSlice<'a> {
        Self::from(slice_from_ptr_mut(buf, len))
    }
}

impl<'a> From<&'a mut [u8]> for Utf8CStrSlice<'a> {
    fn from(buf: &'a mut [u8]) -> Utf8CStrSlice<'a> {
        buf[0] = b'\0';
        Utf8CStrSlice { used: 0, buf }
    }
}

impl Utf8CStrBuf for Utf8CStrSlice<'_> {
    #[inline(always)]
    fn buf(&self) -> &[u8] {
        self.buf
    }

    #[inline(always)]
    fn len(&self) -> usize {
        self.used
    }

    #[inline(always)]
    unsafe fn mut_buf(&mut self) -> &mut [u8] {
        self.buf
    }

    #[inline(always)]
    unsafe fn set_len(&mut self, len: usize) {
        self.used = len;
    }

    #[inline(always)]
    fn append(&mut self, s: &str) -> usize {
        utf8_cstr_append(self, s.as_bytes())
    }

    #[inline(always)]
    fn append_lossy(&mut self, s: &[u8]) -> usize {
        utf8_cstr_append_lossy(self, s)
    }

    #[inline(always)]
    unsafe fn append_unchecked(&mut self, s: &[u8]) -> usize {
        utf8_cstr_append(self, s)
    }
}

// UTF-8 validated + null terminated buffer on the stack
pub struct Utf8CStrArr<const N: usize> {
    used: usize,
    buf: [u8; N],
}

impl<const N: usize> Utf8CStrArr<N> {
    pub fn new() -> Self {
        Utf8CStrArr {
            used: 0,
            buf: [0; N],
        }
    }
}

impl<const N: usize> Utf8CStrBuf for Utf8CStrArr<N> {
    #[inline(always)]
    fn buf(&self) -> &[u8] {
        &self.buf
    }

    #[inline(always)]
    fn len(&self) -> usize {
        self.used
    }

    #[inline(always)]
    unsafe fn mut_buf(&mut self) -> &mut [u8] {
        &mut self.buf
    }

    #[inline(always)]
    unsafe fn set_len(&mut self, len: usize) {
        self.used = len;
    }

    #[inline(always)]
    fn append(&mut self, s: &str) -> usize {
        utf8_cstr_append(self, s.as_bytes())
    }

    #[inline(always)]
    fn append_lossy(&mut self, s: &[u8]) -> usize {
        utf8_cstr_append_lossy(self, s)
    }

    #[inline(always)]
    unsafe fn append_unchecked(&mut self, s: &[u8]) -> usize {
        utf8_cstr_append(self, s)
    }

    #[inline(always)]
    fn capacity(&self) -> usize {
        N
    }
}

impl Default for Utf8CStrArr<4096> {
    fn default() -> Self {
        Utf8CStrArr::<4096>::new()
    }
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

// Provide a way to have a heap allocated, UTF-8 validated, and null terminated string
pub trait StringExt {
    fn nul_terminate(&mut self) -> &mut [u8];
}

impl StringExt for String {
    fn nul_terminate(&mut self) -> &mut [u8] {
        self.reserve(1);
        // SAFETY: the string is reserved to have enough capacity to fit in the null byte
        // SAFETY: the null byte is explicitly added outside of the string's length
        unsafe {
            let buf = slice::from_raw_parts_mut(self.as_mut_ptr(), self.len() + 1);
            *buf.get_unchecked_mut(self.len()) = b'\0';
            buf
        }
    }
}

// UTF-8 validated + null terminated string slice
pub struct Utf8CStr([u8]);

impl Utf8CStr {
    pub fn from_cstr(cstr: &CStr) -> Result<&Utf8CStr, StrErr> {
        // Validate the buffer during construction
        str::from_utf8(cstr.to_bytes())?;
        Ok(unsafe { Self::from_bytes_unchecked(cstr.to_bytes_with_nul()) })
    }

    pub fn from_bytes(buf: &[u8]) -> Result<&Utf8CStr, StrErr> {
        Self::from_cstr(CStr::from_bytes_with_nul(buf)?)
    }

    pub fn from_bytes_mut(buf: &mut [u8]) -> Result<&mut Utf8CStr, StrErr> {
        CStr::from_bytes_with_nul(buf)?;
        str::from_utf8(buf)?;
        // Both condition checked
        unsafe { Ok(mem::transmute(buf)) }
    }

    pub fn from_string(s: &mut String) -> &mut Utf8CStr {
        let buf = s.nul_terminate();
        // SAFETY: the null byte is explicitly added to the buffer
        unsafe { mem::transmute(buf) }
    }

    #[inline(always)]
    pub unsafe fn from_bytes_unchecked(buf: &[u8]) -> &Utf8CStr {
        mem::transmute(buf)
    }

    #[inline(always)]
    pub unsafe fn from_bytes_unchecked_mut(buf: &mut [u8]) -> &mut Utf8CStr {
        mem::transmute(buf)
    }

    pub unsafe fn from_ptr<'a>(ptr: *const c_char) -> Result<&'a Utf8CStr, StrErr> {
        if ptr.is_null() {
            return Err(StrErr::NullPointerError);
        }
        Self::from_cstr(unsafe { CStr::from_ptr(ptr) })
    }

    #[inline(always)]
    pub fn as_bytes_with_nul(&self) -> &[u8] {
        &self.0
    }

    #[inline(always)]
    pub fn as_ptr(&self) -> *const c_char {
        self.0.as_ptr().cast()
    }

    #[inline(always)]
    pub fn as_mut_ptr(&mut self) -> *mut c_char {
        self.0.as_mut_ptr().cast()
    }

    #[inline(always)]
    pub fn as_cstr(&self) -> &CStr {
        // SAFETY: Already validated as null terminated during construction
        unsafe { CStr::from_bytes_with_nul_unchecked(&self.0) }
    }

    #[inline(always)]
    pub fn as_str(&self) -> &str {
        // SAFETY: Already UTF-8 validated during construction
        // SAFETY: The length of the slice is at least 1 due to null termination check
        unsafe { str::from_utf8_unchecked(self.0.get_unchecked(..self.0.len() - 1)) }
    }

    #[inline(always)]
    pub fn as_str_mut(&mut self) -> &mut str {
        // SAFETY: Already UTF-8 validated during construction
        // SAFETY: The length of the slice is at least 1 due to null termination check
        unsafe { str::from_utf8_unchecked_mut(self.0.get_unchecked_mut(..self.0.len() - 1)) }
    }
}

impl Deref for Utf8CStr {
    type Target = str;

    #[inline(always)]
    fn deref(&self) -> &str {
        self.as_str()
    }
}

impl DerefMut for Utf8CStr {
    #[inline(always)]
    fn deref_mut(&mut self) -> &mut Self::Target {
        self.as_str_mut()
    }
}

// File system path extensions types

pub struct FsPath(Utf8CStr);

impl FsPath {
    #[inline(always)]
    pub fn from<'a, T: AsRef<Utf8CStr>>(value: T) -> &'a FsPath {
        unsafe { mem::transmute(value.as_ref()) }
    }

    #[inline(always)]
    pub fn from_mut<'a, T: AsMut<Utf8CStr>>(mut value: T) -> &'a mut FsPath {
        unsafe { mem::transmute(value.as_mut()) }
    }
}

impl Deref for FsPath {
    type Target = Utf8CStr;

    #[inline(always)]
    fn deref(&self) -> &Utf8CStr {
        &self.0
    }
}

impl DerefMut for FsPath {
    #[inline(always)]
    fn deref_mut(&mut self) -> &mut Utf8CStr {
        &mut self.0
    }
}

pub struct FsPathBuf<'a>(&'a mut dyn Utf8CStrBuf);

impl<'a> FsPathBuf<'a> {
    pub fn new(value: &'a mut dyn Utf8CStrBuf) -> Self {
        FsPathBuf(value)
    }

    pub fn join<T: AsRef<str>>(self, path: T) -> Self {
        fn inner(buf: &mut dyn Utf8CStrBuf, path: &str) {
            if path.starts_with('/') {
                buf.clear();
            } else {
                buf.append("/");
            }
            buf.append(path);
        }
        inner(self.0, path.as_ref());
        self
    }

    pub fn join_fmt<T: Display>(self, name: T) -> Self {
        fn inner(buf: &mut dyn Utf8CStrBuf, path: Arguments) {
            buf.write_fmt(path).ok();
        }
        inner(self.0, format_args!("/{}", name));
        self
    }
}

impl<'a> Deref for FsPathBuf<'a> {
    type Target = FsPath;

    fn deref(&self) -> &FsPath {
        FsPath::from(&self.0)
    }
}

impl<'a> DerefMut for FsPathBuf<'a> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        FsPath::from_mut(&mut self.0)
    }
}

// Boilerplate trait implementations

macro_rules! impl_str {
    ($( ($t:ty, $($g:tt)*) )*) => {$(
        impl<$($g)*> AsRef<Utf8CStr> for $t {
            #[inline(always)]
            fn as_ref(&self) -> &Utf8CStr {
                self
            }
        }
        impl<$($g)*> AsRef<str> for $t {
            #[inline(always)]
            fn as_ref(&self) -> &str {
                self.as_str()
            }
        }
        impl<$($g)*> AsRef<CStr> for $t {
            #[inline(always)]
            fn as_ref(&self) -> &CStr {
                self.as_cstr()
            }
        }
        impl<$($g)*> AsRef<OsStr> for $t {
            #[inline(always)]
            fn as_ref(&self) -> &OsStr {
                OsStr::new(self.as_str())
            }
        }
        impl<$($g)*> AsRef<Path> for $t {
            #[inline(always)]
            fn as_ref(&self) -> &Path {
                Path::new(self.as_str())
            }
        }
        impl<$($g)*> Display for $t {
            #[inline(always)]
            fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
                Display::fmt(self.as_str(), f)
            }
        }
        impl<$($g)*> Debug for $t {
            #[inline(always)]
            fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
                Debug::fmt(self.as_str(), f)
            }
        }
        impl<$($g)*> PartialEq<str> for $t {
            #[inline(always)]
            fn eq(&self, other: &str) -> bool {
                self.as_str() == other
            }
        }
        impl<$($g)*> PartialEq<$t> for str {
            #[inline(always)]
            fn eq(&self, other: &$t) -> bool {
                self == other.as_str()
            }
        }
        impl<$($g)*> PartialEq<CStr> for $t {
            #[inline(always)]
            fn eq(&self, other: &CStr) -> bool {
                self.as_cstr() == other
            }
        }
        impl<$($g)*> PartialEq<$t> for CStr {
            #[inline(always)]
            fn eq(&self, other: &$t) -> bool {
                self == other.as_cstr()
            }
        }
        impl<T: AsRef<Utf8CStr>, $($g)*> PartialEq<T> for $t {
            #[inline(always)]
            fn eq(&self, other: &T) -> bool {
                self.as_bytes_with_nul() == other.as_ref().as_bytes_with_nul()
            }
        }
    )*}
}

impl_str!(
    (Utf8CStr,)
    (FsPath,)
    (FsPathBuf<'_>,)
);

macro_rules! impl_str_buf {
    ($( ($t:ty, $($g:tt)*) )*) => {$(
        impl_str!(($t, $($g)*));
        impl<$($g)*> $t {
            #[inline(always)]
            pub fn append<T: AsRef<str>>(&mut self, s: T) -> usize {
                utf8_cstr_append(self, s.as_ref().as_bytes())
            }
        }
        impl<$($g)*> Utf8CStrBufImpl for $t {}
        impl<$($g)*> Write for $t {
            #[inline(always)]
            fn write_str(&mut self, s: &str) -> fmt::Result {
                self.append(s);
                Ok(())
            }
        }
        impl<$($g)*> Deref for $t {
            type Target = Utf8CStr;

            #[inline(always)]
            fn deref(&self) -> &Utf8CStr {
                self.as_utf8_cstr()
            }
        }
        impl<$($g)*> DerefMut for $t {
            #[inline(always)]
            fn deref_mut(&mut self) -> &mut Utf8CStr {
                self.as_utf8_cstr_mut()
            }
        }
        impl<$($g)*> AsMut<Utf8CStr> for $t {
            #[inline(always)]
            fn as_mut(&mut self) -> &mut Utf8CStr {
                self
            }
        }
    )*}
}

impl_str_buf!(
    (Utf8CStrSlice<'_>,)
    (Utf8CStrArr<N>, const N: usize)
);

// The cstr! macro is copied from https://github.com/bytecodealliance/rustix/blob/main/src/cstr.rs

#[macro_export]
macro_rules! cstr {
    ($($str:tt)*) => {{
        assert!(
            !($($str)*).bytes().any(|b| b == b'\0'),
            "cstr argument contains embedded NUL bytes",
        );
        #[allow(unused_unsafe)]
        unsafe {
            $crate::Utf8CStr::from_bytes_unchecked(concat!($($str)*, "\0").as_bytes())
        }
    }};
}

#[macro_export]
macro_rules! raw_cstr {
    ($($str:tt)*) => {{
        cstr!($($str)*).as_ptr()
    }};
}
