use crate::consts::{MAGISK_FULL_VER, MAIN_CONFIG};
use crate::db::Sqlite3;
use crate::ffi::{get_magisk_tmp, ModuleInfo, RequestCode};
use crate::get_prop;
use crate::logging::{magisk_logging, start_log_daemon};
use crate::package::ManagerInfo;
use base::libc::{O_CLOEXEC, O_RDONLY};
use base::{
    cstr, info, libc, open_fd, warn, BufReadExt, Directory, FsPath, FsPathBuf, LoggedResult,
    ReadExt, Utf8CStr, Utf8CStrBufArr, WriteExt,
};
use bit_set::BitSet;
use bytemuck::{bytes_of, bytes_of_mut, Pod, Zeroable};
use std::fs::File;
use std::io;
use std::io::{BufReader, ErrorKind, IoSlice, IoSliceMut, Read, Write};
use std::os::fd::{FromRawFd, OwnedFd, RawFd};
use std::os::unix::net::{AncillaryData, SocketAncillary, UnixStream};
use std::sync::{Mutex, OnceLock};

// Global magiskd singleton
pub static MAGISKD: OnceLock<MagiskD> = OnceLock::new();

#[repr(u32)]
enum BootState {
    PostFsDataDone = (1 << 0),
    LateStartDone = (1 << 1),
    BootComplete = (1 << 2),
    SafeMode = (1 << 3),
}

#[derive(Default)]
#[repr(transparent)]
struct BootStateFlags(u32);

impl BootStateFlags {
    fn contains(&self, stage: BootState) -> bool {
        (self.0 & stage as u32) != 0
    }

    fn set(&mut self, stage: BootState) {
        self.0 |= stage as u32;
    }
}

pub const AID_ROOT: i32 = 0;
pub const AID_SHELL: i32 = 2000;
pub const AID_APP_START: i32 = 10000;
pub const AID_APP_END: i32 = 19999;
pub const AID_USER_OFFSET: i32 = 100000;

pub const fn to_app_id(uid: i32) -> i32 {
    uid % AID_USER_OFFSET
}

pub const fn to_user_id(uid: i32) -> i32 {
    uid / AID_USER_OFFSET
}

#[derive(Default)]
pub struct MagiskD {
    pub sql_connection: Mutex<Option<Sqlite3>>,
    pub manager_info: Mutex<ManagerInfo>,
    boot_stage_lock: Mutex<BootStateFlags>,
    pub module_list: OnceLock<Vec<ModuleInfo>>,
    sdk_int: i32,
    pub is_emulator: bool,
    is_recovery: bool,
}

impl MagiskD {
    pub fn is_recovery(&self) -> bool {
        self.is_recovery
    }

    pub fn sdk_int(&self) -> i32 {
        self.sdk_int
    }

    pub fn set_module_list(&self, module_list: Vec<ModuleInfo>) {
        self.module_list.set(module_list).ok();
    }

    pub fn module_list(&self) -> &Vec<ModuleInfo> {
        self.module_list.get().unwrap()
    }

    pub fn app_data_dir(&self) -> &'static Utf8CStr {
        if self.sdk_int >= 24 {
            cstr!("/data/user_de")
        } else {
            cstr!("/data/user")
        }
    }

    // app_id = app_no + AID_APP_START
    // app_no range: [0, 9999]
    pub fn get_app_no_list(&self) -> BitSet {
        let mut list = BitSet::new();
        let _: LoggedResult<()> = try {
            let mut app_data_dir = Directory::open(self.app_data_dir())?;
            // For each user
            loop {
                let entry = match app_data_dir.read()? {
                    None => break,
                    Some(e) => e,
                };
                let mut user_dir = match entry.open_as_dir() {
                    Err(_) => continue,
                    Ok(dir) => dir,
                };
                // For each package
                loop {
                    match user_dir.read()? {
                        None => break,
                        Some(e) => {
                            let attr = e.get_attr()?;
                            let app_id = to_app_id(attr.st.st_uid as i32);
                            if (AID_APP_START..=AID_APP_END).contains(&app_id) {
                                let app_no = app_id - AID_APP_START;
                                list.insert(app_no as usize);
                            }
                        }
                    }
                }
            }
        };
        list
    }

    pub fn boot_stage_handler(&self, client: i32, code: i32) {
        // Make sure boot stage execution is always serialized
        let mut state = self.boot_stage_lock.lock().unwrap();

        let code = RequestCode { repr: code };
        match code {
            RequestCode::POST_FS_DATA => {
                if check_data() && !state.contains(BootState::PostFsDataDone) {
                    if self.post_fs_data() {
                        state.set(BootState::SafeMode);
                    }
                    state.set(BootState::PostFsDataDone);
                }
                unsafe { libc::close(client) };
            }
            RequestCode::LATE_START => {
                unsafe { libc::close(client) };
                if state.contains(BootState::PostFsDataDone) && !state.contains(BootState::SafeMode)
                {
                    self.late_start();
                    state.set(BootState::LateStartDone);
                }
            }
            RequestCode::BOOT_COMPLETE => {
                unsafe { libc::close(client) };
                if state.contains(BootState::PostFsDataDone) {
                    state.set(BootState::BootComplete);
                    self.boot_complete()
                }
            }
            _ => {
                unsafe { libc::close(client) };
            }
        }
    }
}

