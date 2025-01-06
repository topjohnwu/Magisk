#![feature(format_args_nl)]
#![feature(try_blocks)]
#![feature(let_chains)]
#![feature(fn_traits)]
#![allow(clippy::missing_safety_doc)]

use base::Utf8CStr;
use cert::read_certificate;
use daemon::{daemon_entry, find_apk_path, get_magiskd, MagiskD};
use db::get_default_db_settings;
use logging::{
    android_logging, magisk_logging, zygisk_close_logd, zygisk_get_logd, zygisk_logging,
};
use mount::{clean_mounts, find_preinit_device, revert_unmount, setup_mounts};
use resetprop::{persist_delete_prop, persist_get_prop, persist_get_props, persist_set_prop};
use su::get_default_root_settings;

mod cert;
#[path = "../include/consts.rs"]
mod consts;
mod daemon;
mod db;
mod logging;
mod mount;
mod resetprop;
mod su;

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

    extern "C++" {
        include!("include/resetprop.hpp");

        #[cxx_name = "prop_cb"]
        type PropCb;
        unsafe fn get_prop_rs(name: *const c_char, persist: bool) -> String;
        unsafe fn prop_cb_exec(
            cb: Pin<&mut PropCb>,
            name: *const c_char,
            value: *const c_char,
            serial: u32,
        );
    }

    unsafe extern "C++" {
        #[namespace = "rust"]
        #[cxx_name = "Utf8CStr"]
        type Utf8CStrRef<'a> = base::ffi::Utf8CStrRef<'a>;

        include!("include/daemon.hpp");

        #[cxx_name = "get_magisk_tmp_rs"]
        fn get_magisk_tmp() -> Utf8CStrRef<'static>;
        #[cxx_name = "resolve_preinit_dir_rs"]
        fn resolve_preinit_dir(base_dir: Utf8CStrRef) -> String;

        fn switch_mnt_ns(pid: i32) -> i32;
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
    enum RootAccess {
        Disabled,
        AppsOnly,
        AdbOnly,
        AppsAndAdb,
    }

    #[repr(i32)]
    enum MultiuserMode {
        OwnerOnly,
        OwnerManaged,
        User,
    }

    #[repr(i32)]
    enum MntNsMode {
        Global,
        Requester,
        Isolate,
    }

    #[derive(Default)]
    struct DbSettings {
        root_access: RootAccess,
        multiuser_mode: MultiuserMode,
        mnt_ns: MntNsMode,
        boot_count: i32,
        denylist: bool,
        zygisk: bool,
    }

    #[repr(i32)]
    enum SuPolicy {
        Query,
        Deny,
        Allow,
    }

    struct RootSettings {
        policy: SuPolicy,
        log: bool,
        notify: bool,
    }

    unsafe extern "C++" {
        include!("include/sqlite.hpp");

        fn sqlite3_errstr(code: i32) -> *const c_char;

        type sqlite3;
        fn open_and_init_db() -> *mut sqlite3;

        type DbValues;
        type DbStatement;

        fn get_int(self: &DbValues, index: i32) -> i32;
        #[cxx_name = "get_str"]
        fn get_text(self: &DbValues, index: i32) -> &str;

        fn bind_text(self: Pin<&mut DbStatement>, index: i32, val: &str) -> i32;
        fn bind_int64(self: Pin<&mut DbStatement>, index: i32, val: i64) -> i32;
    }

    extern "Rust" {
        fn rust_test_entry();
        fn android_logging();
        fn magisk_logging();
        fn zygisk_logging();
        fn zygisk_close_logd();
        fn zygisk_get_logd() -> i32;
        fn find_apk_path(pkg: Utf8CStrRef, data: &mut [u8]) -> usize;
        fn read_certificate(fd: i32, version: i32) -> Vec<u8>;
        fn setup_mounts();
        fn clean_mounts();
        fn find_preinit_device() -> String;
        fn revert_unmount(pid: i32);
        unsafe fn persist_get_prop(name: Utf8CStrRef, prop_cb: Pin<&mut PropCb>);
        unsafe fn persist_get_props(prop_cb: Pin<&mut PropCb>);
        unsafe fn persist_delete_prop(name: Utf8CStrRef) -> bool;
        unsafe fn persist_set_prop(name: Utf8CStrRef, value: Utf8CStrRef) -> bool;

        #[namespace = "rust"]
        fn daemon_entry();
    }

    // FFI for MagiskD
    extern "Rust" {
        type MagiskD;
        fn setup_logfile(&self);
        fn is_recovery(&self) -> bool;
        fn sdk_int(&self) -> i32;
        fn boot_stage_handler(&self, client: i32, code: i32);

        #[cxx_name = "get_db_settings"]
        fn get_db_settings_for_cxx(&self, cfg: &mut DbSettings) -> bool;
        fn get_db_setting(&self, key: DbEntryKey) -> i32;
        #[cxx_name = "set_db_setting"]
        fn set_db_setting_for_cxx(&self, key: DbEntryKey, value: i32) -> bool;
        fn get_db_string(&self, key: DbEntryKey) -> String;
        #[cxx_name = "rm_db_string"]
        fn rm_db_string_for_cxx(&self, key: DbEntryKey) -> bool;
        #[cxx_name = "db_exec"]
        fn db_exec_for_cxx(&self, client_fd: i32);
        #[cxx_name = "get_root_settings"]
        fn get_root_settings_for_cxx(&self, uid: i32, settings: &mut RootSettings) -> bool;

        #[cxx_name = "DbSettings"]
        fn get_default_db_settings() -> DbSettings;
        #[cxx_name = "RootSettings"]
        fn get_default_root_settings() -> RootSettings;
        #[cxx_name = "MagiskD"]
        fn get_magiskd() -> &'static MagiskD;
    }
    unsafe extern "C++" {
        #[allow(dead_code)]
        fn reboot(self: &MagiskD);
        fn post_fs_data(self: &MagiskD) -> bool;
        fn late_start(self: &MagiskD);
        fn boot_complete(self: &MagiskD);
    }
}

fn rust_test_entry() {}

pub fn get_prop(name: &Utf8CStr, persist: bool) -> String {
    unsafe { ffi::get_prop_rs(name.as_ptr(), persist) }
}
