use base::ffi::LogLevel;
use base::*;
use std::fmt::Arguments;

extern "C" {
    fn klog_write(msg: *const u8, len: i32);
}

pub fn setup_klog() {
    const PREFIX: &[u8; 12] = b"magiskinit: ";
    const PFX_LEN: usize = PREFIX.len();

    fn klog_fmt(_: LogLevel, args: Arguments) {
        let mut buf: [u8; 4096] = [0; 4096];
        buf[..PFX_LEN].copy_from_slice(PREFIX);
        let len = fmt_to_buf(&mut buf[PFX_LEN..], args) + PFX_LEN;
        unsafe {
            klog_write(buf.as_ptr(), len as i32);
        }
    }

    fn klog_write_impl(_: LogLevel, msg: &[u8]) {
        unsafe {
            klog_write(msg.as_ptr(), msg.len() as i32);
        }
    }

    let logger = Logger {
        fmt: klog_fmt,
        write: klog_write_impl,
        flags: 0,
    };
    exit_on_error(false);
    unsafe {
        LOGGER = logger;
    }
}
