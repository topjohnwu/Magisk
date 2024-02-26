use std::fs::File;
use std::io::Write;
use std::mem;
use std::os::fd::{FromRawFd, RawFd};

use base::{debug, Utf8CStr};

pub fn inject_magisk_rc(fd: RawFd, tmp_dir: &Utf8CStr) {
    debug!("Injecting magisk rc");

    let mut file = unsafe { File::from_raw_fd(fd) };

    write!(
        file,
        r#"
on post-fs-data
    start logd
    exec {1} 0 0 -- {0}/magisk --post-fs-data

on property:vold.decrypt=trigger_restart_framework
    exec {1} 0 0 -- {0}/magisk --service

on nonencrypted
    exec {1} 0 0 -- {0}/magisk --service

on property:sys.boot_completed=1
    exec {1} 0 0 -- {0}/magisk --boot-complete

on property:init.svc.zygote=stopped
    exec {1} 0 0 -- {0}/magisk --zygote-restart
"#,
        tmp_dir, "u:r:magisk:s0"
    )
    .ok();

    mem::forget(file)
}
