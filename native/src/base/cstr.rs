use cxx::{type_id, ExternType};
use libc::c_char;
use std::borrow::Borrow;
use std::cmp::min;
use std::ffi::{CStr, FromBytesWithNulError, OsStr};
use std::fmt::{Debug, Display, Formatter, Write};
use std::ops::{Deref, DerefMut};
use std::os::unix::ffi::OsStrExt;
use std::path::{Path, PathBuf};
use std::str::Utf8Error;
use std::{fmt, mem, slice, str};
use thiserror::Error;

use crate::slice_from_ptr_mut;

// Utf8CStr types are UTF-8 validated and null terminated strings.
//
// Several Utf8CStr types:
//
// Utf8CStr: can only exist as reference, similar to &str
// Utf8CString: dynamically sized buffer allocated on the heap, similar to String
// Utf8CStrBufRef: reference to a fixed sized buffer
// Utf8CStrBufArr<N>: fixed sized buffer allocated on the stack
//
// For easier usage, please use the helper functions in cstr_buf.
//
// In most cases, these are the types being used
//
// &Utf8CStr: whenever a printable null terminated string is needed
// &mut dyn Utf8CStrBuf: whenever we need a buffer that needs to support appending
//                       strings to the end, and has to be null terminated
// &mut dyn Utf8CStrBuf: whenever we need a pre-allocated buffer that is large enough to fit
//                       in the result, and has to be null terminated
//
// All types dereferences to &Utf8CStr.
// Utf8CString, Utf8CStrBufRef, and Utf8CStrBufArr<N> implements Utf8CStrBuf.

// Public helper functions

pub mod cstr_buf {
    use super::{Utf8CStrBufArr, Utf8CStrBufRef, Utf8CString};

    #[inline(always)]
    pub fn with_capacity(capacity: usize) -> Utf8CString {
        Utf8CString::with_capacity(capacity)
    }

    #[inline(always)]
    pub fn default() -> Utf8CStrBufArr<4096> {
        Utf8CStrBufArr::default()
    }

    #[inline(always)]
    pub fn new<const N: usize>() -> Utf8CStrBufArr<N> {
        Utf8CStrBufArr::new()
    }

    #[inline(always)]
    pub fn wrap(buf: &mut [u8]) -> Utf8CStrBufRef {
        Utf8CStrBufRef::from(buf)
    }

    #[inline(always)]
    pub unsafe fn wrap_ptr<'a>(buf: *mut u8, len: usize) -> Utf8CStrBufRef<'a> {
        unsafe { Utf8CStrBufRef::from_ptr(buf, len) }
    }
}

// Trait definitions

pub trait Utf8CStrBuf:
    Write + AsRef<Utf8CStr> + AsMut<Utf8CStr> + Deref<Target = Utf8CStr> + DerefMut
{
    // The length of the string without the terminating null character.
    // assert_true(len <= capacity - 1)
    fn len(&self) -> usize;
    // Set the length of the string
    //
    // It is your responsibility to:
    // 1. Null terminate the string by setting the next byte after len to null
    // 2. Ensure len <= capacity - 1
    // 3. All bytes from 0 to len is valid UTF-8 and does not contain null
    unsafe fn set_len(&mut self, len: usize);
    fn push_str(&mut self, s: &str) -> usize;
    fn push_lossy(&mut self, s: &[u8]) -> usize;
    // The capacity of the internal buffer. The maximum string length this buffer can contain
    // is capacity - 1, because the last byte is reserved for the terminating null character.
    fn capacity(&self) -> usize;
    fn clear(&mut self);

    #[inline(always)]
    fn is_empty(&self) -> bool {
        self.len() == 0
    }
}

trait Utf8CStrBufWithSlice: Utf8CStrBuf {
    fn buf(&self) -> &[u8];
    unsafe fn mut_buf(&mut self) -> &mut [u8];
}

trait AsUtf8CStr {
    fn as_utf8_cstr(&self) -> &Utf8CStr;
    fn as_utf8_cstr_mut(&mut self) -> &mut Utf8CStr;
}

