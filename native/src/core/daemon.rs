use crate::bootstages::BootState;
use crate::consts::{
    MAGISK_FILE_CON, MAGISK_FULL_VER, MAGISK_PROC_CON, MAGISK_VER_CODE, MAGISK_VERSION,
    MAIN_CONFIG, MAIN_SOCKET, ROOTMNT, ROOTOVL,
};
use crate::db::Sqlite3;
use crate::ffi::{
    ModuleInfo, RequestCode, RespondCode, denylist_handler, get_magisk_tmp, scan_deny_apps,
};
use crate::logging::{android_logging, magisk_logging, setup_logfile, start_log_daemon};
use crate::module::remove_modules;
use crate::package::ManagerInfo;
use crate::resetprop::{get_prop, set_prop};
use crate::selinux::restore_tmpcon;
use crate::socket::{IpcRead, IpcWrite};
use crate::su::SuInfo;
use crate::thread::ThreadPool;
use crate::zygisk::ZygiskState;
use base::const_format::concatcp;
use base::{
    AtomicArc, BufReadExt, FileAttr, FsPathBuilder, LoggedResult, ReadExt, ResultExt, Utf8CStr,
    Utf8CStrBuf, WriteExt, cstr, fork_dont_care, info, libc, log_err, set_nice_name,
};
use nix::fcntl::OFlag;
use nix::mount::MsFlags;
use nix::sys::signal::SigSet;
use nix::unistd::{dup2_stderr, dup2_stdin, dup2_stdout, getpid, getuid, setsid};
use num_traits::AsPrimitive;
use std::fmt::Write as _;
use std::io::{BufReader, Write};
use std::os::fd::{AsFd, AsRawFd, IntoRawFd, RawFd};
use std::os::unix::net::{UCred, UnixListener, UnixStream};
use std::process::{Command, exit};
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::{Mutex, OnceLock};
use std::time::Duration;

// Global magiskd singleton
pub static MAGISKD: OnceLock<MagiskD> = OnceLock::new();

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
    pub boot_stage_lock: Mutex<BootState>,
    pub module_list: OnceLock<Vec<ModuleInfo>>,
    pub zygisk_enabled: AtomicBool,
    pub zygisk: Mutex<ZygiskState>,
    pub cached_su_info: AtomicArc<SuInfo>,
    pub sdk_int: i32,
    pub is_emulator: bool,
    is_recovery: bool,
    exe_attr: FileAttr,
}

