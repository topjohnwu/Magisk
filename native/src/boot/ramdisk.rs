use std::cmp::Ordering;
use std::collections::HashMap;
use std::str::from_utf8;

use base::libc::{S_IFDIR, S_IFMT, S_IFREG};
use base::{LoggedResult, Utf8CStr};

use crate::check_env;
use crate::cpio::{Cpio, CpioEntry};
use crate::patch::{patch_encryption, patch_verity};

pub trait MagiskCpio {
    fn patch(&mut self);
    fn test(&self) -> i32;
    fn restore(&mut self) -> LoggedResult<()>;
    fn backup(&mut self, origin: &Utf8CStr, skip_compress: bool) -> LoggedResult<()>;
}

const MAGISK_PATCHED: i32 = 1 << 0;
const UNSUPPORTED_CPIO: i32 = 1 << 1;
const SONY_INIT: i32 = 1 << 2;

impl MagiskCpio for Cpio {
    fn patch(&mut self) {
        let keep_verity = check_env("KEEPVERITY");
        let keep_force_encrypt = check_env("KEEPFORCEENCRYPT");
        eprintln!(
            "Patch with flag KEEPVERITY=[{}] KEEPFORCEENCRYPT=[{}]",
            keep_verity, keep_force_encrypt
        );
        self.entries.retain(|name, entry| {
            let fstab = (!keep_verity || !keep_force_encrypt)
                && entry.mode & S_IFMT == S_IFREG
                && !name.starts_with(".backup")
                && !name.starts_with("twrp")
                && !name.starts_with("recovery")
                && name.starts_with("fstab");
            if !keep_verity {
                if fstab {
                    eprintln!("Found fstab file [{}]", name);
                    let len = patch_verity(entry.data.as_mut_slice());
                    if len != entry.data.len() {
                        entry.data.resize(len, 0);
                    }
                } else if name == "verity_key" {
                    return false;
                }
            }
            if !keep_force_encrypt && fstab {
                let len = patch_encryption(entry.data.as_mut_slice());
                if len != entry.data.len() {
                    entry.data.resize(len, 0);
                }
            }
            true
        });
    }

    fn test(&self) -> i32 {
        let mut ret = 0;
        for file in [
            "sbin/launch_daemonsu.sh",
            "sbin/su",
            "init.xposed.rc",
            "boot/sbin/launch_daemonsu.sh",
        ] {
            if self.exists(file) {
                return UNSUPPORTED_CPIO;
            }
        }
        for file in [
            ".backup/.magisk",
            "init.magisk.rc",
            "overlay/init.magisk.rc",
        ] {
            if self.exists(file) {
                ret |= MAGISK_PATCHED;
                break;
            }
        }
        if self.exists("init.real") {
            ret |= SONY_INIT;
        }
        ret
    }

    fn restore(&mut self) -> LoggedResult<()> {
        let mut backups = HashMap::<String, Box<CpioEntry>>::new();
        let mut rm_list = String::new();
        self.entries
            .extract_if(|name, _| name.starts_with(".backup/"))
            .for_each(|(name, mut entry)| {
                if name == ".backup/.rmlist" {
                    if let Ok(data) = from_utf8(&entry.data) {
                        rm_list.push_str(data);
                    }
                } else if name != ".backup/.magisk" {
                    let new_name = if name.ends_with(".xz") && entry.decompress() {
                        &name[8..name.len() - 3]
                    } else {
                        &name[8..]
                    };
                    eprintln!("Restore [{}] -> [{}]", name, new_name);
                    backups.insert(new_name.to_string(), entry);
                }
            });
        self.rm(".backup", false);
        if rm_list.is_empty() && backups.is_empty() {
            self.entries.clear();
            return Ok(());
        }
        for rm in rm_list.split('\0') {
            if !rm.is_empty() {
                self.rm(rm, false);
            }
        }
        self.entries.extend(backups);

        Ok(())
    }

    fn backup(&mut self, origin: &Utf8CStr, skip_compress: bool) -> LoggedResult<()> {
        let mut backups = HashMap::<String, Box<CpioEntry>>::new();
        let mut rm_list = String::new();
        backups.insert(
            ".backup".to_string(),
            Box::new(CpioEntry {
                mode: S_IFDIR,
                uid: 0,
                gid: 0,
                rdevmajor: 0,
                rdevminor: 0,
                data: vec![],
            }),
        );
        let mut o = Cpio::load_from_file(origin)?;
        o.rm(".backup", true);
        self.rm(".backup", true);

        let mut lhs = o.entries.into_iter().peekable();
        let mut rhs = self.entries.iter().peekable();

        loop {
            enum Action<'a> {
                Backup(String, Box<CpioEntry>),
                Record(&'a String),
                Noop,
            }
            let action = match (lhs.peek(), rhs.peek()) {
                (Some((l, _)), Some((r, re))) => match l.as_str().cmp(r.as_str()) {
                    Ordering::Less => {
                        let (l, le) = lhs.next().unwrap();
                        Action::Backup(l, le)
                    }
                    Ordering::Greater => Action::Record(rhs.next().unwrap().0),
                    Ordering::Equal => {
                        let (l, le) = lhs.next().unwrap();
                        let action = if re.data != le.data {
                            Action::Backup(l, le)
                        } else {
                            Action::Noop
                        };
                        rhs.next();
                        action
                    }
                },
                (Some(_), None) => {
                    let (l, le) = lhs.next().unwrap();
                    Action::Backup(l, le)
                }
                (None, Some(_)) => Action::Record(rhs.next().unwrap().0),
                (None, None) => {
                    break;
                }
            };
            match action {
                Action::Backup(name, mut entry) => {
                    let backup = if !skip_compress && entry.compress() {
                        format!(".backup/{}.xz", name)
                    } else {
                        format!(".backup/{}", name)
                    };
                    eprintln!("Backup [{}] -> [{}]", name, backup);
                    backups.insert(backup, entry);
                }
                Action::Record(name) => {
                    eprintln!("Record new entry: [{}] -> [.backup/.rmlist]", name);
                    rm_list.push_str(&format!("{}\0", name));
                }
                Action::Noop => {}
            }
        }
        if !rm_list.is_empty() {
            backups.insert(
                ".backup/.rmlist".to_string(),
                Box::new(CpioEntry {
                    mode: S_IFREG,
                    uid: 0,
                    gid: 0,
                    rdevmajor: 0,
                    rdevminor: 0,
                    data: rm_list.as_bytes().to_vec(),
                }),
            );
        }
        self.entries.extend(backups);

        Ok(())
    }
}
