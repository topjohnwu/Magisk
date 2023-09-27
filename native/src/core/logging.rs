use std::cmp::min;
use std::ffi::{c_char, c_void};
use std::fmt::Write as FmtWrite;
use std::fs::File;
use std::io::{IoSlice, Read, Write};
use std::os::fd::{AsRawFd, FromRawFd, RawFd};
use std::ptr::null_mut;
use std::{fs, io};

use bytemuck::{bytes_of, bytes_of_mut, write_zeroes, Pod, Zeroable};
use num_derive::{FromPrimitive, ToPrimitive};
use num_traits::FromPrimitive;

use base::ffi::LogLevel;
use base::libc::{
    clock_gettime, getpid, gettid, localtime_r, pipe2, pthread_sigmask, sigaddset, sigset_t,
    sigtimedwait, timespec, tm, CLOCK_REALTIME, O_CLOEXEC, PIPE_BUF, SIGPIPE, SIG_BLOCK,
    SIG_SETMASK,
};
use base::*;

use crate::daemon::{MagiskD, MAGISKD};
use crate::logging::LogFile::{Actual, Buffer};
use crate::LOGFILE;

#[allow(dead_code, non_camel_case_types)]
#[derive(FromPrimitive, ToPrimitive)]
#[repr(i32)]
enum ALogPriority {
    ANDROID_LOG_UNKNOWN = 0,
    ANDROID_LOG_DEFAULT,
    ANDROID_LOG_VERBOSE,
    ANDROID_LOG_DEBUG,
    ANDROID_LOG_INFO,
    ANDROID_LOG_WARN,
    ANDROID_LOG_ERROR,
    ANDROID_LOG_FATAL,
    ANDROID_LOG_SILENT,
}

type ThreadEntry = extern "C" fn(*mut c_void) -> *mut c_void;

extern "C" {
    fn __android_log_write(prio: i32, tag: *const c_char, msg: *const c_char);
    fn strftime(buf: *mut c_char, len: usize, fmt: *const c_char, tm: *const tm) -> usize;

    fn zygisk_fetch_logd() -> RawFd;
    fn new_daemon_thread(entry: ThreadEntry, arg: *mut c_void);
}

fn level_to_prio(level: LogLevel) -> i32 {
    match level {
        LogLevel::Error => ALogPriority::ANDROID_LOG_ERROR as i32,
        LogLevel::Warn => ALogPriority::ANDROID_LOG_WARN as i32,
        LogLevel::Info => ALogPriority::ANDROID_LOG_INFO as i32,
        LogLevel::Debug => ALogPriority::ANDROID_LOG_DEBUG as i32,
        _ => 0,
    }
}

pub fn android_logging() {
    fn android_log_write(level: LogLevel, msg: &Utf8CStr) {
        unsafe {
            __android_log_write(level_to_prio(level), raw_cstr!("Magisk"), msg.as_ptr());
        }
    }

    let logger = Logger {
        write: android_log_write,
        flags: 0,
    };
    exit_on_error(false);
    unsafe {
        LOGGER = logger;
    }
}

pub fn magisk_logging() {
    fn magisk_log_write(level: LogLevel, msg: &Utf8CStr) {
        unsafe {
            __android_log_write(level_to_prio(level), raw_cstr!("Magisk"), msg.as_ptr());
        }
        magisk_log_to_pipe(level_to_prio(level), msg);
    }

    let logger = Logger {
        write: magisk_log_write,
        flags: 0,
    };
    exit_on_error(false);
    unsafe {
        LOGGER = logger;
    }
}

pub fn zygisk_logging() {
    fn zygisk_log_write(level: LogLevel, msg: &Utf8CStr) {
        unsafe {
            __android_log_write(level_to_prio(level), raw_cstr!("Magisk"), msg.as_ptr());
        }
        zygisk_log_to_pipe(level_to_prio(level), msg);
    }

    let logger = Logger {
        write: zygisk_log_write,
        flags: 0,
    };
    exit_on_error(false);
    unsafe {
        LOGGER = logger;
    }
}

#[derive(Copy, Clone, Pod, Zeroable)]
#[repr(C)]
struct LogMeta {
    prio: i32,
    len: i32,
    pid: i32,
    tid: i32,
}