impl MagiskD {
    pub fn get() -> &'static MagiskD {
        unsafe { MAGISKD.get().unwrap_unchecked() }
    }

    pub fn sdk_int(&self) -> i32 {
        self.sdk_int
    }

    pub fn app_data_dir(&self) -> &'static Utf8CStr {
        if self.sdk_int >= 24 {
            cstr!("/data/user_de")
        } else {
            cstr!("/data/user")
        }
    }

    fn handle_request_sync(&self, mut client: UnixStream, code: RequestCode) {
        match code {
            RequestCode::CHECK_VERSION => {
                #[cfg(debug_assertions)]
                let s = concatcp!(MAGISK_VERSION, ":MAGISK:D");
                #[cfg(not(debug_assertions))]
                let s = concatcp!(MAGISK_VERSION, ":MAGISK:R");

                client.write_encodable(s).log_ok();
            }
            RequestCode::CHECK_VERSION_CODE => {
                client.write_pod(&MAGISK_VER_CODE).log_ok();
            }
            RequestCode::START_DAEMON => {
                setup_logfile();
            }
            RequestCode::STOP_DAEMON => {
                // Unmount all overlays
                denylist_handler(-1);

                // Restore native bridge property
                self.zygisk.lock().unwrap().restore_prop();

                client.write_pod(&0).log_ok();

                // Terminate the daemon!
                exit(0);
            }
            _ => {}
        }
    }

    fn handle_request_async(&self, mut client: UnixStream, code: RequestCode, cred: UCred) {
        match code {
            RequestCode::DENYLIST => {
                denylist_handler(client.into_raw_fd());
            }
            RequestCode::SUPERUSER => {
                self.su_daemon_handler(client, cred);
            }
            RequestCode::ZYGOTE_RESTART => {
                info!("** zygote restarted");
                self.prune_su_access();
                scan_deny_apps();
                if self.zygisk_enabled.load(Ordering::Relaxed) {
                    self.zygisk.lock().unwrap().reset(false);
                }
            }
            RequestCode::SQLITE_CMD => {
                self.db_exec_for_cli(client).ok();
            }
            RequestCode::REMOVE_MODULES => {
                let do_reboot: bool = client.read_decodable().log().unwrap_or_default();
                remove_modules();
                client.write_pod(&0).log_ok();
                if do_reboot {
                    self.reboot();
                }
            }
            RequestCode::ZYGISK => {
                self.zygisk_handler(client);
            }
            _ => {}
        }
    }

    fn reboot(&self) {
        if self.is_recovery {
            Command::new("/system/bin/reboot").arg("recovery").status()
        } else {
            Command::new("/system/bin/reboot").status()
        }
        .ok();
    }

    #[cfg(feature = "check-client")]
    fn is_client(&self, pid: i32) -> bool {
        let mut buf = cstr::buf::new::<32>();
        write!(buf, "/proc/{pid}/exe").ok();
        if let Ok(attr) = buf.follow_link().get_attr() {
            attr.st.st_dev == self.exe_attr.st.st_dev && attr.st.st_ino == self.exe_attr.st.st_ino
        } else {
            false
        }
    }

    #[cfg(not(feature = "check-client"))]
    fn is_client(&self, pid: i32) -> bool {
        true
    }

    fn handle_requests(&'static self, mut client: UnixStream) {
        let Ok(cred) = client.peer_cred() else {
            // Client died
            return;
        };

        // There are no abstractions for SO_PEERSEC yet, call the raw C API.
        let mut context = cstr::buf::new::<256>();
        unsafe {
            let mut len: libc::socklen_t = context.capacity().as_();
            libc::getsockopt(
                client.as_raw_fd(),
                libc::SOL_SOCKET,
                libc::SO_PEERSEC,
                context.as_mut_ptr().cast(),
                &mut len,
            );
        }
        context.rebuild().ok();

        let is_root = cred.uid == 0;
        let is_shell = cred.uid == 2000;
        let is_zygote = &context == "u:r:zygote:s0";

        if !is_root && !is_zygote && !self.is_client(cred.pid.unwrap_or(-1)) {
            // Unsupported client state
            client.write_pod(&RespondCode::ACCESS_DENIED.repr).log_ok();
            return;
        }

        let mut code = -1;
        client.read_pod(&mut code).ok();
        if !(0..RequestCode::END.repr).contains(&code)
            || code == RequestCode::_SYNC_BARRIER_.repr
            || code == RequestCode::_STAGE_BARRIER_.repr
        {
            // Unknown request code
            return;
        }

        let code = RequestCode { repr: code };

        // Permission checks
        match code {
            RequestCode::POST_FS_DATA
            | RequestCode::LATE_START
            | RequestCode::BOOT_COMPLETE
            | RequestCode::ZYGOTE_RESTART
            | RequestCode::SQLITE_CMD
            | RequestCode::DENYLIST
            | RequestCode::STOP_DAEMON => {
                if !is_root {
                    client.write_pod(&RespondCode::ROOT_REQUIRED.repr).log_ok();
                    return;
                }
            }
            RequestCode::REMOVE_MODULES => {
                if !is_root && !is_shell {
                    // Only allow root and ADB shell to remove modules
                    client.write_pod(&RespondCode::ACCESS_DENIED.repr).log_ok();
                    return;
                }
            }
            RequestCode::ZYGISK => {
                if !is_zygote {
                    // Invalid client context
                    client.write_pod(&RespondCode::ACCESS_DENIED.repr).log_ok();
                    return;
                }
            }
            _ => {}
        }

        if client.write_pod(&RespondCode::OK.repr).is_err() {
            return;
        }

        if code.repr < RequestCode::_SYNC_BARRIER_.repr {
            self.handle_request_sync(client, code)
        } else if code.repr < RequestCode::_STAGE_BARRIER_.repr {
            ThreadPool::exec_task(move || {
                self.handle_request_async(client, code, cred);
            })
        } else {
            ThreadPool::exec_task(move || {
                self.boot_stage_handler(client, code);
            })
        }
    }
}

