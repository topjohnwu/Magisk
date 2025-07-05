use crate::consts::{PREINITMIRR, ROOTMNT, ROOTOVL};
use crate::ffi::MagiskInit;
use base::{
    BufReadExt, Directory, FsPathBuilder, LoggedResult, ResultExt, Utf8CStr, Utf8CString,
    clone_attr,
    const_format::concatcp,
    cstr, debug,
    libc::{O_CLOEXEC, O_CREAT, O_RDONLY, O_WRONLY},
};
use std::{fs::File, io::BufReader, io::Read, io::Write, mem, os::fd::FromRawFd, os::fd::RawFd};

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

pub fn inject_custom_rc(mut rc_list: Vec<String>, fd: RawFd, tmp_dir: &Utf8CStr) {
    let mut file = unsafe { File::from_raw_fd(fd) };
    rc_list.iter().for_each(|rc| {
        // Replace template arguments of rc scripts with dynamic paths
        let rc = rc.replace("${MAGISKTMP}", tmp_dir.as_str());
        write!(file, "\n{}\n", rc).ok();
    });
    mem::forget(file);
    rc_list.clear();
}

pub struct OverlayAttr(Utf8CString, Utf8CString);

impl MagiskInit {
    pub(crate) fn parse_config_file(&mut self) {
        if let Ok(fd) = cstr!("/data/.backup/.magisk").open(O_RDONLY) {
            let mut reader = BufReader::new(fd);
            reader.foreach_props(|key, val| {
                if key == "PREINITDEVICE" {
                    self.preinit_dev = val.to_string();
                    return false;
                }
                true
            })
        }
    }

    pub(crate) fn load_overlay_rc(&mut self, overlay: &Utf8CStr, module_path: &Utf8CStr) {
        if let Ok(mut dir) = Directory::open(overlay) {
            let init_rc = cstr::buf::dynamic(256)
                .join_path(overlay)
                .join_path("init.rc");
            if init_rc.exists() {
                // Do not allow overwrite init.rc
                init_rc.remove().log_ok();
            }
            loop {
                match dir.read() {
                    Ok(None) => break,
                    Ok(Some(e)) => {
                        let recursive = cstr::buf::dynamic(256).join_path(module_path).exists();
                        if (recursive && e.is_dir()) || e.is_file() {
                            let buf = &mut cstr::buf::dynamic(256);
                            e.resolve_path(buf).log_ok();
                            if e.is_file() && buf.ends_with(".rc") {
                                let mut path;
                                if recursive {
                                    path = cstr::buf::dynamic(256)
                                        .join_path(buf.replace(module_path.as_str(), ""));
                                } else {
                                    path =
                                        cstr::buf::dynamic(256).join_path("/").join_path(e.name());
                                }
                                if path.exists() {
                                    debug!("Replace rc script [{}] -> [{}]", path, buf);
                                } else {
                                    path = cstr::buf::dynamic(256).join_path(buf);
                                    debug!("Found rc script [{}]", path);
                                    let mut rc_content = String::new();
                                    if let Ok(mut file) = path.open(O_RDONLY | O_CLOEXEC) {
                                        file.read_to_string(&mut rc_content).log_ok();
                                        self.rc_list.push(rc_content);
                                        drop(file);
                                        path.remove().log_ok();
                                    }
                                }
                            } else if e.is_dir() {
                                self.load_overlay_rc(buf, module_path);
                            }
                        } else {
                            continue;
                        }
                    }
                    Err(_) => break,
                }
            }
        }
    }

    pub(crate) fn handle_modules_rc(&mut self, root_dir: &Utf8CStr) {
        if let Ok(mut dir) = Directory::open(cstr!(concatcp!("/data/", PREINITMIRR))) {
            loop {
                match dir.read() {
                    Ok(None) => break,
                    Ok(Some(e)) if e.is_dir() => {
                        let buf = &mut cstr::buf::dynamic(256);
                        e.resolve_path(buf).log_ok();
                        let path = cstr::buf::dynamic(256).join_path(&mut *buf);
                        self.load_overlay_rc(&path, buf);
                        let desc_path = cstr::buf::dynamic(256).join_path(root_dir);
                        path.copy_to(&desc_path).log_ok();
                    }
                    Ok(_) => continue,
                    Err(_) => break,
                }
            }
        }
    }

    fn mount_impl(
        &mut self,
        src_dir: &Utf8CStr,
        dest_dir: &Utf8CStr,
        mount_list: &mut String,
    ) -> LoggedResult<()> {
        let mut dir = Directory::open(src_dir)?;
        let mut con = cstr::buf::default();
        loop {
            match &dir.read()? {
                None => return Ok(()),
                Some(e) => {
                    let name = e.name();
                    let src = cstr::buf::dynamic(256).join_path(src_dir).join_path(name);
                    let dest = cstr::buf::dynamic(256).join_path(dest_dir).join_path(name);
                    if dest.exists() {
                        if e.is_dir() {
                            // Recursive
                            self.mount_impl(&src, &dest, mount_list)?;
                        } else {
                            debug!("Mount [{}] -> [{}]", src, dest);
                            clone_attr(&dest, &src)?;
                            dest.get_secontext(&mut con)?;
                            src.bind_mount_to(&dest, false)?;
                            self.overlay_con
                                .push(OverlayAttr(dest.to_owned(), con.to_owned()));
                            mount_list.push_str(dest.as_str());
                            mount_list.push('\n');
                        }
                    }
                }
            }
        }
    }

    pub(crate) fn mount_overlay(&mut self, dest: &Utf8CStr) {
        let mut mount_list = String::new();
        self.mount_impl(cstr!(ROOTOVL), dest, &mut mount_list)
            .log_ok();
        if let Ok(mut fd) = cstr!(ROOTMNT).create(O_CREAT | O_WRONLY, 0) {
            fd.write(mount_list.as_bytes()).log_ok();
        }
    }

    pub(crate) fn restore_overlay_contexts(&self) {
        self.overlay_con.iter().for_each(|attr| {
            let OverlayAttr(path, con) = attr;
            path.set_secontext(con).log_ok();
        })
    }
}
