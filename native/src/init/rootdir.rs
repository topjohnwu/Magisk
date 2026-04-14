use crate::consts::{INTERNAL_DIR, MAGISK_PROC_CON, PREINITMIRR, ROOTMNT, ROOTOVL};
use crate::ffi::MagiskInit;
use base::nix::fcntl::OFlag;
use base::nix::unistd::UnlinkatFlags;
use base::{
    BufReadExt, Directory, FsPathBuilder, LoggedResult, ResultExt, Utf8CStr, Utf8CString,
    clone_attr, cstr, debug, fclone_attr, xfork,
    const_format::concatcp,
};
use std::fs::File;
use std::io::{BufReader, Read, Write};
use std::os::fd::AsRawFd;

const ISOLATED_PATH: &str = concatcp!("/", INTERNAL_DIR, "/isolated");

fn inject_magisk_rc(file: &mut File, tmp_dir: &Utf8CStr) {
    debug!("Injecting magisk rc");

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
"#,
        "u:r:magisk:s0", tmp_dir
    )
    .ok();
}

fn inject_custom_rc(rc_list: &[String], file: &mut File, tmp_dir: &Utf8CStr) {
    rc_list.iter().for_each(|rc| {
        // Replace template arguments of rc scripts with dynamic paths
        let rc = rc.replace("${MAGISKTMP}", tmp_dir.as_str());
        write!(file, "\n{}\n", rc).ok();
    });
}

pub struct OverlayAttr(Utf8CString, Utf8CString);

impl MagiskInit {
    pub(crate) fn patch_rc_scripts(
        &mut self,
        src_path: &Utf8CStr,
        tmp_path: &Utf8CStr,
        writable: bool,
    ) -> bool {
        debug!("Patching {} in {}", "init.rc", src_path);
        let mut src_dir = match Directory::open(src_path) {
            Ok(dir) => dir,
            Err(_) => return false,
        };

        let dest_path = if writable {
            cstr::buf::dynamic(256).join_path(src_path)
        } else {
            let path = cstr::buf::dynamic(256).join_path(ROOTOVL).join_path(src_path);
            if path.mkdirs(0o755).is_err() {
                return false;
            }
            path
        };

        let dest_dir = match Directory::open(&dest_path) {
            Ok(dir) => dir,
            Err(_) => return false,
        };

        {
            let src_rc = match src_dir
                .open_as_file_at(cstr!("init.rc"), OFlag::O_RDONLY | OFlag::O_CLOEXEC, 0)
            {
                Ok(file) => file,
                Err(_) => return false,
            };
            if writable {
                src_dir
                    .unlink_at(cstr!("init.rc"), UnlinkatFlags::NoRemoveDir)
                    .ok();
            }
            let mut dest_rc = match dest_dir.open_as_file_at(
                cstr!("init.rc"),
                OFlag::O_WRONLY | OFlag::O_CREAT | OFlag::O_TRUNC | OFlag::O_CLOEXEC,
                0,
            ) {
                Ok(file) => file,
                Err(_) => return false,
            };

            let mut reader = BufReader::new(src_rc);
            reader.for_each_line(|line| {
                if line.contains("start vaultkeeper") {
                    debug!("Remove vaultkeeper");
                    return true;
                }
                if line.starts_with("service flash_recovery") {
                    debug!("Remove flash_recovery");
                    writeln!(dest_rc, "service flash_recovery /system/bin/true").ok();
                    return true;
                }
                if line.starts_with("on property:persist.sys.zygote.early=") {
                    debug!("Invalidate persist.sys.zygote.early");
                    writeln!(dest_rc, "on property:persist.sys.zygote.early.xxxxx=true").ok();
                    return true;
                }
                write!(dest_rc, "{}", line).ok();
                true
            });

            writeln!(dest_rc).ok();
            inject_custom_rc(&self.rc_list, &mut dest_rc, tmp_path);
            inject_magisk_rc(&mut dest_rc, tmp_path);
            fclone_attr(reader.into_inner().as_raw_fd(), dest_rc.as_raw_fd()).log_ok();
        }

        loop {
            let Ok(entry) = src_dir.read() else {
                break;
            };
            let Some(entry) = entry else {
                break;
            };
            let name = entry.name().to_owned();
            let name_str = name.as_str();
            if !name_str.starts_with("init.zygote") || !name_str.ends_with(".rc") {
                continue;
            }

            let src_rc = match src_dir.open_as_file_at(&name, OFlag::O_RDONLY | OFlag::O_CLOEXEC, 0)
            {
                Ok(file) => file,
                Err(_) => continue,
            };

            if writable {
                src_dir
                    .unlink_at(&name, UnlinkatFlags::NoRemoveDir)
                    .ok();
            }
            let mut dest_rc = match dest_dir.open_as_file_at(
                &name,
                OFlag::O_WRONLY | OFlag::O_CREAT | OFlag::O_TRUNC | OFlag::O_CLOEXEC,
                0,
            ) {
                Ok(file) => file,
                Err(_) => continue,
            };

            debug!("Patching {} in {}", name, src_path);

            let mut reader = BufReader::new(src_rc);
            reader.for_each_line(|line| {
                if line.starts_with("service zygote ") {
                    debug!("Inject zygote restart");
                    write!(dest_rc, "{}", line).ok();
                    writeln!(
                        dest_rc,
                        "    onrestart exec {} 0 0 -- {}/magisk --zygote-restart",
                        MAGISK_PROC_CON,
                        tmp_path
                    )
                    .ok();
                    return true;
                }
                write!(dest_rc, "{}", line).ok();
                true
            });
            fclone_attr(reader.into_inner().as_raw_fd(), dest_rc.as_raw_fd()).log_ok();
        }

        src_dir.contains_path(cstr!("init.fission_host.rc"))
    }

