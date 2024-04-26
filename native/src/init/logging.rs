use std::cell::UnsafeCell;
use std::fs::File;
use std::io::{IoSlice, Write};

use base::libc::{
    makedev, mknod, syscall, SYS_dup3, O_CLOEXEC, O_RDWR, O_WRONLY, STDERR_FILENO, STDIN_FILENO,
    STDOUT_FILENO, S_IFCHR,
};
use base::{cstr, exit_on_error, open_fd, raw_cstr, FsPath, LogLevel, Logger, Utf8CStr, LOGGER};

// SAFETY: magiskinit is single threaded
static mut KMSG: UnsafeCell<Option<File>> = UnsafeCell::new(None);

pub fn setup_klog() {
    unsafe {
        // Shut down first 3 fds
        let mut fd = open_fd!(cstr!("/dev/null"), O_RDWR | O_CLOEXEC);
        if fd.is_err() {
            mknod(raw_cstr!("/null"), S_IFCHR | 0o666, makedev(1, 3));
            fd = open_fd!(cstr!("/null"), O_RDWR | O_CLOEXEC);
            FsPath::from(cstr!("/null")).remove().ok();
        }
        if let Ok(ref fd) = fd {
            syscall(SYS_dup3, fd, STDIN_FILENO, O_CLOEXEC);
            syscall(SYS_dup3, fd, STDOUT_FILENO, O_CLOEXEC);
            syscall(SYS_dup3, fd, STDERR_FILENO, O_CLOEXEC);
        }

        // Then open kmsg fd
        let mut fd = open_fd!(cstr!("/dev/kmsg"), O_WRONLY | O_CLOEXEC);
        if fd.is_err() {
            mknod(raw_cstr!("/kmsg"), S_IFCHR | 0o666, makedev(1, 11));
            fd = open_fd!(cstr!("/kmsg"), O_WRONLY | O_CLOEXEC);
            FsPath::from(cstr!("/kmsg")).remove().ok();
        }
        *KMSG.get() = fd.map(|fd| fd.into()).ok();
    }

    // Disable kmsg rate limiting
    if let Ok(rate) = open_fd!(
        cstr!("/proc/sys/kernel/printk_devkmsg"),
        O_WRONLY | O_CLOEXEC
    ) {
        let mut rate = File::from(rate);
        writeln!(rate, "on").ok();
    }

    fn kmsg_log_write(_: LogLevel, msg: &Utf8CStr) {
        if let Some(kmsg) = unsafe { &mut *KMSG.get() } {
            let io1 = IoSlice::new("magiskinit: ".as_bytes());
            let io2 = IoSlice::new(msg.as_bytes());
            let _ = kmsg.write_vectored(&[io1, io2]).ok();
        }
    }

    let logger = Logger {
        write: kmsg_log_write,
        flags: 0,
    };
    exit_on_error(false);
    unsafe {
        LOGGER = logger;
    }
}
