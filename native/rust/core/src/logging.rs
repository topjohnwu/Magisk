use std::fmt::Arguments;
use base::*;

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

pub fn android_logging() {
    fn android_log_impl(prio: i32, args: Arguments) {
        let mut buf: [u8; 4096] = [0; 4096];
        fmt_to_buf(&mut buf, args);
        unsafe {
            __android_log_write(prio, b"Magisk\0".as_ptr(), buf.as_ptr());
        }
    }

    let logger = Logger {
        d: |args| { android_log_impl(ALogPriority::ANDROID_LOG_DEBUG as i32, args) },
        i: |args| { android_log_impl(ALogPriority::ANDROID_LOG_INFO as i32, args) },
        w: |args| { android_log_impl(ALogPriority::ANDROID_LOG_WARN as i32, args) },
        e: |args| { android_log_impl(ALogPriority::ANDROID_LOG_ERROR as i32, args) }
    };
    exit_on_error(false);
    unsafe {
        LOGGER = logger;
    }
}

pub fn magisk_logging() {
    fn magisk_log_impl(prio: i32, args: Arguments) {
        let mut buf: [u8; 4096] = [0; 4096];
        let len = fmt_to_buf(&mut buf, args);
        unsafe {
            __android_log_write(prio, b"Magisk\0".as_ptr(), buf.as_ptr());
            magisk_log_write(prio, buf.as_ptr(), len as i32);
        }
    }

    let logger = Logger {
        d: |args| { magisk_log_impl(ALogPriority::ANDROID_LOG_DEBUG as i32, args) },
        i: |args| { magisk_log_impl(ALogPriority::ANDROID_LOG_INFO as i32, args) },
        w: |args| { magisk_log_impl(ALogPriority::ANDROID_LOG_WARN as i32, args) },
        e: |args| { magisk_log_impl(ALogPriority::ANDROID_LOG_ERROR as i32, args) }
    };
    exit_on_error(false);
    unsafe {
        LOGGER = logger;
    }
}

pub fn zygisk_logging() {
    fn zygisk_log_impl(prio: i32, args: Arguments) {
        let mut buf: [u8; 4096] = [0; 4096];
        let len = fmt_to_buf(&mut buf, args);
        unsafe {
            __android_log_write(prio, b"Magisk\0".as_ptr(), buf.as_ptr());
            zygisk_log_write(prio, buf.as_ptr(), len as i32);
        }
    }

    let logger = Logger {
        d: |args| { zygisk_log_impl(ALogPriority::ANDROID_LOG_DEBUG as i32, args) },
        i: |args| { zygisk_log_impl(ALogPriority::ANDROID_LOG_INFO as i32, args) },
        w: |args| { zygisk_log_impl(ALogPriority::ANDROID_LOG_WARN as i32, args) },
        e: |args| { zygisk_log_impl(ALogPriority::ANDROID_LOG_ERROR as i32, args) }
    };
    exit_on_error(false);
    unsafe {
        LOGGER = logger;
    }
}