pub fn daemon_entry() {
    start_log_daemon();
    magisk_logging();
    info!("Magisk {} daemon started", MAGISK_FULL_VER);

    let is_emulator = get_prop(cstr!("ro.kernel.qemu"), false) == "1"
        || get_prop(cstr!("ro.boot.qemu"), false) == "1"
        || get_prop(cstr!("ro.product.device"), false).contains("vsoc");

    // Load config status
    let mut buf = Utf8CStrBufArr::<64>::new();
    let path = FsPathBuf::new(&mut buf)
        .join(get_magisk_tmp())
        .join(MAIN_CONFIG);
    let mut is_recovery = false;
    if let Ok(file) = path.open(O_RDONLY | O_CLOEXEC) {
        let mut file = BufReader::new(file);
        file.foreach_props(|key, val| {
            if key == "RECOVERYMODE" {
                is_recovery = val == "true";
                return false;
            }
            true
        });
    }

    let mut sdk_int = -1;
    if let Ok(file) = FsPath::from(cstr!("/system/build.prop")).open(O_RDONLY | O_CLOEXEC) {
        let mut file = BufReader::new(file);
        file.foreach_props(|key, val| {
            if key == "ro.build.version.sdk" {
                sdk_int = val.parse::<i32>().unwrap_or(-1);
                return false;
            }
            true
        });
    }
    if sdk_int < 0 {
        // In case some devices do not store this info in build.prop, fallback to getprop
        sdk_int = get_prop(cstr!("ro.build.version.sdk"), false)
            .parse::<i32>()
            .unwrap_or(-1);
    }
    info!("* Device API level: {}", sdk_int);

    let magiskd = MagiskD {
        sdk_int,
        is_emulator,
        is_recovery,
        ..Default::default()
    };
    MAGISKD.set(magiskd).ok();
}

fn check_data() -> bool {
    if let Ok(fd) = open_fd!(cstr!("/proc/mounts"), O_RDONLY | O_CLOEXEC) {
        let file = File::from(fd);
        let mut mnt = false;
        BufReader::new(file).foreach_lines(|line| {
            if line.contains(" /data ") && !line.contains("tmpfs") {
                mnt = true;
                return false;
            }
            true
        });
        if !mnt {
            return false;
        }
        let crypto = get_prop(cstr!("ro.crypto.state"), false);
        return if !crypto.is_empty() {
            if crypto != "encrypted" {
                // Unencrypted, we can directly access data
                true
            } else {
                // Encrypted, check whether vold is started
                !get_prop(cstr!("init.svc.vold"), false).is_empty()
            }
        } else {
            // ro.crypto.state is not set, assume it's unencrypted
            true
        };
    }
    false
}

pub fn get_magiskd() -> &'static MagiskD {
    unsafe { MAGISKD.get().unwrap_unchecked() }
}

pub trait IpcRead {
    fn ipc_read_int(&mut self) -> io::Result<i32>;
    fn ipc_read_string(&mut self) -> io::Result<String>;
    fn ipc_read_vec<E: Pod>(&mut self) -> io::Result<Vec<E>>;
}

impl<T: Read> IpcRead for T {
    fn ipc_read_int(&mut self) -> io::Result<i32> {
        let mut val: i32 = 0;
        self.read_pod(&mut val)?;
        Ok(val)
    }

