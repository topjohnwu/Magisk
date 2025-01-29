use crate::consts::MODULEROOT;
use crate::daemon::{to_user_id, IpcRead, MagiskD, UnixSocketExt};
use crate::ffi::{
    get_magisk_tmp, restore_zygisk_prop, update_deny_flags, ZygiskRequest, ZygiskStateFlags,
};
use base::libc::{O_CLOEXEC, O_CREAT, O_RDONLY};
use base::{
    cstr, error, fork_dont_care, libc, open_fd, raw_cstr, warn, Directory, FsPathBuf, LoggedError,
    LoggedResult, Utf8CStrBufArr, WriteExt,
};
use std::fmt::Write;
use std::os::fd::{AsRawFd, FromRawFd, RawFd};
use std::os::unix::net::UnixStream;
use std::ptr;
use std::sync::atomic::Ordering;

const UNMOUNT_MASK: u32 =
    ZygiskStateFlags::ProcessOnDenyList.repr | ZygiskStateFlags::DenyListEnforced.repr;

pub fn zygisk_should_load_module(flags: u32) -> bool {
    flags & UNMOUNT_MASK != UNMOUNT_MASK && flags & ZygiskStateFlags::ProcessIsMagiskApp.repr == 0
}

impl MagiskD {
    pub fn zygisk_handler(&self, client: i32) {
        let mut client = unsafe { UnixStream::from_raw_fd(client) };
        let _: LoggedResult<()> = try {
            let code = ZygiskRequest {
                repr: client.ipc_read_int()?,
            };
            match code {
                ZygiskRequest::GetInfo => self.get_process_info(client)?,
                ZygiskRequest::ConnectCompanion => self.connect_zygiskd(client),
                ZygiskRequest::GetModDir => self.get_mod_dir(client)?,
                _ => {}
            }
        };
    }

    pub fn zygisk_reset(&self, mut restore: bool) {
        if !self.zygisk_enabled.load(Ordering::Acquire) {
            return;
        }

        if restore {
            self.zygote_start_count.store(1, Ordering::Release);
        } else {
            *self.zygiskd_sockets.lock().unwrap() = (None, None);
            if self.zygote_start_count.fetch_add(1, Ordering::AcqRel) > 3 {
                warn!("zygote crashes too many times, rolling-back");
                restore = true;
            }
        }

        if restore {
            restore_zygisk_prop();
        }
    }

    fn get_module_fds(&self, is_64_bit: bool) -> Option<Vec<RawFd>> {
        self.module_list.get().map(|module_list| {
            module_list
                .iter()
                .map(|m| if is_64_bit { m.z64 } else { m.z32 })
                .collect()
        })
    }

    fn exec_zygiskd(is_64_bit: bool, remote: UnixStream) {
        // This fd has to survive exec
        unsafe {
            libc::fcntl(remote.as_raw_fd(), libc::F_SETFD, 0);
        }

        // Start building the exec arguments
        let mut exe = Utf8CStrBufArr::<64>::new();

        #[cfg(target_pointer_width = "64")]
        let magisk = if is_64_bit { "magisk" } else { "magisk32" };

        #[cfg(target_pointer_width = "32")]
        let magisk = "magisk";

        let exe = FsPathBuf::new(&mut exe).join(get_magisk_tmp()).join(magisk);

        let mut fd_str = Utf8CStrBufArr::<16>::new();
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

    fn connect_zygiskd(&self, mut client: UnixStream) {
        let mut zygiskd_sockets = self.zygiskd_sockets.lock().unwrap();
        let result: LoggedResult<()> = try {
            let is_64_bit = client.ipc_read_int()? != 0;
            let socket = if is_64_bit {
                &mut zygiskd_sockets.1
            } else {
                &mut zygiskd_sockets.0
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
                    Self::exec_zygiskd(is_64_bit, remote);
                }
                *socket = Some(local);
                let local = socket.as_mut().unwrap();
                if let Some(module_fds) = self.get_module_fds(is_64_bit) {
                    local.send_fds(&module_fds)?;
                }
                if local.ipc_read_int()? != 0 {
                    Err(LoggedError::default())?;
                }
                local
            };
            socket.send_fds(&[client.as_raw_fd()])?;
        };
        if result.is_err() {
            error!("zygiskd startup error");
        }
    }

    fn get_process_info(&self, mut client: UnixStream) -> LoggedResult<()> {
        let uid = client.ipc_read_int()?;
        let process = client.ipc_read_string()?;
        let is_64_bit = client.ipc_read_int()? != 0;
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
        if zygisk_should_load_module(flags) {
            if let Some(module_list) = self.module_list.get() {
                let module_fds: Vec<RawFd> = module_list
                    .iter()
                    .map(|m| if is_64_bit { m.z64 } else { m.z32 })
                    .collect();
                client.send_fds(&module_fds)?;
            }
        }

        // If we're not in system_server, we are done
        if uid != 1000 || process != "system_server" {
            return Ok(());
        }

        // Read all failed modules
        let failed_ids: Vec<i32> = client.ipc_read_vec()?;
        if let Some(module_list) = self.module_list.get() {
            for id in failed_ids {
                let mut buf = Utf8CStrBufArr::default();
                let path = FsPathBuf::new(&mut buf)
                    .join(MODULEROOT)
                    .join(&module_list[id as usize].name)
                    .join("zygisk");
                // Create the unloaded marker file
                Directory::open(&path)?.open_fd(cstr!("unloaded"), O_CREAT | O_RDONLY, 0o644)?;
            }
        }

        Ok(())
    }

    fn get_mod_dir(&self, mut client: UnixStream) -> LoggedResult<()> {
        let id = client.ipc_read_int()?;
        let module = &self.module_list.get().unwrap()[id as usize];
        let mut buf = Utf8CStrBufArr::default();
        let dir = FsPathBuf::new(&mut buf).join(MODULEROOT).join(&module.name);
        let fd = open_fd!(&dir, O_RDONLY | O_CLOEXEC)?;
        client.send_fds(&[fd.as_raw_fd()])?;
        Ok(())
    }
}
