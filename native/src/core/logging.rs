use base::ffi::LogLevel;
use base::*;
use std::fmt::Arguments;

#[allow(dead_code, non_camel_case_types)]
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

extern "C" {
    fn __android_log_write(prio: i32, tag: *const u8, msg: *const u8);
    fn magisk_log_write(prio: i32, msg: *const u8, len: i32);
    fn zygisk_log_write(prio: i32, msg: *const u8, len: i32);
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
    fn android_log_fmt(level: LogLevel, args: Arguments) {
        let mut buf: [u8; 4096] = [0; 4096];
        fmt_to_buf(&mut buf, args);
        unsafe {
            __android_log_write(level_to_prio(level), b"Magisk\0".as_ptr(), buf.as_ptr());
        }
    }
    fn android_log_write(level: LogLevel, msg: &[u8]) {
        unsafe {
            __android_log_write(level_to_prio(level), b"Magisk\0".as_ptr(), msg.as_ptr());
        }
    }

    let logger = Logger {
        fmt: android_log_fmt,
        write: android_log_write,
        flags: 0,
    };
    exit_on_error(false);
    unsafe {
        LOGGER = logger;
    }
}

pub fn magisk_logging() {
    fn magisk_fmt(level: LogLevel, args: Arguments) {
        let mut buf: [u8; 4096] = [0; 4096];
        let len = fmt_to_buf(&mut buf, args);
        unsafe {
            __android_log_write(level_to_prio(level), b"Magisk\0".as_ptr(), buf.as_ptr());
            magisk_log_write(level_to_prio(level), buf.as_ptr(), len as i32);
        }
    }
    fn magisk_write(level: LogLevel, msg: &[u8]) {
        unsafe {
            __android_log_write(level_to_prio(level), b"Magisk\0".as_ptr(), msg.as_ptr());
            magisk_log_write(level_to_prio(level), msg.as_ptr(), msg.len() as i32);
        }
    }

    let logger = Logger {
        fmt: magisk_fmt,
        write: magisk_write,
        flags: 0,
    };
    exit_on_error(false);
    unsafe {
        LOGGER = logger;
    }
}

pub fn zygisk_logging() {
    fn zygisk_fmt(level: LogLevel, args: Arguments) {
        let mut buf: [u8; 4096] = [0; 4096];
        let len = fmt_to_buf(&mut buf, args);
        unsafe {
            __android_log_write(level_to_prio(level), b"Magisk\0".as_ptr(), buf.as_ptr());
            zygisk_log_write(level_to_prio(level), buf.as_ptr(), len as i32);
        }
    }
    fn zygisk_write(level: LogLevel, msg: &[u8]) {
        unsafe {
            __android_log_write(level_to_prio(level), b"Magisk\0".as_ptr(), msg.as_ptr());
            zygisk_log_write(level_to_prio(level), msg.as_ptr(), msg.len() as i32);
        }
    }

    let logger = Logger {
        fmt: zygisk_fmt,
        write: zygisk_write,
        flags: 0,
    };
    exit_on_error(false);
    unsafe {
        LOGGER = logger;
    }
}