impl<T: Utf8CStrBufWithSlice> AsUtf8CStr for T {
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

// Implementation for Utf8CString

fn utf8_cstr_buf_append(buf: &mut dyn Utf8CStrBufWithSlice, s: &[u8]) -> usize {
    let mut used = buf.len();
    if used >= buf.capacity() - 1 {
        // Truncate
        return 0;
    }
    let dest = unsafe { &mut buf.mut_buf()[used..] };
    let len = min(s.len(), dest.len() - 1);
    if len > 0 {
        dest[..len].copy_from_slice(&s[..len]);
    }
    dest[len] = b'\0';
    used += len;
    unsafe { buf.set_len(used) };
    len
}

fn utf8_cstr_append_lossy(buf: &mut dyn Utf8CStrBuf, s: &[u8]) -> usize {
    let mut len = 0_usize;
    for chunk in s.utf8_chunks() {
        len += buf.push_str(chunk.valid());
        if !chunk.invalid().is_empty() {
            len += buf.push_str(char::REPLACEMENT_CHARACTER.encode_utf8(&mut [0; 4]));
        }
    }
    len
}

pub trait StringExt {
    fn nul_terminate(&mut self) -> &mut [u8];
}

impl StringExt for String {
    fn nul_terminate(&mut self) -> &mut [u8] {
        self.reserve(1);
        // SAFETY: the string is reserved to have enough capacity to fit in the null byte
        // SAFETY: the null byte is explicitly added outside the string's length
        unsafe {
            let buf = slice::from_raw_parts_mut(self.as_mut_ptr(), self.len() + 1);
            *buf.get_unchecked_mut(self.len()) = b'\0';
            buf
        }
    }
}

impl StringExt for PathBuf {
    #[allow(mutable_transmutes)]
    fn nul_terminate(&mut self) -> &mut [u8] {
        self.reserve(1);
        // SAFETY: the PathBuf is reserved to have enough capacity to fit in the null byte
        // SAFETY: the null byte is explicitly added outside the PathBuf's length
        unsafe {
            let bytes: &mut [u8] = mem::transmute(self.as_mut_os_str().as_bytes());
            let buf = slice::from_raw_parts_mut(bytes.as_mut_ptr(), bytes.len() + 1);
            *buf.get_unchecked_mut(bytes.len()) = b'\0';
            buf
        }
    }
}

pub struct Utf8CString(String);

impl Default for Utf8CString {
    fn default() -> Self {
        Utf8CString::with_capacity(256)
    }
}

impl Utf8CString {
    pub fn with_capacity(capacity: usize) -> Utf8CString {
        Utf8CString::from(String::with_capacity(capacity))
    }

    pub fn ensure_capacity(&mut self, capacity: usize) {
        if self.capacity() >= capacity {
            return;
        }
        self.0.reserve(capacity - self.0.len())
    }
}

impl AsUtf8CStr for Utf8CString {
    #[inline(always)]
    fn as_utf8_cstr(&self) -> &Utf8CStr {
        // SAFETY: the internal string is always null terminated
        unsafe { mem::transmute(slice::from_raw_parts(self.0.as_ptr(), self.0.len() + 1)) }
    }

    #[inline(always)]
    fn as_utf8_cstr_mut(&mut self) -> &mut Utf8CStr {
        // SAFETY: the internal string is always null terminated
        unsafe {
            mem::transmute(slice::from_raw_parts_mut(
                self.0.as_mut_ptr(),
                self.0.len() + 1,
            ))
        }
    }
}

impl Utf8CStrBuf for Utf8CString {
    #[inline(always)]
    fn len(&self) -> usize {
        self.0.len()
    }

    unsafe fn set_len(&mut self, len: usize) {
        unsafe {
            self.0.as_mut_vec().set_len(len);
        }
    }

    fn push_str(&mut self, s: &str) -> usize {
        self.0.push_str(s);
        self.0.nul_terminate();
        s.len()
    }

    #[inline(always)]
    fn push_lossy(&mut self, s: &[u8]) -> usize {
        utf8_cstr_append_lossy(self, s)
    }

    fn capacity(&self) -> usize {
        self.0.capacity()
    }

    fn clear(&mut self) {
        self.0.clear();
        self.0.nul_terminate();
    }
}

impl From<String> for Utf8CString {
    fn from(mut value: String) -> Self {
        value.nul_terminate();
        Utf8CString(value)
    }
}

impl Borrow<Utf8CStr> for Utf8CString {
    fn borrow(&self) -> &Utf8CStr {
        self.deref()
    }
}

// UTF-8 validated + null terminated reference to buffer
pub struct Utf8CStrBufRef<'a> {
    used: usize,
    buf: &'a mut [u8],
}

impl<'a> Utf8CStrBufRef<'a> {
    pub unsafe fn from_ptr(buf: *mut u8, len: usize) -> Utf8CStrBufRef<'a> {
        unsafe { Self::from(slice_from_ptr_mut(buf, len)) }
    }
}

impl<'a> From<&'a mut [u8]> for Utf8CStrBufRef<'a> {
    fn from(buf: &'a mut [u8]) -> Utf8CStrBufRef<'a> {
        buf[0] = b'\0';
        Utf8CStrBufRef { used: 0, buf }
    }
}

