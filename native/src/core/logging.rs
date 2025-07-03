use crate::consts::{LOG_PIPE, LOGFILE};
use crate::ffi::get_magisk_tmp;
use crate::logging::LogFile::{Actual, Buffer};
use base::libc::{
    O_CLOEXEC, O_RDWR, O_WRONLY, PIPE_BUF, SIG_BLOCK, SIG_SETMASK, SIGPIPE, getpid, gettid,
    localtime_r, pthread_sigmask, sigaddset, sigset_t, sigtimedwait, time_t, timespec, tm,
};
use base::{
    FsPathBuilder, LOGGER, LogLevel, Logger, ReadExt, Utf8CStr, Utf8CStrBuf, WriteExt,
    const_format::concatcp, cstr, libc, raw_cstr,
};
use bytemuck::{Pod, Zeroable, bytes_of, write_zeroes};
use num_derive::{FromPrimitive, ToPrimitive};
use num_traits::FromPrimitive;
use std::cmp::min;
use std::ffi::{c_char, c_void};
use std::fmt::Write as FmtWrite;
use std::fs::File;
use std::io::{IoSlice, Read, Write};
use std::mem::ManuallyDrop;
use std::os::fd::{FromRawFd, RawFd};
use std::ptr::null_mut;
use std::sync::atomic::{AtomicI32, Ordering};
use std::sync::{Arc, Mutex};
use std::time::{SystemTime, UNIX_EPOCH};
use std::{fs, io};

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

unsafe extern "C" {
    fn __android_log_write(prio: i32, tag: *const c_char, msg: *const c_char);
    fn strftime(buf: *mut c_char, len: usize, fmt: *const c_char, tm: *const tm) -> usize;
    fn new_daemon_thread(entry: ThreadEntry, arg: *mut c_void);
}

fn level_to_prio(level: LogLevel) -> i32 {
    match level {
        LogLevel::Error | LogLevel::ErrorCxx => ALogPriority::ANDROID_LOG_ERROR as i32,
        LogLevel::Warn => ALogPriority::ANDROID_LOG_WARN as i32,
        LogLevel::Info => ALogPriority::ANDROID_LOG_INFO as i32,
        LogLevel::Debug => ALogPriority::ANDROID_LOG_DEBUG as i32,
    }
}

fn android_log_write(level: LogLevel, msg: &Utf8CStr) {
    unsafe {
        __android_log_write(level_to_prio(level), raw_cstr!("Magisk"), msg.as_ptr());
    }
}

pub fn android_logging() {
    let logger = Logger {
        write: android_log_write,
        flags: 0,
    };
    unsafe {
        LOGGER = logger;
    }
}

pub fn magisk_logging() {
    fn magisk_log_write(level: LogLevel, msg: &Utf8CStr) {
        android_log_write(level, msg);
        magisk_log_to_pipe(level_to_prio(level), msg);
    }

    let logger = Logger {
        write: magisk_log_write,
        flags: 0,
    };
    unsafe {
        LOGGER = logger;
    }
}

