use crate::ffi::backup_init;
use crate::mount::is_rootfs;
use crate::twostage::hexpatch_init_for_second_stage;
use crate::{
    ffi::{BootConfig, MagiskInit, magisk_proxy_main},
    logging::setup_klog,
};
use base::{
    LibcReturn, LoggedResult, ResultExt, cstr, info,
    libc::{basename, getpid, mount, umask},
    raw_cstr,
};
use std::{
    ffi::{CStr, c_char},
    ptr::null,
};

impl MagiskInit {
    fn new(argv: *mut *mut c_char) -> Self {
        Self {
            preinit_dev: String::new(),
            mount_list: Vec::new(),
            overlay_con: Vec::new(),
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
        }
    }

    fn first_stage(&self) {
        info!("First Stage Init");
        self.prepare_data();

        if !cstr!("/sdcard").exists() && !cstr!("/first_stage_ramdisk/sdcard").exists() {
            self.hijack_init_with_switch_root();
            self.restore_ramdisk_init();
        } else {
            self.restore_ramdisk_init();
            // Fallback to hexpatch if /sdcard exists
            hexpatch_init_for_second_stage(true);
        }
    }

    fn second_stage(&mut self) {
        info!("Second Stage Init");

        cstr!("/init").unmount().ok();
        cstr!("/system/bin/init").unmount().ok(); // just in case
        cstr!("/data/init").remove().ok();

        unsafe {
            // Make sure init dmesg logs won't get messed up
            *self.argv = raw_cstr!("/system/bin/init") as *mut _;
        }

        // Some weird devices like meizu, uses 2SI but still have legacy rootfs
        if is_rootfs() {
            // We are still on rootfs, so make sure we will execute the init of the 2nd stage
            let init_path = cstr!("/init");
            init_path.remove().ok();
            init_path
                .create_symlink_to(cstr!("/system/bin/init"))
                .log_ok();
            self.patch_rw_root();
        } else {
            self.patch_ro_root();
        }
    }

    fn legacy_system_as_root(&mut self) {
        info!("Legacy SAR Init");
        self.prepare_data();
        let is_two_stage = self.mount_system_root();
        if is_two_stage {
            hexpatch_init_for_second_stage(false);
        } else {
            self.patch_ro_root();
        }
    }

    fn rootfs(&mut self) {
        info!("RootFS Init");
        self.prepare_data();
        self.restore_ramdisk_init();
        self.patch_rw_root();
    }

    fn recovery(&self) {
        info!("Ramdisk is recovery, abort");
        self.restore_ramdisk_init();
        cstr!("/.backup").remove_all().ok();
    }

    fn restore_ramdisk_init(&self) {
        cstr!("/init").remove().ok();

        let orig_init = backup_init();

        if orig_init.exists() {
            orig_init.rename_to(cstr!("/init")).log_ok();
        } else {
            // If the backup init is missing, this means that the boot ramdisk
            // was created from scratch, and the real init is in a separate CPIO,
            // which is guaranteed to be placed at /system/bin/init.
            cstr!("/init")
                .create_symlink_to(cstr!("/system/bin/init"))
                .log_ok();
        }
    }

    fn start(&mut self) -> LoggedResult<()> {
        if !cstr!("/proc/cmdline").exists() {
            cstr!("/proc").mkdir(0o755)?;
            unsafe {
                mount(
                    raw_cstr!("proc"),
                    raw_cstr!("/proc"),
                    raw_cstr!("proc"),
                    0,
                    null(),
                )
            }
            .check_io_err()?;
            self.mount_list.push("/proc".to_string());
        }
        if !cstr!("/sys/block").exists() {
            cstr!("/sys").mkdir(0o755)?;
            unsafe {
                mount(
                    raw_cstr!("sysfs"),
                    raw_cstr!("/sys"),
                    raw_cstr!("sysfs"),
                    0,
                    null(),
                )
            }
            .check_io_err()?;
            self.mount_list.push("/sys".to_string());
        }

        setup_klog();

        self.config.init();

        let argv1 = unsafe { *self.argv.offset(1) };
        if !argv1.is_null() && unsafe { CStr::from_ptr(argv1) == c"selinux_setup" } {
            self.second_stage();
        } else if self.config.skip_initramfs {
            self.legacy_system_as_root();
        } else if self.config.force_normal_boot {
            self.first_stage();
        } else if cstr!("/sbin/recovery").exists() || cstr!("/system/bin/recovery").exists() {
            self.recovery();
        } else if self.check_two_stage() {
            self.first_stage();
        } else {
            self.rootfs();
        }

        // Finally execute the original init
        self.exec_init();

        Ok(())
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn main(
    argc: i32,
    argv: *mut *mut c_char,
    _envp: *const *const c_char,
) -> i32 {
    unsafe {
        umask(0);

        let name = basename(*argv);

        if CStr::from_ptr(name) == c"magisk" {
            return magisk_proxy_main(argc, argv);
        }

        if getpid() == 1 {
            MagiskInit::new(argv).start().log_ok();
        }

        1
    }
}
