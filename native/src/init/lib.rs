#![feature(format_args_nl)]
#![feature(once_cell_try)]
#![feature(try_blocks)]
#![allow(clippy::missing_safety_doc)]

use logging::setup_klog;
// Has to be pub so all symbols in that crate is included
pub use magiskpolicy;
use mount::{is_device_mounted, switch_root};
use rootdir::{collect_overlay_contexts, inject_magisk_rc, reset_overlay_contexts};

mod getinfo;
mod init;
mod logging;
mod mount;
mod rootdir;
mod selinux;
mod twostage;

#[cxx::bridge]
pub mod ffi {
    #[derive(Debug)]
    struct KeyValue {
        key: String,
        value: String,
    }

    struct BootConfig {
        skip_initramfs: bool,
        force_normal_boot: bool,
        rootwait: bool,
        emulator: bool,
        slot: [c_char; 3],
        dt_dir: [c_char; 64],
        fstab_suffix: [c_char; 32],
        hardware: [c_char; 32],
        hardware_plat: [c_char; 32],
        partition_map: Vec<KeyValue>,
    }

    struct MagiskInit {
        preinit_dev: String,
        mount_list: Vec<String>,
        argv: *mut *mut c_char,
        config: BootConfig,
    }

    unsafe extern "C++" {
        include!("init.hpp");

        #[namespace = "rust"]
        #[cxx_name = "Utf8CStr"]
        type Utf8CStrRef<'a> = base::ffi::Utf8CStrRef<'a>;

        unsafe fn magisk_proxy_main(argc: i32, argv: *mut *mut c_char) -> i32;
        fn backup_init() -> Utf8CStrRef<'static>;
    }

    #[namespace = "rust"]
    extern "Rust" {
        fn setup_klog();
        fn inject_magisk_rc(fd: i32, tmp_dir: Utf8CStrRef);
        fn switch_root(path: Utf8CStrRef);
        fn is_device_mounted(dev: u64, target: Pin<&mut CxxString>) -> bool;
        fn collect_overlay_contexts(src: Utf8CStrRef);
        fn reset_overlay_contexts();
    }

    // BootConfig
    extern "Rust" {
        fn print(self: &BootConfig);
    }
    unsafe extern "C++" {
        fn init(self: &mut BootConfig);
        type kv_pairs;
        fn set(self: &mut BootConfig, config: &kv_pairs);
    }

    // MagiskInit
    extern "Rust" {
        fn patch_sepolicy(self: &MagiskInit, src: Utf8CStrRef, out: Utf8CStrRef);
        fn parse_config_file(self: &mut MagiskInit);
    }
    unsafe extern "C++" {
        // Used in Rust
        fn mount_system_root(self: &mut MagiskInit) -> bool;
        fn patch_rw_root(self: &mut MagiskInit);
        fn patch_ro_root(self: &mut MagiskInit);

        // Used in C++
        unsafe fn setup_tmp(self: &MagiskInit, path: *const c_char);
        fn collect_devices(self: &MagiskInit);
        fn mount_preinit_dir(self: &MagiskInit);
        unsafe fn find_block(self: &MagiskInit, partname: *const c_char) -> u64;
        fn hijack_sepolicy(self: &mut MagiskInit) -> bool;
    }
}
