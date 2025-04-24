use base::{
    LOGGER, LogLevel, Logger, SilentResultExt, Utf8CStr, cstr,
    libc::{
        O_CLOEXEC, O_RDWR, O_WRONLY, S_IFCHR, STDERR_FILENO, STDIN_FILENO, STDOUT_FILENO, SYS_dup3,
        makedev, mknod, syscall,
    },
    raw_cstr,
};
use std::mem::ManuallyDrop;
use std::{
    fs::File,
    io::{IoSlice, Write},
    os::fd::{FromRawFd, IntoRawFd, RawFd},
};

// SAFETY: magiskinit is single threaded
static mut KMSG: RawFd = -1;

pub fn setup_klog() {
    unsafe {
        // Shut down first 3 fds
        let mut fd = cstr!("/dev/null").open(O_RDWR | O_CLOEXEC).silent();
        if fd.is_err() {
            mknod(raw_cstr!("/null"), S_IFCHR | 0o666, makedev(1, 3));
            fd = cstr!("/null").open(O_RDWR | O_CLOEXEC).silent();
            cstr!("/null").remove().ok();
        }
        if let Ok(ref fd) = fd {
            syscall(SYS_dup3, fd, STDIN_FILENO, O_CLOEXEC);
            syscall(SYS_dup3, fd, STDOUT_FILENO, O_CLOEXEC);
            syscall(SYS_dup3, fd, STDERR_FILENO, O_CLOEXEC);
        }

        // Then open kmsg fd
        let mut fd = cstr!("/dev/kmsg").open(O_WRONLY | O_CLOEXEC).silent();
        if fd.is_err() {
            mknod(raw_cstr!("/kmsg"), S_IFCHR | 0o666, makedev(1, 11));
            fd = cstr!("/kmsg").open(O_WRONLY | O_CLOEXEC).silent();
            cstr!("/kmsg").remove().ok();
        }
        KMSG = fd.map(|fd| fd.into_raw_fd()).unwrap_or(-1);
    }

    // Disable kmsg rate limiting
    if let Ok(mut rate) = cstr!("/proc/sys/kernel/printk_devkmsg").open(O_WRONLY | O_CLOEXEC) {
        writeln!(rate, "on").ok();
    }

    fn kmsg_log_write(_: LogLevel, msg: &Utf8CStr) {
        let fd = unsafe { KMSG };
        if fd >= 0 {
            let io1 = IoSlice::new("magiskinit: ".as_bytes());
            let io2 = IoSlice::new(msg.as_bytes());
            let mut kmsg = ManuallyDrop::new(unsafe { File::from_raw_fd(fd) });
            let _ = kmsg.write_vectored(&[io1, io2]).ok();
        }
    }

    let logger = Logger {
        write: kmsg_log_write,
        flags: 0,
    };
    unsafe {
        LOGGER = logger;
    }
}
