use std::fmt::Arguments;
use base::*;

extern "C" {
    fn klog_write(msg: *const u8, len: i32);
}

pub fn setup_klog() {
    fn klog_impl(args: Arguments) {
        const PREFIX: &[u8; 12] = b"magiskinit: ";
        const PFX_LEN: usize = PREFIX.len();

        let mut buf: [u8; 4096] = [0; 4096];
        buf[..PFX_LEN].copy_from_slice(PREFIX);
        let len = fmt_to_buf(&mut buf[PFX_LEN..], args) + PFX_LEN;
        unsafe {
            klog_write(buf.as_ptr(), len as i32);
        }
    }

    let logger = Logger {
        d: klog_impl,
        i: klog_impl,
        w: klog_impl,
        e: klog_impl,
    };
    exit_on_error(false);
    unsafe {
        LOGGER = logger;
    }
}
