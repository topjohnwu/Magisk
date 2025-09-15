use crate::consts::MODULEROOT;
use crate::daemon::{MagiskD, to_user_id};
use crate::ffi::{ZygiskRequest, ZygiskStateFlags, get_magisk_tmp, update_deny_flags};
use crate::resetprop::{get_prop, set_prop};
use crate::socket::{IpcRead, UnixSocketExt};
use base::libc::STDOUT_FILENO;
use base::{
    Directory, FsPathBuilder, LoggedResult, ResultExt, Utf8CStr, WriteExt, cstr, fork_dont_care,
    libc, log_err, raw_cstr, warn,
};
use nix::fcntl::OFlag;
use std::fmt::Write;
use std::os::fd::{AsRawFd, RawFd};
use std::os::unix::net::UnixStream;
use std::ptr;
use std::sync::atomic::Ordering;

const NBPROP: &Utf8CStr = cstr!("ro.dalvik.vm.native.bridge");
const ZYGISKLDR: &str = "libzygisk.so";
const UNMOUNT_MASK: u32 =
    ZygiskStateFlags::ProcessOnDenyList.repr | ZygiskStateFlags::DenyListEnforced.repr;

pub fn zygisk_should_load_module(flags: u32) -> bool {
    flags & UNMOUNT_MASK != UNMOUNT_MASK && flags & ZygiskStateFlags::ProcessIsMagiskApp.repr == 0
}

#[allow(unused_variables)]
fn exec_zygiskd(is_64_bit: bool, remote: UnixStream) {
    // This fd has to survive exec
    unsafe {
        libc::fcntl(remote.as_raw_fd(), libc::F_SETFD, 0);
    }

    // Start building the exec arguments

    #[cfg(target_pointer_width = "64")]
    let magisk = if is_64_bit { "magisk" } else { "magisk32" };

    #[cfg(target_pointer_width = "32")]
    let magisk = "magisk";

    let exe = cstr::buf::new::<64>()
        .join_path(get_magisk_tmp())
        .join_path(magisk);

    let mut fd_str = cstr::buf::new::<16>();
    write!(fd_str, "{}", remote.as_raw_fd()).ok();
    unsafe {
        libc::execl(
            exe.as_ptr(),
            raw_cstr!(""),
            raw_cstr!("zygisk"),
            raw_cstr!("companion"),
            fd_str.as_ptr(),
            ptr::null() as *const libc::c_char,
        );
        libc::exit(-1);
    }
}

#[derive(Default)]
pub struct ZygiskState {
    pub lib_name: String,
    sockets: (Option<UnixStream>, Option<UnixStream>),
    start_count: u32 = 1,
}

impl ZygiskState {
    fn connect_zygiskd(&mut self, mut client: UnixStream, daemon: &MagiskD) -> LoggedResult<()> {
        let is_64_bit: bool = client.read_decodable()?;
        let socket = if is_64_bit {
            &mut self.sockets.1
        } else {
            &mut self.sockets.0
        };

        if let Some(fd) = socket {
            // Make sure the socket is still valid
            let mut pfd = libc::pollfd {
                fd: fd.as_raw_fd(),
                events: 0,
                revents: 0,
            };
            if unsafe { libc::poll(&mut pfd, 1, 0) } != 0 || pfd.revents != 0 {
                // Any revent means error
                *socket = None;
            }
        }

        let socket = if let Some(fd) = socket {
            fd
        } else {
            // Create a new socket pair and fork zygiskd process
            let (local, remote) = UnixStream::pair()?;
            if fork_dont_care() == 0 {
                exec_zygiskd(is_64_bit, remote);
            }
            *socket = Some(local);
            let local = socket.as_mut().unwrap();
            if let Some(module_fds) = daemon.get_module_fds(is_64_bit) {
                local.send_fds(&module_fds)?;
            }
            if local.read_decodable::<i32>()? != 0 {
                return log_err!();
            }
            local
        };
        socket.send_fds(&[client.as_raw_fd()])?;
        Ok(())
    }

    pub fn reset(&mut self, mut restore: bool) {
        if restore {
            self.start_count = 1;
        } else {
            self.sockets = (None, None);
            self.start_count += 1;
            if self.start_count > 3 {
                warn!("zygote crashed too many times, rolling-back");
                restore = true;
            }
        }

        if restore {
            self.restore_prop();
        } else {
            self.set_prop();
        }
    }