impl Utf8CStrBufWithSlice for Utf8CStrBufRef<'_> {
    #[inline(always)]
    fn buf(&self) -> &[u8] {
        self.buf
    }

    #[inline(always)]
    unsafe fn mut_buf(&mut self) -> &mut [u8] {
        self.buf
    }
}

// UTF-8 validated + null terminated buffer on the stack
pub struct Utf8CStrBufArr<const N: usize> {
    used: usize,
    buf: [u8; N],
}

impl<const N: usize> Utf8CStrBufArr<N> {
    pub fn new() -> Self {
        Utf8CStrBufArr {
            used: 0,
            buf: [0; N],
        }
    }
}

impl<const N: usize> Utf8CStrBufWithSlice for Utf8CStrBufArr<N> {
    #[inline(always)]
    fn buf(&self) -> &[u8] {
        &self.buf
    }

    #[inline(always)]
    unsafe fn mut_buf(&mut self) -> &mut [u8] {
        &mut self.buf
    }
}

impl Default for Utf8CStrBufArr<4096> {
    fn default() -> Self {
        Utf8CStrBufArr::<4096>::new()
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

// UTF-8 validated + null terminated string slice
#[repr(transparent)]
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

    pub fn from_string(s: &mut String) -> &mut Utf8CStr {
        let buf = s.nul_terminate();
        // SAFETY: the null byte is explicitly added to the buffer
        unsafe { mem::transmute(buf) }
    }

    #[inline(always)]
    pub const unsafe fn from_bytes_unchecked(buf: &[u8]) -> &Utf8CStr {
        unsafe { mem::transmute(buf) }
    }

    #[inline(always)]
    unsafe fn from_bytes_unchecked_mut(buf: &mut [u8]) -> &mut Utf8CStr {
        unsafe { mem::transmute(buf) }
    }

    pub unsafe fn from_ptr<'a>(ptr: *const c_char) -> Result<&'a Utf8CStr, StrErr> {
        if ptr.is_null() {
            return Err(StrErr::NullPointerError);
        }
        Self::from_cstr(unsafe { CStr::from_ptr(ptr) })
    }

    pub unsafe fn from_ptr_unchecked<'a>(ptr: *const c_char) -> &'a Utf8CStr {
        unsafe {
            let cstr = CStr::from_ptr(ptr);
            Self::from_bytes_unchecked(cstr.to_bytes_with_nul())
        }
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

impl ToOwned for Utf8CStr {
    type Owned = Utf8CString;

    fn to_owned(&self) -> Utf8CString {
        let mut s = Utf8CString::with_capacity(self.len() + 1);
        s.push_str(self.as_str());
        s
    }
}

// Notice that we only implement ExternType on Utf8CStr *reference*
unsafe impl ExternType for &Utf8CStr {
    type Id = type_id!("rust::Utf8CStr");
    type Kind = cxx::kind::Trivial;
}

macro_rules! const_assert_eq {
    ($left:expr, $right:expr $(,)?) => {
        const _: [(); $left] = [(); $right];
    };
}

// Assert ABI layout
const_assert_eq!(size_of::<&Utf8CStr>(), size_of::<[usize; 2]>());
const_assert_eq!(align_of::<&Utf8CStr>(), align_of::<[usize; 2]>());

// File system path extensions types

#[repr(transparent)]
pub struct FsPath(Utf8CStr);

impl FsPath {
    #[inline(always)]
    pub fn from<T: AsRef<Utf8CStr> + ?Sized>(value: &T) -> &FsPath {
        unsafe { mem::transmute(value.as_ref()) }
    }

    #[inline(always)]
    pub fn from_mut<T: AsMut<Utf8CStr> + ?Sized>(value: &mut T) -> &mut FsPath {
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

#[repr(transparent)]
pub struct FsPathFollow(Utf8CStr);

impl Deref for FsPathFollow {
    type Target = Utf8CStr;

    #[inline(always)]
    fn deref(&self) -> &Utf8CStr {
        &self.0
    }
}

impl DerefMut for FsPathFollow {
    #[inline(always)]
    fn deref_mut(&mut self) -> &mut Utf8CStr {
        &mut self.0
    }
}

enum Utf8CStrBufOwned<const N: usize> {
    Dynamic(Utf8CString),
    Fixed(Utf8CStrBufArr<N>),
}

impl<const N: usize> Deref for Utf8CStrBufOwned<N> {
    type Target = dyn Utf8CStrBuf;

    fn deref(&self) -> &Self::Target {
        match self {
            Utf8CStrBufOwned::Dynamic(s) => s,
            Utf8CStrBufOwned::Fixed(arr) => arr,
        }
    }
}

impl<const N: usize> DerefMut for Utf8CStrBufOwned<N> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        match self {
            Utf8CStrBufOwned::Dynamic(s) => s,
            Utf8CStrBufOwned::Fixed(arr) => arr,
        }
    }
}

pub struct FsPathBuf<const N: usize>(Utf8CStrBufOwned<N>);

impl FsPathBuf<0> {
    pub fn new_dynamic(capacity: usize) -> Self {
        FsPathBuf(Utf8CStrBufOwned::Dynamic(Utf8CString::with_capacity(
            capacity,
        )))
    }
}

impl Default for FsPathBuf<4096> {
    fn default() -> Self {
        FsPathBuf(Utf8CStrBufOwned::Fixed(cstr_buf::default()))
    }
}

impl<const N: usize> FsPathBuf<N> {
    pub fn new() -> Self {
        FsPathBuf(Utf8CStrBufOwned::Fixed(cstr_buf::new::<N>()))
    }

    pub fn clear(&mut self) {
        self.0.clear();
    }

    pub fn join<T: AsRef<str>>(mut self, path: T) -> Self {
        fn inner(buf: &mut dyn Utf8CStrBuf, path: &str) {
            if path.starts_with('/') {
                buf.clear();
            }
            if !buf.is_empty() && !buf.ends_with('/') {
                buf.push_str("/");
            }
            buf.push_str(path);
        }
        inner(self.0.deref_mut(), path.as_ref());
        self
    }

    pub fn join_fmt<T: Display>(mut self, name: T) -> Self {
        self.0.write_fmt(format_args!("/{}", name)).ok();
        self
    }
}

impl<const N: usize> Deref for FsPathBuf<N> {
    type Target = FsPath;

    fn deref(&self) -> &FsPath {
        FsPath::from(self.0.deref())
    }
}

impl<const N: usize> DerefMut for FsPathBuf<N> {
    fn deref_mut(&mut self) -> &mut FsPath {
        FsPath::from_mut(self.0.deref_mut())
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
    (FsPathFollow,)
    (FsPathBuf<N>, const N: usize)
    (Utf8CStrBufRef<'_>,)
    (Utf8CStrBufArr<N>, const N: usize)
    (Utf8CString,)
);

macro_rules! impl_str_buf {
    ($( ($t:ty, $($g:tt)*) )*) => {$(
        impl<$($g)*> Write for $t {
            #[inline(always)]
            fn write_str(&mut self, s: &str) -> fmt::Result {
                self.push_str(s);
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
                self.as_utf8_cstr_mut()
            }
        }
    )*}
}

impl_str_buf!(
    (Utf8CStrBufRef<'_>,)
    (Utf8CStrBufArr<N>, const N: usize)
    (Utf8CString,)
);

macro_rules! impl_str_buf_with_slice {
    ($( ($t:ty, $($g:tt)*) )*) => {$(
        impl<$($g)*> Utf8CStrBuf for $t {
            #[inline(always)]
            fn len(&self) -> usize {
                self.used
            }
            #[inline(always)]
            unsafe fn set_len(&mut self, len: usize) {
                self.used = len;
            }
            #[inline(always)]
            fn push_str(&mut self, s: &str) -> usize {
                utf8_cstr_buf_append(self, s.as_bytes())
            }
            #[inline(always)]
            fn push_lossy(&mut self, s: &[u8]) -> usize {
                utf8_cstr_append_lossy(self, s)
            }
            #[inline(always)]
            fn capacity(&self) -> usize {
                self.buf.len()
            }
            #[inline(always)]
            fn clear(&mut self) {
                self.buf[0] = b'\0';
                self.used = 0;
            }
        }
    )*}
}

impl_str_buf_with_slice!(
    (Utf8CStrBufRef<'_>,)
    (Utf8CStrBufArr<N>, const N: usize)
);

#[macro_export]
macro_rules! cstr {
    ($str:expr) => {{
        const NULL_STR: &str = $crate::const_format::concatcp!($str, "\0");
        #[allow(unused_unsafe)]
        unsafe {
            $crate::Utf8CStr::from_bytes_unchecked(NULL_STR.as_bytes())
        }
    }};
}

#[macro_export]
macro_rules! raw_cstr {
    ($str:expr) => {{
        $crate::cstr!($str).as_ptr()
    }};
}

#[macro_export]
macro_rules! path {
    ($str:expr) => {{
        $crate::FsPath::from($crate::cstr!($str))
    }};
}
