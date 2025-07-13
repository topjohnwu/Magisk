#![feature(try_blocks)]
#![feature(let_chains)]
#![feature(fn_traits)]
#![feature(unix_socket_ancillary_data)]
#![feature(unix_socket_peek)]
#![allow(clippy::missing_safety_doc)]

use crate::ffi::SuRequest;
use crate::socket::Encodable;
use base::{Utf8CStr, libc};
use cxx::{ExternType, type_id};
use daemon::{MagiskD, daemon_entry};
use derive::Decodable;
use logging::{android_logging, setup_logfile, zygisk_close_logd, zygisk_get_logd, zygisk_logging};
use mount::{find_preinit_device, revert_unmount};
use resetprop::{persist_delete_prop, persist_get_prop, persist_get_props, persist_set_prop};
use selinux::{lgetfilecon, lsetfilecon, restorecon, setfilecon};
use socket::{recv_fd, recv_fds, send_fd, send_fds};
use std::fs::File;
use std::mem::ManuallyDrop;
use std::ops::DerefMut;
use std::os::fd::FromRawFd;
use su::{get_pty_num, pump_tty, restore_stdin};
use zygisk::zygisk_should_load_module;

#[path = "../include/consts.rs"]
mod consts;
mod daemon;
mod db;
mod logging;
mod module;
mod mount;
mod package;
mod resetprop;
mod selinux;
mod socket;
mod su;
mod zygisk;

#[allow(clippy::needless_lifetimes)]
#[cxx::bridge]
pub mod ffi {
    #[repr(i32)]
    enum RequestCode {
        START_DAEMON,
        CHECK_VERSION,
        CHECK_VERSION_CODE,
        STOP_DAEMON,

        _SYNC_BARRIER_,

        SUPERUSER,
        ZYGOTE_RESTART,
        DENYLIST,
        SQLITE_CMD,
        REMOVE_MODULES,
        ZYGISK,

        _STAGE_BARRIER_,

        POST_FS_DATA,
        LATE_START,
        BOOT_COMPLETE,

        END,
    }

    enum DbEntryKey {
        RootAccess,
        SuMultiuserMode,
        SuMntNs,
        DenylistConfig,
        ZygiskConfig,
        BootloopCount,
        SuManager,
    }

    #[repr(i32)]
    enum MntNsMode {
        Global,
        Requester,
        Isolate,
    }

    #[repr(i32)]
    enum SuPolicy {
        Query,
        Deny,
        Allow,
        Restrict,
    }

    struct ModuleInfo {
        name: String,
        z32: i32,
        z64: i32,
    }

    #[repr(i32)]
    enum ZygiskRequest {
        GetInfo,
        ConnectCompanion,
        GetModDir,
    }

    #[repr(u32)]
    enum ZygiskStateFlags {
        ProcessGrantedRoot = 0x00000001,
        ProcessOnDenyList = 0x00000002,
        DenyListEnforced = 0x40000000,
        ProcessIsMagiskApp = 0x80000000,
    }

    #[derive(Decodable)]
    struct SuRequest {
        target_uid: i32,
        target_pid: i32,
        login: bool,
        keep_env: bool,
        drop_cap: bool,
        shell: String,
        command: String,
        context: String,
        gids: Vec<u32>,
    }

