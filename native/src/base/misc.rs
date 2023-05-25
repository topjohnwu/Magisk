use std::cmp::min;
use std::ffi::CStr;
use std::fmt::Arguments;
use std::{fmt, slice};

pub fn copy_str(dest: &mut [u8], src: &[u8]) -> usize {
    let len = min(src.len(), dest.len() - 1);
    dest[..len].copy_from_slice(&src[..len]);
    dest[len] = b'\0';
    len
}

struct BufFmtWriter<'a> {
    buf: &'a mut [u8],
    used: usize,
}

impl<'a> BufFmtWriter<'a> {
    fn new(buf: &'a mut [u8]) -> Self {
        BufFmtWriter { buf, used: 0 }
    }
}

impl<'a> fmt::Write for BufFmtWriter<'a> {
    // The buffer should always be null terminated
    fn write_str(&mut self, s: &str) -> fmt::Result {
        if self.used >= self.buf.len() - 1 {
            // Silent truncate
            return Ok(());
        }
        self.used += copy_str(&mut self.buf[self.used..], s.as_bytes());
        // Silent truncate
        Ok(())
    }
}

pub fn fmt_to_buf(buf: &mut [u8], args: Arguments) -> usize {
    let mut w = BufFmtWriter::new(buf);
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
        unsafe { std::ffi::CStr::from_bytes_with_nul_unchecked(&$buf[..(len + 1)]) }
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
        unsafe { std::ffi::CStr::from_bytes_with_nul_unchecked(concat!($str, "\0").as_bytes()) }
    }};
}

#[macro_export]
macro_rules! raw_cstr {
    ($s:literal) => {{
        cstr!($s).as_ptr()
    }};
}

pub fn ptr_to_str<'a, T>(ptr: *const T) -> &'a str {
    if ptr.is_null() {
        "(null)"
    } else {
        unsafe { CStr::from_ptr(ptr.cast()) }.to_str().unwrap_or("")
    }
}

pub fn errno() -> &'static mut i32 {
    unsafe { &mut *libc::__errno() }
}

pub fn error_str() -> &'static str {
    unsafe { ptr_to_str(libc::strerror(*errno())) }
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

pub trait FlatData {
    fn as_raw_bytes(&self) -> &[u8]
    where
        Self: Sized,
    {
        unsafe {
            let self_ptr = self as *const Self as *const u8;
            slice::from_raw_parts(self_ptr, std::mem::size_of::<Self>())
        }
    }
    fn as_raw_bytes_mut(&mut self) -> &mut [u8]
    where
        Self: Sized,
    {
        unsafe {
            let self_ptr = self as *mut Self as *mut u8;
            slice::from_raw_parts_mut(self_ptr, std::mem::size_of::<Self>())
        }
    }
}
