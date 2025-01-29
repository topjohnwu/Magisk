use crate::consts::MODULEROOT;
use crate::daemon::{to_user_id, IpcRead, MagiskD, UnixSocketExt};
use crate::ffi::{update_deny_flags, ZygiskRequest, ZygiskStateFlags};
use base::libc::{O_CLOEXEC, O_CREAT, O_RDONLY};
use base::{cstr, open_fd, Directory, FsPathBuf, LoggedResult, Utf8CStrBufArr, WriteExt};
use std::os::fd::{AsRawFd, FromRawFd, RawFd};
use std::os::unix::net::UnixStream;

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
                ZygiskRequest::ConnectCompanion => self.connect_zygiskd(client.as_raw_fd()),
                ZygiskRequest::GetModDir => self.get_mod_dir(client)?,
                _ => {}
            }
        };
    }

    pub fn get_module_fds(&self, is_64_bit: bool) -> Vec<RawFd> {
        if let Some(module_list) = self.module_list.get() {
            module_list
                .iter()
                .map(|m| if is_64_bit { m.z64 } else { m.z32 })
                .collect()
        } else {
            Vec::new()
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