    struct SuAppRequest<'a> {
        uid: i32,
        pid: i32,
        eval_uid: i32,
        mgr_pkg: &'a str,
        mgr_uid: i32,
        request: &'a SuRequest,
    }

    extern "C++" {
        include!("include/resetprop.hpp");

        #[cxx_name = "prop_cb"]
        type PropCb;
        unsafe fn get_prop_rs(name: *const c_char, persist: bool) -> String;
        #[cxx_name = "set_prop"]
        unsafe fn set_prop_rs(name: *const c_char, value: *const c_char, skip_svc: bool) -> i32;
        unsafe fn prop_cb_exec(
            cb: Pin<&mut PropCb>,
            name: *const c_char,
            value: *const c_char,
            serial: u32,
        );
        unsafe fn load_prop_file(filename: *const c_char, skip_svc: bool);
    }

    unsafe extern "C++" {
        #[namespace = "rust"]
        #[cxx_name = "Utf8CStr"]
        type Utf8CStrRef<'a> = base::ffi::Utf8CStrRef<'a>;
        #[cxx_name = "ucred"]
        type UCred = crate::UCred;

        include!("include/core.hpp");

        #[cxx_name = "get_magisk_tmp_rs"]
        fn get_magisk_tmp() -> Utf8CStrRef<'static>;
        #[cxx_name = "resolve_preinit_dir_rs"]
        fn resolve_preinit_dir(base_dir: Utf8CStrRef) -> String;
        fn setup_magisk_env() -> bool;
        fn check_key_combo() -> bool;
        fn disable_modules();
        fn exec_common_scripts(stage: Utf8CStrRef);
        fn exec_module_scripts(state: Utf8CStrRef, modules: &Vec<ModuleInfo>);
        fn install_apk(apk: Utf8CStrRef);
        fn uninstall_pkg(apk: Utf8CStrRef);
        fn update_deny_flags(uid: i32, process: &str, flags: &mut u32);
        fn initialize_denylist();
        fn get_zygisk_lib_name() -> &'static str;
        fn restore_zygisk_prop();
        fn switch_mnt_ns(pid: i32) -> i32;
        fn app_request(req: &SuAppRequest) -> i32;
        fn app_notify(req: &SuAppRequest, policy: SuPolicy);
        fn app_log(req: &SuAppRequest, policy: SuPolicy, notify: bool);
        fn exec_root_shell(client: i32, pid: i32, req: &mut SuRequest, mode: MntNsMode);

        include!("include/sqlite.hpp");

        type sqlite3;
        type DbValues;
        type DbStatement;

        fn sqlite3_errstr(code: i32) -> *const c_char;
        fn open_and_init_db() -> *mut sqlite3;
        fn get_int(self: &DbValues, index: i32) -> i32;
        #[cxx_name = "get_str"]
        fn get_text(self: &DbValues, index: i32) -> &str;
        fn bind_text(self: Pin<&mut DbStatement>, index: i32, val: &str) -> i32;
        fn bind_int64(self: Pin<&mut DbStatement>, index: i32, val: i64) -> i32;
    }

    extern "Rust" {
        fn android_logging();
        fn zygisk_logging();
        fn zygisk_close_logd();
        fn zygisk_get_logd() -> i32;
        fn setup_logfile();
        fn find_preinit_device() -> String;
        fn revert_unmount(pid: i32);
        fn zygisk_should_load_module(flags: u32) -> bool;
        unsafe fn persist_get_prop(name: Utf8CStrRef, prop_cb: Pin<&mut PropCb>);
        unsafe fn persist_get_props(prop_cb: Pin<&mut PropCb>);
        unsafe fn persist_delete_prop(name: Utf8CStrRef) -> bool;
        unsafe fn persist_set_prop(name: Utf8CStrRef, value: Utf8CStrRef) -> bool;
        fn send_fd(socket: i32, fd: i32) -> bool;
        fn send_fds(socket: i32, fds: &[i32]) -> bool;
        fn recv_fd(socket: i32) -> i32;
        fn recv_fds(socket: i32) -> Vec<i32>;
        unsafe fn write_to_fd(self: &SuRequest, fd: i32);
        fn pump_tty(infd: i32, outfd: i32);
        fn get_pty_num(fd: i32) -> i32;
        fn restore_stdin() -> bool;
        fn restorecon();
        fn lgetfilecon(path: Utf8CStrRef, con: &mut [u8]) -> bool;
        fn setfilecon(path: Utf8CStrRef, con: Utf8CStrRef) -> bool;
        fn lsetfilecon(path: Utf8CStrRef, con: Utf8CStrRef) -> bool;

        #[namespace = "rust"]
        fn daemon_entry();
    }

    // Default constructors
    extern "Rust" {
        #[Self = SuRequest]
        #[cxx_name = "New"]
        fn default() -> SuRequest;
    }

    // FFI for MagiskD
    extern "Rust" {
        type MagiskD;
        fn reboot(&self);
        fn sdk_int(&self) -> i32;
        fn zygisk_enabled(&self) -> bool;
        fn boot_stage_handler(&self, client: i32, code: i32);
        fn zygisk_handler(&self, client: i32);
        fn zygisk_reset(&self, restore: bool);
        fn prune_su_access(&self);
        fn su_daemon_handler(&self, client: i32, cred: &UCred);
        #[cxx_name = "get_manager"]
        unsafe fn get_manager_for_cxx(&self, user: i32, ptr: *mut CxxString, install: bool) -> i32;

        fn get_db_setting(&self, key: DbEntryKey) -> i32;
        #[cxx_name = "set_db_setting"]
        fn set_db_setting_for_cxx(&self, key: DbEntryKey, value: i32) -> bool;
        #[cxx_name = "db_exec"]
        fn db_exec_for_cxx(&self, client_fd: i32);

        #[Self = MagiskD]
        #[cxx_name = "Get"]
        fn get() -> &'static MagiskD;
    }
    unsafe extern "C++" {
        fn load_modules(self: &MagiskD) -> Vec<ModuleInfo>;
    }
}

#[repr(transparent)]
pub struct UCred(pub libc::ucred);

unsafe impl ExternType for UCred {
    type Id = type_id!("ucred");
    type Kind = cxx::kind::Trivial;
}

impl SuRequest {
    unsafe fn write_to_fd(&self, fd: i32) {
        unsafe {
            let mut w = ManuallyDrop::new(File::from_raw_fd(fd));
            self.encode(w.deref_mut()).ok();
        }
    }
}

pub fn get_prop(name: &Utf8CStr, persist: bool) -> String {
    unsafe { ffi::get_prop_rs(name.as_ptr(), persist) }
}

pub fn set_prop(name: &Utf8CStr, value: &Utf8CStr, skip_svc: bool) -> bool {
    unsafe { ffi::set_prop_rs(name.as_ptr(), value.as_ptr(), skip_svc) == 0 }
}

pub fn load_prop_file(filename: &Utf8CStr, skip_svc: bool) {
    unsafe { ffi::load_prop_file(filename.as_ptr(), skip_svc) };
}