pub fn zygisk_logging() {
    fn zygisk_log_write(level: LogLevel, msg: &Utf8CStr) {
        android_log_write(level, msg);
        zygisk_log_to_pipe(level_to_prio(level), msg);
    }

    let logger = Logger {
        write: zygisk_log_write,
        flags: 0,
    };
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

const MAX_MSG_LEN: usize = PIPE_BUF - size_of::<LogMeta>();

fn write_log_to_pipe(mut logd: &File, prio: i32, msg: &Utf8CStr) -> io::Result<usize> {
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
    let result = logd.write_vectored(&[io1, io2]);
    if let Err(ref e) = result {
        let mut buf = cstr::buf::default();
        buf.write_fmt(format_args!("Cannot write_log_to_pipe: {e}"))
            .ok();
        android_log_write(LogLevel::Error, &buf);
    }
    result
}

static MAGISK_LOGD_FD: Mutex<Option<Arc<File>>> = Mutex::new(None);

fn with_logd_fd<R, F: FnOnce(&File) -> io::Result<R>>(f: F) {
    let fd = MAGISK_LOGD_FD.lock().unwrap().clone();
    if let Some(logd) = fd
        && f(&logd).is_err()
    {
        // If any error occurs, shut down the logd pipe
        *MAGISK_LOGD_FD.lock().unwrap() = None;
    }
}

fn magisk_log_to_pipe(prio: i32, msg: &Utf8CStr) {
    with_logd_fd(|logd| write_log_to_pipe(logd, prio, msg));
}

// SAFETY: zygisk client code runs single threaded, so no need to prevent data race
static ZYGISK_LOGD: AtomicI32 = AtomicI32::new(-1);

pub fn zygisk_close_logd() {
    unsafe {
        libc::close(ZYGISK_LOGD.swap(-1, Ordering::Relaxed));
    }
}

pub fn zygisk_get_logd() -> i32 {
    // If we don't have the log pipe set, open the log pipe FIFO. This could actually happen
    // multiple times in the zygote daemon (parent process) because we had to close this
    // file descriptor to prevent crashing.
    //
    // For some reason, zygote sanitizes and checks FDs *before* forking. This results in the fact
    // that *every* time before zygote forks, it has to close all logging related FDs in order
    // to pass FD checks, just to have it re-initialized immediately after any
    // logging happens ¯\_(ツ)_/¯.
    //
    // To be consistent with this behavior, we also have to close the log pipe to magiskd
    // to make zygote NOT crash if necessary. We accomplish this by hooking __android_log_close
    // and closing it at the same time as the rest of logging FDs.

    let mut fd = ZYGISK_LOGD.load(Ordering::Relaxed);
    if fd < 0 {
        android_logging();
        let path = cstr::buf::default()
            .join_path(get_magisk_tmp())
            .join_path(LOG_PIPE);
        // Open as RW as sometimes it may block
        fd = unsafe { libc::open(path.as_ptr(), O_RDWR | O_CLOEXEC) };
        if fd >= 0 {
            // Only re-enable zygisk logging if success
            zygisk_logging();
            unsafe {
                libc::close(ZYGISK_LOGD.swap(fd, Ordering::Relaxed));
            }
        } else {
            return -1;
        }
    }
    fd
}

fn zygisk_log_to_pipe(prio: i32, msg: &Utf8CStr) {
    let fd = zygisk_get_logd();
    if fd < 0 {
        // Cannot talk to pipe, abort
        return;
    }

    // Block SIGPIPE
    let mut mask: sigset_t;
    let mut orig_mask: sigset_t;
    unsafe {
        mask = std::mem::zeroed();
        orig_mask = std::mem::zeroed();
        sigaddset(&mut mask, SIGPIPE);
        pthread_sigmask(SIG_BLOCK, &mask, &mut orig_mask);
    }

    let logd = ManuallyDrop::new(unsafe { File::from_raw_fd(fd) });
    let result = write_log_to_pipe(&logd, prio, msg);

    // Consume SIGPIPE if exists, then restore mask
    unsafe {
        let ts: timespec = std::mem::zeroed();
        sigtimedwait(&mask, null_mut(), &ts);
        pthread_sigmask(SIG_SETMASK, &orig_mask, null_mut());
    }

    // If any error occurs, shut down the logd pipe
    if result.is_err() {
        zygisk_close_logd();
    }
}

// The following is implementation for the logging daemon

enum LogFile {
    Buffer(Vec<u8>),
    Actual(File),
}

impl Write for LogFile {
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
        let mut logfile: LogFile = Buffer(Vec::new());

        let mut meta = LogMeta::zeroed();
        let mut msg_buf = [0u8; MAX_MSG_LEN];
        let mut aux = cstr::buf::new::<64>();

        loop {
            // Read request
            write_zeroes(&mut meta);
            pipe.read_pod(&mut meta)?;

            if meta.prio < 0 {
                if let Buffer(ref mut buf) = logfile {
                    fs::rename(LOGFILE, concatcp!(LOGFILE, ".bak")).ok();
                    let mut out = File::create(LOGFILE)?;
                    out.write_all(buf.as_slice())?;
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

            let now = SystemTime::now().duration_since(UNIX_EPOCH).unwrap();

            // Note: the obvious better implementation is to use the rust chrono crate, however
            // the crate cannot fetch the proper local timezone without pulling in a bunch of
            // timezone handling code. To reduce binary size, fallback to use localtime_r in libc.
            unsafe {
                let secs: time_t = now.as_secs() as time_t;
                let mut tm: tm = std::mem::zeroed();
                if localtime_r(&secs, &mut tm).is_null() {
                    continue;
                }
                let len = strftime(aux.as_mut_ptr(), aux.capacity(), raw_cstr!("%m-%d %T"), &tm);
                aux.set_len(len);
                aux.write_fmt(format_args!(
                    ".{:03} {:5} {:5} {} : ",
                    now.subsec_millis(),
                    meta.pid,
                    meta.tid,
                    prio
                ))
                .ok();
            }

            let io1 = IoSlice::new(aux.as_bytes());
            let io2 = IoSlice::new(msg);
            // We don't need to care the written len because we are writing less than PIPE_BUF
            // It's guaranteed to always write the whole thing atomically
            let _ = logfile.write_vectored(&[io1, io2])?;
        }
    }

    writer_loop(arg as RawFd).ok();
    // If any error occurs, shut down the logd pipe
    *MAGISK_LOGD_FD.lock().unwrap() = None;
    null_mut()
}

pub fn setup_logfile() {
    with_logd_fd(|mut logd| {
        let meta = LogMeta {
            prio: -1,
            len: 0,
            pid: 0,
            tid: 0,
        };
        (&mut logd).write_pod(&meta)
    });
}

pub fn start_log_daemon() {
    let path = cstr::buf::default()
        .join_path(get_magisk_tmp())
        .join_path(LOG_PIPE);

    unsafe {
        libc::mkfifo(path.as_ptr(), 0o666);
        libc::chown(path.as_ptr(), 0, 0);
        let read = libc::open(path.as_ptr(), O_RDWR | O_CLOEXEC);
        let write = libc::open(path.as_ptr(), O_WRONLY | O_CLOEXEC);
        *MAGISK_LOGD_FD.lock().unwrap() = Some(Arc::new(File::from_raw_fd(write)));
        new_daemon_thread(logfile_writer, read as *mut c_void);
    }
}
