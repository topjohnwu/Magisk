use crate::ffi::backup_init;
use crate::{
    ffi::{magisk_proxy_main, BootConfig, MagiskInit},
    logging::setup_klog,
};
use base::{
    debug, info,
    libc::{basename, getpid, mount, umask},
    path, raw_cstr, FsPath, LibcReturn, LoggedResult, ResultExt,
};
use std::{
    ffi::{c_char, CStr},
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

    pub(crate) fn legacy_system_as_root(&mut self) {
        info!("Legacy SAR Init");
        self.prepare_data();
        if self.mount_system_root() {
            self.redirect_second_stage();
        } else {
            self.patch_ro_root();
        }
    }

    pub(crate) fn rootfs(&mut self) {
        info!("RootFS Init");
        self.prepare_data();
        debug!("Restoring /init\n");
        path!("/.backup/init").rename_to(path!("/init")).log_ok();
        self.patch_rw_root();
    }

    pub(crate) fn recovery(&self) {
        info!("Ramdisk is recovery, abort");
        self.restore_ramdisk_init();
        path!("/.backup").remove_all().ok();
    }

    pub(crate) fn restore_ramdisk_init(&self) {
        path!("/init").remove().ok();

        let orig_init = FsPath::from(backup_init());

        if orig_init.exists() {
            orig_init.rename_to(path!("/init")).log_ok();
        } else {
            // If the backup init is missing, this means that the boot ramdisk
            // was created from scratch, and the real init is in a separate CPIO,
            // which is guaranteed to be placed at /system/bin/init.
            path!("/system/bin/init")
                .symlink_to(path!("/init"))
                .log_ok();
        }
    }

    fn start(&mut self) -> LoggedResult<()> {
        if !path!("/proc/cmdline").exists() {
            path!("/proc").mkdir(0o755)?;
            unsafe {
                mount(
                    raw_cstr!("proc"),
                    raw_cstr!("/proc"),
                    raw_cstr!("proc"),
                    0,
                    null(),
                )
            }
            .as_os_err()?;
            self.mount_list.push("/proc".to_string());
        }
        if !path!("/sys/block").exists() {
            path!("/sys").mkdir(0o755)?;
            unsafe {
                mount(
                    raw_cstr!("sysfs"),
                    raw_cstr!("/sys"),
                    raw_cstr!("sysfs"),
                    0,
                    null(),
                )
            }
            .as_os_err()?;
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
        } else if path!("/sbin/recovery").exists() || path!("/system/bin/recovery").exists() {
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
