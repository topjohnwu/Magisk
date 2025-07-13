use crate::{StrErr, Utf8CStr, ffi};
use argh::EarlyExit;
use libc::c_char;
use std::fmt::Arguments;
use std::io::Write;
use std::mem::ManuallyDrop;
use std::process::exit;
use std::sync::Arc;
use std::sync::atomic::{AtomicPtr, Ordering};
use std::{fmt, slice, str};

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
                    eprintln!("{output}");
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

pub struct Chunker {
    chunk: Box<[u8]>,
    chunk_size: usize,
    pos: usize,
}

impl Chunker {
    pub fn new(chunk_size: usize) -> Self {
        Chunker {
            // SAFETY: all bytes will be initialized before it is used, tracked by self.pos
            chunk: unsafe { Box::new_uninit_slice(chunk_size).assume_init() },
            chunk_size,
            pos: 0,
        }
    }

    pub fn set_chunk_size(&mut self, chunk_size: usize) {
        self.chunk_size = chunk_size;
        self.pos = 0;
        if self.chunk.len() < chunk_size {
            self.chunk = unsafe { Box::new_uninit_slice(chunk_size).assume_init() };
        }
    }

    // Returns (remaining buf, Option<Chunk>)
    pub fn add_data<'a, 'b: 'a>(&'a mut self, mut buf: &'b [u8]) -> (&'b [u8], Option<&'a [u8]>) {
        let mut chunk = None;
        if self.pos > 0 {
            // Try to fill the chunk
            let len = std::cmp::min(self.chunk_size - self.pos, buf.len());
            self.chunk[self.pos..self.pos + len].copy_from_slice(&buf[..len]);
            self.pos += len;
            // If the chunk is filled, consume it
            if self.pos == self.chunk_size {
                chunk = Some(&self.chunk[..self.chunk_size]);
                self.pos = 0;
            }
            buf = &buf[len..];
        } else if buf.len() >= self.chunk_size {
            // Directly consume a chunk from buf
            chunk = Some(&buf[..self.chunk_size]);
            buf = &buf[self.chunk_size..];
        } else {
            // Copy buf into chunk
            self.chunk[self.pos..self.pos + buf.len()].copy_from_slice(buf);
            self.pos += buf.len();
            return (&[], None);
        }
        (buf, chunk)
    }

    pub fn get_available(&mut self) -> &[u8] {
        let chunk = &self.chunk[..self.pos];
        self.pos = 0;
        chunk
    }
}