fn switch_cgroup(cgroup: &str, pid: i32) {
    let mut buf = cstr::buf::new::<64>()
        .join_path(cgroup)
        .join_path("cgroup.procs");
    if !buf.exists() {
        return;
    }
    if let Ok(mut file) = buf.open(OFlag::O_WRONLY | OFlag::O_APPEND | OFlag::O_CLOEXEC) {
        buf.clear();
        write!(buf, "{pid}").ok();
        file.write_all(buf.as_bytes()).log_ok();
    }
}

fn daemon_entry() {
    set_nice_name(cstr!("magiskd"));
    android_logging();

    // Block all signals
    SigSet::all().thread_set_mask().log_ok();

    // Swap out the original stdio
    if let Ok(null) = cstr!("/dev/null").open(OFlag::O_WRONLY).log() {
        dup2_stdout(null.as_fd()).log_ok();
        dup2_stderr(null.as_fd()).log_ok();
    }
    if let Ok(zero) = cstr!("/dev/zero").open(OFlag::O_RDONLY).log() {
        dup2_stdin(zero).log_ok();
    }

    setsid().log_ok();

    // Make sure the current context is magisk
    if let Ok(mut current) =
        cstr!("/proc/self/attr/current").open(OFlag::O_WRONLY | OFlag::O_CLOEXEC)
    {
        let con = cstr!(MAGISK_PROC_CON);
        current.write_all(con.as_bytes_with_nul()).log_ok();
    }

    start_log_daemon();
    magisk_logging();
    info!("Magisk {MAGISK_FULL_VER} daemon started");

    let is_emulator = get_prop(cstr!("ro.kernel.qemu")) == "1"
        || get_prop(cstr!("ro.boot.qemu")) == "1"
        || get_prop(cstr!("ro.product.device")).contains("vsoc");

    // Load config status
    let magisk_tmp = get_magisk_tmp();
    let mut tmp_path = cstr::buf::new::<64>()
        .join_path(magisk_tmp)
        .join_path(MAIN_CONFIG);
    let mut is_recovery = false;
    if let Ok(main_config) = tmp_path.open(OFlag::O_RDONLY | OFlag::O_CLOEXEC) {
        BufReader::new(main_config).for_each_prop(|key, val| {
            if key == "RECOVERYMODE" {
                is_recovery = val == "true";
                return false;
            }
            true
        });
    }
    tmp_path.truncate(magisk_tmp.len());

    let mut sdk_int = -1;
    if let Ok(build_prop) = cstr!("/system/build.prop").open(OFlag::O_RDONLY | OFlag::O_CLOEXEC) {
        BufReader::new(build_prop).for_each_prop(|key, val| {
            if key == "ro.build.version.sdk" {
                sdk_int = val.parse::<i32>().unwrap_or(-1);
                return false;
            }
            true
        });
    }
    if sdk_int < 0 {
        // In case some devices do not store this info in build.prop, fallback to getprop
        sdk_int = get_prop(cstr!("ro.build.version.sdk"))
            .parse::<i32>()
            .unwrap_or(-1);
    }
    info!("* Device API level: {sdk_int}");

    restore_tmpcon().log_ok();

    // Escape from cgroup
    let pid = getpid().as_raw();
    switch_cgroup("/acct", pid);
    switch_cgroup("/dev/cg2_bpf", pid);
    switch_cgroup("/sys/fs/cgroup", pid);
    if get_prop(cstr!("ro.config.per_app_memcg")) != "false" {
        switch_cgroup("/dev/memcg/apps", pid);
    }

    // Samsung workaround #7887
    if cstr!("/system_ext/app/mediatek-res/mediatek-res.apk").exists() {
        set_prop(cstr!("ro.vendor.mtk_model"), cstr!("0"));
    }

    // Cleanup pre-init mounts
    tmp_path.append_path(ROOTMNT);
    if let Ok(mount_list) = tmp_path.open(OFlag::O_RDONLY | OFlag::O_CLOEXEC) {
        BufReader::new(mount_list).for_each_line(|line| {
            line.truncate(line.trim_end().len());
            let item = Utf8CStr::from_string(line);
            item.unmount().log_ok();
            true
        })
    }
    tmp_path.truncate(magisk_tmp.len());

    // Remount rootfs as read-only if requested
    if std::env::var_os("REMOUNT_ROOT").is_some() {
        cstr!("/").remount_mount_flags(MsFlags::MS_RDONLY).log_ok();
        unsafe { std::env::remove_var("REMOUNT_ROOT") };
    }

    // Remove all pre-init overlay files to free-up memory
    tmp_path.append_path(ROOTOVL);
    tmp_path.remove_all().ok();
    tmp_path.truncate(magisk_tmp.len());

    let exe_attr = cstr!("/proc/self/exe")
        .follow_link()
        .get_attr()
        .log()
        .unwrap_or_default();

    let daemon = MagiskD {
        sdk_int,
        is_emulator,
        is_recovery,
        exe_attr,
        ..Default::default()
    };
    MAGISKD.set(daemon).ok();

    let sock_path = cstr::buf::new::<64>()
        .join_path(get_magisk_tmp())
        .join_path(MAIN_SOCKET);
    sock_path.remove().ok();

    let Ok(sock) = UnixListener::bind(&sock_path).log() else {
        exit(1);
    };

    sock_path.follow_link().chmod(0o666).log_ok();
    sock_path.set_secontext(cstr!(MAGISK_FILE_CON)).log_ok();

    // Loop forever to listen for requests
    let daemon = MagiskD::get();
    for client in sock.incoming() {
        if let Ok(client) = client.log() {
            daemon.handle_requests(client);
        } else {
            exit(1);
        }
    }
}

