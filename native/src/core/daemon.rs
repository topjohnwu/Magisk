use crate::consts::{MAGISK_FULL_VER, MAIN_CONFIG, SECURE_DIR};
use crate::db::Sqlite3;
use crate::ffi::{
    check_key_combo, disable_modules, exec_common_scripts, exec_module_scripts, get_magisk_tmp,
    initialize_denylist, setup_magisk_env, DbEntryKey, ModuleInfo, RequestCode,
};
use crate::get_prop;
use crate::logging::{magisk_logging, setup_logfile, start_log_daemon};
use crate::mount::{clean_mounts, setup_mounts};
use crate::package::ManagerInfo;
use crate::su::SuInfo;
use base::libc::{O_CLOEXEC, O_RDONLY};
use base::{
    cstr, error, info, libc, open_fd, path, AtomicArc, BufReadExt, FsPathBuf, ResultExt, Utf8CStr,
};
use std::fs::File;
use std::io::BufReader;
use std::os::unix::net::UnixStream;
use std::sync::atomic::{AtomicBool, AtomicU32, Ordering};
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
    pub zygiskd_sockets: Mutex<(Option<UnixStream>, Option<UnixStream>)>,
    pub zygisk_enabled: AtomicBool,
    pub zygote_start_count: AtomicU32,
    pub cached_su_info: AtomicArc<SuInfo>,
    sdk_int: i32,
    pub is_emulator: bool,
    is_recovery: bool,
}

impl MagiskD {
    pub fn get() -> &'static MagiskD {
        unsafe { MAGISKD.get().unwrap_unchecked() }
    }

    pub fn is_recovery(&self) -> bool {
        self.is_recovery
    }

    pub fn zygisk_enabled(&self) -> bool {
        self.zygisk_enabled.load(Ordering::Acquire)
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

    fn post_fs_data(&self) -> bool {
        setup_logfile();
        info!("** post-fs-data mode running");

        self.preserve_stub_apk();

        // Check secure dir
        let secure_dir = path!(SECURE_DIR);
        if !secure_dir.exists() {
            if self.sdk_int < 24 {
                secure_dir.mkdir(0o700).log_ok();
            } else {
                error!("* {} is not present, abort", SECURE_DIR);
                return true;
            }
        }

        self.prune_su_access();

        if !setup_magisk_env() {
            error!("* Magisk environment incomplete, abort");
            return true;
        }

        // Check safe mode
        let boot_cnt = self.get_db_setting(DbEntryKey::BootloopCount);
        self.set_db_setting(DbEntryKey::BootloopCount, boot_cnt + 1)
            .log()
            .ok();
        let safe_mode = boot_cnt >= 2
            || get_prop(cstr!("persist.sys.safemode"), true) == "1"
            || get_prop(cstr!("ro.sys.safemode"), false) == "1"
            || check_key_combo();

        if safe_mode {
            info!("* Safe mode triggered");
            // Disable all modules and zygisk so next boot will be clean
            disable_modules();
            self.set_db_setting(DbEntryKey::ZygiskConfig, 0).log_ok();
            return true;
        }

        exec_common_scripts(cstr!("post-fs-data"));
        self.zygisk_enabled.store(
            self.get_db_setting(DbEntryKey::ZygiskConfig) != 0,
            Ordering::Release,
        );
        initialize_denylist();
        setup_mounts();
        let modules = self.handle_modules();
        self.module_list.set(modules).ok();
        clean_mounts();

        false
    }

    fn late_start(&self) {
        setup_logfile();
        info!("** late_start service mode running");

        exec_common_scripts(cstr!("service"));
        if let Some(module_list) = self.module_list.get() {
            exec_module_scripts(cstr!("service"), module_list);
        }
    }

    fn boot_complete(&self) {
        setup_logfile();
        info!("** boot-complete triggered");

        // Reset the bootloop counter once we have boot-complete
        self.set_db_setting(DbEntryKey::BootloopCount, 0).log_ok();

        // At this point it's safe to create the folder
        let secure_dir = path!(SECURE_DIR);
        if !secure_dir.exists() {
            secure_dir.mkdir(0o700).log_ok();
        }

        self.ensure_manager();
        self.zygisk_reset(true)
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
    let path = FsPathBuf::<64>::new()
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
    if let Ok(file) = path!("/system/build.prop").open(O_RDONLY | O_CLOEXEC) {
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
        zygote_start_count: AtomicU32::new(1),
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