    pub fn set_prop(&mut self) {
        if !self.lib_name.is_empty() {
            return;
        }
        let orig = get_prop(NBPROP);
        self.lib_name = if orig.is_empty() || orig == "0" {
            ZYGISKLDR.to_string()
        } else {
            ZYGISKLDR.to_string() + &orig
        };
        set_prop(NBPROP, Utf8CStr::from_string(&mut self.lib_name));
        // Whether Huawei's Maple compiler is enabled.
        // If so, system server will be created by a special Zygote which ignores the native bridge
        // and make system server out of our control. Avoid it by disabling.
        if get_prop(cstr!("ro.maple.enable")) == "1" {
            set_prop(cstr!("ro.maple.enable"), cstr!("0"));
        }
    }

    pub fn restore_prop(&mut self) {
        let mut orig = "0".to_string();
        if self.lib_name.len() > ZYGISKLDR.len() {
            orig = self.lib_name[ZYGISKLDR.len()..].to_string();
        }
        set_prop(NBPROP, Utf8CStr::from_string(&mut orig));
        self.lib_name.clear();
    }
}

impl MagiskD {
    pub fn zygisk_handler(&self, mut client: UnixStream) {
        let _: LoggedResult<()> = try {
            let code = ZygiskRequest {
                repr: client.read_decodable()?,
            };
            match code {
                ZygiskRequest::GetInfo => self.get_process_info(client)?,
                ZygiskRequest::ConnectCompanion => self
                    .zygisk
                    .lock()
                    .unwrap()
                    .connect_zygiskd(client, self)
                    .log_with_msg(|w| w.write_str("zygiskd startup error"))?,
                ZygiskRequest::GetModDir => self.get_mod_dir(client)?,
                _ => {}
            }
        };
    }

    fn get_module_fds(&self, is_64_bit: bool) -> Option<Vec<RawFd>> {
        self.module_list.get().map(|module_list| {
            module_list
                .iter()
                .map(|m| if is_64_bit { m.z64 } else { m.z32 })
                // All fds passed over sockets have to be valid file descriptors.
                // To work around this issue, send over STDOUT_FILENO as an indicator of an
                // invalid fd as it will always be /dev/null in magiskd.
                .map(|fd| if fd < 0 { STDOUT_FILENO } else { fd })
                .collect()
        })
    }

    fn get_process_info(&self, mut client: UnixStream) -> LoggedResult<()> {
        let uid: i32 = client.read_decodable()?;
        let process: String = client.read_decodable()?;
        let is_64_bit: bool = client.read_decodable()?;
        let mut flags: u32 = 0;
        update_deny_flags(uid, &process, &mut flags);
        if self.get_manager_uid(to_user_id(uid)) == uid {
            flags |= ZygiskStateFlags::ProcessIsMagiskApp.repr
        }
        if self.uid_granted_root(uid) {
            flags |= ZygiskStateFlags::ProcessGrantedRoot.repr
        }

        // First send flags
        client.write_pod(&flags)?;

        // Next send modules
        if zygisk_should_load_module(flags)
            && let Some(module_fds) = self.get_module_fds(is_64_bit)
        {
            client.send_fds(&module_fds)?;
        }

        // If we're not in system_server, we are done
        if uid != 1000 || process != "system_server" {
            return Ok(());
        }

        // Read all failed modules
        let failed_ids: Vec<i32> = client.read_decodable()?;
        if let Some(module_list) = self.module_list.get() {
            for id in failed_ids {
                let path = cstr::buf::default()
                    .join_path(MODULEROOT)
                    .join_path(&module_list[id as usize].name)
                    .join_path("zygisk");
                // Create the unloaded marker file
                if let Ok(dir) = Directory::open(&path) {
                    dir.open_as_file_at(cstr!("unloaded"), OFlag::O_CREAT | OFlag::O_RDONLY, 0o644)
                        .log()
                        .ok();
                }
            }
        }

        Ok(())
    }

    fn get_mod_dir(&self, mut client: UnixStream) -> LoggedResult<()> {
        let id: i32 = client.read_decodable()?;
        let module = &self.module_list.get().unwrap()[id as usize];
        let dir = cstr::buf::default()
            .join_path(MODULEROOT)
            .join_path(&module.name);
        let fd = dir.open(OFlag::O_RDONLY | OFlag::O_CLOEXEC)?;
        client.send_fds(&[fd.as_raw_fd()])?;
        Ok(())
    }
}

// FFI to C++
impl MagiskD {
    pub fn zygisk_enabled(&self) -> bool {
        self.zygisk_enabled.load(Ordering::Acquire)
    }
}