pub fn connect_daemon(code: RequestCode, create: bool) -> LoggedResult<UnixStream> {
    let sock_path = cstr::buf::new::<64>()
        .join_path(get_magisk_tmp())
        .join_path(MAIN_SOCKET);

    fn send_request(code: RequestCode, mut socket: UnixStream) -> LoggedResult<UnixStream> {
        socket.write_pod(&code.repr).log_ok();
        let mut res = -1;
        socket.read_pod(&mut res).log_ok();
        let res = RespondCode { repr: res };
        match res {
            RespondCode::OK => Ok(socket),
            RespondCode::ROOT_REQUIRED => {
                log_err!("Root is required for this operation")
            }
            RespondCode::ACCESS_DENIED => {
                log_err!("Accessed denied")
            }
            _ => {
                log_err!("Daemon error")
            }
        }
    }

    match UnixStream::connect(&sock_path) {
        Ok(socket) => send_request(code, socket),
        Err(e) => {
            if !create || !getuid().is_root() {
                return log_err!("Cannot connect to daemon: {e}");
            }

            let mut buf = cstr::buf::new::<64>();
            if cstr!("/proc/self/exe").read_link(&mut buf).is_err()
                || !buf.starts_with(get_magisk_tmp().as_str())
            {
                return log_err!("Start daemon on magisk tmpfs");
            }

            // Fork a process and run the daemon
            if fork_dont_care() == 0 {
                daemon_entry();
                exit(0);
            }

            // In the client, we keep retry and connect to the socket
            loop {
                if let Ok(socket) = UnixStream::connect(&sock_path) {
                    return send_request(code, socket);
                } else {
                    std::thread::sleep(Duration::from_millis(100));
                }
            }
        }
    }
}

pub fn connect_daemon_for_cxx(code: RequestCode, create: bool) -> RawFd {
    connect_daemon(code, create)
        .map(IntoRawFd::into_raw_fd)
        .unwrap_or(-1)
}