const MAX_MSG_LEN: usize = PIPE_BUF - std::mem::size_of::<LogMeta>();

fn write_log_to_pipe(logd: &mut File, prio: i32, msg: &Utf8CStr) -> io::Result<usize> {
    // Truncate message if needed
    let len = min(MAX_MSG_LEN, msg.len());
    let msg = &msg.as_bytes()[..len];

    let meta = LogMeta {
        prio,
        len: len as i32,
        pid: unsafe { getpid() },
        tid: unsafe { gettid() },
    };

    let io1 = IoSlice::new(bytes_of(&meta));
    let io2 = IoSlice::new(msg);
    logd.write_vectored(&[io1, io2])
}

fn magisk_log_to_pipe(prio: i32, msg: &Utf8CStr) {
    let magiskd = match MAGISKD.get() {
        None => return,
        Some(s) => s,
    };

    let logd_cell = magiskd.logd.lock().unwrap();
    let mut logd_ref = logd_cell.borrow_mut();
    let logd = match logd_ref.as_mut() {
        None => return,
        Some(s) => s,
    };

    let result = write_log_to_pipe(logd, prio, msg);

    // If any error occurs, shut down the logd pipe
    if result.is_err() {
        *logd_ref = None;
    }
}

fn zygisk_log_to_pipe(prio: i32, msg: &Utf8CStr) {
    let magiskd = match MAGISKD.get() {
        None => return,
        Some(s) => s,
    };

    let logd_cell = magiskd.logd.lock().unwrap();
    let mut logd_ref = logd_cell.borrow_mut();
    if logd_ref.is_none() {
        android_logging();
        unsafe {
            let fd = zygisk_fetch_logd();
            if fd < 0 {
                return;
            }
            *logd_ref = Some(File::from_raw_fd(fd));
        }
        // Only re-enable zygisk logging if success
        zygisk_logging();
    };
    let logd = logd_ref.as_mut().unwrap();

    // Block SIGPIPE
    let mut mask: sigset_t;
    let mut orig_mask: sigset_t;
    unsafe {
        mask = std::mem::zeroed();
        orig_mask = std::mem::zeroed();
        sigaddset(&mut mask, SIGPIPE);
        pthread_sigmask(SIG_BLOCK, &mask, &mut orig_mask);
    }

    let result = write_log_to_pipe(logd, prio, msg);

    // Consume SIGPIPE if exists, then restore mask
    unsafe {
        let ts: timespec = std::mem::zeroed();
        sigtimedwait(&mask, null_mut(), &ts);
        pthread_sigmask(SIG_SETMASK, &orig_mask, null_mut());
    }

    // If any error occurs, shut down the logd pipe
    if result.is_err() {
        *logd_ref = None;
    }
}

// The following is implementation for the logging daemon

enum LogFile<'a> {
    Buffer(&'a mut Vec<u8>),
    Actual(File),
}

impl Write for LogFile<'_> {
    fn write(&mut self, buf: &[u8]) -> io::Result<usize> {
        match self {
            Buffer(e) => e.write(buf),
            Actual(e) => e.write(buf),
        }
    }

    fn write_vectored(&mut self, bufs: &[IoSlice<'_>]) -> io::Result<usize> {
        match self {
            Buffer(e) => e.write_vectored(bufs),
            Actual(e) => e.write_vectored(bufs),
        }
    }

    fn flush(&mut self) -> io::Result<()> {
        match self {
            Buffer(e) => e.flush(),
            Actual(e) => e.flush(),
        }
    }
}

