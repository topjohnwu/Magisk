use std::fs;
use std::fs::File;
use std::io::Write;
use std::sync::OnceLock;

use base::ffi::LogLevel;
use base::libc::{
    close, makedev, mknod, open, syscall, unlink, SYS_dup3, O_CLOEXEC, O_RDWR, STDERR_FILENO,
    STDIN_FILENO, STDOUT_FILENO, S_IFCHR,
};
use base::*;

static KMSG: OnceLock<File> = OnceLock::new();

pub fn setup_klog() {
    // Shut down first 3 fds
    unsafe {
        let mut fd = open(raw_cstr!("/dev/null"), O_RDWR | O_CLOEXEC);
        if fd < 0 {
            mknod(raw_cstr!("/null"), S_IFCHR | 0666, makedev(1, 3));
            fd = open(raw_cstr!("/null"), O_RDWR | O_CLOEXEC);
            fs::remove_file("/null").ok();
        }

        syscall(SYS_dup3, fd, STDIN_FILENO, O_CLOEXEC);
        syscall(SYS_dup3, fd, STDOUT_FILENO, O_CLOEXEC);
        syscall(SYS_dup3, fd, STDERR_FILENO, O_CLOEXEC);
        if fd > STDERR_FILENO {
            close(fd);
        }
    }

    if let Ok(kmsg) = File::options().write(true).open("/dev/kmsg") {
        KMSG.set(kmsg).ok();
    } else {
        unsafe {
            mknod(raw_cstr!("/kmsg"), S_IFCHR | 0666, makedev(1, 11));
            KMSG.set(File::options().write(true).open("/kmsg").unwrap())
                .ok();
            unlink(raw_cstr!("/kmsg"));
        }
    }

    // Disable kmsg rate limiting
    if let Ok(mut rate) = File::options()
        .write(true)
        .open("/proc/sys/kernel/printk_devkmsg")
    {
        writeln!(rate, "on").ok();
    }

    fn klog_write_impl(_: LogLevel, msg: &[u8]) {
        if let Some(kmsg) = KMSG.get().as_mut() {
            let mut buf: [u8; 4096] = [0; 4096];
            let mut len = copy_str(&mut buf, b"magiskinit: ");
            len += copy_str(&mut buf[len..], msg);
            kmsg.write_all(&buf[..len]).ok();
        }
    }

    let logger = Logger {
        write: klog_write_impl,
        flags: 0,
    };
    exit_on_error(false);
    unsafe {
        LOGGER = logger;
    }
}
