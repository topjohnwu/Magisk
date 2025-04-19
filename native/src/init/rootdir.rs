use crate::consts::{PREINITMIRR, ROOTMNT, ROOTOVL};
use crate::ffi::MagiskInit;
use base::libc::{O_CLOEXEC, O_CREAT, O_RDONLY, O_WRONLY};
use base::{
    BufReadExt, Directory, FsPath, FsPathBuf, LoggedResult, ResultExt, Utf8CStr, Utf8CString,
    clone_attr, const_format::concatcp, cstr, cstr_buf, debug, path,
};
use std::io::BufReader;
use std::{
    fs::File,
    io::Read,
    io::Write,
    mem,
    os::fd::{FromRawFd, RawFd},
};

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
        let rc = rc.replace("${MAGISKTMP}", tmp_dir.as_str());
        write!(file, "\n{}\n", rc).ok();
    });
    mem::forget(file);
    rc_list.clear();
}

pub struct OverlayAttr(Utf8CString, Utf8CString);

impl MagiskInit {
    pub(crate) fn parse_config_file(&mut self) {
        if let Ok(fd) = path!("/data/.backup/.magisk").open(O_RDONLY) {
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

    pub(crate) fn load_overlay_rc(&mut self, overlay: &Utf8CStr) {
        if let Ok(mut dir) = Directory::open(overlay) {
            let init_rc = FsPathBuf::from(cstr_buf::dynamic(256))
                .join(overlay)
                .join("/init.rc");
            if init_rc.exists() {
                init_rc.remove().ok();
            }
            loop {
                match dir.read() {
                    Ok(Some(e)) if (e.is_file() && e.name().ends_with(".rc")) => {
                        let name = e.name();
                        let mut path = FsPathBuf::from(cstr_buf::dynamic(256))
                            .join("/")
                            .join(e.name());
                        if path.exists() {
                            debug!("Replace rc script [{}]", name);
                        } else {
                            debug!("Found rc script [{}]", name);
                            path = FsPathBuf::from(cstr_buf::dynamic(256))
                                .join(overlay)
                                .join("/")
                                .join(e.name());
                            let mut rc_content = String::new();
                            if let Ok(mut file) = path.open(O_RDONLY | O_CLOEXEC) {
                                file.read_to_string(&mut rc_content).ok();
                                self.rc_list.push(rc_content);
                                drop(file);
                                path.remove().ok();
                            }
                        }
                    }
                    Ok(_) => continue,
                    Err(_) => break,
                }
            }
        }
    }

    pub(crate) fn handle_modules_rc(&mut self) {
        if let Ok(mut dir) = Directory::open(path!(concatcp!("/data/", PREINITMIRR))) {
            loop {
                match dir.read() {
                    Ok(Some(e)) if e.is_dir() => {
                        let name = e.name();
                        let path = FsPathBuf::from(cstr_buf::dynamic(256))
                            .join(path!(concatcp!("/data/", PREINITMIRR)))
                            .join(name);
                        self.load_overlay_rc(&path);
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
        let mut con = cstr_buf::default();
        loop {
            match &dir.read()? {
                None => return Ok(()),
                Some(e) => {
                    let name = e.name();
                    let src = FsPathBuf::from(cstr_buf::dynamic(256))
                        .join(src_dir)
                        .join(name);
                    let dest = FsPathBuf::from(cstr_buf::dynamic(256))
                        .join(dest_dir)
                        .join(name);
                    if dest.exists() {
                        if e.is_dir() {
                            // Recursive
                            self.mount_impl(&src, &dest, mount_list)?;
                        } else {
                            debug!("Mount [{}] -> [{}]", src, dest);
                            clone_attr(&dest, &src)?;
                            dest.get_secontext(&mut con)?;
                            src.bind_mount_to(&dest)?;
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
        if let Ok(mut fd) = path!(ROOTMNT).create(O_CREAT | O_WRONLY, 0) {
            fd.write(mount_list.as_bytes()).log_ok();
        }
    }

    pub(crate) fn restore_overlay_contexts(&self) {
        self.overlay_con.iter().for_each(|attr| {
            let OverlayAttr(path, con) = attr;
            FsPath::from(path).set_secontext(con).log_ok();
        })
    }
}
