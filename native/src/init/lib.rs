#![feature(format_args_nl)]
#![feature(once_cell_try)]
#![feature(try_blocks)]

use logging::setup_klog;
use mount::{is_device_mounted, switch_root};
use rootdir::{collect_overlay_contexts, inject_magisk_rc, reset_overlay_contexts};
// Has to be pub so all symbols in that crate is included
pub use magiskpolicy;
use crate::ffi::{BootConfig, MagiskInit};

mod logging;
mod mount;
mod rootdir;

#[cxx::bridge]
pub mod ffi {
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

    #[namespace = "rust"]
    extern "Rust" {
        fn setup_klog();
        fn inject_magisk_rc(fd: i32, tmp_dir: Utf8CStrRef);
        fn switch_root(path: Utf8CStrRef);
        fn is_device_mounted(dev: u64, target: Pin<&mut CxxString>) -> bool;
        fn collect_overlay_contexts(src: Utf8CStrRef);
        fn reset_overlay_contexts();
        unsafe fn start_magisk_init(argv: *mut *mut c_char);
    }

    unsafe extern "C++" {
        include!("../base/include/base.hpp");

        #[namespace = "rust"]
        #[cxx_name = "Utf8CStr"]
        type Utf8CStrRef<'a> = base::ffi::Utf8CStrRef<'a>;
        fn init(self: &mut BootConfig);
        fn print(self: &BootConfig);
        type kv_pairs;
        fn set(self: &mut BootConfig, config: &kv_pairs);

        unsafe fn setup_tmp(self: &MagiskInit, path: *const c_char);
        fn collect_devices(self: &MagiskInit);
        fn mount_preinit_dir(self: &MagiskInit);
        fn prepare_data(self: &MagiskInit);
        unsafe fn find_block(self: &MagiskInit, partname: *const c_char) -> u64;
        fn mount_system_root(self: &mut MagiskInit) -> bool;

        // Setup and patch root directory
        fn parse_config_file(self: &mut MagiskInit);
        fn patch_rw_root(self: &mut MagiskInit);
        fn patch_ro_root(self: &mut MagiskInit);

        // Two stage init
        fn redirect_second_stage(self: &MagiskInit);
        fn first_stage(self: &MagiskInit);
        fn second_stage(self: &mut MagiskInit);

        // SELinux
        unsafe fn patch_sepolicy(self: &MagiskInit, in_: *const c_char, out: *const c_char);
        fn hijack_sepolicy(self: &mut MagiskInit) -> bool;
        fn exec_init(self: &MagiskInit);
        fn legacy_system_as_root(self: &mut MagiskInit);
        fn rootfs(self: &mut MagiskInit);
        fn start(self: &mut MagiskInit);
        fn init(self: &mut MagiskInit);
    }
}

pub(crate) fn start_magisk_init(argv: *mut *mut std::ffi::c_char) {
    let mut init = MagiskInit {
        preinit_dev: String::new(),
        mount_list: Vec::new(),
        argv,
        config: BootConfig {
            skip_initramfs: false,
            force_normal_boot: false,
            rootwait: false,
            emulator: false,
            slot: [0; 3],
            dt_dir: [0; 64],
            fstab_suffix: [0; 32],
            hardware: [0; 32],
            hardware_plat: [0; 32],
            partition_map: Vec::new(),
        },
    };
    init.init();
    init.start();
}
