use base::{
    debug, libc, Directory, LibcReturn, LoggedResult, ResultExt, Utf8CStr, Utf8CStrBuf,
    Utf8CStrBufArr, WalkResult,
};
use std::fs::File;
use std::io::Write;
use std::mem;
use std::os::fd::{FromRawFd, RawFd};
use std::sync::OnceLock;

pub static OVERLAY_ATTRS: OnceLock<Vec<(String, String)>> = OnceLock::new();

const XATTR_NAME_SELINUX: &[u8] = b"security.selinux\0";

fn get_context<const N: usize>(path: &str, con: &mut Utf8CStrBufArr<N>) -> std::io::Result<()> {
    unsafe {
        let sz = libc::lgetxattr(
            path.as_ptr().cast(),
            XATTR_NAME_SELINUX.as_ptr().cast(),
            con.as_mut_ptr().cast(),
            con.capacity(),
        )
        .check_os_err()?;
        con.set_len((sz - 1) as usize);
    }
    Ok(())
}

fn set_context(path: &str, con: &str) -> std::io::Result<()> {
    unsafe {
        libc::lsetxattr(
            path.as_ptr().cast(),
            XATTR_NAME_SELINUX.as_ptr().cast(),
            con.as_ptr().cast(),
            con.len() + 1,
            0,
        )
        .as_os_err()
    }
}

pub fn collect_overlay_contexts(src: &Utf8CStr) {
    OVERLAY_ATTRS
        .get_or_try_init(|| -> LoggedResult<_> {
            let mut contexts = vec![];
            let mut con = Utf8CStrBufArr::default();
            let mut path = Utf8CStrBufArr::default();
            let mut src = Directory::open(src)?;
            src.path(&mut path)?;
            let src_len = path.len();
            src.post_order_walk(|f| {
                f.path(&mut path)?;

                let path = &path[src_len..];
                if get_context(path, &mut con)
                    .log_with_msg(|w| w.write_fmt(format_args!("collect context {}", path)))
                    .is_ok()
                {
                    debug!("collect context: {:?} -> {:?}", path, con);
                    contexts.push((path.to_string(), con.to_string()));
                }

                Ok(WalkResult::Continue)
            })?;
            Ok(contexts)
        })
        .ok();
}

pub fn reset_overlay_contexts() {
    OVERLAY_ATTRS.get().map(|attrs| {
        for (path, con) in attrs.iter() {
            debug!("set context: {} -> {}", path, con);
            set_context(path, con)
                .log_with_msg(|w| w.write_fmt(format_args!("reset context {}", path)))
                .ok();
        }
        Some(())
    });
}

pub fn inject_magisk_rc(fd: RawFd, tmp_dir: &Utf8CStr) {
    debug!("Injecting magisk rc");

    let mut file = unsafe { File::from_raw_fd(fd) };

    write!(
        file,
        r#"
on post-fs-data
    exec {0} 0 0 -- {1}/magisk --post-fs-data

on property:vold.decrypt=trigger_restart_framework
    exec {0} 0 0 -- {1}/magisk --service

on nonencrypted
    exec {0} 0 0 -- {1}/magisk --service

on property:sys.boot_completed=1
    exec {0} 0 0 -- {1}/magisk --boot-complete

on property:init.svc.zygote=stopped
    exec {0} 0 0 -- {1}/magisk --zygote-restart
"#,
        "u:r:magisk:s0", tmp_dir
    )
    .ok();

    mem::forget(file)
}
