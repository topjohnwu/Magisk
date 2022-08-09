use std::cmp::min;
use std::ffi::CStr;
use std::fmt;
use std::fmt::Arguments;

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
        let remain = &mut self.buf[self.used..];
        let s_bytes = s.as_bytes();
        let copied = min(s_bytes.len(), remain.len() - 1);
        remain[..copied].copy_from_slice(&s_bytes[..copied]);
        self.used += copied;
        self.buf[self.used] = b'\0';
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

// The cstr! macro is inspired by https://github.com/Nugine/const-str

macro_rules! const_assert {
    ($s: expr) => {
        assert!($s)
    };
}

pub struct ToCStr<'a>(pub &'a str);

impl ToCStr<'_> {
    const fn assert_no_nul(&self) {
        let bytes = self.0.as_bytes();
        let mut i = 0;
        while i < bytes.len() {
            const_assert!(bytes[i] != 0);
            i += 1;
        }
    }

    pub const fn eval_len(&self) -> usize {
        self.assert_no_nul();
        self.0.as_bytes().len() + 1
    }

    pub const fn eval_bytes<const N: usize>(&self) -> [u8; N] {
        let mut buf = [0; N];
        let mut pos = 0;
        let bytes = self.0.as_bytes();
        let mut i = 0;
        while i < bytes.len() {
            const_assert!(bytes[i] != 0);
            buf[pos] = bytes[i];
            pos += 1;
            i += 1;
        }
        pos += 1;
        const_assert!(pos == N);
        buf
    }
}

#[macro_export]
macro_rules! cstr {
    ($s:literal) => {{
        const LEN: usize = $crate::ToCStr($s).eval_len();
        const BUF: [u8; LEN] = $crate::ToCStr($s).eval_bytes();
        unsafe { CStr::from_bytes_with_nul_unchecked(&BUF) }
    }};
}

#[macro_export]
macro_rules! r_cstr {
    ($s:literal) => {
        cstr!($s).as_ptr()
    };
}

pub unsafe fn ptr_to_str<'a, T>(ptr: *const T) -> &'a str {
    CStr::from_ptr(ptr.cast()).to_str().unwrap_or("")
}

pub fn errno() -> &'static mut i32 {
    // On Android, errno is obtained through the __errno function for thread local storage
    extern "C" {
        fn __errno() -> *mut i32;
    }
    unsafe { &mut *__errno() }
}

pub fn error_str() -> &'static str {
    unsafe { ptr_to_str(libc::strerror(*errno())) }
}