    fn ipc_read_string(&mut self) -> io::Result<String> {
        let len = self.ipc_read_int()?;
        let mut val = "".to_string();
        self.take(len as u64).read_to_string(&mut val)?;
        Ok(val)
    }

    fn ipc_read_vec<E: Pod>(&mut self) -> io::Result<Vec<E>> {
        let len = self.ipc_read_int()? as usize;
        let mut vec = Vec::new();
        let mut val: E = Zeroable::zeroed();
        for _ in 0..len {
            self.read_pod(&mut val)?;
            vec.push(val.clone());
        }
        Ok(vec)
    }
}

pub trait IpcWrite {
    fn ipc_write_int(&mut self, val: i32) -> io::Result<()>;
    fn ipc_write_string(&mut self, val: &str) -> io::Result<()>;
}

impl<T: Write> IpcWrite for T {
    fn ipc_write_int(&mut self, val: i32) -> io::Result<()> {
        self.write_pod(&val)
    }

    fn ipc_write_string(&mut self, val: &str) -> io::Result<()> {
        self.ipc_write_int(val.len() as i32)?;
        self.write_all(val.as_bytes())
    }
}

pub trait UnixSocketExt {
    fn send_fds(&mut self, fd: &[RawFd]) -> io::Result<()>;
    fn recv_fd(&mut self) -> io::Result<Option<OwnedFd>>;
    fn recv_fds(&mut self) -> io::Result<Vec<OwnedFd>>;
}

impl UnixSocketExt for UnixStream {
    fn send_fds(&mut self, fds: &[RawFd]) -> io::Result<()> {
        match fds.len() {
            0 => self.ipc_write_int(-1)?,
            len => {
                // 4k buffer is reasonable enough
                let mut buf = [0u8; 4096];
                let mut ancillary = SocketAncillary::new(&mut buf);
                if !ancillary.add_fds(fds) {
                    return Err(ErrorKind::OutOfMemory.into());
                }
                let fd_count = len as i32;
                let iov = IoSlice::new(bytes_of(&fd_count));
                self.send_vectored_with_ancillary(&[iov], &mut ancillary)?;
            }
        };
        Ok(())
    }

    fn recv_fd(&mut self) -> io::Result<Option<OwnedFd>> {
        let mut fd_count = 0;
        self.peek(bytes_of_mut(&mut fd_count))?;
        if fd_count < 1 {
            return Ok(None);
        }

        // 4k buffer is reasonable enough
        let mut buf = [0u8; 4096];
        let mut ancillary = SocketAncillary::new(&mut buf);
        let iov = IoSliceMut::new(bytes_of_mut(&mut fd_count));
        self.recv_vectored_with_ancillary(&mut [iov], &mut ancillary)?;
        for msg in ancillary.messages() {
            if let Ok(msg) = msg {
                if let AncillaryData::ScmRights(mut scm_rights) = msg {
                    // We only want the first one
                    let fd = if let Some(fd) = scm_rights.next() {
                        unsafe { OwnedFd::from_raw_fd(fd) }
                    } else {
                        return Ok(None);
                    };
                    // Close all others
                    for fd in scm_rights {
                        unsafe { libc::close(fd) };
                    }
                    return Ok(Some(fd));
                }
            }
        }
        Ok(None)
    }

    fn recv_fds(&mut self) -> io::Result<Vec<OwnedFd>> {
        let mut fd_count = 0;
        // 4k buffer is reasonable enough
        let mut buf = [0u8; 4096];
        let mut ancillary = SocketAncillary::new(&mut buf);
        let iov = IoSliceMut::new(bytes_of_mut(&mut fd_count));
        self.recv_vectored_with_ancillary(&mut [iov], &mut ancillary)?;
        let mut fds: Vec<OwnedFd> = Vec::new();
        for msg in ancillary.messages() {
            if let Ok(msg) = msg {
                if let AncillaryData::ScmRights(scm_rights) = msg {
                    fds = scm_rights
                        .map(|fd| unsafe { OwnedFd::from_raw_fd(fd) })
                        .collect();
                }
            }
        }
        if fd_count as usize != fds.len() {
            warn!(
                "Received unexpected number of fds: expected={} actual={}",
                fd_count,
                fds.len()
            );
        }
        Ok(fds)
    }
}