extern "C" fn logfile_writer(arg: *mut c_void) -> *mut c_void {
    fn writer_loop(pipefd: RawFd) -> io::Result<()> {
        let mut pipe = unsafe { File::from_raw_fd(pipefd) };
        let mut tmp = Vec::new();
        let mut logfile: LogFile = Buffer(&mut tmp);

        let mut meta = LogMeta::zeroed();
        let mut msg_buf = [0u8; MAX_MSG_LEN];
        let mut aux = Utf8CStrBufArr::<64>::new();

        loop {
            // Read request
            write_zeroes(&mut meta);
            pipe.read_exact(bytes_of_mut(&mut meta))?;

            if meta.prio < 0 {
                if matches!(logfile, LogFile::Buffer(_)) {
                    fs::rename(LOGFILE!(), concat!(LOGFILE!(), ".bak")).ok();
                    let mut out = File::create(LOGFILE!())?;
                    out.write_all(tmp.as_slice())?;
                    tmp = Vec::new();
                    logfile = Actual(out);
                }
                continue;
            }

            if meta.len < 0 || meta.len > MAX_MSG_LEN as i32 {
                continue;
            }

            // Read the rest of the message
            let msg = &mut msg_buf[..(meta.len as usize)];
            pipe.read_exact(msg)?;

            // Start building the log string
            aux.clear();
            let prio =
                ALogPriority::from_i32(meta.prio).unwrap_or(ALogPriority::ANDROID_LOG_UNKNOWN);
            let prio = match prio {
                ALogPriority::ANDROID_LOG_VERBOSE => 'V',
                ALogPriority::ANDROID_LOG_DEBUG => 'D',
                ALogPriority::ANDROID_LOG_INFO => 'I',
                ALogPriority::ANDROID_LOG_WARN => 'W',
                ALogPriority::ANDROID_LOG_ERROR => 'E',
                // Unsupported values, skip
                _ => continue,
            };

            // Note: the obvious better implementation is to use the rust chrono crate, however
            // the crate cannot fetch the proper local timezone without pulling in a bunch of
            // timezone handling code. To reduce binary size, fallback to use localtime_r in libc.
            unsafe {
                let mut ts: timespec = std::mem::zeroed();
                let mut tm: tm = std::mem::zeroed();
                if clock_gettime(CLOCK_REALTIME, &mut ts) < 0
                    || localtime_r(&ts.tv_sec, &mut tm).is_null()
                {
                    continue;
                }
                let len = strftime(
                    aux.mut_buf().as_mut_ptr().cast(),
                    aux.capacity(),
                    raw_cstr!("%m-%d %T"),
                    &tm,
                );
                aux.set_len(len);
                let ms = ts.tv_nsec / 1000000;
                aux.write_fmt(format_args!(
                    ".{:03} {:5} {:5} {} : ",
                    ms, meta.pid, meta.tid, prio
                ))
                .ok();
            }

            let io1 = IoSlice::new(aux.as_bytes_with_nul());
            let io2 = IoSlice::new(msg);
            // We don't need to care the written len because we are writing less than PIPE_BUF
            // It's guaranteed to always write the whole thing atomically
            let _ = logfile.write_vectored(&[io1, io2])?;
        }
    }

    writer_loop(arg as RawFd).ok();
    // If any error occurs, shut down the logd pipe
    if let Some(magiskd) = MAGISKD.get() {
        magiskd.close_log_pipe();
    }
    null_mut()
}

impl MagiskD {
    pub fn start_log_daemon(&self) {
        let mut fds: [i32; 2] = [0; 2];
        unsafe {
            if pipe2(fds.as_mut_ptr(), O_CLOEXEC) == 0 {
                let logd = self.logd.lock().unwrap();
                *logd.borrow_mut() = Some(File::from_raw_fd(fds[1]));
                new_daemon_thread(logfile_writer, fds[0] as *mut c_void);
            }
        }
    }

    pub fn get_log_pipe(&self) -> RawFd {
        let logd_cell = self.logd.lock().unwrap();
        let logd_ref = logd_cell.borrow();
        let logd = logd_ref.as_ref();
        match logd {
            None => -1,
            Some(s) => s.as_raw_fd(),
        }
    }

    pub fn close_log_pipe(&self) {
        let guard = self.logd.lock().unwrap();
        *guard.borrow_mut() = None;
    }

    pub fn setup_logfile(&self) {
        let logd_cell = self.logd.lock().unwrap();
        let mut logd_ref = logd_cell.borrow_mut();
        let logd = match logd_ref.as_mut() {
            None => return,
            Some(s) => s,
        };

        let meta = LogMeta {
            prio: -1,
            len: 0,
            pid: 0,
            tid: 0,
        };

        logd.write_all(bytes_of(&meta)).ok();
    }
}