    pub(crate) fn patch_fissiond(&mut self, tmp_path: &Utf8CStr) {
        debug!("Patching fissiond");
        if let Ok(mut fissiond) = base::MappedFile::open(cstr!("/system/bin/fissiond")) {
            let from = b"ro.build.system.fission_single_os";
            let to = b"ro.build.system.xxxxxxxxxxxxxxxxx";
            if fissiond.as_ref().len() >= from.len() {
                for idx in 0..=fissiond.as_ref().len() - from.len() {
                    if &fissiond.as_ref()[idx..idx + from.len()] == from {
                        fissiond.as_mut()[idx..idx + to.len()].copy_from_slice(to);
                        debug!(
                            "Patch @ {:08X} [ro.build.system.fission_single_os] -> [ro.build.system.xxxxxxxxxxxxxxxxx]",
                            idx
                        );
                    }
                }
            }

            let dir = cstr::buf::dynamic(256).join_path(ROOTOVL).join_path("/system/bin");
            dir.mkdirs(0o755).log_ok();
            let out = cstr::buf::dynamic(256)
                .join_path(ROOTOVL)
                .join_path("/system/bin/fissiond");
            if let Ok(mut target) = out.create(
                OFlag::O_WRONLY | OFlag::O_CREAT | OFlag::O_TRUNC | OFlag::O_CLOEXEC,
                0,
            ) {
                target.write_all(fissiond.as_ref()).ok();
                clone_attr(cstr!("/system/bin/fissiond"), &out).log_ok();
            }
        }

        debug!("hijack isolated");
        let Ok(mut hijack) = cstr!("/sys/devices/system/cpu/isolated").open(OFlag::O_RDONLY | OFlag::O_CLOEXEC) else {
            return;
        };

        cstr!(ISOLATED_PATH).mkfifo(0o777).log_ok();
        cstr!(ISOLATED_PATH)
            .bind_mount_to(cstr!("/sys/devices/system/cpu/isolated"), false)
            .log_ok();

        let pid = xfork();
        if pid == 0 {
                let Ok(mut dest) = cstr!(ISOLATED_PATH).open(OFlag::O_WRONLY | OFlag::O_CLOEXEC) else {
                    std::process::exit(1);
                };
                debug!("hijacked isolated");
                cstr!("/sys/devices/system/cpu/isolated").unmount().log_ok();
                cstr!(ISOLATED_PATH).remove().ok();

                let mut content = String::new();
                hijack.read_to_string(&mut content).ok();

                let target = cstr::buf::dynamic(256)
                    .join_path("/dev/cells/cell2")
                    .join_path(tmp_path);
                target.mkdirs(0).log_ok();
                tmp_path.bind_mount_to(&target, true).log_ok();
                self.mount_overlay(cstr!("/dev/cells/cell2"));

                write!(dest, "{}", content).ok();
                std::process::exit(0);
        }
    }
}

impl MagiskInit {
    pub(crate) fn parse_config_file(&mut self) {
        if let Ok(fd) = cstr!("/data/.backup/.magisk").open(OFlag::O_RDONLY) {
            let mut reader = BufReader::new(fd);
            reader.for_each_prop(|key, val| {
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
                        let recursive = cstr::buf::dynamic(256)
                            .join_path(module_path)
                            .exists();
                        if (recursive && e.is_dir()) || e.is_file() {
                            let buf = &mut cstr::buf::dynamic(256);
                            e.resolve_path(buf).log_ok();
                            if e.is_file() && buf.ends_with(".rc") {
                                let mut path;
                                if recursive {
                                    let stripped = buf.as_str()
                                        .strip_prefix(module_path.as_str())
                                        .unwrap_or(buf.as_str());
                                    path = cstr::buf::dynamic(256).join_path(stripped);
                                } else {
                                    path = cstr::buf::dynamic(256)
                                        .join_path("/")
                                        .join_path(e.name());
                                }
                                if path.exists() {
                                    debug!("Replace rc script [{}] -> [{}]", path, buf);
                                } else {
                                    path = cstr::buf::dynamic(256).join_path(buf);
                                    debug!("Found rc script [{}]", path);
                                    let mut rc_content = String::new();
                                    if let Ok(mut file) = path.open(OFlag::O_RDONLY | OFlag::O_CLOEXEC) {
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
                        let mut buf = cstr::buf::dynamic(256);
                        e.resolve_path(&mut buf).log_ok();
                        let path = cstr::buf::dynamic(256).join_path(&buf);
                        self.load_overlay_rc(&path, &buf);
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
        if let Ok(mut fd) = cstr!(ROOTMNT).create(OFlag::O_CREAT | OFlag::O_WRONLY, 0) {
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
